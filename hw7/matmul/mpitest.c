#include "mpi.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	int my_rank, size;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	printf("%d out of %d\n", my_rank, size);

	if (my_rank == 0)
	{
		int v, sv;
		MPI_Get_version(&v, &sv);
		printf("version : %d.%d\n", v, sv);
	}
	MPI_Finalize();
	return 0;
}
