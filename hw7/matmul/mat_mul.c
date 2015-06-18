#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"
#include "timers.h"

#define NDIM 8192

float C[NDIM][NDIM];

int print_matrix = 0;
int validation = 0;

int comm_size = 0; // comm_size must be 2^n
int comm_rank = 0;

void init_matrix(float* a, int n, int m, int si, int sj, int clear)
{
  if (n + si > NDIM || m + sj > NDIM)
  {
    printf("Error: init_matrix\n n : %d, si : %d, m : %d, sj : %d\n", n, si, m, sj);
    exit(0);
  }

  if (clear)
  {
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
        a[i*m + j] = 0;
    }
  }
  else
  {
    for (int i = 0; i < n; i++)
    {
      int k = (si+i) * NDIM + sj;
      
      for (int j = 0; j < m; j++)
        a[i*m + j] = k++;
    }
  }
}

void matmul(float* a, float* b, float* c, int n, int p, int m)
{
  // a : n x p
  // b : p x m
  /*
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
    {
      for (int k = 0; k < p; k++)
        c[i*m + j] += a[i*p + k] * b[k*m + j];
    }
  */
  for (int i=0; i < n; i++)
    for (int k=0; k < p; k++)
      for (int j=0; j < m; j++)
        c[i*m + j] += a[i*p+k] * b[k*m+j];
}


void mpi_matmul()
{
  int bwn = 1, bhn = 1, tmp = comm_size;
  while (tmp > 3)
  {
    tmp /= 4;
    bwn *= 2, bhn *= 2;
  }
  if (tmp > 1)
    bhn *= 2;

  int bwidth = NDIM / bwn, bheight = NDIM / bhn;
  
  int bw = comm_rank % bwn;
  int bh = comm_rank / bwn;

  float* a = (float *)malloc(sizeof(float) * bheight * NDIM);
  float* b = (float *)malloc(sizeof(float) * NDIM * bwidth);
  float* c = (float *)malloc(sizeof(float) * bheight * bwidth);

  timer_start(2);
  init_matrix(a, bheight, NDIM, bh * bheight, 0, 0);
  init_matrix(b, NDIM, bwidth, 0, bw * bwidth, 0);
  init_matrix(c, bheight, bwidth, 0, 0, 1);
  timer_stop(2);

  matmul(a, b, c, bheight, NDIM, bwidth);

  timer_start(3);
  if (comm_rank != 0)
  {
    MPI_Send((void *)c, bheight * bwidth, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
  }
  else
  {
    MPI_Status status;
    float* tmpc = (float *)malloc(sizeof(float) * bheight * bwidth);
    
    for (int i = 0; i < bheight; i++)
      for (int j = 0; j < bwidth; j++)
        C[i][j] = c[i * bwidth + j];

    for (int src = 1; src < comm_size; src++)
    {
      MPI_Recv((void *)tmpc, bheight * bwidth, MPI_FLOAT, src, 0, MPI_COMM_WORLD, &status);
      int cw = src % bwn;
      int ch = src / bwn;
      for (int i = 0; i < bheight; i++)
      {
        for (int j = 0; j < bwidth; j++)
          C[ch * bheight + i][cw * bwidth + j] = tmpc[i * bwidth + j];
      }
    }
  }
  timer_stop(3);
}

/************************** DO NOT TOUCH BELOW HERE ******************************/

void check_mat_mul( float c[NDIM][NDIM] )
{
	int i, j, k;
	float sum;
	int validated = 1;

  float* a = (float *)malloc(sizeof(float) * NDIM * NDIM);
  float* b = (float *)malloc(sizeof(float) * NDIM * NDIM);

  init_matrix(a, NDIM, NDIM, 0, 0, 0);
  init_matrix(b, NDIM, NDIM, 0, 0, 0);

	printf("Validating the result..\n");
	
	// C = AB
	for( i = 0; i < NDIM; i+=10 )
	{
		for( j = 0; j < NDIM; j+=10 )
		{
			sum = 0;
			for( k = 0; k < NDIM; k++ )
			{
				sum += a[i*NDIM + k] * b[k*NDIM + j];
			}

			if( c[i][j] != sum )
			{
				printf("c[%d][%d] is differ(value=%lf correct_value=%lf)!!\n", i, j, c[i][j], sum );
				validated = 0;
			}
		}
	}

	printf("Validation : ");
	if( validated )
		printf("SUCCESSFUL.\n");
	else
		printf("FAILED.\n");
}

void print_mat( float mat[NDIM][NDIM] )
{
	int i, j;

	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			printf("%4.0lf ", mat[i][j]);
		}
		printf("\n");
	}
}

void print_help(const char* prog_name)
{
	printf("Usage: %s [-pvh]\n", prog_name );
	printf("\n");
	printf("OPTIONS\n");
	printf("  -p : print matrix data.\n");
	printf("  -v : validate matrix multiplication.\n");
	printf("  -h : print this page.\n");
}

void parse_opt(int argc, char** argv)
{
	int opt;

	while( (opt = getopt(argc, argv, "pvhikjs:")) != -1 )
	{
		switch(opt)
		{
		case 'p':
			// print matrix data.
			print_matrix = 1;
			break;

		case 'v':
			// validation
			validation = 1;
			break;

		case 'h':
		default:
			print_help(argv[0]);
			exit(0);
			break;
		}
	}
}

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	
  parse_opt( argc, argv );

  timer_init();
  timer_start(1);
  mpi_matmul();
	timer_stop(1);

  if (comm_rank == 0)
  {
    printf("Time elapsed : %lf sec\n", timer_read(1));
    printf("Matrix init : %lf sec\n", timer_read(2));
    printf("Communication : %lf sec\n", timer_read(3));

    if( validation )
      check_mat_mul( C );

    if( print_matrix )
    {
      /*
      float *a = (float *)malloc(sizeof(float) * NDIM * NDIM);
      init_matrix(a, NDIM, NDIM, 0, 0, 0);
      
      float A[NDIM][NDIM];

      for (int i=0; i<NDIM; i++)
        for (int j=0; j<NDIM; j++)
          A[i][j] = a[i*NDIM + j];

      printf("MATRIX A: \n");
      print_mat(A);

      printf("MATRIX B: \n");
      print_mat(A);
      */
      printf("MATRIX C: \n");
      print_mat(C);
    }
  }
  MPI_Finalize();

	return 0;
}
