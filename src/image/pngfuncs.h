// =========================================================================
// Header file for stand-alone PNG functions.
// =========================================================================
// Last modified on 11/11/12; 3/28/13; 7/30/13
// =========================================================================

#ifndef PNGFUNCS_H
#define PNGFUNCS_H

#include <png.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "datastructures/Triple.h"

template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;
class twovector;

namespace pngfunc
{
   typedef Triple<twoDarray*,twoDarray*,twoDarray*> RGB_array;

   extern png_uint_32 width,height;
   extern RGB_array RGB_twoDarray;

// Set and get methods:

   void set_width(int w);
   void set_height(int h);

// Memory management methods:

   void initialize_row_pointers(int nxbins,int nybins,int nchannels=3);
   void allocate_row_pointers();
   unsigned char* generate_charstar_array();
   void allocate_RGB_twoDarrays();
   void allocate_RGB_twoDarrays(int nxbins,int nybins);
   void allocate_RGB_twoDarrays(
      int nxbins,int nybins,double xlo,double xhi,double ylo,double yhi);
   void delete_RGB_twoDarrays();

// PNG file I/O methods:

   bool open_png_file(std::string png_filename);
   void parse_png_file();
   void draw_png_file();
   void close_png_file();
   bool write_output_png_file(std::string output_png_filename);

// RGB methods:

   void enumerate_all_RGB_values();
   bool get_RGB_values(const twovector& image_point,Triple<int,int,int>& rgb);
   Triple<int,int,int> get_pixel_RGB_values(int px,int py);
   void put_pixel_RGB_values(int px,int py,const Triple<int,int,int>& rgb);
   Triple<double,double,double> get_RGB_twoDarray_values(int px,int py);
   void clear_RGB_twoDarrays();
   void fill_RGB_twoDarrays();
   void copy_RGB_twoDarrays(
      const RGB_array& RGB_orig_twoDarray,RGB_array& RGB_copy_twoDarray);
   void antialias_RGB_twoDarrays(const std::vector<double>& filter_frac);
   void smear_RGB_twoDarrays();
   void transfer_RGB_twoDarrays_to_rowpointers();
   int RGB_to_int(const Triple<int,int,int>& rgb);
   Triple<int,int,int> int_to_RGB(int flat);
   Triple<int,int,int> most_frequent_RGB();
   void recolor_RGB_pixels(const Triple<int,int,int>& initial_RGB,
                           const Triple<int,int,int>& replacement_RGB);

// PNGWriter methods (deprecated as of Jan 2014.  Call 
// imagefunc::add_text_to_image() instead! )

   void convert_textlines_to_PNG(
      const std::vector<std::string>& text_lines,
      std::string output_PNG_filename,colorfunc::Color text_color,
      double background_greyscale_intensity);
   void convert_textlines_to_PNG(
      int x_start,int y_offset,int npx,int npy,double angle,
      const std::vector<std::string>& text_lines,
      std::string output_PNG_filename,colorfunc::RGB rgb,
      double background_greyscale_intensity,std::string font_path,
      int font_size);
   void convert_textlines_to_PNG(
      int npx,int npy,std::string output_PNG_filename,
      double background_greyscale_intensity,
      std::string font_path,int fontsize,
      const std::vector<std::string>& text_lines,
      const std::vector<twovector>& xy_start,
      const std::vector<colorfunc::RGB>& text_RGB);

   void add_text_to_PNG_image(
      std::string input_PNG_filename,std::string output_PNG_filename,
      int x_start,int y_offset,double angle,
      std::string text_line,colorfunc::RGB text_rgb,
      std::string font_path,int fontsize);
   void add_text_to_PNG_image(
      std::string input_PNG_filename,std::string output_PNG_filename,
      int fontsize,
      const std::vector<std::string>& text_lines,
      const std::vector<twovector>& xy_start,
      const std::vector<colorfunc::RGB>& text_RGB);

//   std::string add_elapsed_time_to_PNG_image(
//      std::string input_PNG_filename,
//      int px_start,int py_start,
//      std::string text_line,colorfunc::RGB text_rgb,
//      int fontsize);

// PNG conversion methods:

   std::string convert_image_to_PNG(std::string image_filename);

} // pngfuncs namespace

#endif // pngfuncs.h




