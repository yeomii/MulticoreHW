
#include <omp.h> 
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "kmeans.h"

extern int threads;

void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* partitioned)
{
  int* count = (int*)malloc(sizeof(int) * class_n);
  int* count_private;
  Point* centroids_private;

  omp_set_dynamic(0);
  omp_set_num_threads(threads);
  printf("Number of Threads : %d\n", omp_get_max_threads());

  count_private = (int *)malloc(sizeof(int) * class_n * threads);
  centroids_private = (Point *)malloc(sizeof(Point) * class_n * threads);

  for (int iter=0; iter<iteration_n; iter++)
  {
		#pragma omp parallel for default(shared) schedule(static)
		for (int ci=0; ci < class_n * threads; ci++)
		{
			centroids_private[ci].x = 0.0;
      centroids_private[ci].y = 0.0;
      count_private[ci] = 0;
		}

   	// Assignment step
    #pragma omp parallel for default(shared) schedule(static)
    for (int di = 0; di < data_n; di++)
    {
      float min_dist = DBL_MAX;
      for(int ci=0; ci < class_n; ci++)
      {
        Point t;
        t.x = data[di].x - centroids[ci].x;
        t.y = data[di].y - centroids[ci].y;
        float dist = t.x * t.x + t.y * t.y;

        if (dist < min_dist) {
          partitioned[di] = ci;
          min_dist = dist;
        }
      }
    }
			
		#pragma omp barrier

    #pragma omp parallel for default(shared) schedule(static)
    for (int ci=0; ci < class_n; ci++)
    {
      centroids[ci].x = 0.0;
      centroids[ci].y = 0.0;
      count[ci] = 0;
    }

		#pragma omp parallel for default(shared) schedule(static) 
		for (int di = 0; di < data_n; ++di)
		{
			int offset = omp_get_thread_num() * class_n;
      int p = partitioned[di];
			centroids_private[offset + p].x += data[di].x;
      centroids_private[offset + p].y += data[di].y;
      count_private[offset + p]++;
		}

		#pragma omp barrier

		#pragma omp parallel for default(shared) schedule(static)
		for (int ci = 0; ci < class_n; ++ci)
		{
			for (int ti=0; ti < threads; ++ti)
			{
				int idx = class_n * ti + ci;
				centroids[ci].x += centroids_private[idx].x;
				centroids[ci].y += centroids_private[idx].y;
				count[ci] += count_private[idx];
			}
		}

		#pragma omp parallel for default(shared) schedule(static)
		for(int ci=0; ci < class_n ; ++ci)
		{
      centroids[ci].x /= count[ci];
      centroids[ci].y /= count[ci];
		}
  }
}

