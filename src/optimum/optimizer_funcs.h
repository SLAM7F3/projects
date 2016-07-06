// ==========================================================================
// Header file for stand-alone optimizer_func methods
// ==========================================================================
// Last updated on 12/11/08; 2/1/09; 4/6/09
// ==========================================================================

#ifndef OPTIMIZERFUNC_H
#define OPTIMIZERFUNC_H

#include <string>

#include "video/photograph.h"

class camera;
class FeaturesGroup;
class genmatrix;
class homography;
class optimizer;
class rotation;
class threevector;

namespace optimizer_func
{

// Set & get methods

   void set_photo_pair_bundle_adjust_flag(bool flag);
   void set_V(const threevector& V_input);
   double* get_homography_params_array();

// Minimization methods:

   void rosenbrock(double* p, double* x, int m, int n, void* data);
   void rosenbrock_jacobian(double* p, double* jac, int m, int n, void* data);
//   void score(double* input_param, double* x, int m, int n, void* data);

// Rotation homography bundle adjustment methods:

   void initialize_bundle_adjustment();
   void rotation_homography_bundle_adjustment(
      int n_previously_composited_photos,
      const std::vector<int>& indices_of_photos_to_be_composited,
      optimizer* o_ptr);
   void finalize_bundle_adjustment(int ret);
   void copy_params_in_to_buffer(int first_photo_index,int last_photo_index);
   void copy_params_in_to_buffer(const std::vector<int>& photo_indices);
   void copy_params_out_of_buffer(int first_photo_index,int last_photo_index);
   void copy_params_out_of_buffer(const std::vector<int>& photo_indices);

// Rotation homography decomposition methods:

   void symmetric_homography_decomposition_score(
      double* input_param, double* x, int m, int n, void* data);
   void single_pair_homography_decomposition_score(
      double* input_param, double* x, int m, int n, void* data);
   void set_camera_params(
      int p,double* input_param,rotation* R_ptr,rotation* Rinv_ptr,
      genmatrix* K_ptr,genmatrix* Kinv_ptr);
   double get_prefactor(int p,int q);
   void homography_decomposition_score_jacobian(
      double* input_param, double* jacobian, int m, int n, void* data);
   
// Rotation homography decomposition derivative methods:

   void compute_Gren(int p,int q,double* input_param,bool print_flag=false);
   void initialize_R0();
   genmatrix* Gren_derivative(int p,int q,int indep_param_index,
                              double* input_param);
   
   genmatrix* dGren_df(int p,int q,int r,double prefactor_pq,
                       double fu_p,double fu_q,double u0_p,double v0_p);
   genmatrix* dGren_du0(int p,int q,int r,double prefactor_pq,double fu_p);
   genmatrix* dGren_dv0(int p,int q,int r,double prefactor_pq,double fu_p);
   genmatrix* dGren_daz(int p,int q,int r,double prefactor_pq,
                        double az_p,double el_p,double roll_p,
                        double az_q,double el_q,double roll_q);
   genmatrix* dGren_del(int p,int q,int r,double prefactor_pq,
                        double az_p,double el_p,double roll_p,
                        double az_q,double el_q,double roll_q);
   genmatrix* dGren_droll(int p,int q,int r,double prefactor_pq,
                          double az_p,double el_p,double roll_p,
                          double az_q,double el_q,double roll_q);

   double dprefactor_df(int p,int q,int r,double fu_p,double fu_q);
   genmatrix* dK_df(int q,int r);
   genmatrix* dK_du0(int q,int r);
   genmatrix* dK_dv0(int q,int r);
   genmatrix* dKinv_df(int p,int r,double fu_p,double u0,double v0);
   genmatrix* dKinv_du0(int p,int r,double fu_p);
   genmatrix* dKinv_dv0(int p,int r,double fu_p);

// Homography fitting methods:

   void compute_symmetric_renormalized_homographies();
   int generate_homography_params_array(homography* curr_H_ptr);
   void photo_pair_homography_bundle_adjustment();
   void homography_fit_score(
      double* input_param, double* x, int m, int n, void* data);
   void set_H_and_Hinv(double* input_param,homography* H_ptr);
   void reset_homography_params_array(homography* curr_H_ptr);

// Global photosynth parameter estimation methods:

   void global_photosynth_bundle_adjustment(optimizer* o_ptr);
   void copy_global_params_in_to_buffer();
   void copy_global_params_out_of_buffer();
   void tiepoint_reprojection_score(
      double* input_param, double* x, int m, int n, void* data);

// ==========================================================================
// Inlined methods:
// ==========================================================================
   
} // optimizer_func namespace

#endif  // optimizer_funcs.h
