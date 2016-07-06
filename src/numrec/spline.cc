// =======================================================================
// Method spline
// =======================================================================
// Last updatedon 8/25/05
// =======================================================================

#define NRANSI 

#include <iostream>
#include "numrec/spline.h"
#include "numrec/nrutil.h" 

using std::cout;
using std::endl;

namespace numrec
{

// Given arrays x[1..n] and y[1..n] containing a tabulated function
// i.e. y_i=f(x_i), with x1 < x2 < ... < xN and given values yp1 and
// ypn for the first derivative of the interpolating function at
// points 1 and n, respectively, this method returns an array y2[1..n]
// that contains the second derivatives of the interpolating function
// at the tabulated points x_i.  If boolean parameter
// natural_spline_flag==true, the method is signaled to set the
// corresponding boundary condition for a natural spline with zero
// second derviative on that boundary.

   std::vector<double> y2;

   void init_spline_2nd_derivs(
      const std::vector<double>& x,const std::vector<double>& y)
      {
         const int n=x.size();

         double x_array[n],y_array[n],y2_array[n];
         for (int i=0; i<n; i++)
         {
            x_array[i]=x[i];
            y_array[i]=y[i];
         }
         init_spline_2nd_derivs(x_array-1,y_array-1,n,y2_array-1);
         
         y2.clear();
         for (int i=0; i<n; i++)
         {
            y2.push_back(y2_array[i]);
         }
      }

   void init_spline_2nd_derivs(double x[], double y[], int n, double y2[])
      {
         init_spline_2nd_derivs(x,y,n,0,0,y2,true);
      }

   void init_spline_2nd_derivs(
      double x[], double y[], int n, double yp1, 
      double ypn, double y2[], bool natural_spline_flag)
      {
         double* u=vector(1,n-1);

// The lower boundary condition is set either to be "natural" or else
// to have a specified first derviative:

         if (natural_spline_flag)
         {
            y2[1]=u[1]=0.0;
         }
         else
         {
            y2[1]=-0.5;
            u[1]=(3.0/(x[2]-x[1])) * ((y[2]-y[1])/(x[2]-x[1])-yp1);
         }
         
// This is the decomposition loop of the tridiagonal algorithm.  y2
// and u are used for temporary storage of the decomposed factors.

         for (int i=2; i<=n-1; i++)
         {
            double sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
            double p=sig*y2[i-1]+2.0;
            y2[i]=(sig-1.0)/p;
            u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
            u[i]=(6.0*u[i]/(x[i+1]-x[i-1]) - sig*u[i-1])/p;
         }

// The upper boundary condition is set either to be "natural" or else
// to have a specified first derivative:

         double qn,un;
         if (natural_spline_flag)
         {
            qn=un=0;
         }
         else
         {
            qn=0.5;
            un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
         }
         
         y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);

// Next line implements the back-substitution loop of the tridiagonal
// algorithm:

         for (int k=n-1; k>=1; k--)
         {
            y2[k]=y2[k]*y2[k+1]+u[k];
         }
         
         free_vector(u,1,n-1);
      }

// -----------------------------------------------------------------------
// Given arrays xa[1..n] and ya[1..n] which tabulate a function (with
// the xa's in order) and given the array y2a[1..n] which is the
// output from method spline, and given a value of x, this method
// returns a cubic-spline interpolated value y.

   double spline_interp(
      std::vector<double>& xa,std::vector<double>& ya,double x)
      {
         const int n=xa.size();
         double x_array[n],y_array[n],y2_array[n];
         for (int i=0; i<n; i++)
         {
            x_array[i]=xa[i];
            y_array[i]=ya[i];
            y2_array[i]=y2[i];
         }
         return spline_interp(x_array-1,y_array-1,y2_array-1,n,x);
      }

   double spline_interp(
      double xa[], double ya[], double y2a[], int n, double x)
      {

         void nrerror(char error_text[]);

// Find the right place in the table via bisection.  This is optimal
// if sequential calls to this method are at random values of x.  If
// sequential calls are in order and closely spaced, one would do
// better to store previous values of klo and khi and test if they
// remain appropriate on the next call.
         
         int klo=1;
         int khi=n;
         while (khi-klo > 1)
         {
            int k=(khi+klo) >> 1;
            if (xa[k] > x)
            {
               khi=k;
            }
            else
            {
               klo=k;
            }
         }

// klo and khi now bracket the input value of x

         double h=xa[khi]-xa[klo];

// The xa values must be distinct:

         if (h==0) nrerror((char *) "Bad xa input to routine splint");

         double a=(xa[khi]-x)/h;
         double b=(x-xa[klo])/h;

// Evaluate cubic spline polynomial:

         return a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])
            *(h*h)/6.0;
      }
   

} // numrec namespace

#undef NRANSI 





