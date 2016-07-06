// ==========================================================================
// Some useful general functions for C++ programs which involve
// our complex class.
// ==========================================================================
// Last updated on 6/18/03
// ==========================================================================

#include "numrec/nr.h"
#include "general/genfuncs_complex.h"

// ==========================================================================
// Function definitions 
// ==========================================================================

double abs(complex z)
{
   return z.getmod();
}

// ---------------------------------------------------------------------
complex average_complex(complex A[], int Nsize)
{
   double realavg=0;
   double imagavg=0;
   for (int i=0; i<Nsize; i++)
   {
      realavg += A[i].getreal()/Nsize;
      imagavg += A[i].getimag()/Nsize;
   }
   complex complexavg=complex(realavg,imagavg);
   return complexavg;
}

// ---------------------------------------------------------------------
// Subroutine polycoeff recursively calculates the coefficients a^(n)_k
// within the polynomial expansion

// 	P(z) = Prod_(j=1)^n (f_j + g_j z) = sum_(k=0)^n a^(n)_k z^k

// in terms of the complex parameters f and g:

complex polycoeff(int n, int k, complex f[], complex g[])
{
   complex prod;

   if (k==0)
   {
      prod=1;
      for (int i=1; i<=n; i++)
      {
         prod=prod*f[i];
      }
      return prod;
   }
   else if (k==n)
   {
      prod=1;
      for (int i=1; i<=n; i++)
      {
         prod=prod*g[i];
      }
      return prod;
   }
   else
   {
      return f[n]*polycoeff(n-1,k,f,g)+g[n]*polycoeff(n-1,k-1,f,g);
   }
}

// ---------------------------------------------------------------------
complex* new_clear_carray(int nsize)
{
   complex *array_ptr=new complex[nsize];
   for (int i=0; i<nsize; i++) array_ptr[i]=complex(0,0);
   return array_ptr;
}

// ---------------------------------------------------------------------
// This overloaded complex version of routine simpsonsum returns
// Simpson's value for the sum of the elements within a complex array
// between some starting and stopping bin values.  In order to convert
// this sum into a true integral, the result must be multiplied by the
// value for a single abscissa bin's stepsize.  The first 7 finite
// range formulas come from section 25.4 in "Handbook of mathematical
// functions" by Abramowitz and Stegun.  Unfortunately, as of 1/20/00,
// we no longer recall where the extended "Simpson" rule which we use
// to integrate arrays with 8 or more bins comes from.  Boo hoo...

complex simpsonsum_complex(complex f[],int startbin,int stopbin)
{
   int nbins=stopbin-startbin+1;
   complex currsum;
   if (nbins==1)
   {
      currsum=0;
   }
   else if (nbins==2)
   {
      currsum=1./2.*(f[startbin]+f[startbin+1]);
   }
   else if (nbins==3)
   {
      currsum=1./3.*(f[startbin]+4*f[startbin+1]+f[startbin+2]);
   }
   else if (nbins==4)
   {
      currsum=3./8.*(f[startbin]+3*f[startbin+1]+3*f[startbin+2]
                     +f[startbin+3]);
   }
   else if (nbins==5)
   {
      currsum=2./45.*(7*f[startbin]+32*f[startbin+1]+12*f[startbin+2]
                      +32*f[startbin+3]+7*f[startbin+4]);
   }
   else if (nbins==6)
   {
      currsum=5./288.*(19*f[startbin]+75*f[startbin+1]+50*f[startbin+2]
                       +50*f[startbin+3]+75*f[startbin+4]+19*f[startbin+5]);
   }
   else if (nbins==7)
   {
      currsum=1./140.*(41*f[startbin]+216*f[startbin+1]+27*f[startbin+2]
                       +272*f[startbin+3]+27*f[startbin+4]+216*f[startbin+5]
                       +41*f[startbin+6]);
   }
   else
   {
      double c1=17./48.;
      double c2=59./48.;
      double c3=43./48.;
      double c4=49./48.;
    
      currsum  = c1*(f[startbin]+f[stopbin]);
      currsum += c2*(f[startbin+1]+f[stopbin-1]);
      currsum += c3*(f[startbin+2]+f[stopbin-2]);
      currsum += c4*(f[startbin+3]+f[stopbin-3]);
      for (int i=startbin+4; i<=stopbin-4; i++)
      {
         currsum += f[i];
      }
   }
   return(currsum);
}

// ---------------------------------------------------------------------
complex sqr(complex z)
{
   return z*z;
}

// ---------------------------------------------------------------------
complex variance_complex(complex A[], int Nsize)
{
   complex complexavg=average_complex(A,Nsize);
   complex realvar=0;
   complex imagvar=0;
   for (int i=0; i<Nsize; i++)
   {
      realvar=realvar+( sqr(A[i].getreal())-sqr(complexavg.getreal()) )/Nsize;
      imagvar=imagvar+( sqr(A[i].getimag())-sqr(complexavg.getimag()) )/Nsize;
   }
   complex complexvar(realvar.getreal(),imagvar.getreal());
   return complexvar;
}






