#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check(const char* method_name, 
            cl_int error_code, 
            const cl_program* program, 
            const cl_device_id* device){
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

int read_kernel_src(char **buf, const char* filename){
  long length;
  FILE *f = fopen(filename, "r");
  
  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buf = malloc(length+1);
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