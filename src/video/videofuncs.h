// ==========================================================================
// Header file for videofunc namespace
// ==========================================================================
// Last modified on 1/6/16; 2/14/16; 3/13/16; 3/14/16
// ==========================================================================

#ifndef VIDEOFUNCS_H
#define VIDEOFUNCS_H

#include <Magick++.h>
#include <map>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "astro_geo/geopoint.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

class Clock;
class extremal_region;
class gis_database;
class homography;
class Messenger;
class node;
class pa_struct;
class texture_rectangle;

namespace videofunc
{

// RGB methods:

   void demosaic(
      unsigned char* grayImg, unsigned char* rgbImg, int width, int height);
   void gain(unsigned char* rgbImg, int width, int height);
   void RGB_to_grey(
      unsigned char* rgbImg, unsigned char* greyImg, int width, int height);
   void RGB_to_newRGB(
      unsigned char* rgbImg, unsigned char* newrgbImg, int width, int height);
   bool compute_average_subimage_RGB(
      texture_rectangle* tr_ptr, 
      int px_min, int px_max, int py_min, int py_max,
      colorfunc::RGB& average_RGB);

// Jason Cardema's (Group 104) IRIG timing methods:

   double SinceMidnight(pa_struct& x);
   void irig2utc(char timestr[], int irig_sec, int irig_usec);
   void irig2date(char datestr[],int irig_day);
   
// ALIRT imagery generation methods:

//   void set_gross_sensor_cartesian_offsets(threevector& sensor_offset);
   void filter_raw_sensor_positions(
      int n_frames,double dt,double tau,
      std::vector<threevector>& sensor_position);

// KLT testing methods:

   void translate_image(
      unsigned char* inputImg, unsigned char* outputImg,int width, int height,
      int pixel_translation);

// Thumbnail generation methods:

   std::string get_thumbnail_filename(
      std::string input_filename,std::string thumbnail_prefix="");
   void get_thumbnail_dims(
      unsigned int xdim,unsigned int ydim,
      unsigned int& thumbnail_xdim,unsigned int& thumbnail_ydim);
   std::string generate_thumbnail(
      std::string input_image_filename,double zoom_factor,
      std::string thumbnail_prefix="");
   std::string generate_thumbnail(
      std::string input_image_filename,
      unsigned int orig_xdim,unsigned int orig_ydim,
      unsigned int new_xdim,unsigned int new_ydim);
   bool resize_image(
      std::string input_image_filename,
      unsigned int orig_xdim,unsigned int orig_ydim,
      unsigned int new_xdim,unsigned int new_ydim,
      std::string resized_image_filename);
   void resize_image(
      Magick::Image& curr_image,unsigned int new_xdim,unsigned int new_ydim);
   void gaussian_blur_image(Magick::Image& curr_image,double sigma);

   void downsize_image(
      std::string input_image_filename,
      unsigned int max_xdim,unsigned int max_ydim,
      std::string downsized_image_filename);
   void downsize_image(
      std::string input_image_filename,
      unsigned int max_xdim,unsigned int max_ydim,
      std::string downsized_image_filename,
      unsigned int& new_xdim,unsigned int& new_ydim);
   void downsize_image(
      std::string image_filename,unsigned int max_xdim,unsigned int max_ydim,
      unsigned int xdim, unsigned int ydim, 
      std::string downsized_image_filename,
      unsigned int& new_xdim,unsigned int& new_ydim);
   void downsize_image(
      std::string input_image_filename,
      unsigned int max_xdim,unsigned int max_ydim);
   void force_size_image(
      std::string image_filename,
      unsigned int output_xdim,unsigned int output_ydim,
      std::string resized_image_filename);
   void highpass_filter_image(
      std::string image_filename,std::string filtered_image_filename);

// Bundler methods:

   typedef std::map<int,std::pair<threevector,std::vector<int> > > XYZ_MAP;
   extern XYZ_MAP* xyz_map_ptr;

// CAMERAID_XYZ_MAP associates a camera's ID to the XYZ points visible
// to the camera:

   typedef std::map<int,std::vector<threevector> > CAMERAID_XYZ_MAP;
   extern CAMERAID_XYZ_MAP* cameraid_xyz_map_ptr;

   void import_photoID_XYZID_UV_bundler_pairs(
      std::string camera_views_filename,std::vector<int>& photo_ID,
      std::vector<int>& XYZ_ID,std::vector<twovector>& UV_sift);
   void import_reconstructed_XYZ_points(std::string xyz_points_filename);
   CAMERAID_XYZ_MAP* sort_XYZ_points_by_camera_ID();

// Photograph input/output methods:

   std::string get_next_photo_filename(
      const std::vector<std::string>& photo_filename_substrings,
      std::string separator_char,int next_imagenumber);
   std::string find_min_max_photo_numbers(
      std::string subdir_name,int& min_photo_number,int& max_photo_number);
   std::vector<std::string> find_numbered_image_filenames(
      std::string subdir);

// ActiveMQ message methods:

   void broadcast_video_params(
      Messenger* messenger_ptr,int start_imagenumber,int stop_imagenumber,
      int n_images);
   void broadcast_current_image_URL(
      Messenger* messenger_ptr,int n_frames,std::string URL,int npx,int npy);
   void broadcast_selected_image(
      Messenger* messenger_ptr,int image_ID,std::string URL,
      int npx,int npy);

// Orthorectification methods
      
   void compute_image_corner_world_coords(
      std::vector<twovector>& XY,std::vector<twovector>& UV,double Umax,
      homography& H,twovector& lower_left_XY,twovector& lower_right_XY,
      twovector& upper_right_XY,twovector& upper_left_XY);
   void compute_extremal_easting_northing(
      const twovector& lower_left_XY,const twovector& lower_right_XY,
      const twovector& upper_right_XY,const twovector& upper_left_XY,
      int xdim,int ydim,int& new_xdim,int& new_ydim,
      double& min_easting,double& max_easting,
      double& min_northing,double& max_northing);

// Blank image generation methods

   std::string generate_blank_jpgfile(
      int n_horiz_pixels,int n_vertical_pixels);
   std::string generate_blank_pngfile(
      int n_horiz_pixels,int n_vertical_pixels);
   void generate_blank_imagefile(
      int n_horiz_pixels,int n_vertical_pixels,std::string blank_filename);

// Greyscale PNG export methods

   void write_8bit_greyscale_pngfile(
      std::vector<std::vector<unsigned char> >& byte_array, 
      std::string output_filename);

// AVI movie generation methods

   std::string generate_AVI_movie(
      std::string video_codec,std::string input_imagery_subdir,
      std::string image_suffix,double fps,
      std::string output_movie_filename_prefix,
      std::string finished_movie_subdir);
   std::string generate_FLIR_AVI_movie(
      std::string input_imagery_subdir,std::string image_suffix,
      int& AVI_movie_counter,std::string output_movie_filename_prefix,
      std::string finished_movie_subdir,
      double start_time,double stop_time,const std::vector<double>& epoch_time,
      const std::vector<std::string>& filename_stem);

// Color histogram methods

   std::vector<double> generate_color_histogram(
      bool generate_quantized_color_image_flag,
      texture_rectangle* texture_rectangle_ptr,
      std::string& quantized_color_image_filename);
   bool quantize_image_coloring(
      bool generate_quantized_color_image_flag,
      texture_rectangle* texture_rectangle_ptr,
      std::vector<double>& color_bins,int& n_pixels,
      std::string& quantized_color_image_filename);
   void filter_color_image(
      twoDarray* HtwoDarray_ptr,twoDarray* StwoDarray_ptr,
      twoDarray* VtwoDarray_ptr,
      const std::vector<int>& masked_color_bin_numbers,
      texture_rectangle* texture_rectangle_ptr,
      std::string filtered_image_filename);

   void compute_RGB_fluctuations(
      const std::vector<int>& RLE_pixel_IDs,
      const texture_rectangle* texture_rectangle_ptr,
      double& quartile_width_R,double& quartile_width_G,
      double& quartile_width_B);

// Line segment detection methods

   std::vector<linesegment> detect_line_segments(
      texture_rectangle* texture_rectangle_ptr);

// Texture rectangle drawing methods

   void draw_solid_squares(
      const std::vector<twovector>& square_centers,int square_length_in_pixels,
      texture_rectangle* texture_rectangle_ptr,int color_index=-1);
   void draw_line_segments(
      const std::vector<linesegment>& line_segments,
      texture_rectangle* texture_rectangle_ptr,int segment_color_index=-1);
   void display_circles(
      texture_rectangle* texture_rectangle_ptr,
      std::string human_faces_image_filename,
      const std::vector<twovector>& center,const std::vector<double>& radius);
   void display_bboxes(
      const std::vector<fourvector>& bbox_params,
      texture_rectangle* texture_rectangle_ptr,
      int bbox_color_index,int line_thickness);
   void display_bboxes(
      const std::vector<bounding_box>& bboxes,
      texture_rectangle* texture_rectangle_ptr,
      int bbox_color_index,int line_thickness);
   void display_polygon(
      polygon& curr_polygon,texture_rectangle* texture_rectangle_ptr,
      int contour_color_index,int line_thickness);
   void display_polygons(
      const std::vector<polygon>& polygons,
      texture_rectangle* texture_rectangle_ptr,
      int contour_color_index,int line_thickness);
   polygon generate_ellipse_polygon(
      double a,double b,double c,double px_center,double py_center,
      texture_rectangle* texture_rectangle_ptr);
   void draw_ellipse(
      double a,double b,double c,double px_center,double py_center,
      texture_rectangle* texture_rectangle_ptr,
      int color_index,int line_thickness);

// Text annotation methods

   void annotate_image_with_text(
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* text_texture_rectangle_ptr,int fontsize,
      std::vector<std::string>& text_lines,
      std::vector<twovector>& xy_start,
      std::vector<colorfunc::Color>& text_colors);

// Cross correlation methods

   double image_pair_cross_correlation(
      texture_rectangle* texure_rectangle1_ptr,
      texture_rectangle* texure_rectangle2_ptr);
   double image_pair_cross_correlation(
      const std::vector<double>& red1_values,
      const std::vector<double>& green1_values,
      const std::vector<double>& blue1_values,
      const std::vector<double>& red2_values,
      const std::vector<double>& green2_values,
      const std::vector<double>& blue2_values,
      double mu1_R,double mu1_G,double mu1_B,
      double sigma1_R,double sigma1_G,double sigma1_B,
      double mu2_R,double mu2_G,double mu2_B,
      double sigma2_R,double sigma2_G,double sigma2_B);

   void fill_YCbCr_arrays(
      texture_rectangle* texture_rectangle_ptr, 
      double& mu_Y, double& mu_Cb, double& mu_Cr,
      flann::Matrix<float> *Y_matrix_ptr, 
      flann::Matrix<float> *Cb_matrix_ptr,flann::Matrix<float> *Cr_matrix_ptr);

// Discrete cosine transform methods (deprecated as of 1/4/15)

   void generate_cosine_array(int chip_width, int chip_height, 
                              std::vector<double>& cosine_array);
   void compute_DCT(
      int width, int height, const std::vector<double>& cosine_array,
      const flann::Matrix<float> *matrix_ptr,
      flann::Matrix<float> *dct_matrix_ptr);
   void compute_multi_DCT(
      int width, int height, const std::vector<double>& cosine_array,
      const flann::Matrix<float> *matrix1_ptr,
      const flann::Matrix<float> *matrix2_ptr,
      const flann::Matrix<float> *matrix3_ptr,
      flann::Matrix<float> *dct_matrix1_ptr,
      flann::Matrix<float> *dct_matrix2_ptr,
      flann::Matrix<float> *dct_matrix3_ptr);

   void compute_inverse_DCT(
      int width, int height, const std::vector<double>& cosine_array,
      double max_spatial_frequency,
      const flann::Matrix<float> *dct_matrix_ptr,
      flann::Matrix<float> *matrix_ptr);
   void compute_multi_inverse_DCT(
      int width, int height, const std::vector<double>& cosine_array,
      double max_spatial_frequency,
      const flann::Matrix<float> *dct_matrix2_ptr,
      const flann::Matrix<float> *dct_matrix3_ptr,
      flann::Matrix<float> *matrix2_ptr,
      flann::Matrix<float> *matrix3_ptr);

   std::vector<std::vector<unsigned char> > generate_byte_array(
      const texture_rectangle* mask_tr_ptr,
      const Magick::Image* IM_mask_ptr,
      unsigned int px_start, unsigned int px_stop,
      unsigned int py_start, unsigned int py_stop, bool visualize_mask_flag);
   std::vector<std::vector<unsigned char> > generate_byte_array(
      std::string png_filename,
      unsigned int px_start, unsigned int px_stop,
      unsigned int py_start, unsigned int py_stop, bool visualize_mask_flag,
      int nontrivial_pixel_value);
      
// ==========================================================================
// Inlined methods:
// ==========================================================================

   template <class T> inline T CLIP(T x,T y,T z)
   {
      return basic_math::max(y,basic_math::min(x,z));
   }
    
} // videofunc namespace

#endif // videofuncs.h

