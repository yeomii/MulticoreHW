

#ifndef __KMENAS_H__
#define __KMEANS_H__

#include <time.h>

#define GET_TIME(T) __asm__ __volatile__ ("rdtsc\n" : "=A" (T))

struct Point {
    float x, y;
};


// Kmean algorighm
void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* clsfy_result);

int timespec_subtract(struct timespec*, struct timespec*, struct timespec*);

#endif // __KMEANS_H__

