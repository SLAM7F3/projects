// =========================================================================
// Header file for stand-alone image compositing functions.
// =========================================================================
// Last modified on 11/17/03; 5/20/09
// =========================================================================

#ifndef COMPOSITEFUNCS_H
#define COMPOSITEFUNCS_H

#include "math/constants.h"
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace compositefunc
{

// Subsampling and compositing methods:

   void regrid_twoDarray(
      double new_xhi,double new_xlo,double new_yhi,double new_ylo,
      twoDarray const *ztwoDarray_ptr,twoDarray* znew_twoDarray_ptr,
      int percentile_sentinel=-1);
   double average_pixels(
      double x,double y,double x_extent,double y_extent,
      twoDarray const *ztwoDarray_ptr);
   double percentile_pixel_value(
      double x,double y,double x_extent,double y_extent,
      twoDarray const *ztwoDarray_ptr,int percentile_sentinel=-1);

   void extremal_subsample_twoDarray(
      twoDarray const *ztwoDarray_ptr,twoDarray* znew_twoDarray_ptr,
      int extremal_sentinel=2);
   double extremal_pixel_value(
      int px,int py,int m,int n,
      twoDarray const *ztwoDarray_ptr,int extremal_sentinel=2);

   twoDarray* downsample(
      int nxbins_regrid,int nybins_regrid,twoDarray const *ztwoDarray_ptr,
      int percentile_sentinel=-1);
   void downsampled_pixel_numbers(
      double min_pixel_length,int& nxbins_regrid,int& nybins_regrid,
      twoDarray const *ztwoDarray_ptr);
   twoDarray* downsample_to_specified_resolution(
      double min_pixel_length,twoDarray const *ztwoDarray_ptr,
      int percentile_sentinel=-1);
   void high_pass_composite(
      twoDarray const *ztwoDarray1_ptr,twoDarray *ztwoDarray2_ptr,
      twoDarray* zmax_twoDarray_ptr,int percentile_sentinel=-1);
   void low_pass_composite(
      double w1,twoDarray const *ztwoDarray1_ptr,
      double w2,twoDarray *ztwoDarray2_ptr,
      twoDarray* zavg_twoDarray_ptr,int percentile_sentinel=-1);
   void restore_supersampled_from_subsampled_twoDarray(
      double zmin,twoDarray const *zsubsampled_twoDarray_ptr,
      twoDarray *ztwoDarray_orig_ptr,twoDarray *ztwoDarray_ptr);
   void average_identically_sized_twoDarrays(
      double w1,twoDarray const *ztwoDarray1_ptr,
      double w2,twoDarray const *ztwoDarray2_ptr,
      twoDarray* zavg_twoDarray_ptr,double null_value);
   void combine_identically_sized_twoDarrays_in_quadrature(
      twoDarray const *ztwoDarray1_ptr,
      twoDarray const *ztwoDarray2_ptr,twoDarray* zquadrature_twoDarray_ptr,
      double null_value=NEGATIVEINFINITY);
}

#endif // compositefuncs.h



