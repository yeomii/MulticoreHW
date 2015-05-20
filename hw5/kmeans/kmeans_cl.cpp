#include "kmeans.h"

#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include "opencl_util.h"

int global_size=0;
extern int work_group_size;
extern int use_gpu;

void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* partition)
{
  cl_int res;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;

  init_host(&platform, &device, &context, &queue);
  init_kernel("kernel.c", "kmeans", &program, &kernel, &context, &device);

  cl_uint ndim = 1;
  global_size = data_n;
  size_t global[1] = { global_size };
  size_t local[1] = { work_group_size };

  fprintf(stderr, "host init finished\n");

  printf("data_n : %d\n", data_n);
  printf("class_n : %d\n", class_n);
  printf("iteration_n : %d\n", iteration_n);
  printf("work_group_size : %d\n", work_group_size);
  printf("device : %s\n", (use_gpu != 0 ? "gpu" : "cpu"));

  cl_mem mem_data = init_buffer(&context, 1, sizeof(Point) * data_n);
  cl_mem mem_centroids = init_buffer(&context, 0, sizeof(Point) * class_n);
  cl_mem mem_partition = init_buffer(&context, 0, sizeof(int) * data_n);
  cl_mem mem_count = init_buffer(&context, 0, sizeof(int) * class_n);

  write_buffer(&queue, &mem_data, sizeof(Point) * data_n, data);
  write_buffer(&queue, &mem_centroids, sizeof(Point) * class_n, centroids);

  res = clFinish(queue);
  check("clFinish write data, centroids", res, NULL, NULL);

  set_kernel_arg(&kernel, 0, sizeof(cl_mem), (void *) &mem_data);
  set_kernel_arg(&kernel, 1, sizeof(cl_mem), (void *) &mem_centroids);
  set_kernel_arg(&kernel, 2, sizeof(cl_mem), (void *) &mem_partition);
  set_kernel_arg(&kernel, 3, sizeof(cl_mem), (void *) &mem_count);
  set_kernel_arg(&kernel, 4, sizeof(int), (void *) &data_n);
  set_kernel_arg(&kernel, 5, sizeof(int), (void *) &class_n);

  int* count = (int *)malloc(sizeof(int) * class_n);
  Point* new_centroids = (Point *)malloc(sizeof(Point) * class_n);

  for (int i = 0; i < iteration_n; ++i)
  {
    res = clEnqueueNDRangeKernel(queue, kernel, ndim, NULL, global, local, 0, NULL, NULL);
    check("clEnqueueNDRangeKernel", res, NULL, NULL);
    read_buffer(&queue, &mem_partition, sizeof(int) * data_n, partition);
    for (int ci = 0; ci < class_n; ++ci)
    {
      count[ci] = 0;
      new_centroids[ci].x = 0;
      new_centroids[ci].y = 0;
    }
    res = clFinish(queue);
    check("clFinish read partition", res, NULL, NULL);

    for (int pi = 0; pi < data_n; ++pi)
    {
      int part = partition[pi];
      new_centroids[part].x += data[pi].x;
      new_centroids[part].y += data[pi].y;
      count[part]++;
    }

    for (int ci = 0; ci < class_n; ++ci)
    {
      new_centroids[ci].x /= count[ci];
      new_centroids[ci].y /= count[ci];
    }

    write_buffer(&queue, &mem_centroids, sizeof(Point) * class_n, new_centroids);
    fprintf(stderr, "%d th iteration\n", i);
  }
  read_buffer(&queue, &mem_centroids, sizeof(Point) * class_n, centroids);
  read_buffer(&queue, &mem_partition, sizeof(int) * data_n, partition);

  res = clFinish(queue);
  check("clFinish read centroids", res, NULL, NULL);
}