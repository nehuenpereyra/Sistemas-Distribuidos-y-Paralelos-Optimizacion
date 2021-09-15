#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <mpi.h>
#include <pthread.h>
#include <math.h>


// mpicc -lpthread -o punto_dos_paralelo punto_dos_paralelo.c -lm
// mpirun -n CantidadProcesos ./entrega_p1 2^potencian(Cantidad elementos) CantidadHilos
// mpirun -n 2 ./punto_dos_paralelo 5 2 (2 procesos - 32 elementos - 2 hilos)

#define COORDINATOR 0
#define MAX 100


struct Elementos {
    int  numero;
    int ocurrencias;
};

struct Lista_Elementos {
    struct Elementos * lista;
    int dim;
};

/* Calcula los tiempos transcurridos */
double dwalltime();

/* Totaliza la porción del vector que le corresponde al hilo */
int totalizar (int);

/* Ordenamiento de las porciones del vector */
void mergeSort(struct Elementos *, unsigned int );
void merge(struct Elementos *, unsigned int , unsigned int , unsigned int );
int min(int x, int y) { return (x<y)? x :y; }

/* Variables compartidas */
int B; // Tamaño del bloque a dividir, Numero a buscar
double N, stripSize;
int *logical_dim, T;
struct Lista_Elementos *totalize_arr;
struct Elementos *totalize_partial_arr;
int *numbers_partial_arr;
pthread_barrier_t *barrier; 
int id_proc;

/* Función que ejecuta cada hilo para ordenar su porción del arreglo */
void* hilo_ordenamiento(void *);

/* Función que ejecuta cada hilo para totalizar su porción del arreglo */
void* hilo_totalizar(void *);

/* Programa principal */
int main(int argc, char** argv){

    /* Declaranción de variables auxiliares */
    int miID, nrProcesos, i, j;
    double timetick, totalTime;
    int *numbers_arr, *R;
	struct Elementos *ord_totalize_arr;
    int bytes_a_recibir, bytes_a_enviar;
    int include=0;
    int mid_P = nrProcesos * 0.5, end = 0;
    struct Elementos *total_aux;
    int included, logical_dim_aux;
    struct Elementos * sort_aux;
    struct Elementos * aux;
    
 
    /* Inicializa el ambiente. No debe haber sentencias antes*/
    MPI_Init(&argc, &argv);
    
    /* Obtiene el identificador de cada proceso (rank) */
    MPI_Comm_rank(MPI_COMM_WORLD, &miID); id_proc = miID;

    /* Obtiene el numero de procesos creados */
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos);

    /* Lee parámetros de la línea de comando */
	if ((argc != 3) || ((N = pow(2,atoi(argv[1]))) <= 0) ) {
	    printf("\nUsar: %s size num_threads\n  size: Dimension de la matriz num_threads: Numero de hilos\n", argv[0]);
		exit(1);
	}

    if ((int)N % nrProcesos != 0) {
		printf("El tamaño de la matriz debe ser multiplo del numero de procesos.\n");
		exit(1);
	}

	/* Generamos la semilla para generar valores randoms*/
    time_t t;
    srand((unsigned)time(&t));

    /* Se calcula la porcion de trabajo para cada worker */
    stripSize = N / nrProcesos;

    /* Se establecen la cantidad de hilos por proceso */
	T = atoi(argv[2]);
  	B = stripSize / T;

    /* Se inicializa el vector con los hilos y sus Ids */
    pthread_t misThreads[T];
    int threads_ids[T];

    /* Reservar memoria */
	if (miID == COORDINATOR) {
        numbers_arr = (int*) malloc(sizeof(int)*N);
    }
	else  {
		numbers_arr = (int*) malloc(sizeof(int)*stripSize);
	}

    R = (int*) malloc(sizeof(int)*MAX);
    numbers_partial_arr = (int*) malloc(sizeof(int)*N);
    logical_dim = (int*) malloc(sizeof(int) * T);
    totalize_arr = (struct Lista_Elementos *)malloc(sizeof(struct Lista_Elementos)*T);
    
    ord_totalize_arr = (struct Elementos*) malloc(sizeof(struct Elementos)*N);
    total_aux = (struct Elementos*) malloc(sizeof(struct Elementos)*N);
    sort_aux = malloc(sizeof(struct Elementos) * N);
    

    /* Se inicializa el arreglo de barreras */
    barrier = (pthread_barrier_t *)malloc(sizeof(pthread_barrier_t) * T*0.5);
    for (int i = 0; i < T*0.5; i++)
    {
        pthread_barrier_init(&barrier[i], NULL, 2);
    }

    /* Inicialización del vector de N elementos */
	if (miID == COORDINATOR) {
		for (i=0; i<N ; i++)
		    numbers_arr[i] = rand() % 6000;	
	}

    /* Se comienza  contar el tiempo total */
    timetick = dwalltime();

    /* Distribuye V para los distintos procesos */
    MPI_Scatter(numbers_arr, stripSize, MPI_INT, numbers_partial_arr, stripSize, MPI_INT, 0, MPI_COMM_WORLD);

    /* Se crean los hilos */
    for(int id=1; id<T; id++){
        threads_ids[id]=id;
        /* Crea un hilo y lo ejecuta */
        pthread_create(&misThreads[id], NULL, &hilo_totalizar,(void*)&threads_ids[id]);
    }

    threads_ids[0] = 0; // Main será el hilo 0
    (*hilo_totalizar)((void *)&threads_ids[0]);

    /* Espera a que finalize cada hilo */
    for(int id=1;id<T;id++){
        pthread_join(misThreads[id],NULL);
    }

    /* Reducción de los datos de los procesos para obtener los datos totalizados */
    mid_P = nrProcesos * 0.5, end = 0;
    while (mid_P>=1 && end==0)
    {  
        if(miID<mid_P){ 
            MPI_Recv(&bytes_a_recibir, 1, MPI_INT, miID+mid_P, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            logical_dim_aux = bytes_a_recibir/sizeof(struct Elementos);
            MPI_Recv(total_aux, logical_dim_aux*sizeof(struct Elementos), MPI_BYTE, miID+mid_P, 98, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (i = 0; i < logical_dim_aux; i++){
                included=0;
                for (j = 0; (j < totalize_arr[0].dim)&&included!=1; j++)
                {    
                    
                    if(totalize_arr[0].lista[j].numero == total_aux[i].numero){
                        totalize_arr[0].lista[j].ocurrencias += total_aux[i].ocurrencias;
                        included=1;
                    }      
                }
                
                if(included==0){
                    totalize_arr[0].lista[totalize_arr[0].dim].numero = total_aux[i].numero;
                    totalize_arr[0].lista[totalize_arr[0].dim].ocurrencias = total_aux[i].ocurrencias;
                    totalize_arr[0].dim++;
                } 
            }
 
        }else{
            bytes_a_enviar = totalize_arr[0].dim* sizeof(struct Elementos);
            MPI_Send(&bytes_a_enviar, 1, MPI_INT, miID-mid_P, 99, MPI_COMM_WORLD);
            MPI_Send(totalize_arr[0].lista, bytes_a_enviar, MPI_BYTE, miID-mid_P, 98, MPI_COMM_WORLD);
            end++;
        }
        mid_P *= 0.5;
    }



    /* Se calcula cuantos elementos se van a distribuir en cada proceso */
    /* La lista de elementos ya totalizados es variable su dimensión por ello lo distribuimos con un Scatterv */
    int cantidad = 0;
    int *cantidades = malloc(sizeof(int)*nrProcesos);
    int *displs = malloc(sizeof(int)*nrProcesos);
    
    int sum = 0;     
    int rem = totalize_arr[0].dim;
    if(miID==0){
        double value = ceil((double)totalize_arr[0].dim/nrProcesos);
        for (i = 0; i < nrProcesos; i++)
        {
           if(i<nrProcesos-1){
                cantidades[i]=(int)value*sizeof(struct Elementos);
            }else{
                
                cantidades[i]= (totalize_arr[0].dim-(int)value*i)*sizeof(struct Elementos);
            }
            if (rem > 0) {
                rem--;
            }

            displs[i] = sum;
            sum += cantidades[i];
            }
    }
    
    MPI_Scatter(cantidades, 1, MPI_INT,&cantidad, 1, MPI_INT, 0, MPI_COMM_WORLD);
    totalize_partial_arr = (struct Elementos *) malloc(sizeof(struct Elementos)*cantidad);
    

    /* Distribuye el vector totalizado para los distintos procesos */
    MPI_Scatterv(totalize_arr[0].lista,cantidades,displs, MPI_BYTE, totalize_partial_arr, cantidad*sizeof(struct Elementos), MPI_BYTE, 0, MPI_COMM_WORLD);
   
    
    stripSize = cantidad / sizeof(struct Elementos);

    /* Se ordena la porción ya totalizada previamente */
    mergeSort(totalize_partial_arr,stripSize);
    
    /* Reducción de los datos de los procesos para obtener los datos totalizados */ 
    mid_P = nrProcesos * 0.5, end = 0;
    int dim = stripSize;
    for (i = 0; (i < MAX); i++)
    {
        ord_totalize_arr[i]=totalize_partial_arr[i];
    }
    while (mid_P>=1 && end==0)
    {  
        if(miID<mid_P){ 
            MPI_Recv(&bytes_a_recibir, 1, MPI_INT, miID+mid_P, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            logical_dim_aux = bytes_a_recibir/sizeof(struct Elementos);
            MPI_Recv(total_aux, MAX*sizeof(struct Elementos), MPI_BYTE, miID+mid_P, 98, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            /* Como los elementos ya fueron TOTALIZADOS previamente solo aparecen una vez en cada porción */
            /* Los reducimos de manera de irnos quedando con los mayores ya que la porcion YA ESTAN ORDENADADOS PREVIAMENTE */
            aux = (struct Elementos *) malloc(sizeof(struct Elementos)*MAX);
            int index_local=0, index_par=0;
            for (i = 0; i < MAX; i++)
            {
                if (ord_totalize_arr[index_local].ocurrencias >= total_aux[index_par].ocurrencias)
                {
                    aux[i] = ord_totalize_arr[index_local];
                    if(mid_P==1)
                    {
                        R[i] = ord_totalize_arr[index_local].numero;
                    }
                    index_local++;
                }else{
                    aux[i] = total_aux[index_par];
                    if(mid_P==1)
                    {
                        R[i] = total_aux[index_par].numero;
                    }
                    index_par++;
                    
                }
            }
                ord_totalize_arr=aux;      

        }else{
            bytes_a_enviar = MAX* sizeof(struct Elementos);
            MPI_Send(&bytes_a_enviar, 1, MPI_INT, miID-mid_P, 99, MPI_COMM_WORLD);
            MPI_Send(ord_totalize_arr, bytes_a_enviar, MPI_BYTE, miID-mid_P, 98, MPI_COMM_WORLD);
            end++;
        }
        mid_P *= 0.5;
    }
    
    /* Se finaliza de temporizar el tiempo total */
    totalTime = dwalltime() - timetick;

    /* Finaliza el ambiente MPI */
    MPI_Finalize();

    if (miID == COORDINATOR){
        
        printf("****************\nOperación Con un vector de %.0fd elementos\nTiempo total: %lf\n", N, totalTime);
        printf("RESULTADO: [");
        for (int w = 0; w < MAX; w++)
        {
            printf("%d, ",R[w]);
        } 
        printf("]\n");
        printf("Validación: [");
        for (int w = 0; w < MAX; w++)
        {
             printf("(%d,%d), ", ord_totalize_arr[w].numero ,ord_totalize_arr[w].ocurrencias);
        } 
        printf("]\n");
        
    }

   /* Destrucción de las barreras */
    for (int i = 0; i < T*0.5; i++)
    {
        pthread_barrier_destroy(&barrier[i]);
    }

    /* Liberación de memoria */
    free(numbers_arr);
	free(numbers_partial_arr);
    free(ord_totalize_arr);
    free(R);
    free(logical_dim);
    free(totalize_arr);
    free(totalize_partial_arr);
    free(total_aux);
    free(sort_aux);
    

    return(0);
}


void* hilo_totalizar(void *arg){
    /* Variables auxiliares */
    int tid = *(int*)arg;
    int end = 0, mid_T = T * 0.5;
    int i, j, included; 

   totalizar(tid);
    
    /* Reducción */
    while (mid_T>=1 && end==0)
    { 
        pthread_barrier_wait(&barrier[tid&(mid_T-1)]);   
        if(tid<mid_T){  
            for (i = 0; i < totalize_arr[tid+ mid_T].dim; i++){
                included=0;
                for (j = 0; (j < totalize_arr[tid].dim)&&included!=1; j++)
                {    
                    /* Si el elemento ya esta incluido */
                    if(totalize_arr[tid].lista[j].numero == totalize_arr[tid + mid_T].lista[i].numero){
                        totalize_arr[tid].lista[j].ocurrencias += totalize_arr[tid + mid_T].lista[i].ocurrencias;
                        included=1;
                    }      
                }
                /* Si el elemento no esta incluido */
                if(included==0){
                    totalize_arr[tid].lista[totalize_arr[tid].dim].numero = totalize_arr[tid + mid_T].lista[i].numero;
                    totalize_arr[tid].lista[totalize_arr[tid].dim].ocurrencias = totalize_arr[tid + mid_T].lista[i].ocurrencias;
                    totalize_arr[tid].dim++;
                } 
            }
            
            
        }else{
            end++;
        }
        mid_T *= 0.5;
    }

    /*  función que termina la ejecución del hilo */
    if(tid!=0)
        pthread_exit(NULL);
}



double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec/1000000.0;
	return sec;
}

int totalizar (int tid){

    struct Elementos *totalize_arr_aux;
    int i, j, included;
    int logical_dim_aux=0;

    totalize_arr_aux = (struct Elementos*) malloc(sizeof(struct Elementos)*N);
    struct Lista_Elementos aux;

    for (i = 0; i < B; i++)
    {   
        included = 0;
        for (j = 0; j < logical_dim_aux&&included!=1; j++)
        {
            if(totalize_arr_aux[j].numero == numbers_partial_arr[(B*tid)+i]){
                totalize_arr_aux[j].ocurrencias++;
                included=1;
            }
                
        }
        if (included==0){
            totalize_arr_aux[logical_dim_aux].numero = numbers_partial_arr[(B*tid)+i];
            totalize_arr_aux[logical_dim_aux].ocurrencias = 1;
            logical_dim_aux++;
        }
    }

    aux.lista = totalize_arr_aux;
    aux.dim = logical_dim_aux;
    totalize_arr[tid] = aux;
}

void mergeSort(struct Elementos *arr, unsigned int n)
{
   unsigned int curr_size;  
   unsigned int left_start;
 
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
           unsigned int mid = min(left_start + curr_size - 1, n-1);
 
           unsigned int right_end = min(left_start + 2*curr_size - 1, n-1);
 
           // Se fusionan subarreglos arr[left_start...mid] & arr[mid+1...right_end]
           merge(arr, left_start, mid, right_end);
       }
   }
}
 
void merge(struct Elementos *arr, unsigned int l, unsigned int m, unsigned int r)
{
    unsigned int i, j, k;
    unsigned int n1 = m - l + 1;
    unsigned int n2 =  r - m;
 
    /* Se crean arreglos temporales */
    struct Elementos *L, *R;
    L = (struct Elementos*) malloc(sizeof(struct Elementos)*n1);
    R = (struct Elementos*) malloc(sizeof(struct Elementos)*n2);
    
 
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