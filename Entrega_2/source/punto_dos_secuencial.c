#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MAX 100

//gcc -o punto_dos_secuencial punto_dos_secuencial.c -lm
//./punto_dos_secuencial 5
 
struct Elementos {
    int  numero;
    int ocurrencias;
};

/* Calcula los tiempos transcurridos */
double dwalltime();

/* Función para ordenar un arreglo de mayor a menor */
void mergeSort(struct Elementos *, int);

/* Función para fusionar dos arreglos */
void merge(struct Elementos *, int l, int m, int r);
 
/* Función utilizada para encontrar el minimo de dos integers */
int min(int x, int y) { return (x<y)? x :y; }

int main(int argc, char** argv){

    int *V, *R, i, j, included=0;
    double N;
    double timetick, totalTime;
    /* Registro de ocurrencias */
    struct Elementos *occu_reg; 
    int logical_dim=0;    

    /* Lee parámetros de la línea de comando */
	if ((argc != 2) || ((N = pow(2,atoi(argv[1]))) <= 0) ) {
	    printf("\nUsar: %s size \n  size: Tamaño del vector N=2^size\n", argv[0]);
		exit(1);
	}

    /* Generamos la semilla para generar valores randoms */
    time_t t;
    srand((unsigned)time(&t));

    /* Asignación de memoria para los vectores */
    V = (int*) malloc(sizeof(int)*N);
    R = (int*) malloc(sizeof(int)*MAX);
    occu_reg = (struct Elementos*) malloc(sizeof(struct Elementos)*N);
    
    for (i=0; i<N ; i++)
		V[i] = rand() % 3000;
   
    /* Se comienza  contar el tiempo total */
    timetick = dwalltime();

    /* Se calculan las ocurrencias de cada elemento */
    for (i = 0; i < N; i++)
    {   
        included = 0;
        for (j = 0; j < logical_dim; j++)
        {
            if(occu_reg[j].numero==V[i]){
                occu_reg[j].ocurrencias++;
                included=1;
            }
                
        }
        
        if (included==0){
            occu_reg[logical_dim].numero = V[i];
            occu_reg[logical_dim].ocurrencias = 1;
            logical_dim++;
        }
    }
    
    /* Se ordenan las ocurrencias de mayor a menor */
    mergeSort(occu_reg, logical_dim);

    /* Se genera el vector resultante de 100 elementos */
    for (i = 0; i < MAX; i++)
    {
        R[i] = occu_reg[i].numero;
    }
    
    /* Se finaliza de temporizar el tiempo total */
    totalTime = dwalltime() - timetick;

    /* Imprimir tiempos y validación */
    printf("****************\nOperación Con un vector de %.0fd elementos\nTiempo total: %lf\n", N, totalTime);
    
    
    printf("Resultado: [");
    for (i = 0; i < MAX; i++)
    {
        if(i==MAX-1){
            printf("%d",R[i]);
        }else{
            printf("%d,",R[i]);
        }
    }
    printf("] \n");

    printf("**Los %d numeros más frecuentes para validar**\n",MAX+50);
    for (i = 0; i < MAX+50; i++)
    {
        printf("Numero: %d - occurencia: %d \n",occu_reg[i].numero,occu_reg[i].ocurrencias);
    }
    

    /* Liberación de memoria */
	free(V);
    free(R);
	free(occu_reg);
    
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

void mergeSort(struct Elementos *arr, int n)
{
   int curr_size;  
   int left_start;
 
    // Fusiona submatrices de abajo hacia arriba. Primero combine los sub arreglos de tamaño 1 
    // para crear sub arreglos ordenados de tamaño 2, luego combine los sub arreglos de tamaño 2 
    // para crear sub arreglos ordenados de tamaño 4, y así sucesivamente. 
   for (curr_size=1; curr_size<=n-1; curr_size = 2*curr_size)
   {
       // Se elije el punto de partida de diferentes submatrices del tamaño actual 
       for (left_start=0; left_start<n-1; left_start += 2*curr_size)
       {
           // Encuentra el punto final del subarreglo izquierdo. mid + 1 está comenzando
           //  punto de la derecha 
           int mid = min(left_start + curr_size - 1, n-1);
 
           int right_end = min(left_start + 2*curr_size - 1, n-1);
 
           // Se fusionan subarreglos arr[left_start...mid] & arr[mid+1...right_end]
           merge(arr, left_start, mid, right_end);
       }
   }
}
 
void merge(struct Elementos *arr, int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* Se crean arreglos temporales */
    struct Elementos L[n1], R[n2];
 
    /* Se copan los datos a los temporales de L[] y R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
 
    /* Se fusionan las matrices temporales de nuevo en arr[l..r]*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i].ocurrencias >= R[j].ocurrencias)
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    /* Se copia  los elementos restantes de L [], si hay alguno */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    /* Se copia los elementos restantes de R [], si hay alguno */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}