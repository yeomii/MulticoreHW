__kernel void mat_mul(__global float* A,
                      __global float* B,
                      __global float* C,
                      int n)
{
  int i = get_global_id(0);
  int j = get_global_id(1);
	int s = get_global_size(0);

  float sum = 0;
  for (int k = 0; k < n; ++k)
  {
    sum += A[i*n + k] * B[k*s + j];      
  }
  C[i*s + j] = sum;
}

