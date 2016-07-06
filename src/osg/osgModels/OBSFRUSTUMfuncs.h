// ==========================================================================
// Header file for OBSFRUSTUMFUNCS namespace
// ==========================================================================
// Last modified on 11/5/10; 11/7/10; 4/12/11
// ==========================================================================

#ifndef OBSFRUSTUMFUNCS_H
#define OBSFRUSTUMFUNCS_H

#include <vector>
#include "math/threevector.h"
#include "image/TwoDarray.h"

namespace OBSFRUSTUMfunc
{
   void convert_FOVs_to_alpha_beta_angles(
      double horiz_FOV,double vert_FOV,double& alpha,double& beta);
   double Bfunc(double A,double cos_horiz_FOV);
   double Ffunc(double A,double cos_horiz_FOV,double cos_vert_FOV);

   void convert_alpha_beta_angles_to_FOVs(
      double alpha,double beta,double& horiz_FOV,double& vert_FOV) ;

   bool compute_corner_rays(
      const threevector& v_hat,
      double horiz_FOV,double vert_FOV,double roll,double pitch);
   bool compute_corner_rays(
      double alpha,double beta,double roll,double pitch,
      const threevector& v_hat,std::vector<threevector>& ray_corner);

} // OBSFRUSTUMfunc namespace

#endif // OBSFRUSTUMfuncs.h



