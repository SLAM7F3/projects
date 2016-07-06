// ==========================================================================
// Program DETECT_SEGMENTS calls the LSD algorithms/codes by von Gioi
// (Nov 2011) to extract a set of line segments from an input image.
// It then places the resulting segments into angle, magnitude and
// gross image plane location bins.
// ==========================================================================
// Last updated on 9/4/13; 9/5/13; 11/21/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "optimum/emdL1.h"
#include "general/filefuncs.h"
#include "video/photograph.h"
#include "math/prob_distribution.h"
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

   string image_filename;
   vector<string> input_params;
   if (!filefunc::parameter_input(argc,argv,input_params))
   {
      cout << "Enter image filename:" << endl;
      cin >> image_filename;
   }
   else
   {
      image_filename=input_params.back();
   }
//   cout << "image_filename = " << image_filename << endl;
         
// Superpose detected line segments on input image:

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   if (!texture_rectangle_ptr->reset_texture_content(image_filename))
   {
      exit(-1);
   }
   vector<linesegment> linesegments=videofunc::detect_line_segments(
      texture_rectangle_ptr);
   
   int segment_color_index=-1;	// random segment coloring
   videofunc::draw_line_segments(
      linesegments,texture_rectangle_ptr,segment_color_index);
   string linesegments_filename="./linesegments.jpg";
   texture_rectangle_ptr->write_curr_frame(linesegments_filename);

   cout << "Exported detected line segments to "+linesegments_filename
        << endl;

   delete texture_rectangle_ptr;
}
