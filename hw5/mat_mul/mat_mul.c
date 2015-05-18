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
#define GPU_MAX_MEM_ALLOC_SIZE 536870912
#define CPU_MAX_MEM_ALLOC_SIZE 33823679488

int print_matrix=0;
int validation=0;
int Column=1;
int Row=1;
int N=20;
int global_size=10;
int work_group_size=4;
int use_gpu=1;
int adding=0;

void init_host(cl_platform_id* platform, cl_device_id* device, 
                cl_context* context, cl_command_queue* queue)
{
  cl_int res;

  res = clGetPlatformIDs(1, platform, NULL);
  check("clGetPlatformIDs", res, NULL, NULL);
  if (use_gpu != 0)
    res = clGetDeviceIDs(*platform, CL_DEVICE_TYPE_GPU, 1, device, NULL);
  else 
    res = clGetDeviceIDs(*platform, CL_DEVICE_TYPE_CPU, 1, device, NULL);
  check("clGetDeviceIDs", res, NULL, NULL);
  *context = clCreateContext(NULL, 1, device, NULL, NULL, &res);
  check("clCreateContext", res, NULL, NULL);
  *queue = clCreateCommandQueue(*context, *device, 0, &res);
  check("clCreateCommandQueue", res, NULL, NULL);
}

void init_buffer(cl_mem* buffers, size_t* sizes, cl_context* context)
{
  cl_int res;

  buffers[0] = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizes[0], NULL, &res);
  check("clCreateBuffer 0", res, NULL, NULL);
  buffers[1] = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizes[1], NULL, &res);
  check("clCreateBuffer 1", res, NULL, NULL);
  buffers[2] = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizes[2], NULL, &res);
  check("clCreateBuffer 2", res, NULL, NULL);
}

void init_kernel(cl_program* program, cl_kernel* kernel, 
                 cl_context* context, cl_device_id* device)
{
  cl_int res;
  char* kernel_src;
  read_kernel_src(&kernel_src, "kernel.c");
  size_t kernel_src_len = strlen(kernel_src);

  *program = clCreateProgramWithSource(
    *context, 1, (const char **)&kernel_src, &kernel_src_len, &res);
  check("clCreateProgramWithSource", res, NULL, NULL);
  res = clBuildProgram(*program, 1, device, NULL, NULL, NULL);
  check("clBuildProgram", res, program, device);
  *kernel = clCreateKernel(*program, "mat_mul", &res);
  check("clCreateKernel", res, NULL, NULL);
}

void host( int n, int tile_size )
{
	cl_int res;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;

  init_host(&platform, &device, &context, &queue);
  
  cl_program program;
  cl_kernel kernel;
  init_kernel(&program, &kernel, &context, &device);

  size_t sizes[3];
  sizes[0] = sizeof(float) * n * tile_size;
	sizes[1] = sizeof(float) * n * tile_size;
  sizes[2] = sizeof(float) * tile_size * tile_size;

  cl_uint ndim = 2;
  size_t global[2] = { tile_size, tile_size };
  size_t local[2] = { work_group_size, work_group_size };

  fprintf(stderr, "host init finished\n");

  int tile_n = ((n - 1) / tile_size) + 1;
  float* a = (float *)malloc(sizes[0]);
  float* b = (float *)malloc(sizes[1]);
  float* c = (float *)malloc(sizes[2]);

	cl_mem bufs[3];
  cl_mem_flags r_flag = 
    (use_gpu != 0) ? CL_MEM_READ_ONLY : (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
  cl_mem_flags w_flag = 
    (use_gpu != 0) ? CL_MEM_WRITE_ONLY : (CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR);
	bufs[0] = clCreateBuffer(context, r_flag, sizes[0], a, &res);
  check("clCreateBuffer 0", res, NULL, NULL);
	bufs[1] = clCreateBuffer(context, r_flag, sizes[1], b, &res);
  check("clCreateBuffer 1", res, NULL, NULL);
	bufs[2] = clCreateBuffer(context, w_flag, sizes[2], c, &res);
	check("clCreateBuffer 2", res, NULL, NULL);
  
	res = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &bufs[0]);
	check("clSetKernelArg0", res, NULL, NULL);
	res = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*) &bufs[1]);
	check("clSetKernelArg1", res, NULL, NULL);
	res = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*) &bufs[2]);
	check("clSetKernelArg2", res, NULL, NULL);
	res = clSetKernelArg(kernel, 3, sizeof(int), (void*) &n);
	check("clSetKernelArg3", res, NULL, NULL);

	for (int ci = 0; ci < tile_n; ++ci)
  {
		timer_start(5);
    gen_tile(&a, tile_size, n, ci*tile_size, 0, n, n);
    if (use_gpu != 0)
      res = clEnqueueWriteBuffer(queue, bufs[0], CL_FALSE, 0, sizes[0], a, 0, NULL, NULL);
    else
      clEnqueueMapBuffer(queue, bufs[0], CL_FALSE, CL_MAP_READ, 0, sizes[0], 0, NULL, NULL, &res);
    check("clEnqueueWriteBuffer_a", res, NULL, NULL);
		timer_stop(5);

    for (int cj = 0; cj < tile_n; ++cj)
    {
			timer_start(5);
      fprintf(stderr, "(%d, %d) tile started\n", ci, cj);
      gen_tile(&b, n, tile_size, 0, cj*tile_size, n, n);
			timer_stop(5);

      timer_start(2);
      if (use_gpu != 0)
        res = clEnqueueWriteBuffer(queue, bufs[1], CL_FALSE, 0, sizes[1], b, 0, NULL, NULL);
      else
        clEnqueueMapBuffer(queue, bufs[1], CL_FALSE, CL_MAP_READ, 0, sizes[1], 0, NULL, NULL, &res);
      check("clEnqueueWriteBuffer_b", res, NULL, NULL);
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
      if (use_gpu != 0)
        res = clEnqueueReadBuffer(queue, bufs[2], CL_TRUE, 0, sizes[2], c, 0, NULL, NULL);
      else
        clEnqueueMapBuffer(queue, bufs[2], CL_FALSE, CL_MAP_WRITE, 0, sizes[2], 0, NULL, NULL, &res);
      check("clEnqueueReadBuffer", res, NULL, NULL);
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
	else if (N < 2048)
	{
		global_size = N;
		work_group_size = 16;
	}
	else
	{
		global_size = 1024;
		work_group_size = 16;
	}
}

int main(int argc, char** argv)
{
  fprintf(stderr, "start\n");

	parse_opt( argc, argv );
	//adjust_size();

  timer_init();
	timer_start(1);
	host( N, global_size );
	timer_stop(1);

  print_result(N);

	return 0;
}
