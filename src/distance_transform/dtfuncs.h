// =========================================================================
// Header file for stand-alone distance transform functions.
// =========================================================================
// Last modified on 8/9/12; 8/10/12; 9/19/12
// =========================================================================

#ifndef DTFUNCS_H
#define DTFUNCS_H

#include "image/TwoDarray.h"

namespace dtfunc
{
   twoDarray* compute_distance_transform(
      double threshold,const twoDarray* pbinary_twoDarray_ptr,
      double& max_pixel_distance);

   double chamfer_matching_score(
      double threshold,
      const twoDarray* pbinary_twoDarray_ptr,
      const twoDarray* qbinary_twoDarray_ptr);
   
}



#endif // dtfuncs.h



