#include <CL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

#include "opencl_util.h"
#include "timers.h"
#include "HJM_type.h"

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

int use_gpu = 0;
int nGroupSize = 8;
int nSwaptions = Swaptions;
long lTrials = DEFAULT_NUM_TRIALS;

FTYPE *pdSwaptionPrice;
FTYPE *pdYield;
FTYPE *ppdFactors;
FTYPE factors[Factors][N-1];
FTYPE pdForward[N];
FTYPE pdSwapPayoffs[N*Swaptions];
FTYPE pdTotalDrift[N-1];

void parsing(int argc, char *argv[])
{
  printf("PARSEC Benchmark Suite\n");
  fflush(NULL);

  if(argc == 1)
  {
    fprintf(stderr," usage: \n\t-ns [number of swaptions]\n\t-sm [number of simulations]\n\t-gs [group size]\n"); 
    exit(1);
  }

  for (int j=1; j<argc; j++) {
    if (!strcmp("-sm", argv[j])) { lTrials = atoi(argv[++j]); }
    else if (!strcmp("-gs", argv[j])) { nGroupSize = atoi(argv[++j]); } 
    else if (!strcmp("-ns", argv[j])) { nSwaptions = atoi(argv[++j]); } 
    else {
      fprintf(stderr," usage: \n\t-ns [number of swaptions] \n\t-sm [number of simulations]\n\t-gs [group size]\n"); 
    }
  }
/*
  // temporarily groupsize equals number of swaptions 
  if (nGroupSize > 0)
    nGroupSize = nSwaptions;
*/
  printf("Number of Simulations: %d,  Group Size: %d Number of swaptions: %d\n", lTrials, nGroupSize, nSwaptions);
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
  pdSwaptionPrice = (FTYPE *)malloc(sizeof(FTYPE) * nSwaptions * 2); 

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
  
  const size_t global_a[1] = { nSwaptions };
  const size_t global_b[1] = { nSwaptions*nGroupSize };
  const size_t local[1] = { nGroupSize }; //{ nGroupSize };
  
  init_timers();
  start_timer(0);
  
  init_factors();
  init_swaptions();

  // **********Calling the Swaption Pricing Routine*****************

  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  init_host(&platform, &device, &context, &queue);

  cl_program program_a, program_b;
  cl_kernel kernel_a, kernel_b;
  init_kernel("kernel_a.c", "main", &program_a, &kernel_a, &context, &device);
  init_kernel("kernel_b.c", "main", &program_b, &kernel_b, &context, &device);
  
  cl_mem memFactors = init_buffer(&context, 1, sizeof(FTYPE) * Factors * (N-1), NULL);
  cl_mem memForward = init_buffer(&context, 1, sizeof(FTYPE) * N, NULL);
  cl_mem memTotalDrifts = init_buffer(&context, 1, sizeof(FTYPE) * (N-1), NULL);
  
  cl_mem memSumResult = init_buffer(&context, 0, sizeof(FTYPE) * Swaptions * 2  * nGroupSize, NULL);
  cl_mem memSwapPayoffs = init_buffer(&context, 0, sizeof(FTYPE) * Swaptions * iSwapVectorLength, NULL);
  cl_mem memSwaptionPrice = init_buffer(&context, 0, sizeof(FTYPE) * Swaptions * 2, NULL);
  
  res = clFinish(queue);
  check("finish write", res, NULL,NULL);

  set_kernel_arg(&kernel_a, 0, sizeof(cl_mem), (void *) &memSwapPayoffs);
  res = clEnqueueNDRangeKernel(queue, kernel_a, 1, NULL, global_a, local, 0, NULL, NULL);
  check("clEnqueueNDRangeKernel", res, NULL, NULL);
  
  init_other_factors();
  write_buffer(&queue, &memFactors, sizeof(FTYPE)*Factors*(N-1), ppdFactors);
  write_buffer(&queue, &memForward, sizeof(FTYPE)*N, pdForward);
  write_buffer(&queue, &memTotalDrifts, sizeof(FTYPE)*(N-1), pdTotalDrift);

  res = clFinish(queue);
  check("finish kernel_a, write buffer", res, NULL, NULL);

  read_buffer(&queue, &memSwapPayoffs, sizeof(FTYPE)*Swaptions*iSwapVectorLength, pdSwapPayoffs);
  
  set_kernel_arg(&kernel_b, 0, sizeof(cl_mem), (void *) &memFactors);
  set_kernel_arg(&kernel_b, 1, sizeof(cl_mem), (void *) &memForward);
  set_kernel_arg(&kernel_b, 2, sizeof(cl_mem), (void *) &memTotalDrifts);
  set_kernel_arg(&kernel_b, 3, sizeof(cl_mem), (void *) &memSwapPayoffs);
  set_kernel_arg(&kernel_b, 4, sizeof(cl_mem), (void *) &memSumResult);
  //set_kernel_arg(&kernel_b, 4, sizeof(cl_mem), (void *) &memSwaptionPrice);
  set_kernel_arg(&kernel_b, 5, sizeof(long), (void *)(&lTrials));
  
  res = clEnqueueNDRangeKernel(queue, kernel_b, 1, NULL, global_b, local, 0, NULL, NULL);
  check("clEnqueueNDRangeKernel", res, NULL, NULL);
  res = clFinish(queue);
  check("finish kernel_b", res, NULL,NULL);


  double *pdSumResult = (double *)malloc(sizeof(FTYPE)*Swaptions*2*nGroupSize);
  read_buffer(&queue, &memSumResult, sizeof(FTYPE)*Swaptions*2*nGroupSize, pdSumResult);
  //read_buffer(&queue, &memSwaptionPrice, sizeof(FTYPE)*Swaptions*2, pdSwaptionPrice);
  res = clFinish(queue);
  check("clFinish", res, NULL, NULL);

  //**********************************************************
  /*
  for (int i = 0; i < Swaptions; i++)
  {
    fprintf(stderr, "Swaption %d\n", i);
    for (int j = 0; j < iSwapVectorLength; j++)
      fprintf(stderr, "pdSwapPayoffs[%d] = %lf\n", j, pdSwapPayoffs[i*iSwapVectorLength + j]);
  }
*/
  for (int i = 0; i < Swaptions;i++)
  {
    double dSum = 0, dSumSquare = 0;
    for (int j = 0; j < nGroupSize; j++)
    {
      dSum += pdSumResult[i*2*nGroupSize + j*2];
      dSumSquare += pdSumResult[i*2*nGroupSize + j*2 + 1];
    }
    pdSwaptionPrice[i*2] = dSum / lTrials;
    pdSwaptionPrice[i*2 + 1] = sqrt(fabs((dSumSquare-dSum*dSum/lTrials)/(lTrials - 1.0)))/sqrt((double)lTrials);
  }
  for (int i = 0; i < nSwaptions; i++) {
    fprintf(stderr,"Swaption%d: [SwaptionPrice: %.10lf StdError: %.10lf] \n", 
        i, pdSwaptionPrice[i*2], pdSwaptionPrice[i*2+1]);
  }
  //***********************************************************

  stop_timer(0);
  printf("Elapsed Time : %f\n", read_timer(0));

  return 0;
}
