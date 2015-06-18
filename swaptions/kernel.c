#pragma OPENCL EXTENSION cl_amd_fp64 : enable

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

#define A0 2.50662823884
#define A1 -18.61500062529
#define A2 41.39119773534
#define A3 -25.44106049637

#define B0 -8.47351093090
#define B1 23.08336743743
#define B2 -21.06224101826
#define B3 3.13082909833

#define C0 0.3374754822726147
#define C1 0.9761690190917186
#define C2 0.1607979714918209
#define C3 0.0276438810333863
#define C4 0.0038405729373609
#define C5 0.0003951896511919
#define C6 0.0000321767881768
#define C7 0.0000002888167364
#define C8 0.0000003960315187

extern double RanUnif(long *s);
extern double CumNormalInv(double u); 

inline double RanUnif( long *s )
{
  long   ix, k1;
  double dRes;
  
  ix = *s;
  *s = ix+1;
  ix *= 1513517L;
  ix %= 2147483647L; 
  k1 = ix/127773L;
  ix = 16807L*( ix - k1*127773L ) - k1 * 2836L;
  if (ix < 0) ix = ix + 2147483647L;
  dRes = (ix * 4.656612875e-10);
  return (dRes);
} 

inline double CumNormalInv(double u)
{
  double x, r;
  x = u - 0.5;
  if( fabs (x) < 0.42 )
  { 
    r = x * x;
    r = x * ((( A3*r + A2) * r + A1) * r + A0)/
          ((((B3 * r+ B2) * r + B1) * r + B0) * r + 1.0);
    return (r);
  }
  r = u;
  if( x > 0.0 ) r = 1.0 - u;
  r = log(-log(r));
  r = C0 + r * (C1 + r * 
       (C2 + r * (C3 + r * 
       (C4 + r * (C5 + r * (C6 + r * (C7 + r*C8)))))));
  if( x < 0.0 ) r = -r;
  
  return (r);
}

__kernel void main(__constant double* ppdFactors,
                   __constant double* pdForward,
                   __constant double* pdTotalDrift,
                   __global double* pdSumResult, 
                   const long lTrials)
{
  int id = get_global_id(0);
  int gid = get_group_id(0);
  int lid = get_local_id(0);
  int gs = get_local_size(0);
  long chunk = (lTrials / (long)gs);
  long lRndSeed = RndSeed + (long)(lid*chunk*Factors*(N-1));
  double dStrike = (double)gid / (double)Swaptions;
  double sqrt_ddelt = sqrt(ddelt);
  double dStrikeCont = (Compounding == 0) // less than 1
                      ? dStrike 
                      : (1/Compounding) * log(1 + dStrike*Compounding); 
  double dSwapPayoffs = exp(dStrikeCont * PaymentInterval);
  
  double ppdHJMPath[N][N];
  double pdPayoffDiscountFactors[N];
  double pdDiscountingRatePath[N];
  
  double pdSwapPayoffs[iSwapVectorLength];
  double pdSwapRatePath[iSwapVectorLength]; // actual size : iSwapVectorLength
  double pdSwapDiscountFactors[iSwapVectorLength]; // same as above

  double dSumSimSwaptionPrice = 0.0;
  double dSumSquareSimSwaptionPrice = 0.0;
 
  double pdZ[Factors][N];
 
  //#pragma unroll
  for (int i = 0; i < iSwapVectorLength; i++)
    pdSwapPayoffs[i] = 0.0;
  //#pragma unroll
  for (int i = iFreqRatio; i <= iSwapTimePoints; i += iFreqRatio)
  {
    if (i != iSwapTimePoints) { pdSwapPayoffs[i] = dSwapPayoffs - 1; }
    else { pdSwapPayoffs[i] = dSwapPayoffs; }
  }
  
  // Simulations begin ***************************************************
  
  #pragma unroll
  for (long l = chunk*lid; l < chunk*(lid+1); l++)
  {
    #pragma unroll
    for (int j = 0; j < N; j++) { ppdHJMPath[0][j] = pdForward[j]; }

    #pragma unroll
    for (int i = 1; i < N; i++)
      #pragma unroll
      for (int j = 0; j < N; j++) 
        ppdHJMPath[i][j] = 0.0;
    
    #pragma unroll
    for (int i = 0; i < Factors; i++)
    {
      #pragma unroll
      for (int j = 1; j < N; j++)
      {
        pdZ[i][j] = CumNormalInv(RanUnif(&lRndSeed));
      }
    }
    #pragma unroll
    for (int j = 1; j < N; j++)
    {
      #pragma unroll
      for (int k = 0; k < N - j; k++)
      {
        double dTotalShock = 0;
        #pragma unroll
        for (int i = 0; i < Factors; i++) { dTotalShock += ppdFactors[i*(N-1) + k] * pdZ[i][j]; }
        double d = ppdHJMPath[j-1][k+1];
        d += pdTotalDrift[k]*ddelt;
        d += sqrt_ddelt * dTotalShock;
        ppdHJMPath[j][k] = d;
      }
    }
    #pragma unroll
    for (int i = 0; i < N; i++) { pdDiscountingRatePath[i] = ppdHJMPath[i][0]; }
    #pragma unroll
    for (int i = 0; i < N; i++) { pdPayoffDiscountFactors[i] = 1.0; }
    
    double d = 1.0;
    #pragma unroll
    for (int i = 1; i < N; i++)
    {
      d *= exp(-pdDiscountingRatePath[i-1] * ddelt);
      pdPayoffDiscountFactors[i] = d;
    }

    #pragma unroll
    for (int i = 0; i < iSwapVectorLength; i++) { pdSwapRatePath[i] = ppdHJMPath[iSwapStartTimeIndex][i]; }
    #pragma unroll
    for (int i = 0; i < iSwapVectorLength; i++) { pdSwapDiscountFactors[i] = 1.0; }

    d = 1.0;
    #pragma unroll
    for (int i = 1; i < iSwapVectorLength; i++)
    {
      d *= exp(-pdSwapRatePath[i-1]*dSwapVectorFactor);
      pdSwapDiscountFactors[i] = d;
    }

    double dFixedLegValue = 0.0;
    #pragma unroll
    for (int i = 0; i < iSwapVectorLength; i++) { dFixedLegValue += pdSwapPayoffs[i] * pdSwapDiscountFactors[i]; }
    double dDiscSwaptionPayoff = fmax(dFixedLegValue - 1.0, 0) * pdPayoffDiscountFactors[iSwapStartTimeIndex];
    dSumSimSwaptionPrice += dDiscSwaptionPayoff;
    dSumSquareSimSwaptionPrice += dDiscSwaptionPayoff * dDiscSwaptionPayoff;

  }
  // *********************************************************************
  
  pdSumResult[id*2] = dSumSimSwaptionPrice;
  pdSumResult[id*2+1] = dSumSquareSimSwaptionPrice;

}

