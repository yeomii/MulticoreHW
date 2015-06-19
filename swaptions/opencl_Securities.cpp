#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

#include "opencl_util.h"
#include "timers.h"
#include "HJM_type.h"

#ifdef MPI
#include "mpi.h"
#endif

#define MAX_DEV 16
#define N 11
#define Factors 3
#define Swaptions 128
#define Years 5.5
#define Compounding 0.0
#define Maturity 1.0
#define Tenor 2.0
#define PaymentInterval 1.0

#define ddelt 0.5             // (double) (Years / N)
#define iSwapVectorLength 9   // (int)    (N - Maturity/ddelt + 0.5)
#define iSwapStartTimeIndex 2 // (int)    (Maturity/ddelt + 0.5)
#define iSwapTimePoints 4     // (int)    (Tenor/ddelt + 0.5)
#define dSwapVectorYears 4.5  // (double) (iSwapVectorLength * ddelt)
#define dSwapVectorFactor 0.5 // (double) (dSwapVectorYears / iSwapVectorLength)
#define iFreqRatio 2          // (int)    (PaymentInterval/ddelt + 0.5)

#ifdef CPU
int use_gpu = 0;
int nd = 1;
const char* device_name = "CPU";
#else
int use_gpu = 1;
int nd = 4;
const char* device_name = "GPU";
#endif

int nGroupSize = 64;
int nSwaptions = Swaptions;
int nGroupPerSwaption = 1; 
long lTrials = DEFAULT_NUM_TRIALS;

FTYPE *pdSwaptionPrice;
FTYPE *pdYield;
FTYPE *ppdFactors;
FTYPE factors[Factors][N-1];
FTYPE pdForward[N];
FTYPE pdTotalDrift[N-1];

void parsing(int argc, char *argv[])
{

  if(argc == 1)
  {
    fprintf(stderr," usage: \n\t-ns [number of swaptions]\n\t-sm [number of simulations]\n\t-gs [group size]\n"); 
    exit(1);
  }

  for (int j=1; j<argc; j++) {
    if (!strcmp("-sm", argv[j])) { lTrials = atoi(argv[++j]); }
    else if (!strcmp("-gs", argv[j])) { nGroupSize = atoi(argv[++j]); } 
    else if (!strcmp("-ns", argv[j])) { nSwaptions = atoi(argv[++j]); } 
    else if (!strcmp("-ps", argv[j])) { nGroupPerSwaption = atoi(argv[++j]); } 
    else {
      fprintf(stderr," usage: \n\t-ns [number of swaptions] \n\t-sm [number of simulations]\n\t-gs [group size]\n"); 
    }
  }
}

void init_factors()
{
  // initialize input dataset
  //the three rows store vol data for the three factors
  factors[0][0]= .01;
  factors[0][1]= .01;
  factors[0][2]= .01;
  factors[0][3]= .01;
  factors[0][4]= .01;
  factors[0][5]= .01;
  factors[0][6]= .01;
  factors[0][7]= .01;
  factors[0][8]= .01;
  factors[0][9]= .01;

  factors[1][0]= .009048;
  factors[1][1]= .008187;
  factors[1][2]= .007408;
  factors[1][3]= .006703;
  factors[1][4]= .006065;
  factors[1][5]= .005488;
  factors[1][6]= .004966;
  factors[1][7]= .004493;
  factors[1][8]= .004066;
  factors[1][9]= .003679;

  factors[2][0]= .001000;
  factors[2][1]= .000750;
  factors[2][2]= .000500;
  factors[2][3]= .000250;
  factors[2][4]= .000000;
  factors[2][5]= -.000250;
  factors[2][6]= -.000500;
  factors[2][7]= -.000750;
  factors[2][8]= -.001000;
  factors[2][9]= -.001250;
}

void init_swaptions()
{

  pdYield = (FTYPE *)malloc(sizeof(FTYPE) * N);
  pdYield[0] = .1;
  for(int j = 1; j < N; ++j)
    pdYield[j] = pdYield[j-1]+.005;

  ppdFactors = (FTYPE *)malloc(sizeof(FTYPE) * Factors * (N-1));
  for(int i = 0; i < Factors; ++i)
    for(int j = 0 ; j < N-1; ++j)
      ppdFactors[i*(N-1) + j] = factors[i][j];
}

void init_other_factors()
{
  double dPrev = pdForward[0] = pdYield[0];
  for (int i = 1; i < N; i++)
  {
    double dNext = (i+1)*pdYield[i];
    pdForward[i] = dNext - dPrev;
    dPrev = dNext;
  }

  double ppdDrifts[Factors][N-1];
  double dTemp = 0.5 * ddelt;
  for (int i = 0; i < Factors; i++)
    ppdDrifts[i][0] = dTemp * ppdFactors[i*(N-1)] * ppdFactors[i*(N-1)];

  for (int i = 0; i < Factors; i++)
  {
    for (int j = 1; j < N-1; j++)
    {
      ppdDrifts[i][j] = 0;
      for (int k = 0; k < j; k++) { ppdDrifts[i][j] -= ppdDrifts[i][k]; }
      double dSumVol = 0;
      for (int k = 0; k <= j; k++) { dSumVol += ppdFactors[i*(N-1) + k]; }
      ppdDrifts[i][j] += dTemp * dSumVol * dSumVol;
    }
  }
  for (int i = 0; i < N-1; i++)
  {
    pdTotalDrift[i] = 0;
    for (int j = 0; j < Factors; j++) { pdTotalDrift[i] += ppdDrifts[j][i]; }
  }
}

//Please note: Whenever we type-cast to (int), we add 0.5 to ensure that the value is rounded to the correct number. 
//For instance, if X/Y = 0.999 then (int) (X/Y) will equal 0 and not 1 (as (int) rounds down).
//Adding 0.5 ensures that this does not happen. Therefore we use (int) (X/Y + 0.5); instead of (int) (X/Y);

int main(int argc, char *argv[])
{
  int res;
  parsing(argc, argv);
  int blockSize = nGroupSize * nGroupPerSwaption;
 
  // ********* MPI **********************************

  int begin[MAX_DEV] = { 0, 0, 0, 0 };
  int swaptionChunk = Swaptions; // per process

#ifdef SNUCL
  nd = 4;
#endif
#ifdef MPI
  int comm_size = 0, comm_rank = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  swaptionChunk /= comm_size; 
  for (int i=0; i<nd; i++)
    begin[i] = comm_rank * swaptionChunk + i * swaptionChunk / nd;
#else
  for (int i=0; i<nd; i++)
    begin[i] = (swaptionChunk / nd) * i;
#endif

  if(begin[0] == 0)
  {
    printf("PARSEC Benchmark Suite\n");
    printf("Number of Simulations: %d, Group Size: %d Number of swaptions: %d Number of Groups per Swaption : %d device: %s\n", 
      lTrials, nGroupSize, nSwaptions, nGroupPerSwaption, device_name);
    fflush(NULL);
  }
 // ***********************************************
 
  const size_t global[1] = { swaptionChunk/nd * blockSize };
  const size_t local[1] = { nGroupSize };
  
  init_timers();
  start_timer(0);
  
  init_factors();
  init_swaptions();


  // ********* Calling OpenCL Kernel ****************
  
  start_timer(1);
  cl_platform_id platform;
  cl_device_id device[MAX_DEV];
  cl_context context;
  cl_command_queue queue[MAX_DEV];
  
  clGetPlatformIDs(1, &platform, NULL);
  if (use_gpu != 0) { clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, nd, device, NULL); }
  else { clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, nd, device, NULL); }
  context = clCreateContext(NULL, nd, device, NULL, NULL, NULL);
  for (int i=0; i<nd; i++) { queue[i] = clCreateCommandQueue(context, device[i], 0, NULL); }

  cl_program program;
  cl_kernel kernel;
  init_kernel("kernel.c", "main", &program, &kernel, &context, device, nd);
  
  int resultSize = (swaptionChunk / nd) * 2 * blockSize;
  cl_mem memFactors = init_buffer(&context, 1, sizeof(FTYPE) * Factors * (N-1), NULL);
  cl_mem memForward = init_buffer(&context, 1, sizeof(FTYPE) * N, NULL);
  cl_mem memTotalDrifts = init_buffer(&context, 1, sizeof(FTYPE) * (N-1), NULL);
  cl_mem memSumResult[MAX_DEV];
  for (int i=0; i<nd; i++) 
    memSumResult[i] = init_buffer(&context, 0, sizeof(FTYPE) * resultSize, NULL);
  
  init_other_factors();

#ifdef SNUCL
  for(int i=0; i<nd; i++)
  {
    write_buffer(&(queue[i]), &memFactors, sizeof(FTYPE)*Factors*(N-1), ppdFactors);
    write_buffer(&(queue[i]), &memForward, sizeof(FTYPE)*N, pdForward);
    write_buffer(&(queue[i]), &memTotalDrifts, sizeof(FTYPE)*(N-1), pdTotalDrift);
  }
  for (int i=0; i<nd; i++) clFinish(queue[i]);
#else
  write_buffer(&(queue[0]), &memFactors, sizeof(FTYPE)*Factors*(N-1), ppdFactors);
  write_buffer(&(queue[0]), &memForward, sizeof(FTYPE)*N, pdForward);
  write_buffer(&(queue[0]), &memTotalDrifts, sizeof(FTYPE)*(N-1), pdTotalDrift);
#endif

  set_kernel_arg(&kernel, 0, sizeof(cl_mem), (void *) &memFactors);
  set_kernel_arg(&kernel, 1, sizeof(cl_mem), (void *) &memForward);
  set_kernel_arg(&kernel, 2, sizeof(cl_mem), (void *) &memTotalDrifts);
  set_kernel_arg(&kernel, 4, sizeof(long), (void *)(&lTrials));
  set_kernel_arg(&kernel, 6, sizeof(int), (void *)(&nGroupPerSwaption));
  
  for (int i=0; i<nd; i++)
  {
    set_kernel_arg(&kernel, 3, sizeof(cl_mem), (void *) &(memSumResult[i]));
    set_kernel_arg(&kernel, 5, sizeof(int), (void *)&(begin[i]));
    res = clEnqueueNDRangeKernel(queue[i], kernel, 1, NULL, global, local, 0, NULL, NULL);
    check("clEnqueueNDRangeKernel", res, NULL, NULL);
#ifdef SNUCL
    clFinish(queue[i]);
#endif
  }

  double *pdSumResult = (double *)malloc(sizeof(FTYPE) * resultSize * nd);
  for (int i=0; i<nd; i++)
    read_buffer(&(queue[i]), &(memSumResult[i]), sizeof(FTYPE) * resultSize, (pdSumResult + i*resultSize));
  for (int i=0; i<nd; i++) clFinish(queue[i]);
  
  stop_timer(1);

  //**********************************************************
  
  pdSwaptionPrice = (FTYPE *)malloc(sizeof(FTYPE) * swaptionChunk * 2); 
  
  for (int i = 0; i < swaptionChunk;i++)
  {
    double dSum = 0, dSumSquare = 0;
    for (int j = 0; j < blockSize; j++)
    {
      dSum += pdSumResult[i*2*blockSize + j*2];
      dSumSquare += pdSumResult[i*2*blockSize + j*2 + 1];
    }
    pdSwaptionPrice[i*2] = dSum / lTrials;
    pdSwaptionPrice[i*2 + 1] = sqrt(fabs((dSumSquare-dSum*dSum/lTrials)/(lTrials - 1.0)))/sqrt((double)lTrials);
  }

  //**********************************************************

#ifdef MPI

  if (comm_rank != 0)
  {
    MPI_Send((void *)pdSwaptionPrice, swaptionChunk * 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }
  else
  {
    for (int i = 0; i < swaptionChunk; i++) {
      fprintf(stderr,"Swaption%d: [SwaptionPrice: %.10lf StdError: %.10lf] \n", 
        i, pdSwaptionPrice[i*2], pdSwaptionPrice[i*2+1]);
    }
    for (int r = 1; r < comm_size; r++)
    {
      MPI_Recv((void *)pdSwaptionPrice, swaptionChunk * 2, MPI_DOUBLE, r, 0, MPI_COMM_WORLD, NULL);

      for (int i = 0; i < swaptionChunk; i++) {
        fprintf(stderr,"Swaption%d: [SwaptionPrice: %.10lf StdError: %.10lf] \n", 
            i + r*swaptionChunk, pdSwaptionPrice[i*2], pdSwaptionPrice[i*2+1]);
      }
    }
    stop_timer(0);
    printf("Elapsed Time : %f, kernel Time : %f\n", read_timer(0), read_timer(1));
  }
  MPI_Finalize();

#else

  for (int i = 0; i < swaptionChunk; i++) {
    fprintf(stderr,"Swaption%d: [SwaptionPrice: %.10lf StdError: %.10lf] \n", 
        i, pdSwaptionPrice[i*2], pdSwaptionPrice[i*2+1]);
  }

  stop_timer(0);
  printf("Elapsed Time : %f, kernel Time : %f\n", read_timer(0), read_timer(1));

#endif

  //***********************************************************

  return 0;
}
