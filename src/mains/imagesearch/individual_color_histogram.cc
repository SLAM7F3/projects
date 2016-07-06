// ==========================================================================
// Program INDIVIDUAL_COLOR_HISTOGRAM
// ==========================================================================
// Last updated on 3/7/12; 3/8/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string jpg_filename;
   cout << "Enter input jpg filename:" << endl;
   cin >> jpg_filename;

   string subdir="./random_images/";
   jpg_filename=subdir+jpg_filename;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      jpg_filename,NULL);
   if (texture_rectangle_ptr->get_VideoType()==texture_rectangle::unknown)
   {
      cout << "Could not read in image from file!" << endl;
      exit(-1);
   }

   bool generate_quantized_image_flag=true;
   string quantized_color_image_filename="quantized_colors.jpg";
   vector<double> color_histogram=videofunc::generate_color_histogram(
      generate_quantized_image_flag,texture_rectangle_ptr,
      quantized_color_image_filename);

   delete texture_rectangle_ptr;
}

