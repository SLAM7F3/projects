// ========================================================================
// RGB_analyzer 
// ========================================================================
// Last updated on 4/7/13; 8/2/13; 11/17/13; 5/10/14
// ========================================================================

#ifndef RGB_ANALYZER_H
#define RGB_ANALYZER_H

#include <map>
#include <string>
#include "math/lttriple.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

class extremal_region;
class genvector;
class texture_rectangle;
// class vptree;

class RGB_analyzer
{   

  public:

   typedef std::map<std::string,int> COLORNAMES_MAP;
   typedef std::map<int,std::string> COLORINDICES_MAP;
   typedef std::map<triple,int,lttriple> RGBI_MAP;
   typedef std::map<int,triple> IRGB_MAP;

// Initialization, constructor and destructor member functions:

   RGB_analyzer();
   ~RGB_analyzer();
   friend std::ostream& operator<< (
      std::ostream& outstream,const RGB_analyzer& A);

// Set & get member functions:

   std::string get_color_name(int color_index);
   unsigned int get_n_color_indices() const;
   int get_color_index(std::string color_name);
   std::string get_hue_given_index(int h);
   int get_index_given_hue(std::string hue_name);
   int get_RGB_voxel_ID(int R,int G,int B);
   void get_voxel_R_G_B(int ID,int& R,int& G,int& B);

// Quantized color computation member functions:

   void initialize_color_maps();
   void initialize_color_metric();
   std::string compute_quantized_color_name(int R,int G,int B);
   std::string compute_quantized_color_name_from_hsv(
      double h,double s,double v);
   int compute_quantized_color_index(int R,int G,int B);
   int compute_quantized_color_index_from_hsv(double h,double s,double v);
   void compute_quantized_RGB_given_colorname(
      std::string color_name,int& R,int& G,int& B);

   void reset_quantized_color_borders();
   void liberalize_color_borders(std::string liberalized_color);

// Quantized color retrieval member functions:

   void retrieve_quantized_RGB_given_color_index(
      int color_index,int& R_quantized,int& G_quantized,int& B_quantized) 
      const;
   void print_color_histogram(const std::vector<double>& color_hist);
   std::vector<double> propagate_color_hist(
      const std::vector<double>& color_hist);
   double color_histogram_inner_product(
      const std::vector<double>& color_hist1,
      const std::vector<double>& color_hist2);

// Lookup table member functions

   std::string get_lookup_filename(std::string liberalized_color="");
   void export_quantized_RGB_lookup_table(std::string liberalized_color="");
   void import_quantized_RGB_lookup_table(std::string liberalized_color="");

   int retrieve_quantized_color_index_from_lookup_map(
      std::string lookup_map_name,int R,int G,int B);
   std::string retrieve_quantized_colorname_from_lookup_map(
      std::string lookup_map_name,int R,int G,int B);
   int retrieve_quantized_RGB_from_lookup_map(
      std::string lookup_map_name,int R,int G,int B,
      int& R_quantized,int& G_quantized,int& B_quantized);

// Image color quantization member functions

   void quantize_texture_rectangle_colors(
      texture_rectangle* texture_rectangle_ptr,
      std::string lookup_map_name="");
   void smooth_quantized_image(
      const texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr);
   void identify_all_greyish_pixels(
      std::string lookup_map_name,
      texture_rectangle* texture_rectangle_ptr,
      int R_grey,int G_grey,int B_grey);
   bool identify_greyish_pixel(
      std::string lookup_map_name,
      int px,int py,texture_rectangle* texture_rectangle_ptr);
   bool identify_greyish_pixel(
      std::string lookup_map_name,
      int px,int py,texture_rectangle* texture_rectangle_ptr,
      int& R_quantized,int& G_quantized,int& B_quantized);
   void isolate_quantized_colors(
      const texture_rectangle* quantized_texture_rectangle_ptr,
      std::vector<std::string> quantized_colornames,
      texture_rectangle* filtered_texture_rectangle_ptr,
      texture_rectangle* binary_texture_rectangle_ptr);
   int dominant_extremal_region_hue_content(
      std::string lookup_map_name,
      const extremal_region* extremal_region_ptr,
      const texture_rectangle* quantized_texture_rectangle_ptr);

// Bbox color content member functions

   std::vector<double>& compute_color_histogram(
      const texture_rectangle* texture_rectangle_ptr,
      std::string lookup_map_name="");
   std::vector<double>& compute_bbox_color_content(
      int left_pu,int top_pv,int right_pu,int bottom_pv,
      const texture_rectangle* texture_rectangle_ptr,
      std::string lookup_map_name="");
   double vivid_hue_fraction();

// RGB color averaging member functions

   void average_texture_rectangle_colors(
      int nsize,const texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* averaged_texture_rectangle_ptr);
   void median_texture_rectangle_colors(
      int nsize,const texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* median_texture_rectangle_ptr);

  private: 

   double delta_red_orange,delta_orange_yellow,delta_yellow_green;
   double delta_green_cyan,delta_cyan_blue,delta_blue_purple;
   double delta_purple_red;

   double delta_black_dark,delta_dark_bright;
   double max_black_saturation;

   double delta_lightgrey_white,delta_lightgrey_grey,delta_grey_darkgrey;
   double delta_darkgrey_black;

   double delta_vivid_light,delta_grey_light;

   COLORNAMES_MAP colornames_map;
   COLORNAMES_MAP::iterator colornames_iter;
   COLORINDICES_MAP colorindices_map;
   COLORINDICES_MAP::iterator colorindices_iter;
//   RGBI_MAP RGBI_map;
//   RGBI_MAP::iterator rgbi_iter;
   IRGB_MAP indexRGB_map;
//   IRGB_MAP::iterator irgb_iter;

//   std::vector<genvector*> RGB_ptrs;
//   vptree* vptree_ptr;
//   threevector curr_RGB;
   
   twoDarray* colorindex_twoDarray_ptr;
   std::vector<int>* quantized_color_lookup_vector_ptr;

   typedef std::map<std::string,std::vector<int>* > 
      QUANTIZED_COLORS_MAP;
   QUANTIZED_COLORS_MAP quantized_colors_map;
   QUANTIZED_COLORS_MAP::iterator quantized_colors_iter;

// Independent string = liberal color name
// Dependent vector<int>* = quantized color lookup vector pointer

   std::vector<double> color_fracs;

   genmatrix* M_color_ptr;	// Quantized color metric 

   void initialize_member_objects();
   void allocate_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline unsigned int RGB_analyzer::get_n_color_indices() const
{
   return colornames_map.size();
}

inline int RGB_analyzer::get_RGB_voxel_ID(int R,int G,int B)
{
   int ID=R*65536+G*256+B;
   return ID;
}

inline void RGB_analyzer::get_voxel_R_G_B(int ID,int& R,int& G,int& B)
{
   R=ID/65536;
   ID -= R*65536;
   G=ID/256;
   B=ID%256;
}


#endif // RGB_analyzer.h
