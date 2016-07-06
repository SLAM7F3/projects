// ==========================================================================
// DISPLAYFUNCS stand-alone methods
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include <iostream>
#include "image/displayfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;

namespace displayfunc
{
  
// ---------------------------------------------------------------------
// Method interlace_images builds up an interlaced image by taking the
// first pixel from *ztwoDarray1_ptr, the 2nd from *ztwoDarray2_ptr,
// the 3rd from *ztwoDarray1_ptr, etc.

   void interlace_images(
      double z_null,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      twoDarray *ztwoDarray_interlace_ptr)
      {
         int k=0;

         for (unsigned int i=0; i<ztwoDarray1_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<ztwoDarray1_ptr->get_ndim(); j++)
            {
               if (nearly_equal(ztwoDarray1_ptr->get(i,j),z_null) && 
                   ztwoDarray2_ptr->get(i,j) > z_null)
               {
                  ztwoDarray_interlace_ptr->put(
                     i,j,ztwoDarray2_ptr->get(i,j));
               }
               else if (ztwoDarray1_ptr->get(i,j) > z_null && 
                        nearly_equal(ztwoDarray2_ptr->get(i,j),z_null))
               {
                  ztwoDarray_interlace_ptr->put(
                     i,j,ztwoDarray1_ptr->get(i,j));
               }
               else
               {
                  if (is_odd(k++))
                  {
                     ztwoDarray_interlace_ptr->put(i,j,ztwoDarray1_ptr->
                                                   get(i,j));
                  }
                  else
                  {
                     ztwoDarray_interlace_ptr->put(i,j,ztwoDarray2_ptr->
                                                   get(i,j));
                  }
               }
            } // i loop
         } // j loop
      }

// ---------------------------------------------------------------------
// Method twoD_gaussian_peak adds a 2D gaussian function centered at
// (x0,y0) with standard deviation sigma to output image
// *ftwoDarray_ptr.  The gaussian values vary from 0 to 1.  Final
// output values are restricted to not exceed 1.0

   void twoD_gaussian_peak(
      double x0,double y0,double sigma,twoDarray *ftwoDarray_ptr)
      {
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double x,y;
               ftwoDarray_ptr->pixel_to_point(px,py,x,y);
               double arg=(sqr(x-x0)+sqr(y-y0))/sqr(sigma);
               double gaussian=exp(-arg);
               double fnew=basic_math::min(
                  1.0,ftwoDarray_ptr->get(px,py)+gaussian);
               ftwoDarray_ptr->put(px,py,fnew);
            } // loop over py index
         } // loop over px index
          
      }

}
