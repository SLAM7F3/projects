// ====================================================================
// Subroutine POWELL minimizes a function func of n variables.  Input
// consists of an initial starting point p[1..n], an initial matrix
// xi[1..n][1..n] whose columns contain the initial set of directions
// (usually the n unit vectors), and ftol, the fractional tolerance in
// the function value such that failure to decrease by more than this
// amount on one iteration signals completion.  On output, p is set to
// the best point found, xi is the then-current direction set, fret is
// the returned function value at p, and iter is the number of
// iterations taken.  The routine LINMIN is used by POWELL.
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   
// ---------------------------------------------------------------------
// Subroutine init_xi_matrix sets xi equal to the identity matrix:

   void init_xi_matrix(int ndims,double **xi)
      {

         
         for (int i=1; i<=ndims; i++)
         {
            for (int j=1; j<=ndims; j++)
            {
               if (i==j) 
               {
                  xi[i][j]=1;
               }
               else
               {
                  xi[i][j]=0;
               }
            }
         }
      }

// ---------------------------------------------------------------------
   void powell(double p[], double **xi, int n, double ftol, int *iter, 
               double *fret, double (*func)(double [])) 
      { 
         const int ITMAX=200;
         void linmin(double p[], double xi[], int n, double *fret, 
                     double (*func)(double [])); 
         int i,ibig,j; 
         double del,fp,fptt,t,*pt,*ptt,*xit; 
 
         pt=vector(1,n); 
         ptt=vector(1,n); 
         xit=vector(1,n); 
         *fret=(*func)(p); 
         for (j=1;j<=n;j++) pt[j]=p[j]; 
         for (*iter=1;;++(*iter)) 
         { 
            fp=(*fret); 
            ibig=0; 
            del=0.0; 
            for (i=1;i<=n;i++) 
            { 
               for (j=1;j<=n;j++) xit[j]=xi[j][i]; 
               fptt=(*fret); 
               linmin(p,xit,n,fret,func); 
               if (fabs(fptt-(*fret)) > del) 
               { 
                  del=fabs(fptt-(*fret)); 
                  ibig=i; 
               } 
            } 
            if (2.0*fabs(fp-(*fret)) <= ftol*(fabs(fp)+fabs(*fret))) 
            { 
               free_vector(xit,1,n); 
               free_vector(ptt,1,n); 
               free_vector(pt,1,n); 
               return; 
            } 
            if (*iter == ITMAX) 
               nrerror((char *) "powell exceeding maximum iterations."); 
            for (j=1;j<=n;j++) 
            { 
               ptt[j]=2.0*p[j]-pt[j]; 
               xit[j]=p[j]-pt[j]; 
               pt[j]=p[j]; 
            } 
            fptt=(*func)(ptt); 
            if (fptt < fp) 
            { 
               t=2.0*(fp-2.0*(*fret)+fptt)*SQR(fp-(*fret)-del)-
                  del*SQR(fp-fptt); 
               if (t < 0.0) 
               { 
                  linmin(p,xit,n,fret,func); 
                  for (j=1;j<=n;j++) 
                  { 
                     xi[j][ibig]=xi[j][n]; 
                     xi[j][n]=xit[j]; 
                  } 
               } 
            } 
         } 
      } 
} // numrec namespace

#undef NRANSI 





