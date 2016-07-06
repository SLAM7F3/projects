// ==================================================================
// Long period (> 2 x 10**18) random number generator of L'Ecuyer with
// Bays-Durham shuffle and added safeguards.  Returns a uniform random
// deviate between 0.0 and 1.0 (exlusive of the endpoint values).
// Call with idum a negative integer to initialize.  Thereafter, do
// not alter idum between successive deviates in a sequence.  RNMX
// should approximate the largeset floating value that is less than 1.
// ==================================================================	
// Last modified on 8/12/03
// ==================================================================	

namespace numrec
{
   double ran2(long *idum) 
      { 
         const int IM1=2147483563;
         const int IM2=2147483399;
         const double AM=1.0/IM1;
         const int IMM1=IM1-1;
         const int IA1=40014;
         const int IA2=40692;
         const int IQ1=53668;
         const int IQ2=52774;
         const int IR1=12211;
         const int IR2=3791;
         const int NTAB=32;
         const double NDIV=(1+IMM1/NTAB);
         const double EPS=1.2E-7;
         const double RNMX=1.0-EPS;

         int j; 
         long k; 
         static long idum2=123456789; 
         static long iy=0; 
         static long iv[NTAB]; 
         double temp; 
 
         if (*idum <= 0) 
         { 
            if (-(*idum) < 1) *idum=1; 
            else *idum = -(*idum); 
            idum2=(*idum); 
            for (j=NTAB+7;j>=0;j--) 
            { 
               k=(*idum)/IQ1; 
               *idum=IA1*(*idum-k*IQ1)-k*IR1; 
               if (*idum < 0) *idum += IM1; 
               if (j < NTAB) iv[j] = *idum; 
            } 
            iy=iv[0]; 
         } 
         k=(*idum)/IQ1; 
         *idum=IA1*(*idum-k*IQ1)-k*IR1; 
         if (*idum < 0) *idum += IM1; 
         k=idum2/IQ2; 
         idum2=IA2*(idum2-k*IQ2)-k*IR2; 
         if (idum2 < 0) idum2 += IM2; 
         j=iy/NDIV; 
         iy=iv[j]-idum2; 
         iv[j] = *idum; 
         if (iy < 1) iy += IMM1; 
         if ((temp=AM*iy) > RNMX) return RNMX; 
         else return temp; 
      } 
} // numrec namespace
