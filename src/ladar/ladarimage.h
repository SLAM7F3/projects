// ==========================================================================
// Header file for LADARIMAGE base class
// ==========================================================================
// Last modified on 7/25/06; 8/3/06; 3/17/09; 4/5/14
// ==========================================================================

#ifndef LADARIMAGE_H
#define LADARIMAGE_H

#include <osg/Array>
#include <vector>
#include "image/myimage.h"
#include "math/mypolynomial.h"
#include "math/threevector.h"

class parallelogram;
template <class T> class Hashtable;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class ladarimage:public myimage
{
  public:

   enum Data_kind
   {
      z_data,p_data,fused_data,phase_data,direction_data
   };

// Initialization, constructor and destructor functions:

   ladarimage(void);
   ladarimage(const ladarimage& m);
   ladarimage(int nxbins,int nybins);
   ladarimage(const twoDarray& T);
   virtual ~ladarimage();
   ladarimage& operator= (const ladarimage& m);

// Set & get member functions:

   void set_public_software(bool flag);
   void set_xyz_datadir(std::string filename);
   void set_xyz_filenamestr(std::string filename);
   void set_npoints(int n);
   void set_flightpath_poly(const mypolynomial& poly);
   void set_data_bbox_ptr(parallelogram* bbox_ptr);
   std::string get_xyz_datadir() const;
   std::string get_xyz_filenamestr() const;
   void set_gradient_mag_twoDarray_ptr(twoDarray* grad_mag_twoDarray_ptr);
   void set_gradient_phase_twoDarray_ptr(twoDarray* grad_phase_twoDarray_ptr);

   int get_npoints() const;
   void set_xextent(double extent_x);
   void set_yextent(double extent_y);
   void set_zextent(double extent_z);
   double get_xextent() const;
   double get_yextent() const;
   double get_zextent() const;
   parallelogram* get_data_bbox_ptr();
   const parallelogram* get_data_bbox_ptr() const;
   twoDarray* get_p2Darray_orig_ptr();
   twoDarray* get_p2Darray_ptr();
   twoDarray* get_gradient_mag_twoDarray_ptr();
   twoDarray* get_gradient_phase_twoDarray_ptr();
   twoDarray* get_zlaplacian_twoDarray_ptr();
   TwoDarray<threevector>* get_normal_twoDarray_ptr();
   Hashtable<linkedlist*>* get_connected_heights_hashtable_ptr();
   Hashtable<linkedlist*>* get_connected_gradient_phases_hashtable_ptr();

// Metafile display member functions:

   void generate_colortable(
      int ncolorsteps,double zmin,double zmax,double intensity,
      double height,double width,double legloc_x,double legloc_y);
   void writeimage(std::string filename,twoDarray const *ztwoDarray_ptr,
                   bool display_flightpath=false,Data_kind datatype=z_data,
                   std::string title="");
   void update_logfile(std::string main_program_filename);

// Data entry member functions:

   void initialize_image(
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);
   void parse_and_store_input_data(
      double deltax,double deltay,
      bool renormalize_just_xy_values=false,bool renormalize_xyz_data=true,
      bool remove_snowflakes=true,bool rotate_xy_values=false,double theta=0);
   void store_input_data(
      double xmin,double ymin,double xmax,double ymax,
      double deltax,double deltay,
      osg::Vec3Array* vertices_ptr,const osg::FloatArray* probs_ptr=NULL);
   void compute_xyzp_origin_and_extents(
      const std::vector<threevector>& XYZ,const std::vector<double>& p);
   void convert_from_absolute_to_relative_xyz(
      const threevector& absolute_origin,std::vector<threevector>& XYZ);

   void initialize_image_parameters(
      twoDarray*& ztwoDarray_orig_ptr,
      double deltax,double deltay,double xlo=0,double ylo=0);
   void initialize_image_parameters(
      twoDarray*& ztwoDarray_orig_ptr,twoDarray*& ptwoDarray_orig_ptr,
      double deltax,double deltay,double xlo=0,double ylo=0);
   int fill_arrays_with_x_y_z_and_p_values(
      double xoffset,double yoffset,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
      double x[],double y[],double z[],double p[]);

// Flight path member functions:

   void fit_flightpath_poly(
      int nflight_points,double xflight[],double yflight[],
      twoDarray* ztwoDarray_ptr);
   void draw_fitted_flightpath(twoDarray* ztwoDarray_ptr);
   void compute_data_bbox(
      twoDarray* ztwoDarray_ptr,bool scale_data_bbox=true,
      bool conservative_parallelogram_flag=true);
   void compute_trivial_xy_data_bbox(twoDarray const *ztwoDarray_ptr);
   void use_bbox_to_resize_ladarimage(double deltax,double deltay);

// Ground extraction member functions:

   twoDarray* subsample_zimage(twoDarray const *ztwoDarray_ptr);
   void estimate_ground_surface_using_gradient_info(
      double spatial_resolution,twoDarray* const xderiv_twoDarray_ptr,
      twoDarray* const yderiv_twoDarray_ptr,twoDarray* ztwoDarray_ptr);
   void estimate_ground_surface_using_gradient_info(
      double spatial_resolution,twoDarray* const xderiv_twoDarray_ptr,
      twoDarray* const yderiv_twoDarray_ptr,twoDarray* ztwoDarray_ptr,
      bool mask_points_near_border,twoDarray const *mask_twoDarray_ptr);
   twoDarray* interpolate_and_flatten_ground_surface(
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *zground_silhouetted_twoDarray_ptr,
      twoDarray const *ptwoDarray_ptr,
      double correlation_distance=15,double local_threshold_frac=0.33);

// Gradient field computation member functions:

   void compute_gradient_magnitude_field(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr);
   void compute_gradient_direction_field(
      double min_gradmag_threshold,double max_gradmag_threshold,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray* grad_mag_twoDarray_ptr);
   void compute_surface_normal_field(twoDarray const *ztwoDarray_ptr);
   void compute_surface_normal_field(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr);
   twoDarray* compute_gradient_mags_at_mask_locations(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr);
   twoDarray* compute_gradient_phases_at_mask_locations(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr);
   twoDarray* connect_phase_components(
      double min_projected_area,bool plot_connected_phase_components=false);
   void connected_component_phase_gradient_distribution();
   void compute_laplacian_field(twoDarray const *ztwoDarray_ptr);
   void intersect_gradient_and_laplacian_fields(
      double laplacian_threshold);
   void intersect_height_grad_components(
      double height_threshold,twoDarray const *ztwoDarray_ptr);
   twoDarray* intersect_connected_heights_with_phase_grads(
      twoDarray const *zcluster_twoDarray_ptr,
      double intersection_frac_threshold,
      bool discard_component_if_difference_too_small,
      bool plot_surviving_components=false);
   void intersect_binary_image_with_phase_grads(
      twoDarray const *zbinary_twoDarray_ptr,
      bool retain_unity_valued_binary_pixels=true);

// Height and/or intensity fluctuations member functions:

   twoDarray* compute_z_fluctuations(twoDarray const *ztwoDarray_ptr);
   void compute_p_fluctuations(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr);
   void compute_surface_normal_field_variation(
      twoDarray const *ztwoDarray_ptr);
   void compute_local_planarity(
      twoDarray const *ztwoDarray_ptr,twoDarray const *zhilo_twoDarray_ptr,
      twoDarray* zplanar_twoDarray_ptr);
   void compute_local_planarity_in_bbox(
      double width,double length,double theta,
      twoDarray const *ztwoDarray_ptr,twoDarray* zplanar_twoDarray_ptr);
   void accumulate_planarity_info(
      double bbox_rotation_angle,
      twoDarray const *ztwoDarray_ptr,twoDarray const *zexpand_twoDarray_ptr,
      twoDarray const *zplanar_rot_twoDarray_ptr,
      twoDarray* zplanar_summary_twoDarray_ptr,
      twoDarray* planarity_direction_twoDarray_ptr);

// Feature extraction member functions:

   void find_large_hot_lumps(
      twoDarray* ztwoDarray_ptr,bool annotate_ztwoDarray=true);
   void plot_zp_distributions(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr);

  protected:

   bool public_software;

   std::string logfilename; // Full path to logfile for multi image processing
   std::string xyz_datadir; // Full path to Group 94 xyz datafile
   std::string xyz_filenamestr; // Name of Group 94 xyz datafile
   int n_xyz_points; // number of xyz points upon which current ladarimage
		     //  is based

// Time after xyzp image has been initialized.  Value measured in
// seconds since some UNIX reference point:

   time_t start_processing_time;  

// All (x,y,z) data within an ladarimage are measured RELATIVE to the
// following ABSOLUTE set of coordinates:

   double delta_z;	// Ladar data height resolution
   threevector image_origin;	// meters measured from HAFB base
   double xextent,yextent,zextent; 	// meters 
   double pmin,pmax,pextent;	// Extremal detection probability values
   
   mypolynomial flightpath_poly;
   parallelogram* data_bbox_ptr;

// TwoDarray objects p2Darray_orig and p2Darray contain probability
// values for each pixel.  p2Darray_orig essentially contains raw
// probability information; after it is initialized, it is never to be
// touched.  In contrast, p2Darray is the "working" object whose
// contents are altered by all sorts of recursive filters,
// differential thresholds, etc...

   twoDarray* p2Darray_orig_ptr;
   twoDarray* p2Darray_ptr;

// TwoDarray objects gradient_mag_twoDarray and
// gradient_phase_twoDarray hold gradient magnitude and phase
// information:

   twoDarray* gradient_mag_twoDarray_ptr;
   twoDarray* gradient_phase_twoDarray_ptr;
   twoDarray* zlaplacian_twoDarray_ptr;

// Store surface normal vectors within the following TwoDarray<threevector>:

   TwoDarray<threevector>* normal_twoDarray_ptr;

// Hashtables *connected_heights_hashtable_ptr and
// *connected_gradient_phases_hashtable_ptr hold linked lists of pixels
// corresponding to connected z and gradient field components:

   Hashtable<linkedlist*>* connected_heights_hashtable_ptr;
   Hashtable<linkedlist*>* connected_gradient_phases_hashtable_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ladarimage& m);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ladarimage::set_public_software(bool flag)
{
   public_software=flag;
}

inline void ladarimage::set_xyz_datadir(std::string dirname)
{
   xyz_datadir=dirname;
}

inline void ladarimage::set_xyz_filenamestr(std::string filename)
{
   xyz_filenamestr=filename;
   std::cout << "Binary xyz file = " << xyz_filenamestr << std::endl;
}

inline void ladarimage::set_npoints(int n)
{
   n_xyz_points=n;
}

inline void ladarimage::set_flightpath_poly(const mypolynomial& poly)
{
   flightpath_poly=poly;
}

inline void ladarimage::set_data_bbox_ptr(parallelogram* bbox_ptr)
{
   data_bbox_ptr=bbox_ptr;
}

inline void ladarimage::set_gradient_mag_twoDarray_ptr(
   twoDarray* grad_mag_twoDarray_ptr)
{
   gradient_mag_twoDarray_ptr=grad_mag_twoDarray_ptr;
}

inline void ladarimage::set_gradient_phase_twoDarray_ptr(
   twoDarray* grad_phase_twoDarray_ptr)
{
   gradient_phase_twoDarray_ptr=grad_phase_twoDarray_ptr;
}

inline std::string ladarimage::get_xyz_datadir() const
{
   return xyz_datadir;
}

inline std::string ladarimage::get_xyz_filenamestr() const
{
   return xyz_filenamestr;
}

inline int ladarimage::get_npoints() const
{
   return n_xyz_points;
}

inline void ladarimage::set_xextent(double extent_x)
{
   xextent=extent_x;
}

inline void ladarimage::set_yextent(double extent_y)
{
   yextent=extent_y;
}

inline void ladarimage::set_zextent(double extent_z)
{
   zextent=extent_z;
}

inline double ladarimage::get_xextent() const
{
   return xextent;
}

inline double ladarimage::get_yextent() const
{
   return yextent;
}

inline double ladarimage::get_zextent() const
{
   return zextent;
}

inline parallelogram* ladarimage::get_data_bbox_ptr() 
{
   return data_bbox_ptr;
}

inline const parallelogram* ladarimage::get_data_bbox_ptr() const
{
   return data_bbox_ptr;
}

inline twoDarray* ladarimage::get_p2Darray_orig_ptr() 
{
   return p2Darray_orig_ptr;
}

inline twoDarray* ladarimage::get_p2Darray_ptr() 
{
   return p2Darray_ptr;
}

inline twoDarray* ladarimage::get_gradient_mag_twoDarray_ptr() 
{
   return gradient_mag_twoDarray_ptr;
}

inline twoDarray* ladarimage::get_gradient_phase_twoDarray_ptr() 
{
   return gradient_phase_twoDarray_ptr;
}

inline twoDarray* ladarimage::get_zlaplacian_twoDarray_ptr() 
{
   return zlaplacian_twoDarray_ptr;
}

inline TwoDarray<threevector>* ladarimage::get_normal_twoDarray_ptr()
{
   return normal_twoDarray_ptr;
}

inline Hashtable<linkedlist*>* 
ladarimage::get_connected_heights_hashtable_ptr() 
{
   return connected_heights_hashtable_ptr;
}

inline Hashtable<linkedlist*>* 
ladarimage::get_connected_gradient_phases_hashtable_ptr() 
{
   return connected_gradient_phases_hashtable_ptr;
}

#endif // ladarimage.h



