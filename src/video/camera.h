// ==========================================================================
// Header file for CAMERA class
// ==========================================================================
// Last modified on 8/12/13; 11/25/13; 12/19/13
// ==========================================================================

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <string>
#include <vector>
#include <osg/TexMat>
#include "geometry/bounding_box.h"
#include "video/camera_frustum.h"
#include "math/genmatrix.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "math/rotation.h"
#include "math/threevector.h"
#include "math/twovector.h"
#include "image/TwoDarray.h"

class G99VideoDisplay;
class plane;
class polyhedron;

class camera
{

  public:

// Initialization, constructor and destructor functions:

   camera();
   camera(G99VideoDisplay* VD_ptr);
   camera(double fu,double fv,double u0,double v0,
          double kappa2=0,double theta=90);
   camera(const camera& c);
   ~camera();
   camera& operator= (const camera& c);
   friend std::ostream& operator<< 
      (std::ostream& outstream,camera& c);
   void print_params(int ID,std::ofstream& outstream);

// Set & get member functions:

   void set_georegistered_flag(bool flag);
   void set_rho(double rho);
   void set_internal_params(double fu,double fv,double u0,double v0,
                            double theta=90,double kappa2=0);
   void set_f(double f);
   void set_fu(double f);
   double get_fu() const;
   void set_fv(double f);
   double get_fv() const;
   void set_u0(double u0);
   double get_u0() const;
   void set_v0(double v0);
   double get_v0() const;
   double get_theta() const;
   void set_kappas(double k2,double k4);
   double get_kappa2() const;
   double get_kappa4() const;

   void set_rel_az(double az);
   double get_rel_az() const;
   void set_rel_el(double el);
   double get_rel_el() const;
   void set_rel_roll(double roll);
   double get_rel_roll() const;

   void set_UV_corners(
      double min_U,double max_U,double min_V,double max_V);
   bounding_box* get_UV_bbox_ptr();
   const bounding_box* get_UV_bbox_ptr() const;
   void set_world_posn(const threevector& world_posn);
   const threevector& get_world_posn() const;
   void set_Rcamera(double az,double el,double roll);
   void compute_az_el_roll_from_Rcamera();
   void get_az_el_roll_from_Rcamera(double& az,double& el,double& roll);
   void set_Rcamera(const threevector& Uhat,const threevector& Vhat);
   void set_Rcamera(const rotation& R);
   threevector get_pointing_dir() const;

   void set_calibration_flag(bool flag);
   bool get_calibration_flag() const;

   double get_alpha() const;
   double get_beta() const;
   double get_phi() const;
   double get_FOV_u() const;
   double get_FOV_v() const;
   const threevector& get_Uhat() const;
   void set_Uhat(const threevector& uhat);
   const threevector& get_Vhat() const;
   void set_Vhat(const threevector& vhat);
   const threevector& get_What() const;
   void set_What(const threevector& what);
   rotation* get_Rcamera_ptr() const;
   rotation* get_R0_ptr() const;
   rotation* get_R_noah_to_peter_ptr(bool VSFM_flag=false);
   genmatrix* get_K_ptr() const;
   genmatrix* get_Kinv_ptr() const;

   genmatrix* get_P_ptr();
   const genmatrix* get_P_ptr() const;
   void set_video_ptr(G99VideoDisplay* video_ptr);
   void set_UV_corner_world_ray(const std::vector<threevector>& corner_ray);
   const std::vector<threevector>& get_UV_corner_world_ray() const;
   const std::vector<plane*>& get_frusta_plane_ptrs() const;
   
   void set_imageplane(const fourvector& pi);
   plane* get_imageplane_ptr() const;
   plane* get_imageplane_thru_center_ptr() const;

   void set_geolocation(double longitude,double latitude,double altitude);
   void set_geolocation(
      bool northern_hemisphere_flag,int UTM_zonenumber,
      double easting,double northing,double altitude);
   geopoint* get_geolocation_ptr();
   const geopoint* get_geolocation_ptr() const;

   homography* get_H_ptr();
   const homography* get_H_ptr() const;

   camera_frustum* get_camera_frustum_ptr();
   const camera_frustum* get_camera_frustum_ptr() const;
   
// 3x4 projection matrix construction based on tiepoint/tieline input:
   
   void compute_tiepoint_projection_matrix(
      const genmatrix* XYZUV_ptr,
      int n_rows_to_skip=0,int row_to_skip=-1);
   void compute_tiepoint_projection_matrix(
      const genmatrix* XYZUV_ptr,const genmatrix* XYZABC_ptr,
      int n_rows_to_skip=0,int row_to_skip=-1);
   void compute_orthographic_tiepoint_projection_matrix(
      const genmatrix* XYZUV_ptr);
   void parse_xyzuv_data(const genmatrix* XYZUV_ptr,genmatrix* A_ptr,
                         int row_to_skip=-1);
   void parse_orthographic_xyzuv_data(
      const genmatrix* XYZUV_ptr,genmatrix* B_ptr);
   void parse_xyzabc_data(
      const genmatrix* XYZABC_ptr,genmatrix* A_ptr,int starting_row);

   void check_projection_matrix(const genmatrix* XYZUV_ptr);
   double fast_check_projection_matrix(const genmatrix* XYZUV_ptr);
   void check_projection_matrix_for_tielines(const genmatrix* XYZABC_ptr);
   double fast_check_projection_matrix_for_tielines(
      const genmatrix* XYZABC_ptr);

   bool XYZ_in_front_of_camera(double x,double y,double z) const;
   bool XYZ_in_front_of_camera(const threevector& XYZ) const;
   void project_XYZ_to_UV_coordinates(
      double X,double Y,double Z,double& u,double & v) const;
   void project_XYZ_to_UV_coordinates(
      const threevector& XYZ,threevector& UVW);
   void cyl_project_XYZ_to_UV_coordinates(
      double x,double y,double z,double& u,double& v);
   void project_world_to_image_coords_with_radial_correction(
      double X,double Y,double Z,double& u,double& v) const;
   std::vector<threevector> project_polyhedron_face_into_imageplane(
      int f,const polyhedron* polyhedron_ptr,
      bool& face_inside_imageplane_flag);

// Camera parameter setting member functions:

   void construct_internal_parameter_K_matrix();
   void set_aircraft_rotation_angles(double pitch,double roll,double yaw);
   void set_mount_rotation_angles(double alpha,double beta,double phi);
   void compute_rotation_matrix();
   rotation generate_attitude_rotation(double p,double r,double y);
   void extract_attitude_angles(const rotation& R);

// Projection matrix manipulation member functions:

   void clear_previous_projection_matrices();
   void set_projection_matrix(
      genmatrix& P,double alpha=-1,bool temporally_filter_flag=false);
   void alpha_filter_projection_matrix(double alpha);
   void alphabeta_filter_projection_matrix(double alpha,double beta);
   void alphabetagamma_filter_projection_matrix(
      double alpha,double beta,double gamma);

   void construct_seven_param_projection_matrix();
   bool construct_projection_matrix(bool recompute_internal_params_flag=true);
   void construct_projection_matrix_for_fixed_K();
   void compute_corner_ray_and_UV_axes_dirs();
   genmatrix construct_projection_pseudo_inverse();
   bool decompose_projection_matrix();
   void print_external_and_internal_params();

// Field of view member functions:

   double compute_pyramid_sidelength(double movie_downrange_distance) const;
   double compute_movie_downrange_distance(double pyramid_sidelength) const;
   void compute_fields_of_view(
      double Umax, double Umin, double Vmax, double Vmin);

   double focal_param_corresponding_to_image_rescaling(
      int n_vertical_pixels,double fu_init,double rho);
   void rescale_focal_length(
      int n_vertical_pixels,double fu_init,double scale_factor,
      double az_init,double el_init,double roll_init);
   void save_initial_f_az_el_roll_params();
   void restore_initial_f_az_el_roll_params();

   double FOV_width_to_height_ratio(
      const std::vector<threevector>& corner_rays);
   void target_downrange_distance(
      const std::vector<threevector>& corner_rays,double bbox_height,
      double& pyramid_sidelength,double& downrange_distance);

// Bundler to world coordinate transformation member functions:

   void convert_bundler_to_world_coords(
      double x_trans,double y_trans,double z_trans,
      const threevector& bundler_rotation_origin,
      double global_az,double global_el,double global_roll,
      double global_scalefactor);
   void convert_bundler_to_world_coords(
      const threevector& bundler_COM,
      double global_az,double global_el,double global_roll,
      double global_scalefactor,const threevector& world_COM);
   void convert_world_to_bundler_coords(
      rotation& R_noah,threevector& camera_posn_noah);
   void write_camera_package_file(
      std::string package_filename,int photo_ID,std::string photo_filename,
      double frustum_sidelength,double downrange_distance=-1);

// Ray tracing member functions:

   double dotproduct_between_rays(double u1,double v1,double u2,double v2);
   threevector pixel_ray_direction(double u,double v) const;
   threevector pixel_ray_direction(const twovector& UV) const;
   std::vector<threevector> pixel_bbox_ray_directions(
      const bounding_box& bbox);

// Plane projection member functions:

   threevector backproject_imagepoint_to_zplane(
      double u,double v,double world_z);
   threevector backproject_imagepoint_to_zplane(
      const twovector& UV,double world_z);

   threevector principle_lookpoint_on_zplane(double world_z);
   std::vector<threevector> corner_ray_intercepts_with_zplane(double world_z);
   homography* homography_from_zplane_to_imageplane(double z);
   homography* homography_from_imageplane_to_zplane(double z);

//   bool backproject_imagepoint_to_worldplane(
//      double u,double v,plane& world_plane,threevector& intersection_pt);
//   bool backproject_imagepoint_to_worldplane(
//      const twovector& UV,plane& world_plane,threevector& intersection_pt);

   bool backproject_into_world_plane(
      double u,double v,const plane& P,threevector& intersection_pnt) const;
   bool backproject_corner_into_world_plane(
      int c,const plane& P,threevector& intersection_pnt);
   bool backproject_corner_into_world_plane(
      int c,const plane& P,twovector& corner_AB);
   homography* homography_from_worldplane_to_imageplane(plane& P);

// DTED ray tracing member functions:

   bool trace_individual_ray(
      const threevector& e_hat,double min_Z_ground,
      double max_raytrace_range,double ds,
      twoDarray* DTED_ztwoDarray_ptr,threevector& impact_point);
   bool trace_individual_ray(
      const threevector& e_hat,double min_Z_ground,
      double max_raytrace_range,double ds,double scale_factor,
      twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr,
      threevector& impact_point);
   bool trace_individual_ray(
      const threevector& starting_ray_posn,const threevector& e_hat,
      double min_Z_ground,double ds,twoDarray* DTED_ztwoDarray_ptr,
      threevector& impact_point);

  protected:

  private:

   bool calibration_flag,georegistered_flag;
   double rho,m_time;
   double fu,fv,theta,u0,v0,kappa2,kappa4;
   double rel_az,rel_el,rel_roll;
   double m_alpha,m_beta,m_phi;
   double m_roll,m_pitch,m_yaw;
   double FOV_u,FOV_v,sgn_detM;
   double fu_init,fv_init,az_init,el_init,roll_init;
   threevector camera_world_posn,Rcamera_times_camera_world_posn;
   threevector p4,Uhat,Vhat,What;
   bounding_box* UV_bbox_ptr;
   genmatrix *K_ptr,*Kinv_ptr;
   genmatrix *P_ptr,*P_prev_ptr,*Pdot_prev_ptr,*Pdotdot_prev_ptr;
   rotation *R_camera_ptr,*R0_ptr,*R_noah_to_peter_ptr;
   genmatrix *M_ptr,*Minv_ptr;	// 3x3 "scratch" matrix for tmp results
   plane *imageplane_ptr,*imageplane_thru_center_ptr;

   camera_frustum* camera_frustum_ptr;
   std::vector<plane*> frusta_plane_ptrs;

   geopoint* geolocation_ptr;

   G99VideoDisplay* video_ptr;
   std::vector<twovector> UV_corner;
   std::vector<threevector> UV_corner_world_ray;
   homography* H_ptr;

   void allocate_member_objects();
   void initialize_member_objects(G99VideoDisplay* VD_ptr=NULL);
   void docopy(const camera& c);

   void fill_projection_matrix_entries(const genmatrix* const A_ptr);
   void fill_orthographic_projection_matrix_entries(
      const genmatrix* const B_ptr);
   void extract_external_and_internal_params();
   void compute_pinhole_model_params(int sgn,genmatrix& P);
   bool compute_pointing_direction();
   void set_imageplane_thru_center();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void camera::set_georegistered_flag(bool flag)
{
   georegistered_flag=flag;
}

inline void camera::set_rho(double rho)
{
   this->rho=rho;
}

inline void camera::set_f(double f)
{
   fu=fv=f;
}

inline void camera::set_fu(double f)
{
   fu=f;
}

inline double camera::get_fu() const
{
   return fu;
}

inline void camera::set_fv(double f)
{
   fv=f;
}

inline double camera::get_fv() const
{
   return fv;
}

inline void camera::set_u0(double u0)
{
   this->u0=u0;
}

inline double camera::get_u0() const
{
   return u0;
}

inline void camera::set_v0(double v0)
{
   this->v0=v0;
}

inline double camera::get_v0() const
{
   return v0;
}

inline double camera::get_theta() const
{
   return theta;
}

inline void camera::set_kappas(double k2,double k4)
{
   kappa2=k2;
   kappa4=k4;
}

inline double camera::get_kappa2() const
{
   return kappa2;
}

inline double camera::get_kappa4() const
{
   return kappa4;
}

inline void camera::set_rel_az(double az)
{
   rel_az=az;
   set_Rcamera(rel_az,rel_el,rel_roll);
}

inline double camera::get_rel_az() const
{
   return rel_az;
}

inline void camera::set_rel_el(double el)
{
   rel_el=el;
   set_Rcamera(rel_az,rel_el,rel_roll);
}

inline double camera::get_rel_el() const
{
   return rel_el;
}

inline void camera::set_rel_roll(double roll)
{
   rel_roll=roll;
   set_Rcamera(rel_az,rel_el,rel_roll);
}

inline double camera::get_rel_roll() const
{
   return rel_roll;
}

inline bounding_box* camera::get_UV_bbox_ptr()
{
   return UV_bbox_ptr;
}

inline void camera::set_world_posn(const threevector& world_posn)
{
   camera_world_posn=world_posn;
}

inline const threevector& camera::get_world_posn() const
{
//   std::cout << "inside camera::get_world_posn(), this = " << this
//             << std::endl;
//   std::cout << "camera_world_posn = " << camera_world_posn << std::endl;
   return camera_world_posn;
}

inline threevector camera::get_pointing_dir() const
{
//   cout << "inside camera::get_pointing_dir(), What = " << What << endl;
   return -What;
}

inline void camera::set_calibration_flag(bool flag)
{
   calibration_flag=flag;
}

inline bool camera::get_calibration_flag() const
{
   return calibration_flag;
}

inline double camera::get_alpha() const
{
   return m_alpha;
}

inline double camera::get_beta() const
{
   return m_beta;
}

inline double camera::get_phi() const
{
   return m_phi;
}

inline double camera::get_FOV_u() const
{
   return FOV_u;
}

inline double camera::get_FOV_v() const
{
   return FOV_v;
}

inline const threevector& camera::get_Uhat() const
{
   return Uhat;
}

inline void camera::set_Uhat(const threevector& uhat)
{
   Uhat=uhat;
}

inline const threevector& camera::get_Vhat() const
{
   return Vhat;
}

inline void camera::set_Vhat(const threevector& vhat)
{
   Vhat=vhat;
}

inline const threevector& camera::get_What() const
{
   return What;
}

inline void camera::set_What(const threevector& what)
{
   What=what;
}

inline rotation* camera::get_Rcamera_ptr() const
{
   return R_camera_ptr;
}

inline rotation* camera::get_R0_ptr() const
{
   return R0_ptr;
}

inline genmatrix* camera::get_K_ptr() const
{
   return K_ptr;
}

inline genmatrix* camera::get_Kinv_ptr() const
{
   return Kinv_ptr;
}

inline genmatrix* camera::get_P_ptr()
{
   return P_ptr;
}

inline const genmatrix* camera::get_P_ptr() const
{
   return P_ptr;
}

inline void camera::set_video_ptr(G99VideoDisplay* video_ptr)
{
   this->video_ptr=video_ptr;
}

inline void camera::set_UV_corner_world_ray(
   const std::vector<threevector>& corner_ray)
{
   UV_corner_world_ray.clear();
   for (unsigned int c=0; c<corner_ray.size(); c++)
   {
      UV_corner_world_ray.push_back(corner_ray[c]);
   }
}

inline const std::vector<threevector>& camera::get_UV_corner_world_ray() const
{
   return UV_corner_world_ray;
}

inline const std::vector<plane*>& camera::get_frusta_plane_ptrs() const
{
   return frusta_plane_ptrs;
}


inline void camera::set_geolocation(
   double longitude,double latitude,double altitude)
{
   delete geolocation_ptr;
   geolocation_ptr=new geopoint(longitude,latitude,altitude);
}

inline void camera::set_geolocation(
   bool northern_hemisphere_flag,int UTM_zonenumber,
   double easting,double northing,double altitude)
{
   delete geolocation_ptr;
   geolocation_ptr=new geopoint(
      northern_hemisphere_flag,UTM_zonenumber,
      easting,northing,altitude);
}

inline geopoint* camera::get_geolocation_ptr()
{
   return geolocation_ptr;
}

inline const geopoint* camera::get_geolocation_ptr() const
{
   return geolocation_ptr;
}

inline homography* camera::get_H_ptr()
{
   return H_ptr;
}

inline const homography* camera::get_H_ptr() const
{
   return H_ptr;
}

inline camera_frustum* camera::get_camera_frustum_ptr()
{
   return camera_frustum_ptr;
}

inline const camera_frustum* camera::get_camera_frustum_ptr() const
{
   return camera_frustum_ptr;
}

// ---------------------------------------------------------------------
// Member function project_XYZ_to_UV_coordinates() takes in the world
// XYZ coordinates for some point.  It projects this world point down
// onto the UV plane via 3x4 transformation matrix *P_ptr.  This
// method returns the (u,v) coordinates of the projected point.

inline void camera::project_XYZ_to_UV_coordinates(
   double X,double Y,double Z,double& u,double & v) const
{
   double x=P_ptr->get(0,0)*X+P_ptr->get(0,1)*Y+P_ptr->get(0,2)*Z
      +P_ptr->get(0,3);
   double y=P_ptr->get(1,0)*X+P_ptr->get(1,1)*Y+P_ptr->get(1,2)*Z
      +P_ptr->get(1,3);
   double z=P_ptr->get(2,0)*X+P_ptr->get(2,1)*Y+P_ptr->get(2,2)*Z
      +P_ptr->get(2,3);
   u=x/z;
   v=y/z;
}

// ---------------------------------------------------------------------
// Member function cyl_project_XYZ_to_UV_coords() projects XYZ world
// space points onto the UV image plane NOT according to a
// conventional 3x4 projection matrix.  Instead, it uses the limit of
// the perspective projection as the horizontal field-of-view goes
// to zero.  2D planar circles of constant rho=sqrt(X*X+Y*Y) are
// mapped by this cylindrical projection to lines of constant V.

inline void camera::cyl_project_XYZ_to_UV_coordinates(
   double x,double y,double z,double& u,double& v)
{
//   cout << "inside camera::cyl_project_XYZ_to_UV_coordinates()" << endl;

   double X=
      -(x-camera_world_posn.get(0))*What.get(0)
      -(y-camera_world_posn.get(1))*What.get(1)
      -(z-camera_world_posn.get(2))*What.get(2);
   double Y=
      -(x-camera_world_posn.get(0))*Uhat.get(0)
      -(y-camera_world_posn.get(1))*Uhat.get(1)
      -(z-camera_world_posn.get(2))*Uhat.get(2);
   double Z=
       (x-camera_world_posn.get(0))*Vhat.get(0)
      +(y-camera_world_posn.get(1))*Vhat.get(1)
      +(z-camera_world_posn.get(2))*Vhat.get(2);

   u=u0+fu*atan2(Y,X);
   v=v0-fu*Z/sqrt(X*X+Y*Y);
}

// ---------------------------------------------------------------------
// Member function project_world_to_image_coords takes in an XYZ
// 3-vector and projects it onto the UV image plane.  This method
// takes the first nontrivial radial distortion correction into
// account.  

// Note: Since this low-level method is called so many times within
// multiply nested loops, we have intentionally tried to strip it down
// to its bare essentials in order to make it run as fast as
// possible...

inline void camera::project_world_to_image_coords_with_radial_correction(
   double X,double Y,double Z,double& u,double& v) const
{
   double x=R_camera_ptr->get(0,0)*X
      +R_camera_ptr->get(0,1)*Y+R_camera_ptr->get(0,2)*Z
      -Rcamera_times_camera_world_posn.get(0);
   double y=R_camera_ptr->get(1,0)*X
      +R_camera_ptr->get(1,1)*Y+R_camera_ptr->get(1,2)*Z
      -Rcamera_times_camera_world_posn.get(1);
   double z=R_camera_ptr->get(2,0)*X
      +R_camera_ptr->get(2,1)*Y+R_camera_ptr->get(2,2)*Z
      -Rcamera_times_camera_world_posn.get(2);

   double utilde=fu*x/z;
   double vtilde=fv*y/z;

//   double delta_utilde=0;
//   double delta_vtilde=0;

//   const double kappa1=0;
//   if (!nearly_equal(kappa2,0))
//   {
//      double rsq=utilde*utilde+vtilde*vtilde;
//      double delta_utilde=utilde*(kappa2*rsq);
//      double delta_vtilde=vtilde*(kappa2*rsq);
//      double r=sqrt(rsq);
//      delta_utilde=utilde*(kappa1*r+kappa2*rsq);
//      delta_vtilde=vtilde*(kappa1*r+kappa2*rsq);
//   }
         
   double lens_correction=1+kappa2*(utilde*utilde+vtilde*vtilde);
   u=utilde*lens_correction+u0;
   v=vtilde*lens_correction+v0;

//   u=utilde+delta_utilde+u0;
//   v=vtilde+delta_vtilde+v0;
//   return twovector(utilde+delta_utilde+u0,vtilde+delta_vtilde+v0);
}

// ---------------------------------------------------------------------
// Boolean member function XYZ_in_front_of_camera() returns false if
// input XYZ does not have a positive projection along -What.

inline bool camera::XYZ_in_front_of_camera(double x,double y,double z) const
{
   double dotproduct=-1*(
      What.get(0)*(x-camera_world_posn.get(0))+
      What.get(1)*(y-camera_world_posn.get(1))+
      What.get(2)*(z-camera_world_posn.get(2)) );
   return (dotproduct > 0);
}

inline bool camera::XYZ_in_front_of_camera(const threevector& XYZ) const
{
   return XYZ_in_front_of_camera(XYZ.get(0),XYZ.get(1),XYZ.get(2));
}

#endif // camera.h



