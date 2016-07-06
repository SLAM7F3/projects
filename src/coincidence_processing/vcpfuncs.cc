// ==========================================================================
// VCPFUNCS stand-alone methods
// ==========================================================================
// Last modified on 8/17/05; 11/9/11
// ==========================================================================

#include <set>
#include <vector>
#include "math/basic_math.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "coincidence_processing/vcpfuncs.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"
#include "coincidence_processing/voxel_coords.h"

using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace vcpfunc
{
   bool first_image_flag = true;
   double first_z_ground_peak=0;
   double filtered_z_ground=0;
   double prev_filtered_z_ground=0;

// ==========================================================================
// image processing methods
// ========================================================================== 

// Method compute_neighboring_voxels_distance_weights() generates a 1D
// array of 27 weights whose integral equals unity.  Each weight is
// proportional to 1/(1+d) where d denotes the 3-distance from some
// voxel to a central voxel.  

   void compute_neighboring_voxels_distance_weights(double weight[])
      {
         int counter=0;
         for (int i=-1; i<=1; i++)
         {
            for (int j=-1; j<=1; j++)
            {
               for (int k=-1; k<=1; k++)
               {
                  double d=sqrt(double(sqr(i)+sqr(j)+sqr(k)));
                  weight[counter++]=1.0/(1+d);
               } // loop over index k
            } // loop over index j
         } // loop over index i

         double weight_sum=0;
         for (int counter=0; counter<27; counter++)
         {
            weight_sum += weight[counter];
         }
         for (int counter=0; counter<27; counter++)
         {
            weight[counter] /= weight_sum;
         }
      }

// ---------------------------------------------------------------------
// Method coincidence_process_integrated_image is a high-level
// function which consolidates together calls to image cleaning,
// ground plane finding, Group 93 coincidence processing ("PCP") and
// probability conversion methods below.

   bool coincidence_process_integrated_image(
      int pulse_number,const threevector& r_hat,
      VolumetricCoincidenceProcessor* vcp_ptr)
      {
         cout << "Coincidence processing image " << vcp_ptr->imagenumber+1 
              << endl;

         clean_integrated_image(pulse_number,r_hat,vcp_ptr);
//          filter_integrated_image(r_hat,vcp_ptr);
         vcp_ptr->convert_counts_to_log_probs();
//         renormalize_integrated_image_probs(r_hat,vcp_ptr);

	 return true;
      }

// ---------------------------------------------------------------------
// Method clean_integrated_image collects together 3 functions which
// attempt to remove noise from an integrated image.  It first deletes
// voxels whose count levels fall below some specified threshold.  It
// next accumulates persistent hot voxel information within a
// specialized hashtable member of *vcp_ptr.  Finally, this method
// excises dark counts lying near the periphery of the conic Risley
// field-of-view.

   void clean_integrated_image(
      int pulse_number,const threevector& r_hat,
      VolumetricCoincidenceProcessor* vcp_ptr)
      {
         const int counts_threshold=2;
         vcp_ptr->delete_small_count_voxels(counts_threshold);
//         vcp_ptr->delete_small_count_voxels(jigparam::counts_threshold);
      }

} // vcpfunc namespace








