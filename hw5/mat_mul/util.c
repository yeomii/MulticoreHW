#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "util.h"
#include "timers.h"

extern int print_matrix;
extern int validation;
extern int Column;
extern int Row;
extern int N;
extern int global_size;
extern int work_group_size;
extern int use_gpu;

void check_mat_mul( float* a, float* b, float* c, int n, int ts)
{
  int i, j, k;
  float sum;
  int validated = 1;

  printf("Validating the result..\n");
  
  // C = AB
  for( i = 0; i < ts; i++ )
  {
    for( j = 0; j < ts; j++ )
    {
      sum = 0;
      for( k = 0; k < n; k++ )
      {
        sum += a[i*n + k] * b[k*ts + j];
      }

      if( c[i*ts + j] != sum )
      {
        printf("c[%d][%d] is differ(value=%.1lf correct_value=%.1lf)!!\n", i, j, c[i*ts + j], sum );
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

void print_mat( float* mat, int r, int c)
{
  int i, j;

  for( i = 0; i < r; i++ )
  {
    for( j = 0; j < c; j++ )
    {
      printf("%2.1lf ", mat[i*c + j]);
    }
    printf("\n");
  }
}

void print_help(const char* prog_name)
{
  printf("Usage: %s [-pvh] [-n num] [-c num] [-r num]\n", prog_name );
  printf("\n");
  printf("OPTIONS\n");
  printf("  -p : print matrix data.\n");
  printf("  -v : validate matrix multiplication.\n");
  printf("  -h : print this page.\n");
  printf("  -c : use cpu device. (default is gpu device) \n");
  printf("  -n num : size of matrix \n");
  printf("  -g num : limit size of global work items \n");
  printf("  -w num : work group size \n");
}

void parse_opt(int argc, char** argv)
{
  int opt;

  while( (opt = getopt(argc, argv, "pvhikjcs:n:g:w:")) != -1 )
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

    case 'n':
      N = atoi(optarg);
      break;

    case 'g':
      global_size = atoi(optarg);
      if (global_size > N)
        global_size = N;
      break;

    case 'w':
      work_group_size = atoi(optarg);
      if (work_group_size > global_size)
        work_group_size = (global_size > 16) ? 16 : global_size;
      break;

    case 'c':
      use_gpu = 0;
      break;

    case 'h':
    default:
      print_help(argv[0]);
      exit(0);
      break;
    }
  }
}

void gen_tile(float **mat, int r, int c, int si, int sj, int ei, int ej)
{
  int minr = r < ei - si ? r : ei - si;
  int minc = c < ej - sj ? c : ej - sj;
  for (int i = 0; i < r; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      if (i < minr && j < minc)
        (*mat)[i*c + j] = si + i;
      else
        (*mat)[i*c + j] = 0;
    }
  }
}

void init_matrix(float** a, float** b, float** c, int n)
{
  int size = sizeof(float) * n * n;
  *a = (float *) malloc(size);
  *b = (float *) malloc(size);
  *c = (float *) malloc(size);
  
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      int idx = i*n + j;
      (*a)[idx] = i;
      (*b)[idx] = j;
      (*c)[idx] = 0;
    }
  }
}

void print_result(int n)
{
  printf("N is %d\n", n);
  printf("global_size is %d\n", global_size);
  printf("work_group_size is %d\n", work_group_size);
  printf("device is %s\n", (use_gpu != 0 ? "gpu" : "cpu"));

  printf("Time elapsed : %lf sec\n", timer_read(1));

  printf("A, B write buffer time elapsed : %lf sec\n", timer_read(2));
  printf("kernel time elapsed : %lf sec\n", timer_read(3));
  printf("C read buffer time elapsed : %lf sec\n", timer_read(4));
  printf("gen A, B time elapsed : %lf sec\n", timer_read(5));
/*
  if( validation )
    check_mat_mul( a, b, c, n );

  if( print_matrix )
  {
    printf("MATRIX A: \n");
    print_mat(a, n);

    printf("MATRIX B: \n");
    print_mat(b, n);
    
    printf("MATRIX C: \n");
    print_mat(c, n);
  }
*/
}
