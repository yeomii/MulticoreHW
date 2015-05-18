#define TS 16
__kernel void mat_mul(__global float* A,
                      __global float* B,
                      __global float* C,
                      int n)
{
	
	int ts = get_local_size(0);
	int s = get_global_size(0);
	
	int local_r = get_local_id(0);
	int local_c = get_local_id(1);

	int global_r = ts*get_group_id(0) + local_r;
	int global_c = ts*get_group_id(1) + local_c;

	__local float Asub[TS][TS];
	__local float Bsub[TS][TS];

	float sum = 0;
	int tiles = n / ts;
	for(int t=0; t<tiles; t++)
	{
		int tiled_r = ts*t + local_r;
		int tiled_c = ts*t + local_c;
		Asub[local_r][local_c] = A[global_r*s + tiled_c];
		Bsub[local_r][local_c] = B[tiled_r*s + global_c];

		barrier(CLK_LOCAL_MEM_FENCE);
		for(int k=0; k<ts; k++)
		{
			sum += Asub[local_r][k]* Bsub[k][local_c];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	C[global_r*s + global_c] = sum;
}

