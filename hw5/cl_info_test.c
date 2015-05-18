#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

void print_platform(cl_platform_id pid, cl_platform_info pinfo){
	char buf[1024];
	cl_int res;
	res = clGetPlatformInfo(pid, pinfo, 1024, buf, NULL);
	if (res == CL_SUCCESS)
		printf("%s\n\n", buf);
	else if (res == CL_INVALID_VALUE)
		printf("get platform info Error :INVALID_VALUE %d\n", res);
	else if (res == CL_INVALID_PLATFORM)
		printf("get platform info Error :INVALID_PLATFORM %d\n", res);
	else
		printf("get platform info Error :%d\n", res);
}

void print_device_info(cl_device_id id){
	char buf[1024];

	cl_uint size;
	cl_ulong usize;
	size_t sizes[4];

	printf("device id : %u\n", id);
	clGetDeviceInfo(id, CL_DEVICE_NAME, 1024, buf, NULL);
	printf("%s\n", buf);
	clGetDeviceInfo(id, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &size, NULL);
	printf("address bits: %u\n", size);
	clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &usize, NULL);
	printf("device global mem cache size : %lu\n", usize);
	clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint), &size, NULL);
	printf("device global mem cacheline size : %u\n", size);
	clGetDeviceInfo(id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &usize, NULL);
	printf("device global mem global mem size : %lu\n", usize);
	clGetDeviceInfo(id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &usize, NULL);
	printf("device local mem size : %lu\n", usize);
	clGetDeviceInfo(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &size, NULL);
	printf("device max compute units : %u\n", size);
	clGetDeviceInfo(id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &usize, NULL);
	printf("device max mem alloc size : %lu\n", usize);
	clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_uint), &size, NULL);
	printf("max work group size: %u\n", size);
	clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 4, sizes, NULL);
	printf("device max work item sizes : %d, %d, %d\n", sizes[0], sizes[1], sizes[2]);
	printf("\n");
}

void print_device(cl_platform_id pid){
	cl_device_id devices[10];
	cl_uint available;
	cl_int res;

	res = clGetDeviceIDs(pid, CL_DEVICE_TYPE_GPU, 10, devices, &available);
	if (res != CL_SUCCESS)
		printf("get device id Error : %d\n", res);
	print_device_info(devices[0]);
	res = clGetDeviceIDs(pid, CL_DEVICE_TYPE_CPU, 10, devices, &available);
	if (res != CL_SUCCESS)
		printf("get device id Error : %d\n", res);
	print_device_info(devices[0]);
}

int main (int argc, char* argv[]) {
	cl_platform_id platforms[10];
	cl_uint available;
	cl_int res;

	res = clGetPlatformIDs(10, platforms, &available);
	if (res != CL_SUCCESS)
		printf("get platform id Error : %d\n", res);

	for (int i=0; i<(int)available; i++){
		cl_platform_id id = platforms[i];
		printf("platform %d - id : %u\n", i, id);
		print_platform(id, CL_PLATFORM_PROFILE);
		print_platform(id, CL_PLATFORM_VERSION);
		print_platform(id, CL_PLATFORM_NAME);
		print_platform(id, CL_PLATFORM_VENDOR);
		print_device(id);
	}

	return 0;	
}
