// ================================================================
// Last updated on 7/14/03
// ================================================================

// In Nov 02, we encapsulated all Numerical Recipes methods which we
// use within a "numrec" namespace.  So we no longer rename NR's
// "vector" and "matrix" functions as "numrec_vector" and
// "numrec_matrix", for they are now both called via numrec::vector
// and numrec:matrix.

// CAUTION: This is the ANSI C (only) version of the Numerical Recipes
//   utility file numrec/nrutil.h.  Do not confuse this file with the same-named
//   file numrec/nrutil.h that is supplied in the 'misc' subdirectory.
//   *That* file is the one from the book, and contains both ANSI and
//   traditional K&R versions, along with #ifdef macros to select the
//   correct version.  *This* file contains only ANSI C.               

#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

namespace numrec
{
   
#define SQR(a) (a==0 ? 0 : a*a)

#define DSQR(a) (a==0 ? 0 : a*a)

#define DMAX(a,b) (a>b ? a : b)

#define DMIN(a,b) (a<b ? a : b)

#define FMAX(a,b) (a>b ? a : b)

#define FMIN(a,b) (a < b ? a : b)

#define LMAX(a,b) (a > b ? a : b)

#define LMIN(a,b) (a < b ? a : b)

#define IMAX(a,b) (a > b ? a : b)

#define IMIN(a,b) (a < b ? a : b)

#define SIGN(a,b) (b >= 0.0 ? fabs(a) : -fabs(a))

   void nrerror(char error_text[]);
   double *vector(long nl, long nh);
   int *ivector(long nl, long nh);
   unsigned char *cvector(long nl, long nh);
   unsigned long *lvector(long nl, long nh);
   double *dvector(long nl, long nh);
   double **matrix(long nrl, long nrh, long ncl, long nch);
   double **dmatrix(long nrl, long nrh, long ncl, long nch);
   int **imatrix(long nrl, long nrh, long ncl, long nch);
   double **submatrix(
      double **a, long oldrl, long oldrh, long oldcl, 
      long oldch,long newrl, long newcl);
   double **convert_matrix(
      double *a, long nrl, long nrh, long ncl, long nch);
   double ***f3tensor(long nrl, long nrh, long ncl, 
                      long nch, long ndl, long ndh);
   void free_vector(double *v, long nl, long nh);
   void free_ivector(int *v, long nl, long nh);
   void free_cvector(unsigned char *v, long nl, long nh);
   void free_lvector(unsigned long *v, long nl, long nh);
   void free_dvector(double *v, long nl, long nh);
   void free_matrix(double **m, long nrl, long nrh, long ncl, long nch);
   void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
   void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch);
   void free_submatrix(double **b, long nrl, long nrh,long ncl, long nch);
   void free_convert_matrix(double **b, long nrl, long nrh, long ncl, 
                            long nch);
   void free_f3tensor(double ***t, long nrl, long nrh,long ncl, long nch,
                      long ndl, long ndh);

} // numrec namespace

#endif /* _NR_UTILS_H_ */






