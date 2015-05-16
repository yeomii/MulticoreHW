#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "timers.h"

#define NDIM    2048
#define MIN(a, b) (((a)<(b)) ? (a) : (b))
#define MAX(a, b) (((a)>(b)) ? (a) : (b))

float a[NDIM][NDIM];
float b[NDIM][NDIM];
float c[NDIM][NDIM];

int print_matrix = 0;
int validation = 0;
int B = 0;
int Column = 1;
int Row = 1;

struct arg_struct {
	int n;
	int start_i;
	int start_j;
	int end_i;
	int end_j;
};

void *t_mat_mul( void *arguments ){
	struct arg_struct *args = arguments;
	int i, j, k, ii, jj, kk;
	
	//printf ("%dth thread ... i : %d to %d, j : %d to %d\n", args->n, args->start_i, args->end_i, args->start_j, args->end_j);

	for ( k = 0; k < NDIM; k++ )
		for ( i = args->start_i; i < args->end_i; i++)
			for ( j = args->start_j; j < args->end_j; j++)
				c[i][j] += a[i][k] * b[k][j];
	//printf("%dth thread finished\n", args->n);
}

void mat_mul( float c[NDIM][NDIM], float a[NDIM][NDIM], float b[NDIM][NDIM] )
{
	int thread_n = Column * Row;
	pthread_t *p_threads = malloc(thread_n * sizeof(pthread_t));
	struct arg_struct *args = malloc(thread_n * sizeof(struct arg_struct));
	int *status = malloc(thread_n * sizeof(int));
	int i, thr_id;
	int csize = NDIM / Column, rsize = NDIM / Row;
	
	for (i = 0; i < thread_n; i++){
		int r = i / Column, c = i % Column;
		args[i].n = i;
		args[i].start_i = rsize * r;
		args[i].start_j = csize * c;
		args[i].end_i = (r + 1 != Row) ? rsize * (r + 1) : NDIM;
		args[i].end_j = (c + 1 != Column) ? csize * (c + 1) : NDIM;

		thr_id = pthread_create(&p_threads[i], NULL, &t_mat_mul, (void *)&args[i]);
		if (thr_id < 0)
		{
			perror("thread create error : ");
			exit(0);
		} 
	}

	for (i = 0; i < thread_n; i++)
		pthread_join (p_threads[i], (void **) &status[i]);
}

/************************** DO NOT TOUCH BELOW HERE ******************************/

void check_mat_mul( float c[NDIM][NDIM], float a[NDIM][NDIM], float b[NDIM][NDIM] )
{
	int i, j, k;
	float sum;
	int validated = 1;

	printf("Validating the result..\n");
	
	// C = AB
	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			sum = 0;
			for( k = 0; k < NDIM; k++ )
			{
				sum += a[i][k] * b[k][j];
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
			printf("%8.2lf ", mat[i][j]);
		}
		printf("\n");
	}
}

void print_help(const char* prog_name)
{
	printf("Usage: %s [-pvh] [-c num] [-r num]\n", prog_name );
	printf("\n");
	printf("OPTIONS\n");
	printf("  -p : print matrix data.\n");
	printf("  -v : validate matrix multiplication.\n");
	printf("  -h : print this page.\n");
	printf("  -[c, r] num : number of columns or rows in parallel calculation \n");
}

void parse_opt(int argc, char** argv)
{
	int opt;

	while( (opt = getopt(argc, argv, "pvhikjs:c:r:")) != -1 )
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

		case 'c':
			// column division
			Column = atoi(optarg);
			break;

		case 'r':
			// row division
			Row = atoi(optarg);
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
	int i, j, k = 1;

	parse_opt( argc, argv );

	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			a[i][j] = k;
			b[i][j] = k;
			k++;
		}
	}

	timer_start(1);
	mat_mul( c, a, b );
	timer_stop(1);

	printf("Time elapsed : %lf sec\n", timer_read(1));

	if( validation )
		check_mat_mul( c, a, b );

	if( print_matrix )
	{
		printf("MATRIX A: \n");
		print_mat(a);

		printf("MATRIX B: \n");
		print_mat(b);

		printf("MATRIX C: \n");
		print_mat(c);
	}

	return 0;
}
