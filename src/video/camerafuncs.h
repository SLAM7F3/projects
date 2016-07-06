// ==========================================================================
// Header file for camerafunc namespace
// ==========================================================================
// Last modified on 3/24/13; 4/24/13; 7/6/13; 4/3/14
// ==========================================================================

#ifndef CAMERAFUNCS_H
#define CAMERAFUNCS_H

#include <string>

class camera;
class fundamental;
class genmatrix;
class homography;
class rotation;
class texture_rectangle;
class threevector;

namespace camerafunc
{
   void f_and_aspect_ratio_from_horiz_vert_FOVs(
      double FOV_u,double FOV_v,double& f,double& aspect_ratio);
   double aspect_ratio_from_horiz_vert_FOVs(double FOV_u,double FOV_v);
   double vert_FOV_from_horiz_FOV_and_aspect_ratio(
      double FOV_u,double aspect_ratio);
   void f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
      double FOV_u,double aspect_ratio,double& f,double& FOV_v);
   void horiz_vert_FOVs_from_f_and_aspect_ratio(
      double f,double aspect_ratio,double& FOV_u,double& FOV_v);

// Non-square pixel methods:

   double fv_from_vert_FOV(double FOV_v);
   double fu_from_horiz_FOV_and_Umax(double FOV_u,double Umax);

// Fitting methods

   void fit_seven_params(
      camera* camera_ptr,double aspect_ratio,
      genmatrix* XYZUV_ptr,genmatrix* XYZABC_ptr);

// Epipolar geometry methods:

   threevector recover_camera_posn_from_projection_matrix(genmatrix* P_ptr);
   genmatrix* calculate_fundamental_matrix(
      camera* cameraprime_ptr,camera* camera_ptr);
   genmatrix* calculate_fundamental_matrix(
      genmatrix* P_ptr,genmatrix* Pprime_ptr);

// Radial undistortion methods:

   void radially_undistort_image(
      unsigned int Npu,unsigned int Npv,double fu_pixels,double fv_pixels,
      double cu_pixels,double cv_pixels,double k2,double k4,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr);
   void radially_undistort_image(
      unsigned int Npu,unsigned int Npv,double f_pixels,
      double cu_pixels,double cv_pixels,double k2,double k4,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr);

// Image rectification methods:

   void rectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,homography& H,
      std::string rectified_image_filename,
      bool image_to_world_flag=true);
   void rectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      int pu_min,int pu_max,int pv_min,int pv_max,
      homography& H,std::string rectified_image_filename,
      bool image_to_world_flag=true);
   void orthorectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      double Emin,double Emax,double Nmin,double Nmax,
      homography& H,std::string orthorectified_image_filename,
      bool world_to_image_flag=true);
   void orthorectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      std::string background_image_filename,double alpha,
      double Emin,double Emax,double Nmin,double Nmax,
      homography& H,std::string orthorectified_image_filename,
      bool world_to_image_flag=true);
   void orthorectify_image(
      std::string image_filename,
      double Emin,double Emax,double Nmin,double Nmax,homography& H,
      std::string rectified_images_subdir,int rectified_width,
      int orthorectified_height,bool world_to_image_flag=true);

// Wisp imagery manipulation methods:

   void vcorrect_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double A,double phi);
   double sinusoid_func(
      double u,int n_harmonic,double A,double phi,double Umax);
   void ucorrect_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double avg_Delta,double v_avg,double beta,double phi);
   void uv_translate_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double avg_Delta_u,double avg_Delta_v);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // camerafunc namespace

#endif // camerafuncs.h

