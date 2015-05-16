#include "kmeans.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <float.h>
#include <semaphore.h>

extern int thread_n;

typedef struct thread_arg {
	int n, class_n, data_n;
	Point* centroids; 
	Point* data;
	int* partitioned;
}thread_arg ;


void *assign(void *arg){
	thread_arg *args = (thread_arg *)arg;
	int data_i, class_i;
	int start_i = (args->data_n / thread_n) * args->n;
	int end_i = ((args->n + 1) != thread_n) ? ((args->data_n / thread_n) * (args->n + 1)) : args->data_n;
	Point t = {0,0};
	for (data_i = start_i ; data_i < end_i ; data_i++){
		float min_dist = DBL_MAX;
		for (class_i = 0; class_i < args->class_n; class_i++) {
			t.x = args->data[data_i].x - args->centroids[class_i].x;
			t.y = args->data[data_i].y - args->centroids[class_i].y;

			float dist = t.x*t.x + t.y*t.y;
			
			if (dist < min_dist) {
				args->partitioned[data_i] = class_i;
				min_dist = dist;
			}
		}
	}
}

void kmeans(int iter_n, int class_n, int data_n, Point* centroids, Point* data, int* partitioned)
{
	int i, j, class_i, data_i;
	pthread_t p_threads[thread_n];
	thread_arg arg[thread_n];
	int* count = (int*)malloc(sizeof(int) * class_n);

	printf("# of thread : %d\n", thread_n);

	for (j=0; j<thread_n; j++){
		arg[j] = { j, class_n, data_n, centroids, data, partitioned };
	}

	for ( i=0; i<iter_n; i++ ){
		for (j=0; j<thread_n; j++)
			pthread_create(&p_threads[j], NULL, assign, (void *)&arg[j]);
		
		for (j=0; j<thread_n; j++)
			pthread_join(p_threads[j], NULL);

		// Update step
        // Clear sum buffer and class count
		for (class_i = 0; class_i < class_n; class_i++) {
			centroids[class_i].x = 0.0;
            centroids[class_i].y = 0.0;
            count[class_i] = 0;
        }
        // Sum up and count data for each class
        for (data_i = 0; data_i < data_n; data_i++) {         
            centroids[partitioned[data_i]].x += data[data_i].x;
            centroids[partitioned[data_i]].y += data[data_i].y;
            count[partitioned[data_i]]++;
        }
        // Divide the sum with number of class for mean point
        for (class_i = 0; class_i < class_n; class_i++) {
            centroids[class_i].x /= count[class_i];
            centroids[class_i].y /= count[class_i];
        }
		
	}
}

