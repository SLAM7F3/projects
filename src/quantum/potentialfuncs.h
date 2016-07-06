// =========================================================================
// Header file for stand-alone potential functions.
// =========================================================================
// Last modified on 6/15/03
// =========================================================================

#ifndef POTENTIALFUNCS_H
#define POTENTIALFUNCS_H

#include <string>

namespace potentialfunc
{
   enum Potential_type
   {
      box,harmonic_osc,lambda_phi_4,doublewell,mathieu,
      freeparticle,ramp,inverted_parabola,aperiodic_cosine,
      smooth_step,smooth_box,squid,QFP
   };

   std::string get_potential_str(const Potential_type potential_type);
   void evaluate_current_potential_params(
      const Potential_type potential_type,
      double t1,double t2,double t,double potential_param[]);
   void fluctuate_potential_params(
      const Potential_type potential_type,long& seed,
      double potential_param[]);
   void plot_potential_param_time_dependence(
      const Potential_type potential_type,double tmin,double tmax,
      double t1,double t2,std::string imagedir,double potential_param[]);
   double set_xshift(const Potential_type potential_type);

   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double& V,double& dV,double& d2V);
   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double& V,double& dV,double& d2V,double& d3V,double& d4V,
      double& Kconst,bool compute_high_derivs=true);
   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double y,double xshift,double yshift,
      double& V,double& dVdx,double& dVdy,double& laplacianV,
      double& doublelaplacianV,double& Kconst,double& K12);
}

#endif // potentialfuncs.h



