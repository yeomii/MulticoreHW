int read_kernel_src(char **buf, const char* filename);

void check(const char* method_name, cl_int error_code,
  const cl_program* program, const cl_device_id* device);