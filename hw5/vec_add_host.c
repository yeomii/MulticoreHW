#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

int read_kernel_src(char **buf, const char* filename){
  long length;
  FILE *f = fopen(filename, "r");
  
  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buf = malloc(length);
    if (*buf) {
      fread(*buf, 1, length, f);
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

int main(int argc, char const *argv[])
{
  char* kernel_src;
  float* host[3];
  cl_int errcode;
  size_t size = SIZE * sizeof(float);

  if (read_kernel_src(&kernel_src, "vec_add.c") < 0) {
    fprintf(stderr, "read_kernel_src: failed\n");
    return 0;
  }

  for (int i = 0; i < 3; ++i)
    host[i] = (float *)malloc(size);

  for (int i = 0; i < SIZE; i++)
  {
    host[0][i] = (float) i;
    host[1][i] = (float) i*2;
  }

  cl_platform_id platform;
  cl_device_id devices[4];
  cl_uint num_devices;
  cl_context context;

  errcode = clGetPlatformIDs(1, &platform, NULL);
  if (errcode != CL_SUCCESS) 
    fprintf(stderr, "Error at clGetPlatformIds\n");

  errcode = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 4, devices, &num_devices);
  if (errcode != CL_SUCCESS)
    fprintf(stderr, "Error at clGetDeviceIds\n");

  context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &errcode);
  if (errcode != CL_SUCCESS) 
    fprintf(stderr, "Error at clCreateContext");

  cl_command_queue command_queues[4];
  for (int i = 0; i < num_devices; ++i)
  {
    command_queues[i] = clCreateCommandQueue(context, devices[i], 0, &errcode);
    if (errcode != CL_SUCCESS) 
      fprintf(stderr, "Error at clCreateCommandQueue");
  }

  cl_mem buffers[3];
  buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, NULL);
  buffers[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, NULL);
  buffers[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, NULL);

  cl_program program;
  size_t kernel_src_len = strlen(kernel_src);
  program = clCreateProgramWithSource(
    context, 1, (const char**) &kernel_src, &kernel_src_len, NULL);
  clBuildProgram(program, 3, devices, NULL, NULL, NULL);

  cl_kernel kernel;
  kernel = clCreateKernel(program, "vec_add", NULL);
  for (int i = 0; i < 3; ++i)
    clSetKernelArg(kernel, i, sizeof(cl_mem), (void*) &buffers[i]);

  clEnqueueWriteBuffer(command_queues[0], buffers[0], CL_FALSE, 0, size, host[0], 0, NULL, NULL);
  clEnqueueWriteBuffer(command_queues[1], buffers[1], CL_FALSE, 0, size, host[1], 0, NULL, NULL);
  
  size_t global[1] = {SIZE};
  size_t local[1] = {16};
  clEnqueueNDRangeKernel(command_queues[2], kernel, 1, NULL, global, local, 0, NULL, NULL);
  for (int i = 0; i < num_devices; ++i)
  {
    clFinish(command_queues[i]);
  }

  clEnqueueReadBuffer(command_queues[0], buffers[2], CL_TRUE, 0, size, host[2], 0, NULL, NULL);

  for (int i = 0; i < SIZE; ++i)
  {
    printf("C[%d]=%f\n", i, host[2][i]);
  }

  return 0;
}
