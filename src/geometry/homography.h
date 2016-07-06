// =========================================================================
// Header file for homography class
// =========================================================================
// Last modified on 8/7/13; 8/8/13; 10/1/13; 12/5/13
// =========================================================================

#ifndef HOMOGRAPHY_H
#define HOMOGRAPHY_H

#include <iostream>
#include <string>
#include <vector>
#include "math/threevector.h"
#include "math/twovector.h"

class genmatrix;
class plane;

class homography
{

  public:

   homography();
   homography(const genmatrix* Hcopy_ptr);

   ~homography();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const homography& H);

// Set & get member functions:

   void set_A_ptr(genmatrix* A_ptr);
   void set_H_ptr_element(int i,int j,double h_value);
   void set_H(genmatrix& H);
   genmatrix* get_H_ptr();
   const genmatrix* get_H_ptr() const;
   genmatrix* get_Hinv_ptr();
   const genmatrix* get_Hinv_ptr() const;
   std::vector<twovector>& get_XY_sorted();
   std::vector<twovector>& get_UV_sorted();
   std::vector<threevector>& get_nhat_sorted();
   std::vector<double>& get_d_chi();

// 3x3 homography matrix determination:

   bool parse_homography_inputs(
      const std::vector<threevector>& XY,const std::vector<threevector>& UV);
   bool parse_homography_inputs(
      const std::vector<threevector>& XY,const std::vector<threevector>& UV,
      unsigned int n_inputs);
   bool parse_homography_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV);
   bool parse_homography_inputs(
      unsigned int n_features,int p1,int p2,
      const genmatrix* u_meas_ptr,const genmatrix* v_meas_ptr,
      std::vector<twovector>& XY,std::vector<twovector>& UV);
   bool parse_homography_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      unsigned int n_inputs);
   bool parse_tieline_inputs(
      const std::vector<threevector>& abc,
      const std::vector<threevector>& def);

   void compute_homography_matrix(bool tielines_flag=false);
   void rearrange_homogeneous_soln_into_H(
      bool tielines_flag,const genvector& X);

   void project_world_plane_to_image_plane(
      double x,double y,double& u,double& v);
   twovector project_world_plane_to_image_plane(const twovector& r);
   threevector project_world_plane_to_image_plane(const threevector& r);
   twovector project_ray_to_image_plane(const threevector& r);
   void project_image_plane_to_world_plane(
      double u,double v,double& x,double& y);
   twovector project_image_plane_to_world_plane(const twovector& uv);

   double check_homography_matrix(
      std::vector<twovector>& XY,std::vector<twovector>& UV, 
      bool print_flag=true);
   double check_homography_matrix(
      const std::vector<threevector>& XY,const std::vector<threevector>& UV,
      bool print_flag=true);
   double check_homography_matrix(
      std::vector<twovector>& XY,std::vector<twovector>& UV,int n_inputs,
      bool print_flag=true);
   double check_homography_matrix(
      std::vector<twovector>& XY,std::vector<twovector>& UV,
      std::vector<int>& index,unsigned int n_inputs,bool print_flag=true);
   double check_inverse_homography_matrix(
      std::vector<twovector>& UV,std::vector<twovector>& XY,
      unsigned int n_inputs,bool print_flag=true);
   double check_tieline_homography_matrix(
      const std::vector<threevector>& abc,const std::vector<threevector>& def);

// 3D ray projection onto 2D features (video imagery projected onto
// panoramas):

   bool parse_projection_inputs(
      const std::vector<threevector>& nhat,const std::vector<twovector>& UV,
      double input_frac_to_use=1.0);
   double check_ray_projection(
      const std::vector<threevector>& nhat,const std::vector<twovector>& UV, 
      double input_frac_to_use=1.0,bool print_flag=true);

// Homography manipulation methods:

   bool compute_homography_inverse();
   bool enforce_unit_determinant();

// 2D UV to 2D XY planar homography methods:

//   void compute_homography(
//      const std::vector<double>& X,const std::vector<double>& Y,
//      const std::vector<double>& u,const std::vector<double>& v,
//      genmatrix* H_ptr);

   void generate_world_to_world_homography(
      const std::vector<threevector>& XYZ,
      const std::vector<threevector>& UVW);
   threevector world_to_world_map(const threevector& XYZ);

// Panorama/video matching project specific member functions:

   std::vector<homography*> parse_homography_results_file(
      std::string filename);
   void convert_video_to_panorama_coords(
      int video_width,int video_height,int panorama_width,int panorama_height,
      double video_u,double video_v,double& panorama_u,double& panorama_v);

// Camera parameter estimation member functions:

   void compute_camera_params_from_zplane_homography(
      double u0,double v0,
      double& f,double& az,double& el,double& roll,threevector& camera_posn,
      genmatrix& P);
   void compute_extrinsic_params(
      double f,double U0,double V0,rotation& R,threevector& camera_posn);

// Import/export member functions

   void export_matrix(std::string output_filename);
   void import_matrix(std::string input_filename);

  private:

// 2rx9 matrix *A_ptr holds X, Y, U and V information:

   genmatrix* A_ptr;
   
// 3x3 matrix *H_ptr holds the homography transformation from world
// plane to image plane coordinates:

   genmatrix* H_ptr;
   genmatrix* Hinv_ptr;

// 3 vectors P and p hold world and image plane coordinates:

   genvector* P_ptr;
   genvector* p_ptr;

// Pointers for 3D world planes:

   plane *plane1_ptr,*plane2_ptr;

// STL vectors to hold input XY and output UV twovectors sorted
// according to homography projection error:

   std::vector<twovector> XY_sorted,UV_sorted;
   std::vector<threevector> nhat_sorted;

   std::vector<double> d_chi;

   void allocate_member_objects();
   void initialize_member_objects();
   void load(const genmatrix* Hcopy_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void homography::set_A_ptr(genmatrix* A_ptr)
{
   this->A_ptr=A_ptr;
}

inline void homography::set_H_ptr_element(int i,int j,double h_value)
{
   H_ptr->put(i,j,h_value);
}

inline void homography::set_H(genmatrix& H)
{
   *H_ptr=H;
   compute_homography_inverse();
}

inline genmatrix* homography::get_H_ptr()
{
   return H_ptr;
}

inline const genmatrix* homography::get_H_ptr() const
{
   return H_ptr;
}

inline genmatrix* homography::get_Hinv_ptr()
{
   return Hinv_ptr;
}

inline const genmatrix* homography::get_Hinv_ptr() const
{
   return Hinv_ptr;
}

inline std::vector<twovector>& homography::get_XY_sorted()
{
   return XY_sorted;
}

inline std::vector<twovector>& homography::get_UV_sorted()
{
   return UV_sorted;
}

inline std::vector<threevector>& homography::get_nhat_sorted() 
{
   return nhat_sorted;
}

inline std::vector<double>& homography::get_d_chi()
{
   return d_chi;
}

#endif // homography.h




