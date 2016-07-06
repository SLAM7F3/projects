// ==========================================================================
// Compression math functions 
// ==========================================================================
// Last updated on 2/11/16; 2/12/16; 2/23/16
// ==========================================================================

#include <svdlib.h>

#include "math/compressfuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "numrec/nrfuncs.h"

using std::cout;
using std::endl;

namespace compressfunc
{

// Method generate_random_projection_matrix() forms a random
// projection matrix following the discussion in "Random projections
// in dimensionality reduction: Applications to image and text data"
// by Bingham and Mannila, 2001.

   void generate_random_projection_matrix(genmatrix& R)
   {
//      int d_reduced = R.get_mdim();
      int d_orig = R.get_ndim();
      R.clear_values();

      int n_ones, n_neg_ones, n_zeros;
      n_ones = n_neg_ones = n_zeros = 0;

      for(unsigned int py = 0; py < R.get_ndim(); py++)
      {
         for(unsigned int px = 0; px < R.get_mdim(); px++)
         {
            double curr_val = nrfunc::ran1();
            if(curr_val < 0.1666666)
            {
               R.put(px, py, 1);
               n_ones++;
            }
            else if (curr_val > 0.8333333)
            {
               R.put(px, py, -1);
               n_neg_ones++;
            }
            else
            {
               n_zeros++;
            }
            
         } // loop over px index
      } // loop over py index

      double prefactor = sqrt(3.0 / d_orig);
//      double prefactor = sqrt(3.0 * d_orig / d_reduced);
      R = prefactor * R;

// --> R * Rtranspose should be close to (d_reduced x d_reduced)
//     identity matrix

//      double frac_ones = double(n_ones)/(R.get_mdim() * R.get_ndim());
//      double frac_neg_ones = double(n_neg_ones)/(R.get_mdim() * R.get_ndim());
//      double frac_zeros = double(n_zeros)/(R.get_mdim() * R.get_ndim());

//      cout << "n_ones = " << n_ones << " n_neg_ones = " << n_neg_ones
//           << " n_zeros = " << n_zeros << endl;
//      cout << "frac_ones = " << frac_ones 
//           << " frac_neg_ones = " << frac_neg_ones
//           << " frac_zeros = " << frac_zeros << endl;
   }
   

} // compressfunc namespace




