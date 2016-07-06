// ==========================================================================
// Program RANK is a playground for testing the iterative "row-column"
// method of R.F.C. Guerreiro and P.M.Q Aguiar for recovering missing
// elements in rank deficient matrices.  See "Estimation of Rank
// Deficient Matrices from Partial Observations: Two-Step Iterative
// Algorithms" by these authors.
// ==========================================================================
// Last updated on 2/20/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "numerical/euler.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/plane.h"
#include "structmotion/reconstruction.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/twoDarray.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
//   nrfunc::init_time_based_seed();

//   int mdim=2*5;
   int mdim=2*10;
//   int ndim=11;
   int ndim=8;
//   int ndim=10;
//   int mdim=8;
//   int ndim=5;

// Generate a starting random matrix.  Then diminish its rank to
// simulate a perfect measurement matrix:

   genmatrix Worig(mdim,ndim);
   for (int m=0; m<mdim; m++)
   {
      for (int n=0; n<ndim; n++)
      {
         Worig.put(m,n,10*(nrfunc::ran1()-0.5));
      }
   }

   int rank=3;
   genmatrix Wreduced(mdim,ndim);
   Worig.rank_approximation(rank,Wreduced);

   cout << "Wreduced = " << Wreduced << endl;
   cout << "rank(Wreduced) = " << Wreduced.rank() << endl;   

// Introduce noise into rank reduced matrix:

//   double noise_mag=0.0;
   double noise_mag=0.2;
//   double noise_mag=0.5;

   double max_noise=NEGATIVEINFINITY;
   genmatrix Wnoise(mdim,ndim);
   for (int m=0; m<mdim; m++)
   {
      for (int n=0; n<ndim; n++)
      {
         double curr_noise=noise_mag*(nrfunc::ran1()-0.5);
         Wnoise.put(m,n,Wreduced.get(m,n)+curr_noise);
         max_noise=max(max_noise,fabs(curr_noise));
      }
   }

   cout << "Wnoise = " << Wnoise << endl;
   cout << "rank(Wnoise) = " << Wnoise.rank() << endl;   
   
// Eliminate some number of entries in noisy, rank-reduced matrix to
// create "realistic" measurement matrix V:

   int n_missing_data=0;
//   const double missing_frac=0.25;
//   const double missing_frac=0.33;
   const double missing_frac=0.5;
   genmatrix V(Wnoise);
   for (int m=0; m<mdim; m++)
   {
      for (int n=0; n<ndim; n++)
      {
         if (nrfunc::ran1() < missing_frac) 
         {
            V.put(m,n,NEGATIVEINFINITY);
            n_missing_data++;
         }
      }
   }
   cout << "mdim*ndim = " << mdim*ndim << endl;
   cout << "n_missing_data = " << n_missing_data << endl;
   cout << "missing data fraction = " << double(n_missing_data)/
      double(mdim*ndim) << endl << endl;

   cout << "Missing data measurement matrix V = " << V << endl;

// ==========================================================================
// Now try to recover the full, noiseless measurement matrix from
// noisy matrix V which has missing data entries:

   Reconstruction reconstruction;

   reconstruction.set_D_ptr(new genmatrix(mdim,ndim));
   reconstruction.set_missing_data_ptr(new genmatrix(mdim,ndim));
   *(reconstruction.get_D_ptr())=V;

   reconstruction.instantiate_RC_matrices();
   reconstruction.get_D_ptr()->generate_mask_matrix(
      NEGATIVEINFINITY,*(reconstruction.get_missing_data_ptr()) );

   genmatrix Mask( *(reconstruction.get_missing_data_ptr()) );

   genmatrix Wmeasurable(V.elementwise_product(Mask));
   genmatrix Wbest_estimate(mdim,ndim);

//   const int max_searches=10;
//   const int max_searches=100;
//   const int max_searches=300;
   const int max_searches=2000;
   double best_score=POSITIVEINFINITY;
   double best_measurable_score=POSITIVEINFINITY;
   double best_diff=POSITIVEINFINITY;
   double best_measurable_diff=POSITIVEINFINITY;
//   double max_reasonable_determinant=1E7;
   double max_reasonable_determinant=1E8;
//   double max_reasonable_determinant=1E9;
//   double max_reasonable_determinant=1E10;


//   const double tau=max_searches;
//   double t_norm=double(search_counter)/tau;
//   double missing_magnitude=1.0*exp(-t_norm);
   double missing_magnitude=1.0;

   for (int search_counter=0; search_counter< max_searches; search_counter++)
   {

      if (search_counter%10==0)
         cout << search_counter << " " << flush;

// Put random entries into missing data slots within measurement
// matrix W:

      reconstruction.randomly_initialize_missing_data(
         missing_magnitude,Wbest_estimate);

// Initial guess for column factor matrix A = rank approximation to W:

      reconstruction.initialize_column_factor_matrix();

//      int max_iters=150;
      int max_iters=500;
//      int max_iters=1000;
//      int max_iters=2000;

      reconstruction.estimate_RC_matrices(
         max_iters,max_reasonable_determinant);
      
      genmatrix W(reconstruction.get_AB());
      genmatrix Wmask(W.elementwise_product(Mask));
      double score=sqrt((Wreduced-W).sqrd_Frobenius_norm());
      double diff=(Wreduced-W).max_abs_element_value();

      double measurable_score=sqrt( 
         (Wmeasurable-Wmask).sqrd_Frobenius_norm());
      double measurable_diff=(Wmeasurable-Wmask).max_abs_element_value();
      double max_abs_element_diff=W.max_abs_element_difference();

      if (max_abs_element_diff < 10 && (
          measurable_diff < best_measurable_diff ||
          ( nearly_equal(measurable_diff,best_measurable_diff) &&
            measurable_score < best_measurable_score) ) )
//      if (diff < best_diff ||
//          ( nearly_equal(diff,best_diff) && score < best_score ) )
      {


         Wbest_estimate=W;

         best_measurable_diff=measurable_diff;
         best_measurable_score=measurable_score;

         best_diff=diff;
         best_score=sqrt((Wreduced-Wbest_estimate).sqrd_Frobenius_norm());

         const int n_iters=10000;
         reconstruction.refine_measurement_matrix(
            n_iters,Wbest_estimate,Wreduced);

//         max_reasonable_determinant *= 0.8;
         missing_magnitude *= 0.9;

         cout << endl;
         cout << " mag=" << missing_magnitude 
              << " score=" << score
              << " best score=" << best_score
              << " diff=" << diff 
              << " best diff=" << best_diff << endl;
         cout << " measurable score = " << measurable_score
              << " measurable diff = " << measurable_diff << endl << endl;

      } // improved estimate found conditional
      
   } // loop over search_counter index

   cout << "Wbest_estimate = " << Wbest_estimate << endl;
   cout << "Wreduced = " << Wreduced << endl;
   cout << "Wreduced-Wbest_estimate = " 
        << Wreduced-Wbest_estimate << endl;
   cout << "Mask = " << *(reconstruction.get_missing_data_ptr()) << endl;
   cout << "Max abs diff value = " 
        << (Wreduced-Wbest_estimate).max_abs_element_value() << endl;
   cout << "Max noise = " << max_noise << endl;
   cout << "Frobenius diff = " << best_score << endl;
}
