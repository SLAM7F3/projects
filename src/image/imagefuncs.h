// =========================================================================
// Header file for stand-alone image functions.
// =========================================================================
// Last modified on 3/24/16; 3/26/16; 4/9/16; 8/2/16
// =========================================================================

#ifndef IMAGEFUNCS_H
#define IMAGEFUNCS_H

#include <fstream>
#include <Magick++.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "math/twovector.h"

class dataarray;
class linesegment;
class frustum;
class genmatrix;
class myimage;
class parallelepiped;
class parallelogram;
class polygon;
class prob_distribution;
class rectangle;
class threevector;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace imagefunc
{

// Metafile manipulation methods:

   void maximize_pixel_density(
      int nx_max,int ny_max,twoDarray*& ztwoDarray_ptr);
   twoDarray* maximize_pixel_density(
      twoDarray* ztwoDarray_ptr,int nx_max,int ny_max);
   void add_secret_labels(
      std::ofstream& imagestream,twoDarray const *ztwoDarray_ptr);

// Image cropping methods:

   void crop_image(twoDarray *ztwoDarray_ptr);

   void crop_image(std::string filename,int width,int height);
   void crop_image(
      std::string filename,int width,int height,int xoffset,int yoffset);
   void crop_image(
      std::string input_filename,std::string output_filename,
      int width, int height, int xoffset, int yoffset);

   void extract_subimage(
      std::string input_filename,std::string output_filename,
      int width,int height,int xoffset,int yoffset);
   void extract_subimage(
      double Ulo,double Uhi,double Vlo,double Vhi,
      std::string input_filename,std::string output_filename);
   void recolor_image(
      std::string filename,genmatrix* RGB_transform_ptr);
   void crop_and_recolor_image(
      std::string filename,int width,int height,int xoffset,int yoffset,
      genmatrix* RGB_transform_ptr);
   std::string convert_image_to_pgm(std::string filename);

// Geometrical primitives manipulation methods:

   void find_null_border_along_ray(
      double intensity_integral_frac,twoDarray const *ztwoDarray_ptr,
      const linesegment& l,threevector& svec,
      bool integrate_binary_image=true);
   double line_integral_along_segment(
      twoDarray const *ztwoDarray_ptr,const linesegment& l);
   double fast_line_integral_along_segment(
      twoDarray const *ztwoDarray_ptr,const linesegment& l);
   std::pair<int,double> fast_thick_line_integral_along_segment(
      double line_thickness,const double null_value,
      twoDarray const *ztwoDarray_ptr,const linesegment& l);
   void count_bright_and_dark_pixels_along_segment(
      double threshold,int& nbright_pixels,int& ndark_pixels,
      linesegment& l,twoDarray const *ztwoDarray_ptr);
   double surface_integral_inside_polygon(
      twoDarray const *ztwoDarray_ptr,const polygon& p_3D);
   double surface_integral_inside_polygon(
      twoDarray const *ztwoDarray_ptr,twoDarray const *zbinary_twoDarray,
      const polygon& p_3D,double black_pixel_penalty,double& area_integral);
   double surface_integral_inside_n_polygons(
      bool poly_intersection,bool poly_union,unsigned int npolys,
      double zminimum,twoDarray const *ztwoDarray_ptr,const polygon p_3D[],
      double black_pixel_penalty,double& area_integral);
   double n_poly_moment_relative_to_line(
      bool poly_intersection,bool poly_union,int npolys,double zminimum,
      twoDarray const *ztwoDarray_ptr,int moment_order,
      const linesegment& l,polygon p_2D[],
      double black_pixel_penalty,double& area_integral);

// Intensity distribution methods:

   bool intensity_distribution_inside_bbox(
      double minimum_x,double minimum_y,
      double maximum_x,double maximum_y,
      twoDarray const *ztwoDarray_ptr,double zmin,prob_distribution& prob);
   bool intensity_distribution_inside_convex_quadrilateral(
      double zmin,const polygon& quadrilateral,
      twoDarray const *ztwoDarray_ptr,twoDarray *zmask_twoDarray_ptr,
      prob_distribution& prob);
   bool intensity_distribution_inside_convex_quadrilateral(
      double zmin,const polygon& quadrilateral,
      twoDarray const *ztwoDarray_ptr,twoDarray *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      prob_distribution& prob);
   bool intensity_distribution_inside_poly(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double zmin,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      prob_distribution& prob);
   bool intensity_distribution_inside_poly(
      twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon& p_3D,prob_distribution& prob);
   void intensity_distribution_outside_poly(
      twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon& p_3D,prob_distribution& prob);
   void intensity_distribution_inside_n_polys(
      bool poly_intersection,bool poly_union,
      int npolys,twoDarray const *ztwoDarray_ptr,double zmin,
      polygon p_2D[],prob_distribution& prob);
   void intensity_distribution_outside_n_polys(
      unsigned int npolys,twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon p_3D[],prob_distribution& prob);
   bool isolated_region(
      const threevector& center,int min_nonzero_pixels,
      double rmin,double rmax,double& r_isolate,
      twoDarray const *ztwoDarray_ptr);

// Global thresholding methods:

   double min_intensity_above_floor(
      double intensity_floor,twoDarray const *ztwoDarray_ptr);
   double max_intensity_below_ceiling(
      double intensity_ceiling,twoDarray const *ztwoDarray_ptr);
   void threshold_intensities_above_cutoff(
      twoDarray *ztwoDarray_ptr,double cutoff_intensity,double znull=0);
   void threshold_intensities_below_cutoff(
      twoDarray *ztwoDarray_ptr,double cutoff_intensity,double znull=0);
   void threshold_intensities_below_cutoff_frac(
      twoDarray *ztwoDarray_ptr,double cutoff_frac);
   void threshold_intensities_above_cutoff_frac(
      double zmin,twoDarray* ztwoDarray_ptr,
      double cutoff_frac,double znull=0);
   void particular_cutoff_threshold(
      double z_threshold,twoDarray* ztwoDarray_ptr,double znull=0);
   void threshold_intensities_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double hot_threshold,twoDarray* ztwoDarray_ptr);
   void threshold_intensities_inside_poly(
      polygon& poly,double hot_threshold,twoDarray* ztwoDarray_ptr);
   void reset_values_using_another_image(
      const twoDarray* ftwoDarray_ptr,twoDarray* ztwoDarray_ptr,
      double input_f_value,double output_z_value);

// Local thresholding methods:

   void compute_local_thresholds(
      unsigned int n_threshold_centers,double local_threshold_radius,
      double threshold_frac,threevector threshold_center[],
      double local_threshold[],twoDarray const *ztwoDarray_ptr);
   void compute_local_thresholds(
      unsigned int n_threshold_centers,double local_threshold_radius[],
      double threshold_fraction[],threevector threshold_center[],
      double local_threshold[],twoDarray const *ztwoDarray_ptr);
   void differentially_threshold(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      twoDarray* ztwoDarray_ptr);
   void differentially_threshold(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      twoDarray* zthreshold_twoDarray_ptr,twoDarray* ztwoDarray_ptr);
   double fast_threshold_field(
      double sqr_correlation_length,double curr_x,double curr_y,
      const std::vector<twovector>& centers_posn,
      const std::vector<double>& threshold);
   double fast_threshold_field(
      int counter,double sqr_correlation_length,
      std::vector<std::pair<double,int> >* rsq_ptr,
      double curr_x,double curr_y,
      const std::vector<twovector>& centers_posn,
      const std::vector<double>& threshold);
   double threshold_field(
      bool exp_flag,unsigned int ncenters,
      const threevector& curr_posn,const threevector centers_posn[],
      double correlation_length,double threshold[]);
   double verbose_threshold_field(
      bool exp_flag,int ncenters,
      const threevector& curr_posn,const threevector centers_posn[],
      double correlation_length,double threshold[]);
   void differentially_threshold_inside_bbox(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* zthresh_twoDarray_ptr);

// Image size methods:

   bool valid_image_file(std::string image_filename);
   bool corrupted_jpg_file(std::string jpg_filename);
   bool get_image_width_height(
      std::string image_filename,int& width,int& height);
   bool get_image_width_height(
      std::string image_filename,unsigned int& width,unsigned int& height);
   bool get_image_properties(
      std::string image_filename,unsigned int& width,unsigned int& height,
      std::string& format,unsigned int& filesize);
   
// Image properties methods:

   int count_pixels_below_zmax(
      double zmax,twoDarray const *ztwoDarray_ptr);
   int count_pixels_above_zmin(
      double zmin,twoDarray const *ztwoDarray_ptr);
   int count_pixels_in_z_interval(
      double zmin,double zmax,twoDarray const *ztwoDarray_ptr);
   int count_pixels_above_zmin_within_halfplane(
      const threevector& basepoint,const threevector& nhat,
      double zmin,twoDarray const *ztwoDarray_ptr);
   int count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr);
   int count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr,
      int& npixels_inside_bbox);
   int strict_count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr,int& npixels_inside_bbox);
   int count_pixels_above_zmin_outside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr);
   int count_pixels_below_zmax_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmax,twoDarray const *ztwoDarray_ptr,
      int& npixels_inside_bbox);
   int count_pixels_above_zmin_inside_poly(
      double zmin,polygon& poly,twoDarray const *ztwoDarray_ptr);
   int count_pixels_below_zmax_inside_poly(
      double zmax,polygon& poly,twoDarray const *ztwoDarray_ptr);
   int count_pixels_below_zmax_inside_parallelogram(
      double zmax,parallelogram& parallelogram,
      twoDarray const *ztwoDarray_ptr) ;
   double energy(twoDarray const *ztwoDarray_ptr);
   double integrated_intensity(twoDarray const *ztwoDarray_ptr);
   double integrated_intensity(double zmin,twoDarray const *ztwoDarray_ptr);
   double integrated_intensity_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray const *ztwoDarray_ptr);
   void integrated_intensity_inside_poly(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double zmin,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      double& intensity_integral);
   double count_nonzero_pixels_inside_triangle(
      double zmin,const threevector& vertex_0,const threevector& vertex_1,
      const threevector& vertex_2,const twoDarray* ztwoDarray_ptr,
      int& npixels_in_triangle,int& npixels_above_zmin_in_triangle);
   double count_nonzero_pixels_inside_convex_quadrilateral(
      double zmin,const polygon& q,twoDarray const *ztwoDarray_ptr,
      int& npixels_in_quad,int& npixels_above_zmin_in_quad);
   void nonzero_pixels_inside_poly_zones(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      unsigned int nzones,double zone_value[],double zmin,
      int delta_px,int delta_py,
      twoDarray const *zmask_twoDarray_ptr,twoDarray const *ztwoDarray_ptr,
      int npixels_in_zone[],int npixels_above_zmin_in_zone[]);
   void light_nonzero_pixels_inside_poly(
      double zmin,twoDarray *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr);
   int image_intensity_distribution(
      double zmin,twoDarray const *ztwoDarray_ptr,prob_distribution& prob,
      int n_output_bins=30);
   prob_distribution* image_intensity_distribution(
      double zmin,twoDarray const *ztwoDarray_ptr,int& npixels_above_zmin,
      int n_output_bins=30);

// Center of mass and moment of intertia methods:

   bool center_of_mass(twoDarray const *ztwoDarray_ptr,threevector& COM);
   bool center_of_mass(
      double minimum_x,double maximum_x,double minimum_y,double maximum_y,
      twoDarray const *ztwoDarray_ptr,threevector& COM);
   bool center_of_mass(
      unsigned int min_px,unsigned int max_px,
      unsigned int min_py,unsigned int max_py,
      twoDarray const *ztwoDarray_ptr,threevector& COM);
   bool binary_COM_above_zmin(
      double minimum_x,double maximum_x,double minimum_y,double maximum_y,
      double z_min,twoDarray const *ztwoDarray_ptr,twovector& COM);
   bool binary_COM_above_zmin(
      unsigned int min_px,unsigned int max_px,
      unsigned int min_py,unsigned int max_py,double z_min,
      twoDarray const *ztwoDarray_ptr,twovector& COM);

   void moment_of_inertia(
      const threevector& origin,double& Imin,double& Imax,
      twoDarray const *ztwoDarray_ptr);
   void moment_of_inertia(
      const threevector& origin,
      double& Ixx,double& Iyy,double& Ixy,double& Imin,double& Imax,
      twoDarray const *ztwoDarray_ptr);
   void moment_of_inertia(
      const threevector& origin,double& Imin,double& Imax,
      threevector& Imin_hat,threevector& Imax_hat,
      twoDarray const *ztwoDarray_ptr);
   bool moment_of_inertia(
      const threevector& center_pnt,const threevector& a_hat,
      twoDarray const *ztwoDarray_ptr,double& I);

// Image processing methods:

   void compute_pixel_borders(
      twoDarray const *ztwoDarray_ptr,
      unsigned int& px_min,unsigned int& px_max,
      unsigned int& py_min,unsigned int& py_max);

   double integrate_absolute_difference_image(
      double z_threshold,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr);
   double integrate_absolute_difference_image(
      double z_threshold,
      unsigned int bbox_px_min,unsigned int bbox_px_max,
      unsigned int bbox_py_min,unsigned int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr);
   double integrate_absolute_difference_image(
      double z_threshold,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py);
   double integrate_absolute_difference_image(
      double z_threshold,
      unsigned int bbox_px_min,unsigned int bbox_px_max,
      unsigned int bbox_py_min,unsigned int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py);
   double integrate_absolute_difference_image(
      bool binary_threshold_ztwoDarray1,double z_threshold,
      unsigned int bbox_px_min,unsigned int bbox_px_max,
      unsigned int bbox_py_min,unsigned int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py);

// Median filtering methods:

   void median_filter(unsigned int nsize,twoDarray *ztwoDarray_ptr);
   void median_filter(unsigned int nx_size,unsigned int ny_size,
                      twoDarray *ztwoDarray_ptr);
   void median_filter(unsigned int nsize,twoDarray const *ztwoDarray_ptr,
                      twoDarray *ztwoDarray_filtered_ptr);
   void median_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,
      double irrelevant_intensity=POSITIVEINFINITY,
      bool ignore_z_greater_than_irrelevant_intensity=true);
   bool median_filter(
      unsigned int px,unsigned int py,unsigned int nsize,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* ztwoDarray_filtered_ptr,double minimum_z_value);

   void fast_median_filter(
      unsigned int n_intensity_bins, int nsize,  
      twoDarray const *ztwoDarray_ptr, twoDarray *ztwoDarray_filtered_ptr);
   void fast_percentile_filter(
      unsigned int n_intensity_bins, 
      unsigned int nx_size, unsigned int ny_size, double cum_frac,
      twoDarray const *ztwoDarray_ptr, twoDarray *ztwoDarray_filtered_ptr);

   void probability_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double prob_frac,
      double irrelevant_intensity);
   void average_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double irrelevant_intensity);

   void average_nonnull_values(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value);

   void median_fill(
      unsigned int nsize,twoDarray *ztwoDarray_ptr,double null_value);
   void median_fill(
      unsigned int nsize,polygon& bbox,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value);
   void median_fill(
      unsigned int nsize,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value);
   int median_clean(
      unsigned int nsize,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value,
      double max_deviation_ratio);

// Numerical image intensity differentiation methods:

   void brute_twoD_convolve(
      genmatrix* filter_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr);
   double brute_twoD_convolve(
      unsigned int px,unsigned int py,genmatrix* filter_ptr,
      twoDarray const *ztwoDarray_ptr);
   void horiz_derivative_filter(
      unsigned int nsize,const double filter[],twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double dx=1.0,
      double null_value=NEGATIVEINFINITY);
   void vert_derivative_filter(
      unsigned int nsize,const double filter[],twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double dy=1.0,
      double null_value=NEGATIVEINFINITY);

   double horiz_derivative_filter(
      unsigned int px,unsigned int py,unsigned int nsize,const double filter[],
      twoDarray const *ztwoDarray_ptr,double null_value,double dx=1.0);
   double vert_derivative_filter(
      unsigned int px,unsigned int py,unsigned int nsize,const double filter[],
      twoDarray const *ztwoDarray_ptr,double null_value,double dy=1.0);

   void compute_x_y_deriv_fields(
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      double magnification_factor=1,int deriv_order=1);
   void compute_x_y_deriv_fields(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      int deriv_order=1);
   void compute_sobel_gradients(
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr);

   void compute_gradient_magnitude_field(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr);
   void laplacian_field(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray *zlaplacian_twoDarray_ptr);
   void remove_horiz_vert_gradients(
      double crit_angle,twoDarray* xderiv_twoDarray_ptr,
      twoDarray* yderiv_twoDarray_ptr);
   void remove_horiz_vert_gradients(
      double crit_angle,double minimum_x,double minimum_y,
      double maximum_x,double maximum_y,twoDarray* xderiv_twoDarray_ptr,
      twoDarray* yderiv_twoDarray_ptr);
   void draw_gradient_dir_field(
      double zthreshold,twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray* gradient_dir_twoDarray_ptr);

   std::map<int,int> compute_gradient_steps(
      double grad_mag_threshold,double step_distance,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr);

   void compute_gradient_phase_field(
      double zmag_min_threshold,double zmag_max_threshold,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray* gradient_phase_twoDarray_ptr,
      double null_value=NEGATIVEINFINITY);
   void compute_local_pixel_intensity_variation(
      int filter_pixel_size,const twoDarray* ztwoDarray_ptr,
      const twoDarray* zbinary_twoDarray_ptr,
      twoDarray* pseudo_grad_twoDarray_ptr,
      bool convert_phase_to_canonical_intervals=false);
   double edge_gradient(
      double correlation_length,const linesegment& curr_edge,double theta,
      twoDarray const *ztwoDarray_ptr);

// Polar image methods:

   twoDarray* convert_intensities_to_polar_coords(
      twoDarray const *ztwoDarray_ptr);
   myimage* generate_polar_image(
      std::string imagedir,std::string colortablefilename,
      twoDarray *ztwoDarray_polar_ptr);

// Profiling methods:

   void invert_vertical_profile(unsigned int nbins,double vert_profile[]);
   void find_horiz_vert_profile_edges(
      double edge_fraction_of_median,
      int jmax,int kmax,int px_min,int px_max,int py_min,int py_max,
      double horiz_profile_median,double vert_profile_median,
      double horiz_profile[],double vert_profile[],
      twoDarray const *ztwoDarray_ptr,double& minimum_x,double& maximum_x,
      double& minimum_y,double& maximum_y);
   twoDarray* prepare_profile_twoDarray(
      unsigned int nbins,double xmin,double xmax,const double profile_array[]);
   void prepare_profile_twoDarray(
      int curr_row,unsigned int nbins,double xmin,double xmax,
      const double profile_row[],twoDarray* profile_twoDarray_ptr);
   dataarray* prepare_profile_dataarray(
      twoDarray const *profile_twoDarray_ptr,
      std::string title,std::string xlabel,std::string ylabel,
      std::string imagedir,double xtic);
   void plot_profile(dataarray *profile_dataarray_ptr);
   void plot_profile(unsigned int nextralines,std::string extraline[],
                     dataarray *profile_dataarray_ptr);

// Erosion/dilation methods:

   void erode(unsigned int niters,unsigned int nsize,double znull,
              twoDarray const *ztwoDarray_ptr,
              twoDarray* zbuf_twoDarray_ptr,twoDarray* zerode_twoDarray_ptr);
   void erode(unsigned int niters,unsigned int nsize,double znull,
              unsigned int min_px,unsigned int min_py,
              unsigned int max_px,unsigned int max_py,
              twoDarray const *ztwoDarray_ptr,
              twoDarray* zbuf_twoDarray_ptr,twoDarray* zerode_twoDarray_ptr);
   void dilate(int niters,int nsize,twoDarray const *ztwoDarray_ptr,
               twoDarray* zbuf_twoDarray_ptr,twoDarray* zerode_twoDarray_ptr);

// Histogram equalization methods:

   void equalize_intensity_histogram(twoDarray* ztwoDarray_ptr);

// ImageMagick text annotation methods:

   std::string get_gravity_location(bool randomize_gravity_flag);
   Magick::GravityType get_gravity_type(bool randomize_gravity_flag);

   void add_text_to_image(
      std::string text_color,std::string caption,
      std::string eastwest_location,std::string northsouth_location,
      std::string input_image_filename,std::string annotated_image_filename);

   void generate_text_via_ImageMagick(
      colorfunc::RGB& foreground_RGB, colorfunc::RGBA& background_RGBA,
      std::string font_path,int point_size,int img_width,int img_height,
      std::string label,std::string output_image_filename);
   void generate_text_via_ImageMagick(
      bool debug_annotate_flag, bool randomize_gravity_flag,
      colorfunc::RGB& foreground_RGB, colorfunc::RGB& stroke_RGB,
      colorfunc::RGBA& background_RGBA,
      bool underbox_flag, colorfunc::RGB& undercolor_RGB,      
      double strokewidth, std::string font_path,
      int point_size,int img_width,int img_height,
      std::string label,std::string output_image_filename,
      bool drop_shadow_flag);

   std::string generate_ImageMagick_text_convert_cmd(
      bool debug_annotate_flag, bool randomize_gravity_flag,
      int foreground_R, int foreground_G, int foreground_B,
      int stroke_R, int stroke_G, int stroke_B,
      int background_R, int background_G, int background_B, int background_A,
      bool underbox_flag, 
      int undercolor_R, int undercolor_G, int undercolor_B,
      double strokewidth, std::string font_path,
      int point_size,int img_width,int img_height,std::string label,
      bool drop_shadow_flag);
   void generate_ImageMagick_3D_variegated_text(
      std::string input_image_filename, std::string output_image_filename,
      int shade_x, int shade_y);

   void pad_image(unsigned int padded_width, unsigned int padded_height,
                  std::string imagechip_filename, 
                  std::string padded_images_subdir);
   Magick::Image* pad_image(
      unsigned int padded_width, unsigned int padded_height,
      const Magick::Image& input_image);

   void generate_multicolored_chars_via_ImageMagick(
      bool randomize_gravity_flag, std::string background_color,
      std::vector<std::string>& foreground_char_colors,
      double strokewidth, int stroke_R, int stroke_G, int stroke_B,
      double shade_az, double shade_el, 
      std::string font_path,int point_size,int width,int height,
      std::string label, std::string output_image_filename);

// Integral image methods:

   void compute_integral_image(
      const twoDarray* ztwoDarray_ptr, twoDarray* zinteg_twoDarray_ptr);
   double bbox_intensity_integral(
      int px_lo, int px_hi, int py_lo, int py_hi,
      const twoDarray* zinteg_twoDarray_ptr);
   void compute_gradients_integral_image(
      const twoDarray* ztwoDarray_ptr, twoDarray* integ_grads_twoDarray_ptr);
}

#endif // imagefuncs.h



