#pragma OPENCL EXTENSION cl_amd_fp64 : enable

#define Blocksize 16
#define N 11
#define Factors 3
#define Swaptions 128
#define Years 5.5
#define Compounding 0.0
#define Maturity 1.0
#define Tenor 2.0
#define PaymentInterval 1.0
#define RndSeed 100

#define ddelt 0.5             // (double) (Years / N)
#define iSwapVectorLength 9   // (int)    (N - Maturity/ddelt + 0.5)
#define iSwapStartTimeIndex 2 // (int)    (Maturity/ddelt + 0.5)
#define iSwapTimePoints 4     // (int)    (Tenor/ddelt + 0.5)
#define dSwapVectorYears 4.5  // (double) (iSwapVectorLength * ddelt)
#define dSwapVectorFactor 0.5 // (double) (dSwapVectorYears / iSwapVectorLength)
#define iFreqRatio 2          // (int)    (PaymentInterval/ddelt + 0.5)

__kernel void main(__global double * SwapPayoffs) // size N * Swaptions
{
  int id = get_global_id(0);
  double dStrike = (double)id / (double)Swaptions;

  double dStrikeCont = (Compounding == 0) // less than 1
                      ? dStrike 
                      : (1/Compounding) * log(1 + dStrike*Compounding); 

  double pdSwapPayoffs[iSwapVectorLength]; // actual size : iSwapVectorLength
  
  double dSwapPayoffs = exp(dStrikeCont * PaymentInterval);

  for (int i = 0; i < iSwapVectorLength; i++)
    pdSwapPayoffs[i] = 0.0;
  for (int i = iFreqRatio; i <= iSwapTimePoints; i += iFreqRatio)
  {
    if (i != iSwapTimePoints) { pdSwapPayoffs[i] = dSwapPayoffs - 1; }
    else { pdSwapPayoffs[i] = dSwapPayoffs; }
  }
  
  for (int i = 0; i < iSwapVectorLength; i++)
    SwapPayoffs[id*iSwapVectorLength + i] = pdSwapPayoffs[i];
}

