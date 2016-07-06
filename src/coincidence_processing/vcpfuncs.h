// =========================================================================
// Header file for stand-alone VCP functions
// =========================================================================
// Last modified on 1/26/05; 11/9/11
// =========================================================================

#ifndef VCPFUNCS_H
#define VCPFUNCS_H

#include <iostream>
#include <string>
#include "datastructures/Hashtable.h"
#include "math/threevector.h"

class VolumetricCoincidenceProcessor;

namespace vcpfunc
{
   extern double filtered_z_ground;

// Image processing methods:
   
   void compute_neighboring_voxels_distance_weights(double weight[]);
   bool coincidence_process_integrated_image(
      int pulse_number,const threevector& r_hat,
      VolumetricCoincidenceProcessor* vcp_ptr);

   void clean_integrated_image(
      int pulse_number,const threevector& r_hat,
      VolumetricCoincidenceProcessor* vcp_ptr);
   void renormalize_integrated_image_probs(
      const threevector& r_hat,VolumetricCoincidenceProcessor* vcp_ptr);

} // vcpfunc namespace

#endif // coincidence_processing/vcp/vcpfuncs.h




