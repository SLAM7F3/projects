// ==================================================================
// Minimal random number generator of Park and Miller with Bays-Durham
// shuffle and added safeguards.  Returns a uniform random deviate
// between 0.0 and 1.0 (exclusive of the endpoint values).  Call with
// idum a POINTER to a negative integer to initialize.  Thereafter, do
// not alter idum between successive deviates in a sequence.  RNMX
// should approximate the largest doubleing value that is less than 1.
// ==================================================================
// Last modified on 8/12/03
// ==================================================================

namespace numrec
{
   double ran1(long *idum)
      {
         const int IA=16807;
         const int IM=2147483647;
         const double AM=1.0/IM;
         const int IQ=127773;
         const int IR=2836;
         const int NTAB=32;
         const int NDIV= (1+(IM-1)/NTAB);
         const double EPS=1.2E-7;
         const double RNMX=1.0-EPS;
         
         int j;
         long k;
         static long iy=0;
         static long iv[NTAB];
         double temp;

         if (*idum <= 0 || !iy) {
            if (-(*idum) < 1) *idum=1;
            else *idum = -(*idum);
            for (j=NTAB+7;j>=0;j--) {
               k=(*idum)/IQ;
               *idum=IA*(*idum-k*IQ)-IR*k;
               if (*idum < 0) *idum += IM;
               if (j < NTAB) iv[j] = *idum;
            }
            iy=iv[0];
         }
         k=(*idum)/IQ;
         *idum=IA*(*idum-k*IQ)-IR*k;
         if (*idum < 0) *idum += IM;
         j=iy/NDIV;
         iy=iv[j];
         iv[j] = *idum;
         if ((temp=AM*iy) > RNMX) return RNMX;
         else return temp;
      }

} // numrec namespace 



