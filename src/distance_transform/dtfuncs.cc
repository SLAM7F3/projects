// ==========================================================================
// DISTANCETRANSFORM stand-alone methods
// ==========================================================================
// Last modified on 8/9/12; 8/10/12; 9/19/12
// ==========================================================================

#include <iostream>
#include "distance_transform/dtfuncs.h"
#include "distance_transform/imconv.h"
#include "distance_transform/pnmfile.h"
#include "distance_transform/dt.h"
#include "general/stringfuncs.h"
#include "image/TwoDarray.h"

using std::cout;
using std::endl;


namespace dtfunc
{

// Method compute_distance_transform() takes in image *ptwoDarray_ptr
// as well as some threshold value.  It resets all entries in
// *ptwoDarray_ptr below the threshold to zero_byte and all entries above
// the threshold to one_byte.  This method then acts a simple wrapper
// for Felzenszwalb's distance transform codes.  It returns
// dynamically instantiated *qtwoDarray_ptr whose entries indicate
// pixel distance from the binary figure in *ptwoDarray_ptr.

// The entries in *qtwoDarray_ptr are renormalized values lying in
// [0,1].  They need to be multiplied by output parameter max_distance
// in order to convert them to pixel distances.

   twoDarray* compute_distance_transform(
      double threshold,const twoDarray* ptwoDarray_ptr,
      double& max_pixel_distance)
   {
      cout << "inside dtfunc::compute_distance_transform()" << endl;
      cout << "threshold = " << threshold << endl;

      int mdim=ptwoDarray_ptr->get_mdim();
      int ndim=ptwoDarray_ptr->get_ndim();

// Transfer ptwoDarray contents into Felzenszwalb's image object:

      image<uchar>* image_ptr = new image<uchar>(mdim,ndim);
      unsigned char zero_byte=stringfunc::ascii_integer_to_unsigned_char(0);
      unsigned char one_byte=stringfunc::ascii_integer_to_unsigned_char(1);

      int n_zeros=0;
      int n_ones=0;
      for (int py=0; py<ndim; py++)
      {
         for (int px=0; px<mdim; px++)
         {
            double curr_intensity=ptwoDarray_ptr->get(px,py);
            if (curr_intensity < threshold)
            {
               image_ptr->access[py][px]=zero_byte;
               n_zeros++;
            }
            else
            {
               image_ptr->access[py][px]=one_byte;
               n_ones++;
            }
         }
      }
      cout << "n_zeros = " << n_zeros << " n_ones = " << n_ones << endl;

// Call Felzenszwalb's squared-distance transform method:

      image<float>* distances_ptr = dt(image_ptr);
      delete image_ptr;

// Take square roots of squared-distances:

      for (int y = 0; y < distances_ptr->height(); y++) 
      {
         for (int x = 0; x < distances_ptr->width(); x++) 
         {
            imRef(distances_ptr, x, y) = sqrt(imRef(distances_ptr, x, y));
         }
      }

      max_pixel_distance=NEGATIVEINFINITY;
      twoDarray* qtwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      for (int py=0; py<ndim; py++)
      {
         for (int px=0; px<mdim; px++)
         {
            double curr_dist=distances_ptr->access[py][px];
            if (curr_dist > 0)
            {
//               cout << "px = " << px << " py = " << py
//                    << " curr_dist = " << curr_dist << endl;
            }
            qtwoDarray_ptr->put(px,py,curr_dist);
            max_pixel_distance=basic_math::max(max_pixel_distance,curr_dist);
         }
      }
      delete distances_ptr;

      cout << "max_pixel_distance = " << max_pixel_distance << endl;

// Renormalize all entries in *qtwoDarray_ptr s.t. they range from 0
// to 1:

      for (int py=0; py<ndim; py++)
      {
         for (int px=0; px<mdim; px++)
         {
            double curr_dist=qtwoDarray_ptr->get(px,py);
            double renorm_frac_dist=curr_dist/max_pixel_distance;
            qtwoDarray_ptr->put(px,py,renorm_frac_dist);
         }
      }
      
      return qtwoDarray_ptr;
   }
   
// --------------------------------------------------------------------------
// Method chamfer_matching_score() takes in two binary images
// *pbinary_twoDarray_ptr and *qbinary_twoDarray_ptr.  It computes and
// returns the average distance of each non-null entry in
// *qbinary_twoDarray_ptr to the closest non-null entry in
// *pbinary_twoDarray_ptr.

   double chamfer_matching_score(
      double threshold,
      const twoDarray* pbinary_twoDarray_ptr,
      const twoDarray* qbinary_twoDarray_ptr)
   {
//      cout << "inside dtfunc::chamfer_matching_score()" << endl;
      
      double max_pixel_distance;

      twoDarray* p_dt_twoDarray_ptr=compute_distance_transform(
         threshold,pbinary_twoDarray_ptr,max_pixel_distance);
      
      int mdim=pbinary_twoDarray_ptr->get_mdim();
      int ndim=pbinary_twoDarray_ptr->get_ndim();
      
      int q_counter=0;
      double chamfer_score=0;
      for (int px=0; px<mdim; px++)
      {
         for (int py=0; py<ndim; py++)
         {
            if (qbinary_twoDarray_ptr->get(px,py) < threshold) continue;
            chamfer_score += max_pixel_distance*p_dt_twoDarray_ptr->get(px,py);
            q_counter++;
         } // loop over py
      } // loop over px
//      cout << "q_counter = " << q_counter << endl;

      if (q_counter==0)
      {
         chamfer_score=POSITIVEINFINITY;
      }
      else
      {
         chamfer_score /= q_counter;
      }

      delete p_dt_twoDarray_ptr;
      return chamfer_score;
   }
   


} // dtfunc namespace

