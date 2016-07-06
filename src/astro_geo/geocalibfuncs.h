// ==========================================================================
// Header file for GEOCALIB namespace methods
// ==========================================================================
// Last modified on 4/19/07; 5/7/07; 6/8/07
// ==========================================================================

#ifndef GEOCALIB
#define GEOCALIB

#include <iostream>
#include "math/threevector.h"

namespace geocalibfunc
{
   void initialize_Boston_fit_tensor_params();
   void initialize_Baghdad_fit_tensor_params();
   void initialize_NYC_RTV_fit_tensor_params();
   void initialize_NYC_ALIRT_fit_tensor_params();
   void initialize_Lowell_RTV_fit_tensor_params();

   void compute_Boston_UTM(const threevector& p,threevector& UTM_coords);
   void compute_Boston_UTM(
      const twovector& X,double z,twovector& UTM_coords,double& z_new);
   void compute_transformed_UTM(
      const twovector& X,double z,twovector& UTM_coords,double& z_new);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // geocalibfunc namespace

#endif // GEOCALIB.h



