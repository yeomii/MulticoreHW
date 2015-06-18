void check(const char* method_name, 
            cl_int error_code, 
            const cl_program* program, 
            const cl_device_id* device);

int read_kernel_src(char **buf, const char* filename);

void init_host(cl_platform_id* platform, cl_device_id* device, 
                cl_context* context, cl_command_queue* queue);

void init_kernel(const char* src_name, const char* method_name,
                 cl_program* program, cl_kernel* kernel, 
                 cl_context* context, cl_device_id* device);

cl_mem init_buffer(cl_context* context, int read_only, 
                   size_t size, void* host_ptr);

void set_kernel_arg(cl_kernel* kernel, int idx, size_t size, const void* arg);

void write_buffer(cl_command_queue *queue, cl_mem *mem, size_t size,
                  const void* ptr);

void read_buffer(cl_command_queue *queue, cl_mem *mem, size_t size, void *ptr);
