// ====================================================================
// Subroutine AMOEBA performs a multidimensional minimization of the
// function funk(x) where x[1..ndim] is a vector in ndim dimensions,
// using the downhill simplex method of Nelder and Mead.  The matrix
// p[1..ndim+1][1..ndim] is input.  Its ndim+1 rows are
// ndim-dimensional vectors which are the vertices of the starting
// simplex.  Also input is the vector y[1..ndim+1] whose components
// must be preinitialized to the values of funk evaluated at the
// ndim+1 vertices (rows) of p.  Input parameter ftol is the
// fractional convergence tolerance to be achieved in the function
// value.  On output, p and y will have been reset to ndim+1 new
// points all within ftol of a minimum function value, and nfunk gives
// the number of function evaluations taken.
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   
#define SWAP(a,b) {swap=(a);(a)=(b);(b)=swap;} 
 
   void amoeba(double **p, double y[], int ndim, double ftol, 
               double (*funk)(double []), int *nfunk) 
      { 
         const int NMAX=50000;
         double amotry(double **p, double y[], double psum[], int ndim, 
                       double (*funk)(double []), int ihi, double fac); 
         int i,ihi,ilo,inhi,j,mpts=ndim+1; 
         double rtol,sum,swap,ysave,ytry,*psum; 
 
         psum=vector(1,ndim); 
         *nfunk=0; 

         for (j=1;j<=ndim;j++) 
         {
            for (sum=0.0,i=1;i<=mpts;i++) sum += p[i][j];
            psum[j]=sum;
         } 

         for (;;) 
         { 
            ilo=1; 
            ihi = y[1]>y[2] ? (inhi=2,1) : (inhi=1,2); 
            for (i=1;i<=mpts;i++) 
            { 
               if (y[i] <= y[ilo]) ilo=i; 
               if (y[i] > y[ihi]) 
               { 
                  inhi=ihi; 
                  ihi=i; 
               }
               else if (y[i] > y[inhi] && i != ihi) inhi=i; 
            } 
            rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo])); 
            if (rtol < ftol) 
            { 
               SWAP(y[1],y[ilo]) 
                  for (i=1;i<=ndim;i++) SWAP(p[1][i],p[ilo][i]) 
                                           break; 
            } 
//        printf("nfunk = %d  NMAX = %d \n",*nfunk,NMAX);
        
            if (*nfunk >= NMAX) nrerror( (char *) "NMAX exceeded"); 
            *nfunk += 2; 
            ytry=amotry(p,y,psum,ndim,funk,ihi,-1.0); 
            if (ytry <= y[ilo]) 
               ytry=amotry(p,y,psum,ndim,funk,ihi,2.0); 
            else if (ytry >= y[inhi]) 
            { 
               ysave=y[ihi]; 
               ytry=amotry(p,y,psum,ndim,funk,ihi,0.5); 
               if (ytry >= ysave) 
               { 
                  for (i=1;i<=mpts;i++) 
                  { 
                     if (i != ilo) 
                     { 
                        for (j=1;j<=ndim;j++) 
                           p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]); 
                        y[i]=(*funk)(psum); 
                     } 
                  } 
                  *nfunk += ndim; 
                  for (j=1;j<=ndim;j++) 
                  {
                     for (sum=0.0,i=1;i<=mpts;i++) sum += p[i][j];
                     psum[j]=sum;
                  } 
               } 
            }
            else --(*nfunk); 
         } 
         free_vector(psum,1,ndim); 
      } 
#undef SWAP 
#undef NRANSI 

// ====================================================================
// Subroutine AMOTRY extrapolates by a factor fac through the face of
// the simplex across from the high point, tries it, and replaces the
// high point if the new point is better.
// ====================================================================
// Last modified on 12/26/00
// ====================================================================

#define NRANSI 
#include "numrec/nrutil.h" 
 
   double amotry(double **p, double y[], double psum[], int ndim, 
                 double (*funk)(double []), int ihi, double fac) 
      { 
         int j; 
         double fac1,fac2,ytry,*ptry; 
 
         ptry=vector(1,ndim); 
         fac1=(1.0-fac)/ndim; 
         fac2=fac1-fac; 
         for (j=1;j<=ndim;j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2; 
         ytry=(*funk)(ptry); 
         if (ytry < y[ihi]) 
         { 
            y[ihi]=ytry; 
            for (j=1;j<=ndim;j++) 
            { 
               psum[j] += ptry[j]-p[ihi][j]; 
               p[ihi][j]=ptry[j]; 
            } 
         } 
         free_vector(ptry,1,ndim); 
         return ytry; 
      } 

} // numrec namespace

#undef NRANSI 



