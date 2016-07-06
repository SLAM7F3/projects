// ==========================================================================
// Filter functions header file
// ==========================================================================
// Last updated on 6/19/08; 8/18/11; 7/1/13
// ==========================================================================

#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include "math/threevector.h"
class genmatrix;

namespace filterfunc
{

// Gaussian filter methods:

   void gaussian_filter(int nhbins,double dx,double sigma,double h[]);
   void gaussian_filter(double dx,double sigma,std::vector<double>& h);
   int gaussian_filter_size(double std_dev,double delta_x);
   int gaussian_filter_size(double std_dev,double delta_x,
                            double e_folding_distance);
   void gaussian_filter(
      int nsize,int deriv_order,double std_dev,double delta_x,
      double filter[]);
   std::vector<double> gaussian_filter(
      int nsize,int deriv_order,double std_dev,double delta_x);

// 2D Gaussian filter methods:

   genmatrix* gaussian_2D_filter(
      double dx,double sigma,double e_folding_distance);
   genmatrix* gaussian_2D_filter(double dx,double sigma,int nhbins);

// Brute force filter methods:

   void brute_force_filter(
      int nxbins,int nhbins,double x[],double h[],double y[],
      bool wrap_around_input_values=false);
   void brute_force_filter(
      const std::vector<double>& x,const std::vector<double>& h,
      std::vector<double>& y,bool wrap_around_input_values);
   void brute_force_filter(
      const std::vector<double>& x,const std::vector<bool>& xOK_flag,
      const std::vector<double>& h,std::vector<double>& y,
      bool wrap_around_input_values);
   void boxcar_filter(int nhbins,std::vector<double>& h);
   void boxcar_filter(int nhbins,double h[]);
   void circularly_shift_filter(
      int nhbins,double dx,double x_shift,const double h[],
      double h_shifted[]);
   void circularly_replicate_filter(
      int nhbins,double dx,int n_copies,double x_shift,
      double h[],double h_replicate[]);

// Alpha-beta-gamma filter methods:

   double alpha_filter(double curr_raw_x,double prev_filtered_x,double alpha);
   threevector alpha_filter(
      const threevector& curr_raw_r,const threevector& prev_filtered_r,
      double alpha);
   void alphabeta_filter(
      double curr_raw_x,double prev_filtered_x,double prev_filtered_xdot,
      double& curr_filtered_x,double& curr_filtered_xdot,
      double alpha,double beta,double dt);
   void alphabetagamma_filter(
      double curr_raw_x,double prev_filtered_x,double prev_filtered_xdot,
      double prev_filtered_xdotdot,double& curr_filtered_x,
      double& curr_filtered_xdot,double& curr_filtered_xdotdot,
      double alpha,double beta,double gamma,double dt);

// Savitzky-Golay filter methods:
      
   void savitzky_golay_filter
     (int nbins,double h_savitsky[],int deriv_order=0,int poly_order=4,
      int n_filter_points=16);
   void savitzky_smooth
     (int nbins,double func[],double filtered_func[],
      int deriv_order=0,int poly_order=4,int n_filter_points=16);

} // filterfunc namespace
 
#endif // filter.h
