// ========================================================================
// texture_rectangle class provides functionality for displaying
// images and videos within OSG TextureRectangles.
// ========================================================================
// Last updated on 8/1/16; 8/5/16; 8/9/16; 8/28/16
// ========================================================================

#ifndef TEXTURE_RECTANGLE_H
#define TEXTURE_RECTANGLE_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Array>
#include <osgSim/ColorRange>
#include <osg/Image>
#include <osg/Geometry>
#include <osgDB/Registry>
#include <osg/TextureRectangle>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "datastructures/Quadruple.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"
#include "video/VidFile.h"

#include "ffmpeg/ReferenceCountedArray.h"

class AnimationController;
class bounding_box;
class ColorMap;
class extremal_region;     
class FFMPEGVideo;

typedef Quadruple<twoDarray*,twoDarray*,twoDarray*,twoDarray*> RGBA_array;

class texture_rectangle
{   

  public:

   enum VideoType
   {
      still_image=0,video,G99Vid,unknown
   };

// Initialization, constructor and destructor member functions:

   texture_rectangle();
   texture_rectangle(int width,int height,int n_images,int n_channels,
                     AnimationController* AC_ptr);
   texture_rectangle(std::string filename,AnimationController* AC_ptr);
   ~texture_rectangle();
   friend std::ostream& operator<< (
      std::ostream& outstream,const texture_rectangle& t);

// Set & get member functions:

   void set_video_filename(std::string filename);
   std::string get_video_filename();
   const std::string& get_video_filename() const;
   VideoType get_VideoType() const;
   AnimationController* get_AnimationController_ptr();
   const AnimationController* get_AnimationController_ptr() const;

   int get_image_size_in_bytes() const;
   VidFile* get_VidFile_ptr();
   const VidFile* get_VidFile_ptr() const;

   void set_m_image_ptr(unsigned char *m_ptr);
   unsigned char* get_m_image_ptr();
   const unsigned char* get_m_image_ptr() const;
   unsigned char* get_m_colorimage_ptr();
   const unsigned char* get_m_colorimage_ptr() const;

   void reset_UV_coords(double min_U,double max_U,double min_V,double max_V);
   double get_minU() const;
   double get_minV() const;
   double get_maxU() const;
   double get_maxV() const;
   double get_dU() const;
   double get_dV() const;
   threevector get_midpoint() const;

   int get_imagenumber() const;
   void set_first_imagenumber(int i);
   int get_first_imagenumber() const;
   int get_last_imagenumber() const;
   void set_panel_number(int p);

   void setWidth(unsigned int width);
   unsigned int getWidth() const;
   void setHeight(unsigned int height);
   unsigned int getHeight() const;
   unsigned int get_next_width() const;
   unsigned int get_next_height() const;

   unsigned int getNchannels() const;
   ColorMap* get_ColorMap_ptr();
   const ColorMap* get_ColorMap_ptr() const;
   osg::Image* get_image_ptr();
   const osg::Image* get_image_ptr() const;
   osg::TextureRectangle* get_TextureRectangle_ptr();
   const osg::TextureRectangle* get_TextureRectangle_ptr() const;

   std::string& get_output_image_string();
   const std::string& get_output_image_string() const;

   RGBA_array get_RGBA_twoDarrays(bool include_alpha_channel_flag=true);
   void set_from_RGB_twoDarrays(RGBA_array& rgba_array);

   twoDarray* get_ptwoDarray_ptr();
   const twoDarray* get_ptwoDarray_ptr() const;
   twoDarray* instantiate_ptwoDarray_ptr();
   twoDarray* refresh_ptwoDarray_ptr();
   twoDarray* refresh_ptwoDarray_ptr(int channel_ID);
   void reset_ptwoDarray_ptr(twoDarray* qtwoDarray_ptr);
   twoDarray* export_sub_twoDarray(
      unsigned int pu_start,unsigned int pu_stop,
      unsigned int pv_start,unsigned int pv_stop);

// Video initialization member functions:

   bool import_photo_from_file(std::string photo_filename);
   bool fast_import_photo_from_file(std::string photo_filename);
   void initialize_G99_video();
   void initialize_ntf_image();
   void initialize_twoDarray_image(
      const twoDarray* ptwoDarray_ptr,
      int n_channels=4,bool blank_png_flag=false);
   void initialize_RGB_twoDarray_image(const twoDarray* RtwoDarray_ptr);
   void initialize_general_image(int width, int height);
   void fill_twoDarray_image(
      const twoDarray* ptwoDarray_ptr,unsigned int n_channels,
      bool blank_png_flag=false);
   void initialize_general_video();
   void set_TextureRectangle_image();
   twoDarray* fill_ptwoDarray_from_single_channel_byte_data();

// Stringstream member functions:

   void read_image_from_char_buffer(
      std::string input_image_suffix,const char* buffer_ptr,int image_size);

// Image numbering member functions:

   int get_Nimages() const;	     
   void set_first_frame_to_display(int i);
   void set_last_frame_to_display(int i);
   int get_first_frame_to_display() const;
   int get_last_frame_to_display() const;

// Frame display member functions:
   
   void display_current_frame();
   void displayFrame( int p_framenum );
   void set_image();
   void read_and_set_image();

// Colormap member functions:

   void change_color_map(int map_number=0);

// Pixel coordinates member functions:

   std::pair<int,int> get_pixel_coords(double u,double v) const;
   void get_pixel_coords(
      double u,double v,unsigned int& pu,unsigned int& pv) const;
   void get_pixel_coords_and_fracs(
      double u,double v,unsigned int& pu,unsigned int& pv,
      double& ufrac,double& vfrac) const;
   void get_uv_coords(unsigned int pu,unsigned int pv,double& u,double& v) 
      const;
   std::pair<double,double> get_uv_coords(
      unsigned int pu,unsigned int pv) const;

   void RandomNeighborCoordinates(
      unsigned int px,unsigned int py,
      unsigned int& px_neighbor,unsigned int& py_neighbor) const;
   int lexicographical_order(unsigned int pu,unsigned int pv) const;

// Pixel intensity set & get member functions:

   void set_pixel_intensity_value(unsigned int pu,unsigned int pv,int value);
   int get_pixel_intensity(unsigned int px,unsigned int py) const;
   int get_pixel_intensity_value(unsigned int px,unsigned int py) const;
   int fast_get_pixel_intensity_value(unsigned int px, unsigned int py) const;

   int get_intensity(double u,double v) const;
   void clear_all_intensities();
   std::vector<double> get_pixel_region_intensity_values(
      unsigned int px_lo,unsigned int px_hi,
      unsigned int py_lo,unsigned int py_hi,const unsigned char* data_ptr);
   void get_pixel_region_intensity_moments(
      int px_lo,int px_hi,int py_lo,int py_hi,
      double& mu_intensity,double& sigma_intensity);

// RGB get & set member functions:

   void get_pixel_RGB_values(
      unsigned int pu,unsigned int pv,int& R,int& G,int& B) const;
   void fast_get_pixel_RGB_values(
      unsigned int pu,unsigned int pv,int& R,int& G,int& B) const;
   int fast_get_pixel_R_value(unsigned int pu,unsigned int pv) const;
   int fast_get_pixel_G_value(unsigned int pu,unsigned int pv) const;
   int fast_get_pixel_B_value(unsigned int pu,unsigned int pv) const;

   void get_pixel_row_RGB_values(
      unsigned int pu,unsigned int pv,unsigned int n_pixels_in_rows,
      int* R, int* G, int* B) const;
   void get_RGB_values(double u,double v,int& R,int& G,int& B) const;
   Triple<int,int,int> get_RGB_values(double u,double v) const;

   bool get_pixel_hsv_values(
      unsigned int pu,unsigned int pv,double& h,double& s,double& v) const;
   bool get_pixel_hsva_values(
      unsigned int pu,unsigned int pv,double& h,double& s,double& v, double&a) 
      const;
   void get_hsv_values(
      double U,double V,double& h,double& s,double& v) const;

   void get_pixel_RGBhsv_values(
      unsigned int pu,unsigned int pv,
      int& R,int& G,int& B,double& h,double& s,double& v) const;
   void set_pixel_RGB_values(
      unsigned int pu,unsigned int pv,int R,int G,int B);

   void set_pixel_hue(unsigned int pu,unsigned int pv,double hue);
   void set_pixel_hsv_values(
      unsigned int pu,unsigned int pv,double h,double s,double v);
   void set_pixel_hsva_values(
      unsigned int pu,unsigned int pv,double h,double s,double v, double a);

   void set_RGB_values(double u,double v,int R,int G,int B);
   void set_pixel_RGBA_values(
      unsigned int pu,unsigned int pv,int R,int G,int B,int A);
   void set_pixel_RGBA_values(
      unsigned int px,unsigned int py,unsigned char* data_ptr,
      int R,int G,int B,int A);
   void get_pixel_RGBA_values(
      unsigned int pu,unsigned int pv,int& R,int& G,int& B,int& A) const;

   void get_all_RGB_values(
      std::vector<double>& red_values,std::vector<double>& green_values,
      std::vector<double>& blue_values);
   void get_all_RGB_values(
      const unsigned char* data_ptr,
      std::vector<double>& red_values,std::vector<double>& green_values,
      std::vector<double>& blue_values);

   void copy_RGB_values(texture_rectangle* texture_rectangle_ptr);

// Pixel region RGB member functions:

   void get_pixel_region_RGB_values(
      unsigned int px,unsigned int py,int n_size);
   void get_pixel_region_RGB_values(
      unsigned int px,unsigned int py,int n_size,
      const unsigned char* data_ptr);
   void get_pixel_region_RGB_values(
     unsigned int px_lo,unsigned int px_hi,
     unsigned int py_lo,unsigned int py_hi,
     const unsigned char* data_ptr);
   void get_pixel_region_RGB_moments(
      int px,int py,int n_size,
      twoDarray* mu_R_twoDarray_ptr,
      twoDarray* mu_G_twoDarray_ptr,
      twoDarray* mu_B_twoDarray_ptr,
      twoDarray* sigma_R_twoDarray_ptr,
      twoDarray* sigma_G_twoDarray_ptr,
      twoDarray* sigma_B_twoDarray_ptr,
      double& mu_R,double& mu_G,double& mu_B,
      double& sigma_R,double& sigma_G,double& sigma_B);
   void get_pixel_region_RGB_moments(
     int px_lo,int px_hi,int py_lo,int py_hi,
     double& mu_R,double& mu_G,double& mu_B,
     double& sigma_R,double& sigma_G,double& sigma_B);
   void get_pixel_RGB_means(double& mu_R,double& mu_G,double& mu_B);

// Drawing member functions:

   void fill_circle(int pu,int pv,double radius,colorfunc::Color c);
   void draw_pixel_bbox(const bounding_box& bbox,colorfunc::Color c,
      int thickness = 0);
   void draw_pixel_bbox(const bounding_box& bbox,int R, int G, int B,
      int thickness = 0);
   void draw_pixel_bbox(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      colorfunc::Color c, int thickness = 0);
   void draw_pixel_bbox(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      int R,int G,int B, int thickness = 0);

   void fill_pixel_bbox(
      const bounding_box& bbox,int R, int G, int B);
   void fill_pixel_bbox(
      int px_min, int px_max, int py_min, int py_max,
      int R,int G,int B);
   int count_colored_pixels(int R,int G,int B, double RGB_threshold);
   int count_colored_pixels(
      int px_min, int px_max, int py_min, int py_max, int R, int G, int B,
      double RGB_threshold);

// RGB color to greyscale conversion member functions:

   void reset_all_RGB_values(int R, int G, int B);
   void reset_all_RGBA_values(int R, int G, int B, int A);
   void clear_all_RGB_values();
   void clear_all_RGBA_values();
   void convert_color_image_to_greyscale();
   void convert_color_image_to_h_s_or_v(int color_channel);
   void convert_color_image_to_single_color_channel(
      int c,bool generate_greyscale_image_flag);
   void convert_color_image_to_luminosity();

// Greyscale to RGB conversion member functions:

   void convert_grey_values_to_hues(double hue_min=300,double hue_max=0);
   void convert_greyscale_image_to_hue_colored(double output_alpha);
   void convert_greyscale_image_to_hue_colored(
      double hue_min,double hue_max,double output_alpha);
   void convert_greyscale_image_to_hue_value_colored(
      double hue,double output_alpha);
   void convert_single_twoDarray_to_three_channels(
      const twoDarray* qtwoDarray_ptr,bool randomize_blue_values_flag);
   void minutely_perturb_RGB_values();
   texture_rectangle* generate_RGB_from_grey_texture_rectangle();

// Texture content manipulation member functions:

   void modify_hsv_in_region(
      unsigned int px_lo, unsigned int px_hi,
      unsigned int py_lo, unsigned int py_hi,
      double hue,double min_s,double min_v);
   void globally_perturb_hsv(double delta_h, double delta_s, double delta_v);
   void globally_perturb_hsv(
      int px_lo, int px_hi,
      int py_lo, int py_hi,
      double delta_h, double delta_s, double delta_v);
   void add_gaussian_noise(double sigma);
   void add_gaussian_noise(
      int pu_lo, int pu_hi,
      int pv_lo, int pv_hi, double sigma);

// Multiple texture overlay member functions:

   bool overlay(std::string ontop_img_filename, int qx, int qy);
   void overlay_layer(int pu_start, int pv_start, 
                      texture_rectangle* overlay_tr_ptr,
                      bool generate_mask_flag = false);

// Video file output member functions:

   void write_dotVidfile(
      std::string output_filename,
      const std::vector<unsigned char*>& charstar_vector);
   void write_curr_frame(
      std::string output_filename,int n_horiz_output_pixels=-1);
   void write_curr_frame(
      const twovector& lower_left_corner_fracs,
      const twovector& upper_right_corner_fracs,
      std::string output_filename,int n_horiz_output_pixels=-1);
   void write_curr_frame(
      unsigned int px_start,unsigned int px_stop,
      unsigned int py_start,unsigned int py_stop,
      std::string output_filename,int n_horiz_output_pixels=-1);
   void write_curr_subframe(
      int px_start, int px_stop,
      int py_start, int py_stop,
      std::string output_filename, bool horiz_flipped_flag = false);
   void write_curr_subframe(
      int px_start, int px_stop,
      int py_start, int py_stop,
      int px_offset, int py_offset,
      std::string output_filename, unsigned int outside_frame_value,
      bool horiz_flipped_flag = false);

   void retrieve_curr_subframe_byte_data(
      const twovector& lower_left_corner_fracs,
      const twovector& upper_right_corner_fracs,
      std::string output_image_suffix,bool draw_central_bbox_flag=false,
      int n_horiz_output_pixels=-1);
   void generate_blank_image_file(
      unsigned int mdim,unsigned int ndim,std::string blank_filename,
      double grey_level);
   
// Photograph manipulation member functions:

   bool read_next_photo();
   void modify_next_photo_filename(std::string& next_photo_filename);
   bool reset_texture_content(std::string photo_filename);

   void check_all_pixel_RGB_values();

   double compute_image_entropy(
      bool filter_intensities_flag,int color_channel_ID);
   double compute_image_entropy(
      unsigned int pu_start,unsigned int pu_stop,
      unsigned int pv_start,unsigned int pv_stop,
      bool filter_intensities_flag,int color_channel_ID);
   void compute_RGB_image_entropies(
     unsigned int pu_start,unsigned int pu_stop,
     unsigned int pv_start,unsigned int pv_stop,
     double& R_entropy,double& G_entropy,double& B_entropy);

   void RGB_entropy_integral_images(
      twoDarray* SrtwoDarray_ptr, twoDarray* SgtwoDarray_ptr, 
      twoDarray* SbtwoDarray_ptr);
   void bbox_RGB_entropies(
      int px_lo, int px_hi, int py_lo, int py_hi,
      twoDarray* SrtwoDarray_ptr, twoDarray* SgtwoDarray_ptr, 
      twoDarray* SbtwoDarray_ptr, double& Sr, double& Sg, double& Sb);

// Lode's computer graphics tutorial member functions:

   bool pop(unsigned int &px,unsigned int &py);
   bool push(unsigned int px,unsigned int py);
   void emptyStack();
   void floodFill(
      unsigned int px,unsigned int py,int flood_R,int flood_G,int flood_B,
      int init_R,int init_G,int init_B,
      double local_threshold,double global_threshold,
      std::vector<std::pair<int,int> >& filled_pixels,
      std::vector<threevector>& encountered_RGBs);
   void floodFill(
      texture_rectangle* filtered_texture_rectangle_ptr,
      twoDarray* segmentation_mask_twoDarray_ptr,
      twoDarray* mu_R_twoDarray_ptr,
      twoDarray* mu_G_twoDarray_ptr,
      twoDarray* mu_B_twoDarray_ptr,
      twoDarray* sigma_R_twoDarray_ptr,
      twoDarray* sigma_G_twoDarray_ptr,
      twoDarray* sigma_B_twoDarray_ptr,
      unsigned int px,unsigned int py,int flood_R,int flood_G,int flood_B,
      int bbox_size,double local_mu_threshold,double global_mu_threshold,
      double local_sigma_threshold,double global_sigma_threshold);

   void floodDarkGrey(
      unsigned int px,unsigned int py,
      int flood_R,int flood_G,int flood_B,
      int init_R,int init_G,int init_B,
      double black_v,double darkgrey_v,
      std::vector<std::pair<int,int> >& filled_pixels,
      std::vector<threevector>& encountered_RGBs);
   void floodLightGrey(
      unsigned int px,unsigned int py,
      int flood_R,int flood_G,int flood_B,
      int init_R,int init_G,int init_B,
      std::vector<std::pair<int,int> >& filled_pixels,
      std::vector<threevector>& encountered_RGBs);

   threevector find_interior_median_RGBs(
      const extremal_region* extremal_region_ptr);
   double find_perimeter_seeds(
      extremal_region* extremal_region_ptr,
      const twoDarray* cc_twoDarray_ptr,std::vector<twovector>& perim_seeds,
      threevector& median_perimeter_RGB);
   void floodfill_color_region_bbox(
      unsigned int seed_pu,unsigned int seed_pv,bounding_box* symbol_bbox_ptr);
   void floodfill_black_pixels(
      int flood_R,int flood_G,int flood_B,
      double max_seed_v_threshold=0.175,
      double black_v=0.12,double darkgrey_v=0.22);
   void floodfill_white_pixels(int flood_R,int flood_G,int flood_B);
   
  protected:

  private: 

   double min_U,max_U,min_V,max_V,dU,dV;
   FFMPEGVideo* FFMPEGVideo_ptr;

   VideoType video_type;
   bool colormap_flag;
   unsigned char *m_image, *m_color_image;
   unsigned int m_VidWidth,m_VidHeight,m_Nchannels;
   unsigned int next_width, next_height;
   int first_imagenumber,Nimages;
   int first_frame_to_display,last_frame_to_display;
   int image_size_in_bytes,prev_imagenumber;
   int panel_number;
   int GLimageDepth,GLpacking;  
   GLenum GLformat;
   GLenum GLtype;
   GLint GLinternalTextureFormat;
   std::vector<double> region_R_values,region_G_values,region_B_values;
   std::vector<double> region_intensity_values;

   std::string video_filename;
   std::string output_image_string;
   osgSim::ColorRange* colorrange_ptr;
   ColorMap* ColorMap_ptr;
   VidFile* m_g99Video;
   twoDarray *ptwoDarray_ptr;
   twoDarray *RtwoDarray_ptr,*GtwoDarray_ptr,*BtwoDarray_ptr,*AtwoDarray_ptr;

   AnimationController* AnimationController_ptr;

   unsigned int* stack;
   unsigned int stackPointer,stackSize;

   osg::ref_ptr<osg::Image> image_refptr;
   osg::Image::AllocationMode allocation_mode;
   osg::ref_ptr<osgDB::Registry> Registry_refptr;
   osg::ref_ptr<osg::TextureRectangle> TextureRectangle_refptr;

   void initialize_member_objects();
   void allocate_member_objects();

// Video initialization member functions:
   
   void set_GLformat();

   void check_pixel_bounds(unsigned int& p) const;
   void set_pixel_RGB_values(
      unsigned int px,unsigned int py,unsigned char* data_ptr,
      int R,int G,int B);

   int get_pixel_intensity_value(
      unsigned int px,unsigned int py,const unsigned char* data_ptr) const;

   void get_pixel_RGB_values(
      unsigned int px,unsigned int py,const unsigned char* data_ptr,
      int& R,int& G,int& B) const;

   void get_pixel_RGBA_values(
      unsigned int px,unsigned int py,const unsigned char* data_ptr,
      int& R,int& G,int& B,int& A) const;

   Triple<int,int,int> get_pixel_RGB_values(
      unsigned int px,unsigned int py,unsigned char* data_ptr) const;
   Quadruple<int,int,int,int> get_pixel_RGBA_values(
      unsigned int px,unsigned int py,unsigned char* data_ptr) const;

   void generate_central_bbox(
      double chip_height,double chip_width,double frac_from_center,
      colorfunc::Color bbox_color,unsigned char* m_subimage);
   void color_subimage_array(
      unsigned char* m_subimage,int pixel_offset,
      colorfunc::RGB_bytes bbox_color);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void texture_rectangle::set_video_filename(std::string filename)
{
   video_filename=filename;
}

inline texture_rectangle::VideoType texture_rectangle::get_VideoType() const
{
   return video_type;
}

inline AnimationController* texture_rectangle::get_AnimationController_ptr()
{
   return AnimationController_ptr;
}

inline const AnimationController* texture_rectangle::get_AnimationController_ptr() const
{
   return AnimationController_ptr;
}

inline int texture_rectangle::get_image_size_in_bytes() const
{
   return image_size_in_bytes;
}

inline VidFile* texture_rectangle::get_VidFile_ptr()
{
   return m_g99Video;
}

inline const VidFile* texture_rectangle::get_VidFile_ptr() const
{
   return m_g99Video;
}

inline void texture_rectangle::set_m_image_ptr(unsigned char *m_ptr)
{
   m_image = m_ptr;
}

inline unsigned char* texture_rectangle::get_m_image_ptr()
{
//   std::cout << "inside texture_rectangle::get_m_image_ptr()" << std::endl;
   
   if (m_image==NULL)
   {
//      std::cout << "m_image = NULL" << std::endl;
//      std::cout << "image_refptr->valid() = " << image_refptr->valid()
//                << std::endl;
      return image_refptr->data();
   }
   else
   {
//      std::cout << "m_image != NULL" << std::endl;
      return m_image;
   }
}

inline const unsigned char* texture_rectangle::get_m_image_ptr() const
{
//   std::cout << "inside texture_rectangle::get_m_image_ptr()" << std::endl;

   if (m_image==NULL)
   {
//      cout << "m_image = NULL" << endl;
//      std::cout << "image_refptr->valid() = " << image_refptr->valid()
//                << std::endl;
      return image_refptr->data();
   }
   else
   {
      return m_image;
   }
}

inline unsigned char* texture_rectangle::get_m_colorimage_ptr()
{
   return m_color_image;
}

inline const unsigned char* texture_rectangle::get_m_colorimage_ptr() const
{
   return m_color_image;
}

inline double texture_rectangle::get_minU() const
{
   return min_U;
}

inline double texture_rectangle::get_minV() const
{
   return min_V;
}

inline double texture_rectangle::get_maxU() const
{
   return max_U;
}

inline double texture_rectangle::get_maxV() const
{
   return max_V;
}

inline double texture_rectangle::get_dU() const
{
   return dU;
}

inline double texture_rectangle::get_dV() const
{
   return dV;
}

// Member function get_midpoint returns the center of the video
// display in an XYZ form which can be used for 3D moving/picking
// purposes:

inline threevector texture_rectangle::get_midpoint() const
{
   return threevector(0.5*(get_maxU()-get_minU()),0,
                      0.5*(get_maxV()-get_minV()));
}

inline void texture_rectangle::setWidth(unsigned int width)
{ 
   m_VidWidth=width;
}

inline unsigned int texture_rectangle::getWidth() const
{ 
   return m_VidWidth; 
}

inline void texture_rectangle::setHeight(unsigned int height)
{ 
   m_VidHeight=height;
}

inline unsigned int texture_rectangle::getHeight() const
{ 
   return m_VidHeight; 
}

inline unsigned int texture_rectangle::get_next_width() const
{ 
   return next_width;
}

inline unsigned int texture_rectangle::get_next_height() const
{ 
   return next_height;
}

inline unsigned int texture_rectangle::getNchannels() const
{
   return image_refptr->getPixelSizeInBits() / 8;
//    return m_Nchannels;
}

inline void texture_rectangle::set_first_imagenumber(int i)
{
   first_imagenumber=i;
}

inline int texture_rectangle::get_first_imagenumber() const
{
   return first_imagenumber;
}

inline int texture_rectangle::get_last_imagenumber() const
{
   return first_imagenumber+get_Nimages()-1;
}

inline void texture_rectangle::set_panel_number(int p)
{
   panel_number=p;
}

inline void texture_rectangle::get_pixel_coords(
   double u,double v,unsigned int& pu,unsigned int& pv) const
{
   pu = basic_math::round( u * (m_VidHeight - 1));
   pv = basic_math::round ((1 - v) * (m_VidHeight - 1));
}

inline void texture_rectangle::get_pixel_coords_and_fracs(
   double u,double v,unsigned int& pu,unsigned int& pv,
   double& ufrac,double& vfrac) const
{
   const double TINY=1E-10;
   double pu_tot=((u-min_U)/(max_U-min_U))*(m_VidWidth-TINY);
   double pv_tot=((v-min_V)/(max_V-min_V))*(m_VidHeight-TINY);
   pu=basic_math::mytruncate(pu_tot);
   ufrac=pu_tot-pu;
   pv=basic_math::mytruncate(pv_tot);
   vfrac=pv_tot-pv;

   pv=(m_VidHeight-1)-pv;
   vfrac=1-vfrac;
}

inline void texture_rectangle::get_uv_coords(
   unsigned int pu,unsigned int pv,double& u,double& v) const
{
   u = double(pu) / double (m_VidHeight - 1);
   v = 1 - double(pv) / double (m_VidHeight - 1);
}

inline int texture_rectangle::get_intensity(double u,double v) const
{
   if (u >= min_U && u <= max_U && v >= min_V && v <= max_V)
   {
      unsigned int pu,pv;
      get_pixel_coords(u,v,pu,pv);
      return m_g99Video->pixel_greyscale_intensity_value(
         pu,pv,get_m_image_ptr());
   }
   return -1; // missing data sentinel value
}

inline ColorMap* texture_rectangle::get_ColorMap_ptr()
{
   return ColorMap_ptr;
}

inline const ColorMap* texture_rectangle::get_ColorMap_ptr() const
{
   return ColorMap_ptr;
}

inline osg::Image* texture_rectangle::get_image_ptr()
{
   return image_refptr.get();
}

inline const osg::Image* texture_rectangle::get_image_ptr() const
{
   return image_refptr.get();
}

inline std::string& texture_rectangle::get_output_image_string()
{
   return output_image_string;
}

inline const std::string& texture_rectangle::get_output_image_string() const
{
   return output_image_string;
}

inline osg::TextureRectangle* texture_rectangle::get_TextureRectangle_ptr()
{
   return TextureRectangle_refptr.get();
}

inline const osg::TextureRectangle* texture_rectangle::get_TextureRectangle_ptr() const
{
   return TextureRectangle_refptr.get();
}



#endif // texture_rectangle.h
