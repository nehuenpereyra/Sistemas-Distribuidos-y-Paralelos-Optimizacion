#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//gcc -o punto_uno_secuencial punto_uno_secuencial.c
//./punto_uno_secuencial TamañoMatriz
//./punto_uno_secuencial 32

double dwalltime();

/* Calcula el valor promedio de la matriz */
double promedio_matriz(int, double *); 

/* Valida la matriz resultante e imprime el tiempo */
void validation(int, double *, double);

int main(int argc, char** argv){

    int N, i, j, k;
    double * A, * B, *C, *D, *R, *ac, *bd, timetick, totalTime, promA, promB, total;
    register double aux = 0;


    /* Lee parámetros de la línea de comando */
	if ((argc != 2) || ((N = atoi(argv[1])) <= 0) ) {
	    printf("\nUsar: %s size \n  size: Dimension de la matriz\n", argv[0]);
		exit(1);
	}

    /* Generamos la semilla para generar valores randoms */
    time_t t;
    srand((unsigned)time(&t));

    /* Asignación de memoria para las matrices */
    A = (double*) malloc(sizeof(double)*N*N);
    B = (double*) malloc(sizeof(double)*N*N);
	C = (double*) malloc(sizeof(double)*N*N);
	D = (double*) malloc(sizeof(double)*N*N);
    R = (double*) malloc(sizeof(double)*N*N);
    ac = (double*) malloc(sizeof(double)*N*N);
    bd = (double*) malloc(sizeof(double)*N*N);
    
    for (i=0; i<N ; i++)
		for (j=0; j<N ; j++){
			A[i*N+j] = 1;
			C[i*N+j] = 1;
			B[i*N+j] = 1;
			D[i*N+j] = 1;
	}

    for (i=0; i<N ; i++)
		for (j=0; j<N ; j++){
            ac[i*N+j] = 0;
            bd[i*N+j] = 0;
	}

    /* Se comienza  contar el tiempo total */
    timetick = dwalltime();


    /* Se realiza la operacion de multiplicacion de matrices de forma optimizada */
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k < N; k++)
            {
                aux += A[i * N + k] * C[k + j * N];
            }
            ac[i * N + j] = aux;
        }
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            aux = 0.0;
            for (k = 0; k < N; k++)
            {
                aux += B[i * N + k] * D[k + j * N];
            }
            bd[i * N + j] = aux;
        }
    }
    

    /* Se calcula el promedio de las matrices */
    total = 0;
    for (i=0; i<N; i++){
		for (j=0; j<N ;j++ ) {
			total += A[i * N + j]; 
		}
	}
    promA = total / (N*N);

    total = 0;
    for (i=0; i<N; i++){
		for (j=0; j<N ;j++ ) {
			total += B[i * N + j]; 
		}
	}
    promB = total / (N*N);
    
    /* Se calcula la matriz resultado */
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            R[i * N + j] = (promB * ac[i * N + j]) + (promA *bd[i * N + j]);
        }
    }

    /* Se finaliza de temporizar el tiempo total */
    totalTime = dwalltime() - timetick;

    /* Imprimir tiempos */
    validation(N, R, totalTime);

    /* Liberación de memoria */
	free(A);
	free(B);
	free(C);
	free(D);
	free(R);
    
    return(0);
}

double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec/1000000.0;
	return sec;
}

double promedio_matriz(int N, double *M){
    int i, j, k =1;
    double total=0;

    for (i=0; i<N; i++){
		for (j=0; j<N ;j++ ) {
			total += M[i * N + j]; 
		}
	}

    return total/N;
} 

void validation(int N, double *c, double totalTime)
{
    int i, j, k, check=1;
    for(i=0;i<N;i++)
	    for(j=0;j<N;j++)
		    check=check&&(c[i*N+j]==N+N);

	if(check){
		printf("Multiplicación de matrices resultado correcto\n");
	}else{
		printf("Multiplicación de matrices resultado erroneo\n");
	}

	printf("****************\nMultiplicación de matrices (N=%d)\nTiempo total: %lf\n", N, totalTime);
}

