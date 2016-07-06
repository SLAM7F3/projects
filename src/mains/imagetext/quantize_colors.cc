// ==========================================================================
// Program QUANTIZE_COLORS
// ==========================================================================
// Last updated on 5/6/14; 5/10/14; 5/11/14
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
   string liberalized_color="";
//   string liberalized_color="green";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(
      liberalized_color);

   string output_subdir="./quantized_colors/";
   filefunc::dircreate(output_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();

   string images_subdir="./images/HouseNumbers/van/PilotKnob/";

//   string image_filename=images_subdir+"test_house.jpg";


   string PilotKnob_subdir="./images/HouseNumbers/van/PilotKnob/";

   string image_filename=PilotKnob_subdir+
//      "000038-000035-20130430-210448-3630-SYS-Cam2.jpg";
      "000067-000064-20130430-210502-8630-SYS-Cam2.jpg";
//      "000071-000068-20130430-210504-8590-SYS-Cam2.jpg";

   cout << endl;
   cout << "image_filename = " << image_filename << endl;
   cout << endl;   

   texture_rectangle_ptr->reset_texture_content(image_filename);
   quantized_texture_rectangle_ptr->reset_texture_content(image_filename);

   RGB_analyzer_ptr->quantize_texture_rectangle_colors(
      quantized_texture_rectangle_ptr,liberalized_color);

   int n_filter_iters=0;
//   int n_filter_iters=4;
   for (int filter_iter=0; filter_iter < n_filter_iters; filter_iter++)
   {
      RGB_analyzer_ptr->smooth_quantized_image(
         texture_rectangle_ptr,quantized_texture_rectangle_ptr);
   }
   string quantized_filename=images_subdir+"quantized.jpg";
//   string filtered_quantized_filename=video_subdir+
//      "filtered_"+stringfunc::number_to_string(frame_number)+".jpg";
   quantized_texture_rectangle_ptr->write_curr_frame(
      quantized_filename);
   string banner="Exported "+quantized_filename;
   outputfunc::write_big_banner(banner);

   string montage_cmd="montageview NO_DISPLAY "+image_filename+" "
      +quantized_filename;
   sysfunc::unix_command(montage_cmd);

   vector<double> color_histogram = RGB_analyzer_ptr-> 
      compute_color_histogram(texture_rectangle_ptr);
   RGB_analyzer_ptr->print_color_histogram(color_histogram);


   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete quantized_texture_rectangle_ptr;
}

