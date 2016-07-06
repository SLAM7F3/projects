// =========================================================================
// Header file for stand-alone display functions.
// =========================================================================
// Last modified on 7/6/04
// =========================================================================

#ifndef DISPLAYFUNCS_H
#define DISPLAYFUNCS_H

template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace displayfunc
{
   void interlace_images(
      double z_null,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      twoDarray *ztwoDarray_interlace_ptr);

   void twoD_gaussian_peak(
      double x0,double y0,double sigma,twoDarray *ftwoDarray_ptr);
}

#endif // displayfuncs.h




