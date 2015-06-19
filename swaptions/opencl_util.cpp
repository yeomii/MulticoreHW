#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opencl_util.h"

extern int use_gpu;

void check(const char* method_name, 
    cl_int error_code, 
    const cl_program* program, 
    const cl_device_id* device)
{
  if(error_code == CL_SUCCESS)
    return;

  if (error_code == CL_BUILD_PROGRAM_FAILURE)
  {
    fprintf(stderr, "Error : %s, Result : CL_BUILD_PROGRAM_FAILURE\n", method_name);
    char* build_log;
    size_t log_size;
    clGetProgramBuildInfo(*program, *device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    build_log = (char *)malloc(log_size+1);
    clGetProgramBuildInfo(*program, *device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
    build_log[log_size] = '\0';
    fprintf(stderr, build_log);
  }
  else
  {
    fprintf(stderr, "Error : %s, Result : %d\n", method_name, error_code);
  }
  exit(0);
}

int read_kernel_src(char **buf, const char* filename)
{
  long length;
  FILE *f = fopen(filename, "r");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buf = (char *)malloc(length+1);
    if (*buf) {
      fread(*buf, 1, length, f);
      (*buf)[length-1] = 0;
      fclose(f);
      return 0;
    } else {
      fclose(f);
      return -1;
    }
  } else {
    return -1;
  }
}

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

void init_kernel(const char* src_name, const char* method_name,
    cl_program* program, cl_kernel* kernel, 
    cl_context* context, cl_device_id* device, cl_uint num_devices)
{
  cl_int res;
  char* kernel_src;
  read_kernel_src(&kernel_src, src_name);
  size_t kernel_src_len = strlen(kernel_src);

  *program = clCreateProgramWithSource(
      *context, 1, (const char **)&kernel_src, &kernel_src_len, &res);
  check("clCreateProgramWithSource", res, NULL, NULL);
  res = clBuildProgram(*program, num_devices, device, NULL, NULL, NULL);
  check("clBuildProgram", res, program, device);
  *kernel = clCreateKernel(*program, method_name, &res);
  check("clCreateKernel", res, NULL, NULL);
}

cl_mem init_buffer(cl_context* context, int read_only, 
    size_t size, void* host_ptr)
{
  cl_int res;
  cl_mem_flags flag;
  if (read_only != 0)
    flag = CL_MEM_READ_ONLY ;
  else
    flag = CL_MEM_WRITE_ONLY ;

  cl_mem buf = clCreateBuffer(*context, flag, size, NULL, &res);
  check("clCreateBuffer", res, NULL, NULL);
  return buf;
}

void set_kernel_arg(cl_kernel* kernel, int idx, size_t size, const void* arg)
{
  cl_int res = clSetKernelArg(*kernel, idx, size, arg);
  check("clSetKernelArg", res, NULL, NULL);
}

void write_buffer(cl_command_queue *queue, cl_mem *mem, size_t size,
    const void* ptr)
{
  cl_int res;
  res = clEnqueueWriteBuffer(
      *queue, *mem, CL_FALSE, 0, size, ptr, 0, NULL, NULL);
  check("clEnqueueWriteBuffer", res, NULL, NULL);
}

void read_buffer(cl_command_queue *queue, cl_mem *mem, size_t size, void *ptr)
{
  cl_int res;
  res = clEnqueueReadBuffer(
      *queue, *mem, CL_FALSE, 0, size, ptr, 0, NULL, NULL);
  check("clEnqueueReadBuffer", res, NULL, NULL);
}

