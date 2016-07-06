// ==========================================================================
// Program QUANTIZE_IMAGE_COLORS queries the user to enter some input
// image file.  It first computes and displays the color histogram for
// the input image.  QUANTIZE_IMAGE_COLORS then exports the
// color-quantized version of the image to a quantized_colors
// subdirectory.
// ==========================================================================
// Last updated on 8/2/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/RGB_analyzer.h"


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table();

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();

   string image_filename;
   cout << "Enter full path for input image file:" << endl;
   cin >> image_filename;
   string image_subdir=filefunc::getdirname(image_filename);
   string image_basename=filefunc::getbasename(image_filename);
         
   texture_rectangle_ptr->reset_texture_content(image_filename);
   vector<double> color_fracs=RGB_analyzer_ptr->compute_color_histogram(
      texture_rectangle_ptr);

   int n_quantized_colors=RGB_analyzer_ptr->get_n_color_indices();
   vector<string> color_names;
   for (int c=0; c<n_quantized_colors; c++)
   {
      color_names.push_back(RGB_analyzer_ptr->get_color_name(c));
   }

   templatefunc::Quicksort_descending(color_fracs,color_names);

   string banner="COLOR HISTOGRAM";
   outputfunc::write_big_banner(banner);
   for (int m=0; m<n_quantized_colors; m++)
   {
      cout << color_names[m] << "   "
           << color_fracs[m] << endl;
   }

   quantized_texture_rectangle_ptr->reset_texture_content(image_filename);
   RGB_analyzer_ptr->quantize_texture_rectangle_colors(
      quantized_texture_rectangle_ptr);

   string output_subdir=image_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);
   string quantized_filename=output_subdir+"quantized_"+image_basename;
   quantized_texture_rectangle_ptr->write_curr_frame(quantized_filename);
   banner="Exported "+quantized_filename;
   outputfunc::write_big_banner(banner);

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete quantized_texture_rectangle_ptr;
}

