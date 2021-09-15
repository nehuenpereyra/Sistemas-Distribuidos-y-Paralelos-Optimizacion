#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#define XFILA 0
#define XCOLUMNA 1
#define ZERO 1
#define NOTZERO 0

/* Calcula los tiempos trasncurridos */
double dwalltime();

/* Inicializa una matriz por filas o columnas */
void init_matriz(double *, int, int, int);

/* Inicializa una matriz triangular inferior por filas */
void init_matriz_LF(double *, double *, int);

/* Inicializa una matriz triangular superior por columnas */
void init_matriz_UC(double *, double *, int);

/* Dado un valor n retorna el valor resultante de aplicar fibonacci */
int fib(int n);

/* Comprueba si son iguales las dos matrices pasadas por parametro */
void validate(double *, double *, int);

int main(int argc, char *argv[])
{
    double *A, *B, *C, *U, *L, *UO, *LO, *au, *lb, *ca, *total, *ff, *R1, *R2;
    int *F;
    int i, j, k, N;
    register double aux;
    double timetick;

    /* Se controla los argumentos pasados al programa */
    if (argc < 2)
    {
        printf("\n Falta un argumento: N dimension de la matriz \n");
        return 0;
    }

    N = atoi(argv[1]);

    /* Matrices principales */
    A = (double *)malloc(sizeof(double) * N * N);
    B = (double *)malloc(sizeof(double) * N * N);
    C = (double *)malloc(sizeof(double) * N * N);
    U = (double *)malloc(sizeof(double) * N * N);
    UO = (double *)malloc(sizeof(double) * N * (N+1)/2); // Matriz Optimizada
    L = (double *)malloc(sizeof(double) * N * N);
    LO = (double *)malloc(sizeof(double) * N * (N+1)/2); // Matriz Optimizada
    F = (int *)malloc(sizeof(int) * N * N);

    /* Matrices intermedias */
    au = (double *)malloc(sizeof(double) * N * N);
    lb = (double *)malloc(sizeof(double) * N * N);
    ca = (double *)malloc(sizeof(double) * N * N);
    total = (double *)malloc(sizeof(double) * N * N);
    ff = (double *)malloc(sizeof(double) * N * N);

    /* Matrices de resultados */
    R1 = (double *)malloc(sizeof(double) * N * N);
    R2 = (double *)malloc(sizeof(double) * N * N);

    /* Generamos la semilla para generar valores randoms*/
    time_t t;
    srand((unsigned)time(&t));

    /* Inicializacion de las matrices */
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            F[i * N + j] = rand() % 40 + 1; //  rand () % (N-M+1) + M; genera valores entre M=1 y N=40

    init_matriz(A, N, XFILA, NOTZERO);
    init_matriz_LF(L, LO, N);
    init_matriz(C, N, XFILA, NOTZERO);
    init_matriz_UC(U, UO, N);
    init_matriz(B, N, XCOLUMNA, NOTZERO);

    init_matriz(au, N, XFILA, ZERO);
    init_matriz(lb, N, XFILA, ZERO);
    init_matriz(ca, N, XFILA, ZERO);
    init_matriz(total, N, XFILA, ZERO);
    init_matriz(ff, N, XFILA, ZERO);
    init_matriz(R1, N, XFILA, ZERO);
    init_matriz(R2, N, XFILA, ZERO);

    /* Se comienza a calcular el tiempo */
    timetick = dwalltime();

    /* Se realizan las operaciones */

    /* Se realiza la operacion de multiplicacion de matrices de forma optimizada */

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k <= j; k++)
            {
                aux += A[i * N + k] * U[k + j * N];
            }
            au[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k <= i; k++)
            {
                aux += L[i * N + k] * B[k + j * N];
            }
            lb[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < i; j++)
        {
            aux = A[i * N + j];
            A[i * N + j] = A[j * N + i];
            A[j * N + i] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k < N; k++)
            {
                aux += C[i * N + k] * A[k + j * N];
            }
            au[i * N + j] = aux;
        }
    }

    /* Se suman los resultados intermedios y se multiplica por 0.2 ya que es más optimo que dividir por 5 */
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            total[i * N + j] = (au[i * N + j] + lb[i * N + j] + ca[i * N + j]) * 0.2;
        }
    }

    /* Se aplica fibonacci iterativo (es más optimo que el recursivo) a cada elemento de la matriz F*/
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++){
            ff[i * N + j] = fib(F[i * N + j]);
        }
    }

    /* Se suman las matrices resultantes para obtener el resultado sin ignorar los ceros de las matrices tringuales */
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++){
            R1[i * N + j] = total[i * N + j] + ff[i * N + j];
        }
    }

    printf("Tiempo total con las matrices con los ceros: %f \n", dwalltime() - timetick);

    /* Volviendo a inicializar las matrices necesarias */
    init_matriz(au, N, XFILA, ZERO);
    init_matriz(lb, N, XFILA, ZERO);
    init_matriz(ca, N, XFILA, ZERO);
    init_matriz(total, N, XFILA, ZERO);
    init_matriz(ff, N, XFILA, ZERO);

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < i; j++)
        {
            aux = A[i * N + j];
            A[i * N + j] = A[j * N + i];
            A[j * N + i] = aux;
        }
    }

    /* Se comienza a tomar el tiempo  nuevamente */
    timetick = dwalltime();

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k <= j; k++)
            {
                aux += A[i * N + k] * UO[k+(j*(j+1))/2];
            }
            au[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k <= i; k++)
            {
                aux += LO[k+(i*(i+1))/2] * B[k + j * N];
            }
            lb[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < i; j++)
        {
            aux = A[i * N + j];
            A[i * N + j] = A[j * N + i];
            A[j * N + i] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k < N; k++)
            {
                aux += C[i * N + k] * A[k + j * N];
            }
            au[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            total[i * N + j] = (au[i * N + j] + lb[i * N + j] + ca[i * N + j]) * 0.2;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++){
            ff[i * N + j] = fib(F[i * N + j]);
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++){
            R2[i * N + j] = total[i * N + j] + ff[i * N + j];
        }
    }

    printf("Tiempo total con las matrices sin los ceros: %f \n", dwalltime() - timetick);

    validate(R1, R2, N);

    free(A);
    free(B);
    free(C);
    free(U);
    free(L);
    free(au);
    free(lb);
    free(ca);
    free(total);
    free(ff);
    free(R1);
    free(R2);
    return (0);
}

double dwalltime(){
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

void init_matriz(double *matriz, int N, int xfila, int zero){
    int i, j, x;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            x = zero ? 0 : rand();
            if (xfila == 0)
            {
                matriz[i * N + j] = x;
            }
            else
                matriz[i + j * N] = x;
        }
    }
}

void init_matriz_LF(double *matriz, double *vector, int N){
    int i, j;
    double x;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (i < j)
            {
                matriz[i * N + j] = 0;
            }
            else
            {
                x = rand();
                matriz[i * N + j] = x;
                vector[j + ((i * (i + 1))/2)] = x;
            }
        }
    }
}

void init_matriz_UC(double *matriz,double *vector, int N){
    int i, j;
    double x;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (i > j)
            {
                matriz[i + j * N] = 0;
            }
            else
            {
                x = rand();
                matriz[i + j * N] = x;
                vector[i + (j * ( j + 1 ) / 2)] = x;
            }
        }
    }
}

int fib(int n){
    int j = 0, i = 1, k, t;
    for (k = 1; k <= n; k++)
    {
        t = i + j;
        i = j;
        j = t;
    }
    return j;
}

void validate(double *m1, double *m2, int N){

    int i, j, k;
    int check = 1;
    
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
	        check=check&&(m1[i*N+j]==m2[i*N+j]);
        }
    }   

    if(check){
        printf("La operación (AU+LB+CA)/5+Fib(F) se resolvio correctamente\n");
    }else{
        printf("La operación (AU+LB+CA)/5+Fib(F) se resolvio erroneamente\n");
    }
}
