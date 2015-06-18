#include <stddef.h>
#include <float.h>
#include <stdlib.h>
#include "mpi.h"
#include "kmeans.h"

MPI_Datatype pointtype;

void createType()
{
  int blocklens[] = {1, 1};
  MPI_Aint indices[2];
  indices[0] = (MPI_Aint)offsetof(struct Point, x);
  indices[1] = (MPI_Aint)offsetof(struct Point, y);

  MPI_Datatype types[2] = {MPI_FLOAT, MPI_FLOAT};

  MPI_Type_create_struct(2, blocklens, indices, types, &pointtype);
  MPI_Type_commit(&pointtype);
}

void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* partitioned)
{
  int comm_size, comm_rank;
  Point t;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  int begin = (data_n / comm_size) * comm_rank;
  int end = (comm_rank + 1 != comm_size) ? (data_n / comm_size) * (comm_rank + 1) : data_n;
  int gap = end - begin;

  int* new_partition = (int *)malloc(sizeof(int) * gap);
  int* new_count = (int *)malloc(sizeof(int) * class_n);
  Point* new_centroids = (Point *)malloc(sizeof(Point) * class_n);

  createType();

  for (int i=0; i<iteration_n; i++)
  {
    for (int ci = 0; ci < class_n; ci++)
    {
      new_count[ci] = 0;
      new_centroids[ci].x = 0;
      new_centroids[ci].y = 0;
    }

    for (int di = 0; di < gap; di++)
    {
      float min = DBL_MAX;
      
      for (int ci = 0; ci < class_n; ci++)
      {
        t.x = data[di + begin].x - centroids[ci].x;
        t.y = data[di + begin].y - centroids[ci].y;

        float dist = t.x * t.x + t.y * t.y;

        if (dist < min)
        {
          new_partition[di] = ci;
          min = dist;
        }
      }
      new_count[new_partition[di]]++;
      new_centroids[new_partition[di]].x += data[di + begin].x;
      new_centroids[new_partition[di]].y += data[di + begin].y;
    }

    if (comm_rank != 0)
    {
      MPI_Send((void *)new_count, class_n, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send((void *)new_centroids, class_n, pointtype, 0, 0, MPI_COMM_WORLD);
      if (i+1 != iteration_n)
        MPI_Recv((void *)centroids, class_n, pointtype, 0, 0, MPI_COMM_WORLD, NULL);
      else
        MPI_Send((void *)new_partition, gap, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
      MPI_Status status1, status2;
      int* tmp_count = (int *)malloc(sizeof(int) * class_n);
      Point* tmp_centroids = (Point *)malloc(sizeof(Point) * class_n);
      
      for (int src = 1; src < comm_size ; src++)
      {
        MPI_Recv((void *)tmp_count, class_n, MPI_INT, src, 0, MPI_COMM_WORLD, &status1);
        MPI_Recv((void *)tmp_centroids, class_n, pointtype, src, 0, MPI_COMM_WORLD, &status2);
        
        for (int ci=0; ci<class_n; ci++)
        {
          new_count[ci] += tmp_count[ci];
          new_centroids[ci].x += tmp_centroids[ci].x;
          new_centroids[ci].y += tmp_centroids[ci].y;
        }
      }
      for (int ci=0; ci<class_n; ci++)
      {
        centroids[ci].x = new_centroids[ci].x / new_count[ci];
        centroids[ci].y = new_centroids[ci].y / new_count[ci];
      }
      
      if (i+1 != iteration_n)
      {
        for (int src=1; src<comm_size; src++)
          MPI_Send((void *)centroids, class_n, pointtype, src, 0, MPI_COMM_WORLD);
      }
      else
      {
        for (int src = 1; src < comm_size; src++)
          MPI_Recv((void *)(partitioned + (gap * src)), (src + 1 != comm_size ? gap : data_n - gap * src),
                   pointtype, src, 0, MPI_COMM_WORLD, &status1);
        for (int di = 0; di < gap; di++)
          partitioned[di] = new_partition[di];
      }
    }
  }

  MPI_Finalize();
  if (comm_rank != 0)
    exit(0);
}

