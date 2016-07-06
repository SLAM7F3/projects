// ========================================================================
// G99VideoDisplay class provides functionality for displaying G99
// movie files.
// ========================================================================
// Last updated on 3/2/11; 3/6/14; 3/8/14; 3/28/14; 6/7/14
// ========================================================================

#ifndef G99VIDEO_DISPLAY_H
#define G99VIDEO_DISPLAY_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Array>
#include <osg/Image>
#include <osg/Geometry>
#include <osg/TexMat>
#include "geometry/homography.h"
#include "datastructures/Quadruple.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"
#include "image/TwoDarray.h"
#include "video/VidFile.h"

class AnimationController;

typedef Quadruple<twoDarray*,twoDarray*,twoDarray*,twoDarray*> RGBA_array;

namespace osg
{
   class CullFace;
}


class G99VideoDisplay
{   

  public:

// Initialization, constructor and destructor member functions:

   G99VideoDisplay(std::string& video_filename,AnimationController* AC_ptr,
                   bool hide_backface_flag=false);
   G99VideoDisplay(texture_rectangle* tr_ptr);
   ~G99VideoDisplay();

// Set & get member functions:

   void set_dynamic_frame_flag(bool flag);
   bool get_dynamic_frame_flag() const;
   void set_annotated_pixels_flag(bool flag);
   bool get_annotated_pixels_flag(); 
   void set_video_filename(std::string filename);
   std::string get_video_filename() const;

   texture_rectangle* get_texture_rectangle_ptr(int t=0);
   const texture_rectangle* get_texture_rectangle_ptr(int t=0) const;
   osg::TexMat* get_TexMat_ptr();
   const osg::TexMat* get_TexMat_ptr() const;

   osg::Geometry* get_Geometry_ptr(int geom_index=0);
   const osg::Geometry* get_Geometry_ptr(int geom_index=0) const;
   osg::Vec3Array* get_vertices_ptr();
   const osg::Vec3Array* get_vertices_ptr() const;

   VidFile* get_VidFile_ptr();
   const VidFile* get_VidFile_ptr() const;
   unsigned char* get_m_image_ptr();
   const unsigned char* get_m_image_ptr() const;
   unsigned char* get_m_colorimage_ptr();
   const unsigned char* get_m_colorimage_ptr() const;
   
   double get_minU() const;
   double get_minV() const;
   double get_maxU() const;
   double get_maxV() const;
   double get_dU() const;
   double get_dV() const;

   int get_first_imagenumber() const;
   int get_last_imagenumber() const;

   unsigned int getWidth(int t=0) const;
   unsigned int getHeight(int t=0) const;
   double get_aspect_ratio(int t=0) const;
   unsigned int getNchannels(int t=0) const;

   osg::Image* get_image_ptr();
   const osg::Image* get_image_ptr() const;

   RGBA_array& get_RGBA_twoDarray() { return RGBA_twoDarray; }
   void set_RGBA_twoDarray_ptr(int i,twoDarray* ztwoDarray_ptr);
   twoDarray* get_RGBA_twoDarray(int i);

// Texture member functions:

   texture_rectangle* generate_texture_rectangle(
      std::string& video_filename,AnimationController* AC_ptr);
   void set_texture_fracs(const twovector& lower_left_frac,
                          const twovector& lower_right_frac,
                          const twovector& upper_right_frac,
                          const twovector& upper_left_frac);
   void set_default_texture_fracs();
   void reset_texture_coords();
   void rescale_image(double scalefactor);   

// Geometry member functions:

   void equate_texture_coords_to_vertices();
   void reset_geom_vertices(
      const threevector& top_right,const threevector& top_left,
      const threevector& bottom_left,const threevector& bottom_right);
   void dirtyGeomDisplay();
   threevector compute_corners_COM();
   threevector get_midpoint() const;

// Subtexture member functions:

   twovector& get_lower_left_texture_frac();
   const twovector& get_lower_left_texture_frac() const;
   twovector& get_lower_right_texture_frac();
   const twovector& get_lower_right_texture_frac() const;
   twovector& get_upper_right_texture_frac();
   const twovector& get_upper_right_texture_frac() const;
   twovector& get_upper_left_texture_frac();
   const twovector& get_upper_left_texture_frac() const;

// Alpha blending member functions:

   void set_alpha(double alpha,int geom_index=0);
   double get_alpha(int geom_index=0);
   bool increase_alpha(double delta_alpha=0.05);
   bool decrease_alpha(double delta_alpha=0.05);

// Video chip member functions:

   void compute_2D_chip(
      const twovector& lower_left_texture_fracs,
      const twovector& lower_right_texture_fracs,
      const twovector& upper_right_texture_fracs,
      const twovector& upper_left_texture_fracs);

   void initialize_3D_chip(
      const twovector& lower_left_texture_fracs,
      const twovector& lower_right_texture_fracs,
      const twovector& upper_right_texture_fracs,
      const twovector& upper_left_texture_fracs,
      const threevector& video_top_right, 
      const threevector& video_top_left,
      const threevector& video_bottom_left, 
      const threevector& video_bottom_right);
   void compute_image_to_Z_plane_homographies(
      const twovector& bottom_left_texture_fracs,
      const twovector& bottom_right_texture_fracs,
      const twovector& upper_right_texture_fracs,
      const twovector& upper_left_texture_fracs,
      const threevector& video_top_right, 
      const threevector& video_top_left,
      const threevector& video_bottom_left, 
      const threevector& video_bottom_right);
   void compute_3D_chip(
      const twovector& lower_left_texture_fracs,
      const twovector& lower_right_texture_fracs,
      const twovector& upper_right_texture_fracs,
      const twovector& upper_left_texture_fracs);
   void compute_3D_chip(
      const threevector& bottom_left,const threevector& bottom_right,
      const threevector& top_right,const threevector& top_left);

// Image numbering member functions:

   unsigned int get_n_textures() const;
   int get_Nimages() const;	     
   int get_first_frame_to_display() const;
   int get_last_frame_to_display() const;

// Frame display member functions:
   
   void display_current_frame();
   void displayFrame( int p_framenum );
   void set_image();

// Colormap member functions:

   void change_color_map(int map_number=0);

// RGBA member functions:

   void convert_to_RGB(
      unsigned char* data_ptr,double intensity_threshold,
      const prob_distribution& p_gaussian,std::vector<double>& h,
      std::vector<double>& s,std::vector<double>& v,
      std::vector<double>& non_negligible_intensities);
   void generate_RGBA_twoDarrays();
   void delete_RGBA_twoDarrays();
   void convert_charstar_array_to_RGBA_twoDarrays(unsigned char* data_ptr);
   unsigned char* convert_RGBAarrays_to_charstar_array();
   unsigned char* convert_RGBAarrays_to_charstar_array(int mdim,int ndim);
   void convert_RGBAarrays_to_charstar_array(
      int NBYTES_PER_PIXEL,int mdim,int ndim,unsigned char* data);

// Pixel value manipulation member functions:

   std::pair<unsigned int,unsigned int> get_pixel_coords(double u,double v);
   void get_pixel_coords(double u,double v,unsigned int& pu,unsigned int& pv);
   void get_pixel_coords_and_fracs(
      double u,double v,unsigned int& pu,unsigned int& pv,
      double& ufrac,double& vfrac);
   std::pair<double,double> get_uv_coords(int pu,int pv);
   void get_uv_coords(int pu,int pv,double& u,double& v);

   int get_intensity(double u,double v);
   void get_pixel_RGB_values(unsigned int pu,unsigned int pv,
                             int& R,int& G,int& B);
   void get_RGB_values(double u,double v,int& R,int& G,int& B);
   Triple<int,int,int> get_RGB_values(double u,double v);
   void set_pixel_RGB_values(unsigned int pu,unsigned int pv,
                             int R,int G,int B);
   void set_RGB_values(double u,double v,int R,int G,int B);

// Intensity regularization member functions:

   std::pair<double,double> compute_median_greyscale_image_intensity(
      double intensity_threshold);
   void regularize_greyscale_image_intensities(
      std::string input_greyscale_video_filename,double intensity_threshold,
      const std::pair<double,double>& median_intensity);
   void equalize_greyscale_intensity_histograms(
      std::string input_greyscale_video_filename,double intensity_threshold);
   void equalize_greyscale_intensity_histograms(
      std::string input_greyscale_video_filename,
      std::vector<double> intensity_thresholds);
   void equalize_RGB_intensity_histograms(
      std::string input_RGB_video_filename,double intensity_threshold);

// Homography member functions:

   void planar_orthorectify(
      homography& H_in,double xmin,double xmax,double ymin,double ymax,
      unsigned char* transformed_data_ptr);

   void enable_alpha_blending(double alpha,int ndims,int geom_index=0);

   void fill_drawable_geom(int geom_index);

   void SetupTextureMatrix();

  protected:

   int get_imagenumber() const;


  private:

   bool externally_constructed_texture_rectangle_flag;
   bool dynamic_frame_flag,hide_backface_flag;
   std::vector<texture_rectangle*> texture_rectangle_ptrs;
   twovector lower_left_texture_frac,lower_right_texture_frac,
      upper_right_texture_frac,upper_left_texture_frac;

   std::vector<osg::ref_ptr<osg::Geometry> > geom_refptrs;
   osg::ref_ptr<osg::Vec3Array> vertices_refptr;
   osg::ref_ptr<osg::TexMat> TexMat_refptr;
   osg::ref_ptr<osg::CullFace> CullFace_refptr;

   double Zplane_avg;
   homography H;

   bool annotated_pixels_flag;
   std::vector<double> median_intensities,quartile_widths;
   osg::ref_ptr<osg::Vec3Array> texturecoords_refptr;

   RGBA_array RGBA_twoDarray;

// Video initialization member functions:

   void initialize_member_objects();
   void allocate_member_objects();
   osg::Geometry* generate_geometry();

   void initialize_geom_vertices(int geom_index=0);

   void initialize_texture_rectangle(int t);
   void clip_texture_fracs(twovector& V);


   void draw_annotation();

   void check_pixel_bounds(unsigned int& p);
   void set_pixel_RGB_values(
      int px,int py,unsigned char* data_ptr,int R,int G,int B);
//   void set_pixel_RGBA_values(
//      int px,int py,unsigned char* data_ptr,int R,int G,int B,int A);
   void get_pixel_RGB_values(
      int px,int py,unsigned char* data_ptr,int& R,int& G,int& B);
   void get_pixel_RGBA_values(
      int px,int py,unsigned char* data_ptr,int& R,int& G,int& B,int& A);
   Triple<int,int,int> get_pixel_RGB_values(
      int px,int py,unsigned char* data_ptr);
   Quadruple<int,int,int,int> get_pixel_RGBA_values(
      int px,int py,unsigned char* data_ptr);

/*
// Subtexture member functions:

   double f_frac(double U) const;
   double g_frac(double U) const;
   double s_frac(double V) const;
   double t_frac(double V) const;
   threevector interpolated_vertex_posn(
      const twovector& texture_fracs,
      const threevector& video_bottom_left,
      const threevector& video_bottom_right,
      const threevector& video_top_left,
      const threevector& tvideo_top_right) const;
*/

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void G99VideoDisplay::set_dynamic_frame_flag(bool flag)
{
   dynamic_frame_flag=flag;
}

inline bool G99VideoDisplay::get_dynamic_frame_flag() const
{
   return dynamic_frame_flag;
}

inline void G99VideoDisplay::set_annotated_pixels_flag(bool flag)
{
   annotated_pixels_flag=flag;
}

inline bool G99VideoDisplay::get_annotated_pixels_flag()
{
   return annotated_pixels_flag;
}

inline void G99VideoDisplay::set_video_filename(
   std::string filename)
{
   get_texture_rectangle_ptr()->set_video_filename(filename);
}

inline std::string G99VideoDisplay::get_video_filename() const
{
   return get_texture_rectangle_ptr()->get_video_filename();
}

inline texture_rectangle* G99VideoDisplay::get_texture_rectangle_ptr(int t)
{
   return texture_rectangle_ptrs[t];
}

inline const texture_rectangle* G99VideoDisplay::get_texture_rectangle_ptr(
   int t) const
{
   return texture_rectangle_ptrs[t];
}

inline osg::TexMat* G99VideoDisplay::get_TexMat_ptr()
{
   return TexMat_refptr.get();
}

inline const osg::TexMat* G99VideoDisplay::get_TexMat_ptr() const
{
   return TexMat_refptr.get();
}

inline osg::Geometry* G99VideoDisplay::get_Geometry_ptr(int geom_index)
{
   return geom_refptrs[geom_index].get();
}

inline const osg::Geometry* G99VideoDisplay::get_Geometry_ptr(int geom_index) 
   const
{
   return geom_refptrs[geom_index].get();
}

inline osg::Vec3Array* G99VideoDisplay::get_vertices_ptr()
{
   return vertices_refptr.get();
}

inline const osg::Vec3Array* G99VideoDisplay::get_vertices_ptr() const
{
   return vertices_refptr.get();
}

inline VidFile* G99VideoDisplay::get_VidFile_ptr()
{
   return get_texture_rectangle_ptr()->get_VidFile_ptr();
}

inline const VidFile* G99VideoDisplay::get_VidFile_ptr() const
{
   return get_texture_rectangle_ptr()->get_VidFile_ptr();
}

inline unsigned char* G99VideoDisplay::get_m_image_ptr()
{
   return get_texture_rectangle_ptr()->get_m_image_ptr();
}

inline const unsigned char* G99VideoDisplay::get_m_image_ptr() const
{
   return get_texture_rectangle_ptr()->get_m_image_ptr();
}

inline unsigned char* G99VideoDisplay::get_m_colorimage_ptr()
{
   return get_texture_rectangle_ptr()->get_m_colorimage_ptr();
}

inline const unsigned char* G99VideoDisplay::get_m_colorimage_ptr() const
{
   return get_texture_rectangle_ptr()->get_m_colorimage_ptr();
}

inline double G99VideoDisplay::get_minU() const
{
   return get_texture_rectangle_ptr()->get_minU();
}

inline double G99VideoDisplay::get_minV() const
{
   return get_texture_rectangle_ptr()->get_minV();
}

inline double G99VideoDisplay::get_maxU() const
{
   return get_texture_rectangle_ptr()->get_maxU();
}

inline double G99VideoDisplay::get_maxV() const
{
   return get_texture_rectangle_ptr()->get_maxV();
}

inline double G99VideoDisplay::get_dU() const
{
   return get_texture_rectangle_ptr()->get_dU();
}

inline double G99VideoDisplay::get_dV() const
{
   return get_texture_rectangle_ptr()->get_dV();
}

// Member function get_midpoint returns the center of the video
// display in an XYZ form which can be used for 3D moving/picking
// purposes:

inline threevector G99VideoDisplay::get_midpoint() const
{
   return threevector(0.5*(get_maxU()-get_minU()),0,
                      0.5*(get_maxV()-get_minV()));
}

inline unsigned int G99VideoDisplay::getWidth(int t) const
{ 
   return get_texture_rectangle_ptr(t)->getWidth();
}

inline unsigned int G99VideoDisplay::getHeight(int t) const
{ 
   return get_texture_rectangle_ptr(t)->getHeight();
}

inline double G99VideoDisplay::get_aspect_ratio(int t) const
{
   return double(getWidth(t))/double(getHeight(t));
}

inline unsigned int G99VideoDisplay::getNchannels(int t) const
{
   return get_texture_rectangle_ptr(t)->getNchannels();
}

inline int G99VideoDisplay::get_first_imagenumber() const
{
   return get_texture_rectangle_ptr()->get_first_imagenumber();
}

inline int G99VideoDisplay::get_last_imagenumber() const
{
   return get_texture_rectangle_ptr()->get_last_imagenumber();
}

inline void G99VideoDisplay::get_pixel_coords(
   double u,double v,unsigned int& pu,unsigned int& pv)
{
   pu = basic_math::round( u * (getHeight() - 1));
   pv = basic_math::round ((1 - v) * (getHeight() - 1));
}

inline void G99VideoDisplay::get_pixel_coords_and_fracs(
   double u,double v,unsigned int& pu,unsigned int& pv,
   double& ufrac,double& vfrac)
{
   const double TINY=1E-10;
   double pu_tot=((u-get_minU())/(get_maxU()-get_minU()))*(getWidth()-TINY);
   double pv_tot=((v-get_minV())/(get_maxV()-get_minV()))*(getHeight()-TINY);
   pu=basic_math::mytruncate(pu_tot);
   ufrac=pu_tot-pu;
   pv=basic_math::mytruncate(pv_tot);
   vfrac=pv_tot-pv;

   pv=(getHeight()-1)-pv;
   vfrac=1-vfrac;
}

inline void G99VideoDisplay::get_uv_coords(int pu,int pv,double& u,double& v)
{
   u = double(pu) / double (getHeight() - 1);
   v = 1 - double(pv) / double (getHeight() - 1);
}

inline int G99VideoDisplay::get_intensity(double u,double v)
{
   if (u >= get_minU() && u <= get_maxU() && v >= get_minV() && 
       v <= get_maxV())
   {
      unsigned int pu,pv;
      get_pixel_coords(u,v,pu,pv);
      return get_VidFile_ptr()->pixel_greyscale_intensity_value(
         pu,pv,get_m_image_ptr());
   }
   return -1; // missing data sentinel value
}

inline void G99VideoDisplay::get_RGB_values(
   double u,double v,int& R,int& G,int& B)
{
//   R=G=B=-1; // missing data values
   if (u >= get_minU() && u <= get_maxU() && v >= get_minV() 
       && v <= get_maxV())
   {
      unsigned int pu,pv;
      get_pixel_coords(u,v,pu,pv);
      get_pixel_RGB_values(pu,pv,get_m_image_ptr(),R,G,B);
   }
}

inline osg::Image* G99VideoDisplay::get_image_ptr()
{
   return get_texture_rectangle_ptr()->get_image_ptr();
}

inline const osg::Image* G99VideoDisplay::get_image_ptr() const
{
   return get_texture_rectangle_ptr()->get_image_ptr();
}

inline void G99VideoDisplay::set_RGBA_twoDarray_ptr(
   int i,twoDarray* ztwoDarray_ptr)
{
   switch (i)
   {
      case 0: 
         RGBA_twoDarray.first=ztwoDarray_ptr;
         return;
      case 1:
         RGBA_twoDarray.second=ztwoDarray_ptr;
         return;
      case 2:
         RGBA_twoDarray.third=ztwoDarray_ptr;
         return;
      case 3:
         RGBA_twoDarray.fourth=ztwoDarray_ptr;
         return;
      default:
         std::cout << "Error in G99VideoDisplay::set_RGBA_twoDarray(int i)"
                   << std::endl;
         std::cout << "index i = " << i << std::endl;
         return;
   }
}

inline twoDarray* G99VideoDisplay::get_RGBA_twoDarray(int i)
{
   switch (i)
   {
      case 0: 
         return RGBA_twoDarray.first;
      case 1:
         return RGBA_twoDarray.second;
      case 2:
         return RGBA_twoDarray.third;
      case 3:
         return RGBA_twoDarray.fourth;
      default:
         std::cout << "Error in G99VideoDisplay::get_RGBA_twoDarray(int i)"
                   << std::endl;
         std::cout << "index i = " << i << std::endl;
         return RGBA_twoDarray.first;
   }
}

inline twovector& G99VideoDisplay::get_lower_left_texture_frac()
{
   return lower_left_texture_frac;
}

inline const twovector& G99VideoDisplay::get_lower_left_texture_frac() const
{
   return lower_left_texture_frac;
}

inline twovector& G99VideoDisplay::get_lower_right_texture_frac()
{
   return lower_right_texture_frac;
}

inline const twovector& G99VideoDisplay::get_lower_right_texture_frac() const
{
   return lower_right_texture_frac;
}

inline twovector& G99VideoDisplay::get_upper_right_texture_frac()
{
   return upper_right_texture_frac;
}

inline const twovector& G99VideoDisplay::get_upper_right_texture_frac() const
{
   return upper_right_texture_frac;
}

inline twovector& G99VideoDisplay::get_upper_left_texture_frac()
{
   return upper_left_texture_frac;
}

inline const twovector& G99VideoDisplay::get_upper_left_texture_frac() const
{
   return upper_left_texture_frac;
}

#endif
