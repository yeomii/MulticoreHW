#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timers.h"

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
  r = (double)log(-log(r));
  r = C0 + r * (C1 + r * 
       (C2 + r * (C3 + r * 
       (C4 + r * (C5 + r * (C6 + r * (C7 + r*C8)))))));
  if( x < 0.0 ) r = -r;
  
  return (r);
}

void swaption(double* ppdFactors,
              double* pdForward,
              double* pdTotalDrift,
              double* pdSumResult,
              long lTrials,
              int id,
              int gid,
              int gs)
{
  long chunk = (lTrials / (long) gs);
  long lRndSeed = 100 + (long)(gid * chunk * Factors * (N-1));
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

  double sqrt_ddelt = sqrt(ddelt);
  double ppdHJMPath[N][N];
  double pdPayoffDiscountFactors[N];
  double pdDiscountingRatePath[N];
  
  double pdSwapRatePath[iSwapVectorLength]; 
  double pdSwapDiscountFactors[iSwapVectorLength]; 

  double dSumSimSwaptionPrice = 0.0;
  double dSumSquareSimSwaptionPrice = 0.0;
 
  double pdZ[Factors][N];
 
  //printf("%d Swaption\n", id);

  // Simulations begin ***************************************************
  
  for (long l = 0; l < lTrials; l++)
  {
    for (int j = 0; j < N; j++) { ppdHJMPath[0][j] = pdForward[j]; }

    for (int i = 1; i < N; i++)
      for (int j = 0; j < N; j++)
        ppdHJMPath[i][j] = 0.0;
    
    start_timer(1);
    for (int i = 0; i < Factors; i++)
    {
      for (int j = 1; j < N; j++)
      {
        double rand = RanUnif(&lRndSeed);
        pdZ[i][j] = CumNormalInv(rand);
      }
    }
    stop_timer(1);
    start_timer(2);
    for (int j = 1; j < N; j++)
    {
      for (int k = 0; k < N - j; k++)
      {
        double dTotalShock = 0;
        for (int i = 0; i < Factors; i++) { dTotalShock += ppdFactors[i*(N-1) + k] * pdZ[i][j]; }
        double d = ppdHJMPath[j-1][k+1];
        d += pdTotalDrift[k]*ddelt;
        d += sqrt_ddelt * dTotalShock;
        ppdHJMPath[j][k] = d;
      }
    }
    stop_timer(2);

    start_timer(3);
    for (int i = 0; i < N; i++) { pdDiscountingRatePath[i] = ppdHJMPath[i][0]; }
    for (int i = 0; i < N; i++) { pdPayoffDiscountFactors[i] = 1.0; }
    
    double d = 1.0;
    for (int i = 1; i < N; i++)
    {
      d *= exp(-pdDiscountingRatePath[i-1] * ddelt);
      pdPayoffDiscountFactors[i] = d;
    }
    
    for (int i = 0; i < iSwapVectorLength; i++) { 
      pdSwapRatePath[i] = ppdHJMPath[iSwapStartTimeIndex][i]; }
    for (int i = 0; i < iSwapVectorLength; i++) { pdSwapDiscountFactors[i] = 1.0; }

    d = 1.0;
    for (int i = 1; i < iSwapVectorLength; i++)
    {
      d *= exp(-pdSwapRatePath[i-1]*dSwapVectorFactor);
      pdSwapDiscountFactors[i] = d;
    }
    stop_timer(3);
    double dFixedLegValue = 0.0;
    for (int i = 0; i < iSwapVectorLength; i++) { 
      dFixedLegValue += pdSwapPayoffs[i] * pdSwapDiscountFactors[i]; 
    }
    double dSwaptionPayoff = fmax(dFixedLegValue - 1.0, 0);
    double dDiscSwaptionPayoff = dSwaptionPayoff * pdPayoffDiscountFactors[iSwapStartTimeIndex];
    dSumSimSwaptionPrice += dDiscSwaptionPayoff;
    dSumSquareSimSwaptionPrice += dDiscSwaptionPayoff * dDiscSwaptionPayoff;

  }
  // *********************************************************************
  pdSwaptionPrice[id*2] = dSumSimSwaptionPrice/lTrials; // MeanPrice
  pdSwaptionPrice[id*2 + 1] = sqrt(fabs((dSumSquareSimSwaptionPrice-dSumSimSwaptionPrice*dSumSimSwaptionPrice/lTrials)/(lTrials-1.0)))/
                              sqrt((double)lTrials); // StdError

}


int main (int argc, char* argv[])
{
  double factors[Factors][N-1];
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
  
  long lTrials = 1000000;
  for (int j=1; j<argc; j++) {
    if (!strcmp("-sm", argv[j])) { lTrials = atoi(argv[++j]); }
    else {
      fprintf(stderr," usage: \n\t-ns [number of swaptions] \n\t-sm [number of simulations]\n\t-gs [group size]\n"); 
    }
  }

  double *pdSwaptionPrice = (double *)malloc(sizeof(double) * Swaptions * 2); 

  double *pdYield = (double *)malloc(sizeof(double) * N);
  pdYield[0] = .1;
  for(int j = 1; j < N; ++j)
    pdYield[j] = pdYield[j-1]+.005;

  double *ppdFactors = (double *)malloc(sizeof(double) * Factors * (N-1));
  for(int i = 0; i < Factors; ++i)
    for(int j = 0 ; j < N-1; ++j)
      ppdFactors[i*(N-1) + j] = factors[i][j];
 
  double pdForward[N];
  double dPrev = pdForward[0] = pdYield[0];
  for (int i = 1; i < N; i++)
  {
    double dNext = (i+1)*pdYield[i];
    pdForward[i] = dNext - dPrev;
    dPrev = dNext;
  }

  double pdTotalDrift[N-1];
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

  init_timers();
  for (int i = 0; i < Swaptions; i++)
  {
    swaption(ppdFactors, pdForward, pdTotalDrift, pdSwaptionPrice, lTrials, i);
    printf("Swaption%d: [SwaptionPrice: %.10lf StdError: %.10lf] \n", 
        i, pdSwaptionPrice[i*2], pdSwaptionPrice[i*2+1]);
  }

  printf("%f %f %f", read_timer(1), read_timer(2), read_timer(3));
  return 0;
}
