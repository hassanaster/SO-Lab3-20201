/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * @brief      This file implements an iterative saxpy operation, upgrading V0 and V1. We assigned the Workload for each thread.
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Danny Munera
 * @date       2020
 * @author     Miriam Arango, Yesison Quinto, Luisa V2
 * @date       2020
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <time.h>


// Variables to obtain command line parameters
int p = 10000000; // vector size
int n_threads = 2; // {number of threads to create} 
int max_iters = 1000; 
// Variables to perform SAXPY operation
double* X;
double a;
double* Y;
double* Y_avgs;
int i, j, it;
//mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//data struct to assign the job charge for each thread
struct jobThread{
    int startPosition;
    int endPosition;
};
//inicialization from some variables to assign the workload
int cont;
int start;
int end = -1;

void* saxpy(void* arg);

int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	unsigned int seed = 1; // this is to generate the ramdom numbers for each vector
  	
	// Variables to get execution time
	struct timeval t_start, t_end;
    struct jobThread jobT;
	double exec_time;

	// Getting input values
	int opt;
	//option p,s,n,i ??
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  //getopt??
		switch(opt){  
			case 'p':  
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647); 
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'n':  
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;  
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt); 
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}  
	}  
	srand(seed); 

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	

	// initializing data
	X = (double*) malloc(sizeof(double) * p);
	Y = (double*) malloc(sizeof(double) * p);
	Y_avgs = (double*) malloc(sizeof(double) * max_iters);

	for(int i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for(int i = 0; i < max_iters; i++){
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

//DEBUG
#ifdef DEBUG
	printf("vector X= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ",X[i]);
	}
	printf("%f ]\n",X[p-1]);

	printf("vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("a= %f \n", a);	
#endif

	 
	//Getting the time to start the real process to get the results in SAXPY function
	gettimeofday(&t_start, NULL);
	
	pthread_t h[n_threads];
	//We get how much positions we would have left if the workload p/n isn't exact to add the rest to some workload threads
	if(p%n_threads!=0)
         cont = p - ((p/n_threads)*n_threads);
	 
	//Created the threads with their workload assigned
    for (j=0; j<n_threads;j++) {
		start= end +1;
		if(p%n_threads==0){
			end = end + (p/n_threads);
   		 }else if(cont>0){
			end = end + p/n_threads+1;
        	cont--;
    	} else{
			end = end+ (p/n_threads);
		}
		//printf("Ejecutando j: %d veces, startPosition: %d, endPosition: %d, cont: %d \n", j, start, end, cont);
		jobT.endPosition=end;
		jobT.startPosition=start;
        pthread_create (&h[j], NULL, *saxpy, (void *) &jobT);
		pthread_join (h[j], NULL);
    }
    //Getting the final time the SAXPY function last with threads and workload added to each one 
	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("RES: final vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);
#endif
	
	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);
	return 0;
	
}	


void* saxpy(void* arg){
    struct jobThread *jobT;
    jobT = (struct jobThread *)arg;
    for(it = 0; it < max_iters; it++){
		jobT->startPosition=0;
        for(i = jobT->startPosition; i <=jobT->endPosition; i++){
			jobT->startPosition=i;
			//printf("i: %d\n", i);
            Y[i] = Y[i] + a * X[i];
			Y_avgs[it] += Y[i];
		
		}
		pthread_mutex_lock(&mutex);
		Y_avgs[it] = Y_avgs[it] / p;
		pthread_mutex_unlock(&mutex);
	}
    pthread_exit(NULL);
}