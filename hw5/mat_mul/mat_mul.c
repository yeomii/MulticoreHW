#include <CL/opencl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "timers.h"
#include "util.h"
#include "opencl_util.h"

#define MIN(a, b) (((a)<(b)) ? (a) : (b))
#define MAX(a, b) (((a)>(b)) ? (a) : (b))
#define MAX_MEM_ALLOC_LIMIT 100000000

int print_matrix=0;
int validation=0;
int Column=1;
int Row=1;
int N=20;
int global_size=10;
int work_group_size=4;
int use_gpu=1;

void host( int n, int tile_size )
{
	cl_int res;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;

  /* n is evenly divisable by tile_size */ 
  if (n % tile_size != 0)
    n = ((n/tile_size) + 1) * tile_size; 

  init_host(&platform, &device, &context, &queue);
  
  cl_program program;
  cl_kernel kernel;
  init_kernel("kernel.c", "mat_mul", &program, &kernel, &context, &device);

  size_t sizes[3];
  sizes[0] = sizeof(float) * n * tile_size;
	sizes[1] = sizeof(float) * n * tile_size;
  sizes[2] = sizeof(float) * tile_size * tile_size;

  cl_uint ndim = 2;
  size_t global[2] = { tile_size, tile_size };
  size_t local[2] = { work_group_size, work_group_size };

  fprintf(stderr, "host init finished\n");

  int tile_n = n / tile_size;
  float* a = (float *)malloc(sizes[0]);
  float* b = (float *)malloc(sizes[1]);
  float* c = (float *)malloc(sizes[2]);

	cl_mem bufs[3];
  bufs[0] = init_buffer(&context, 1, sizes[0], a);
  bufs[1] = init_buffer(&context, 1, sizes[1], a);
  bufs[2] = init_buffer(&context, 0, sizes[2], a);
  
  set_kernel_arg(&kernel, 0, sizeof(cl_mem), (void*) &bufs[0]);
  set_kernel_arg(&kernel, 1, sizeof(cl_mem), (void*) &bufs[1]);
  set_kernel_arg(&kernel, 2, sizeof(cl_mem), (void*) &bufs[2]);
  set_kernel_arg(&kernel, 3, sizeof(int), (void*) &n);

	for (int ci = 0; ci < tile_n; ++ci)
  {
		timer_start(5);
    gen_tile(&a, tile_size, n, ci*tile_size, 0, N, N);
    write_buffer(&queue, &(bufs[0]), sizes[0], a);
		timer_stop(5);

    for (int cj = 0; cj < tile_n; ++cj)
    {
			timer_start(5);
      fprintf(stderr, "(%d, %d) tile started\n", ci, cj);
      gen_tile(&b, n, tile_size, 0, cj*tile_size, N, N);
      timer_stop(5);

      timer_start(2);
      write_buffer(&queue, &(bufs[1]), sizes[1], b);
      res = clFinish(queue);
      check("clFinish_writebuffer_b", res, NULL, NULL);
      timer_stop(2);

      fprintf(stderr, "a, b write buffer\n");

      timer_start(3);
      res = clEnqueueNDRangeKernel(queue, kernel, ndim, NULL, global, local, 0, NULL, NULL);
      check("clEnqueueNDRangeKernel", res, NULL, NULL);
      res = clFinish(queue);
      check("clFinish_kernel", res, NULL, NULL);
      timer_stop(3);

      fprintf(stderr, "kernel\n");

      timer_start(4);
      read_buffer(&queue, &(bufs[2]), sizes[2], c);
      res = clFinish(queue);
      check("clFinish_read", res, NULL, NULL);
      timer_stop(4);

      fprintf(stderr, "(%d, %d) tile finished\n", ci, cj);

      if (print_matrix){
        print_mat(c, tile_size, tile_size);
      }
      if (validation){
        check_mat_mul(a, b, c, n, tile_size);
      }
    }
  }
}

void adjust_size()
{
	if (N < 16)
	{
		global_size = N;
		work_group_size = N;
	}
	else if (N < 10000)
	{
		global_size = (N % 16 == 0) ? N : ((int)(N/16) + 1) * 16;
		work_group_size = 16;
	}
	else
	{
    work_group_size = 16;
    if (N % 10000 == 0)
      global_size = 1000;
    else
      global_size = 1024;
/*
    int limit = MAX_MEM_ALLOC_SIZE / N;
    if (limit % 16 != 0)
      limit = limit - (limit % 16);
    int fit_global, r = limit; 
    for(i=limit; i > limit - 1000 ; i-=16)
    {
      int t = N % i;
      if (t == 0)
      {
        fit_global = i;
        break;
      }
      else if (t < r)
      {
        r = t;
        fit_global = i;
      }
    }
		global_size = fit_global;
*/
	}
}

int main(int argc, char** argv)
{
  fprintf(stderr, "start\n");

	parse_opt( argc, argv );
	adjust_size();

  timer_init();
	timer_start(1);
	host( N, global_size );
	timer_stop(1);

  print_result(N);

	return 0;
}
