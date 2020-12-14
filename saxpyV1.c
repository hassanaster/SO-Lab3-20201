/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * @brief      Update V0: we created the n threads without assign the workload to each threat with a lock in the clitical code
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Danny Munera V0
 * @date       2020
 * @author     Miriam Arango, Yesison Quinto, Luisa V1
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
int i, it;
//Lock to ensure a non-race condition
pthread_mutex_t mutex;


void* saxpy(void* arg);


int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	unsigned int seed = 1; // this is to generate the ramdom numbers for each vector
  	
	// Variables to get execution time
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
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
			printf("option -%c needs a value\n", optopt);   //optopt ?
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

	for(i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for(i = 0; i < max_iters; i++){
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

	
	gettimeofday(&t_start, NULL);
	

	pthread_t h[n_threads];
    for (i=0; i<n_threads;i++) {
        pthread_create (&h[i], NULL, *saxpy, NULL);
        pthread_join (h[i], NULL);
    }
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

/*
*	Function to parallelize 
*/

//SAXPY iterative SAXPY mfunction with the lock in the critical code

void* saxpy(void* arg){
    for(it = 0; it < max_iters; it++){
		pthread_mutex_lock(&mutex);
        for(i = 0; i < p; i++){
            Y[i] = Y[i] + a * X[i];
			Y_avgs[it] += Y[i];
            pthread_mutex_unlock(&mutex);
		}
		Y_avgs[it] = Y_avgs[it] / p;
	}
    pthread_exit(NULL);
}