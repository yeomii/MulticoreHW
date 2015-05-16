#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <float.h>

typedef struct Point {
	float x, y
} Point;

int thread_n = 4;
Point* centoids;




int main(){
	int i, j;
	pthread_t* threads = (pthread_t *) malloc(thread_n * sizeof(pthread_t))	;

	for (i=0; i<10; i++)
	{
		for (j=0; j<thread_n; j++){

		}
	}

}





