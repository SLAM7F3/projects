// ==========================================================================
// Header file for optimizer class
// ==========================================================================
// Last modified on 1/17/11; 2/26/11; 6/4/11
// ==========================================================================

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>
#include "math/Genarray.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "math/rotation.h"

class FeaturesGroup;
class genmatrix;
class homography;

class optimizer
{

  public:

// Initialization, constructor and destructor functions:

   optimizer();
   optimizer(photogroup* photogroup_ptr);
   optimizer(const optimizer& o);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~optimizer();
   optimizer& operator= (const optimizer& o);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const optimizer& o);

// Set and get member functions:

   void set_fit_external_params_flag(bool flag);
   bool get_fit_external_params_flag() const;
   
   void set_sigma(double sigma);
   double get_sigma() const;

   double get_external_params_weight() const;
   
   int get_n_params_per_photo() const;
   unsigned int get_n_photos() const;
   void set_photogroup_ptr(photogroup* photogroup_ptr);
   photogroup* get_photogroup_ptr();
   const photogroup* get_photogroup_ptr() const;
   void set_bundler_photogroup_ptr(photogroup* photogroup_ptr);
   photogroup* get_bundler_photogroup_ptr();
   const photogroup* get_bundler_photogroup_ptr() const;

   homography* get_homography_ptr(int p,int q);
   homography* get_homography_ptr_given_photo_indices(int i,int j);
   FeaturesGroup* get_FeaturesGroup_ptr();
   const FeaturesGroup* get_FeaturesGroup_ptr() const;
   genmatrix* get_u_meas_ptr();
   const genmatrix* get_u_meas_ptr() const;
   genmatrix* get_v_meas_ptr();
   const genmatrix* get_v_meas_ptr() const;

// Camera parameter manipulation member functions:

   void print_camera_parameters(int curr_n_photos) const;

// Homography computation member functions:

   void extract_photo_feature_info(FeaturesGroup* FeaturesGroup_ptr);
   void extract_manual_feature_info(FeaturesGroup* FeaturesGroup_ptr);
   void compute_renormalized_homographies();
   void readin_renormalized_homographies(std::string homography_filename);

// Camera parameter estimation member functions:

   void estimate_internal_and_relative_rotation_params(int curr_n_photos);
   void estimate_relative_rotation_params(
      int p,int q,homography* Hpq_ptr);
   double homography_error(int p, int q,const genmatrix& H);
   double projection_error(const std::vector<int>& photo_indices);
   double projection_error(int q);
   void bundle_adjust_for_rotating_camera(int n_photos);
   void iteratively_bundle_adjust_for_rotating_camera(int curr_n_photos);
   void group_bundle_adjust_for_rotating_camera(int curr_n_photos);

   void bundle_adjust_for_global_photosynth_params(
      FeaturesGroup* manual_FeaturesGroup_ptr);

// Camera rotation from image and world space ray matching member functions:

   void compute_world_and_imagespace_feature_rays();
   void compute_world_and_imagespace_feature_rays(
      const threevector& camera_posn);
   double compute_scalefactor_between_world_and_imagespace_rays();
   rotation compute_rotation_between_imagespace_rays_and_world_rays();
   rotation compute_rotation_between_imagespace_rays_and_world_rays(
      const std::vector<threevector>& curr_imagespace_ray,
      const std::vector<threevector>& curr_worldspace_ray);
   rotation compute_rotation_between_imagespace_rays_and_world_rays(
      double& delta_thetas_mu,double& delta_thetas_sigma);
   rotation compute_rotation_between_imagespace_rays_and_world_rays(
      const std::vector<threevector>& curr_imagespace_ray,
      const std::vector<threevector>& curr_worldspace_ray,
      double& delta_thetas_mu,double& delta_thetas_sigma);

   rotation randomly_compute_rotation_between_imagespace_rays_and_world_rays(
      const threevector& camera_posn);
   bool compute_inlier_rotation_between_imagespace_rays_and_world_rays(
      double min_dotproduct,const rotation& R_global,
      unsigned int& max_n_inliers,rotation& inlier_R_global,
      double& delta_thetas_mu,double& delta_thetas_sigma);
   rotation compute_RANSAC_rotation_between_imagespace_rays_and_world_rays(
      unsigned int iter,const threevector& camera_posn,double& delta_thetas_mu,
      double& delta_thetas_sigma);

  private:

   bool fit_external_params_flag;
   unsigned int n_photos;
   int n_params_per_photo,n_params;
   double sigma,sqr_sigma;
   double external_params_weight;
   photogroup *photogroup_ptr,*bundler_photogroup_ptr;	
	// just ptrs, not actually object
   std::vector<double> homography_params;
   FeaturesGroup* FeaturesGroup_ptr; // just ptr to FG and not dyn FG itself
   genmatrix *u_meas_ptr,*u_manual_ptr;
   genmatrix *v_meas_ptr,*v_manual_ptr;
   rotation* R0_ptr;
   Genarray<homography*>* homography_ptrs_ptr;
   std::vector<threevector> XYZ_manual,worldspace_ray,imagespace_ray;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const optimizer& o);

   void estimate_relative_rotation(int p,int q);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void optimizer::set_fit_external_params_flag(bool flag)
{
   fit_external_params_flag=flag;
}

inline bool optimizer::get_fit_external_params_flag() const
{
   return fit_external_params_flag;
}

inline void optimizer::set_sigma(double sigma)
{
   this->sigma=sigma;
   sqr_sigma=sqr(sigma);
}

inline double optimizer::get_sigma() const
{
   return sigma;
}

inline double optimizer::get_external_params_weight() const
{
   return external_params_weight;
}

inline int optimizer::get_n_params_per_photo() const
{
   return n_params_per_photo;
}

inline void optimizer::set_photogroup_ptr(photogroup* photogroup_ptr)
{
   this->photogroup_ptr=photogroup_ptr;
}

inline photogroup* optimizer::get_photogroup_ptr()
{
   return photogroup_ptr;
}

inline const photogroup* optimizer::get_photogroup_ptr() const
{
   return photogroup_ptr;
}

inline void optimizer::set_bundler_photogroup_ptr(
   photogroup* photogroup_ptr)
{
   bundler_photogroup_ptr=photogroup_ptr;
}

inline photogroup* optimizer::get_bundler_photogroup_ptr()
{
   return bundler_photogroup_ptr;
}

inline const photogroup* optimizer::get_bundler_photogroup_ptr() const
{
   return bundler_photogroup_ptr;
}

inline unsigned int optimizer::get_n_photos() const
{
   return n_photos;
}

inline FeaturesGroup* optimizer::get_FeaturesGroup_ptr()
{
   return FeaturesGroup_ptr;
}

inline const FeaturesGroup* optimizer::get_FeaturesGroup_ptr() const
{
   return FeaturesGroup_ptr;
}

inline genmatrix* optimizer::get_u_meas_ptr()
{
   return u_meas_ptr;
}

inline const genmatrix* optimizer::get_u_meas_ptr() const
{
   return u_meas_ptr;
}

inline genmatrix* optimizer::get_v_meas_ptr()
{
   return v_meas_ptr;
}

inline const genmatrix* optimizer::get_v_meas_ptr() const
{
   return v_meas_ptr;
}



#endif  // optimizer.h



