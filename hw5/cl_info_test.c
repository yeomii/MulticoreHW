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
		printf("%s\n", buf);
	else if (res == CL_INVALID_VALUE)
		printf("get platform info Error :INVALID_VALUE %d\n", res);
	else if (res == CL_INVALID_PLATFORM)
		printf("get platform info Error :INVALID_PLATFORM %d\n", res);
	else
		printf("get platform info Error :%d\n", res);
}

void print_device(cl_platform_id pid){
	cl_device_id devices[10];
	cl_uint available;
	cl_int res;
	char buf[10240];

	res = clGetDeviceIDs(pid, CL_DEVICE_TYPE_ALL, 10, devices, &available);
	if (res != CL_SUCCESS)
		printf("get device id Error : %d\n", res);
	
	for (int i=0; i<(int)available; i++){
		cl_device_id id = devices[i];
		cl_uint size;
		printf("device %d - id : %u\n", i, id);
		clGetDeviceInfo(id, CL_DEVICE_NAME, 10240, buf, NULL);
		printf("%s\n", buf);
		clGetDeviceInfo(id, CL_DEVICE_PROFILE, 10240, buf, NULL);
		printf("%s\n", buf);
		clGetDeviceInfo(id, CL_DEVICE_VERSION, 10240, buf, NULL);
		printf("%s\n", buf);
		clGetDeviceInfo(id, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &size, NULL);
		printf("address bits: %d\n", size);
		clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_uint), &size, NULL);
		printf("max work group size: %d\n", size);
	}

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
