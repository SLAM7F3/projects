// =========================================================================
// Header file for affine_transform class
// =========================================================================
// Last modified on 7/26/13; 7/27/13; 11/21/13; 4/5/14
// =========================================================================

#ifndef AFFINE_TRANSFORM_H
#define AFFINE_TRANSFORM_H

#include <iostream>
#include <string>
#include <vector>
#include "math/threevector.h"
#include "math/twovector.h"

class genmatrix;
class plane;

class affine_transform
{

  public:

   affine_transform();

   ~affine_transform();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const affine_transform& AT);

// Set & get member functions:

   void reset_max_n_inliers();
   int get_max_n_inliers() const;
   genmatrix* get_A_ptr();
   const genmatrix* get_A_ptr() const;
   twovector& get_trans();
   const twovector& get_trans() const;

// 3x3 affine_transform matrix determination:

   bool parse_affine_transform_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV);
   bool parse_affine_transform_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      unsigned int n_inputs);
   void fit_affine_transformation();

   void project_world_plane_to_image_plane(
      double x,double y,double& u,double& v);
   twovector project_world_plane_to_image_plane(const twovector& r);
   threevector project_world_plane_to_image_plane(const threevector& r);
   void project_image_plane_to_world_plane(
      double u,double v,double& x,double& y);
   twovector project_image_plane_to_world_plane(const twovector& uv);

   double check_affine_transformation(
      std::vector<twovector>& XY,std::vector<twovector>& UV);

   int count_affine_inliers(
      std::vector<twovector>& XY,std::vector<twovector>& UV,
      double max_delta_UV);
   void export_affine_inliers(
      std::vector<twovector>& XY,std::vector<twovector>& UV,
      double max_delta_UV);
   std::vector<int> export_affine_inlier_indices(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      double max_delta_UV);

  private:

   int max_n_inliers;	// default value = -1
   
// 2rx6 matrix *M_ptr holds X and Y information:

   genmatrix* M_ptr;
   
// 2r dimensional vector *B_ptr holds U and V information:

   genvector* B_ptr;

// 2x2 matrix *A_ptr and 2-dimensional translation vector holds the
// affine_transform transformation from world
// plane to image plane coordinates:

   genmatrix *A_ptr,*Ainv_ptr,*best_A_ptr;
   twovector trans,best_trans;

   std::vector<twovector> XY_sorted,UV_sorted;

   void allocate_member_objects();
   void initialize_member_objects();
   void load(const genmatrix* Hcopy_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void affine_transform::reset_max_n_inliers() 
{
   max_n_inliers=0;
}

inline int affine_transform::get_max_n_inliers() const
{
   return max_n_inliers;
}

inline genmatrix* affine_transform::get_A_ptr() 
{
   return A_ptr;
}

inline const genmatrix* affine_transform::get_A_ptr() const
{
   return A_ptr;
}

inline twovector& affine_transform::get_trans()
{
   return trans;
}

inline const twovector& affine_transform::get_trans() const
{
   return trans;
}

#endif // affine_transform.h




