// ==========================================================================
// Program MATCH_BBOXES

//			./match_bboxes

// ==========================================================================
// Last updated on 5/10/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <dlib/disjoint_subsets.h>

#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "color/colortext.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "math/ltduple.h"
#include "numrec/nrfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;    
using std::cout;
using std::endl;
using std::flush;
using std::set;
using std::string;
using std::vector;


// ==========================================================================
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   
   string images_subdir=
      "./images/HouseNumbers/van/PilotKnob/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer;
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table();

   int index_start=0;
   int index_stop=1;
//   int index_stop=image_filenames.size();
   for (int image_index=index_start; image_index<index_stop; image_index++)
   {
      string image_filename=image_filenames[image_index];
      cout << "image_filename = " << image_filename << endl;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);

// Trellis

//      twovector top_left(1.03181 , 0.483917);
//      twovector bottom_right(1.3025 , 0.352202);
   
// Background yellow flowers

//      twovector top_left(1.13 , 0.298022);
//      twovector bottom_right(1.17158 , 0.270814);

// Leafy trees:

//      twovector top_left(0.0155397 , 0.691403);
//      twovector bottom_right(0.158201 , 0.604016);

// Darker pine trees:

//      twovector top_left(1.14394 , 0.673706);
//      twovector bottom_right(1.26553 , 0.603177);

// Tall tree:

      twovector top_left2(1.04151 , 0.601915);
      twovector bottom_right2(1.09084 , 0.564174);
      
// More hedge:

//      twovector top_left(0.34735 , 0.222673);
//      twovector bottom_right(0.358077 , 0.213921);


// Rose:
//      twovector top_left2(0.169244 , 0.339842);
//      twovector bottom_right2(0.174892 , 0.334129);

// Bush:

//      twovector top_left2(0.762435 , 0.150732);
//      twovector bottom_right2(0.79785 , 0.117827);

// Sky:
      twovector top_left(1.13611 , 0.905888);
      twovector bottom_right(1.18395 , 0.864631);
      
// White house side:

//      twovector top_left(0.382724 , 0.449995);
//      twovector bottom_right(0.420815 , 0.41409);


      unsigned int min_pu,max_pu,min_pv,max_pv;
      texture_rectangle_ptr->get_pixel_coords(
         top_left.get(0),top_left.get(1),min_pu,min_pv);
      texture_rectangle_ptr->get_pixel_coords(
         bottom_right.get(0),bottom_right.get(1),max_pu,max_pv);

      vector<double> color_histogram1 = RGB_analyzer_ptr-> 
         compute_bbox_color_content(
            min_pu,min_pv,max_pu,max_pv,texture_rectangle_ptr);
      RGB_analyzer_ptr->print_color_histogram(color_histogram1);
      texture_rectangle_ptr->draw_pixel_bbox(
         min_pu,max_pu,min_pv,max_pv,colorfunc::purple);



      texture_rectangle_ptr->get_pixel_coords(
         top_left2.get(0),top_left2.get(1),min_pu,min_pv);
      texture_rectangle_ptr->get_pixel_coords(
         bottom_right2.get(0),bottom_right2.get(1),max_pu,max_pv);

      vector<double> color_histogram2 = RGB_analyzer_ptr-> 
         compute_bbox_color_content(
            min_pu,min_pv,max_pu,max_pv,texture_rectangle_ptr);
      RGB_analyzer_ptr->print_color_histogram(color_histogram2);
      texture_rectangle_ptr->draw_pixel_bbox(
         min_pu,max_pu,min_pv,max_pv,colorfunc::yellow);


      double inner_product = RGB_analyzer_ptr->color_histogram_inner_product(
         color_histogram1, color_histogram2);
      cout << "inner_product = " << inner_product << endl;

      string output_filename="annotated.jpg";
      texture_rectangle_ptr->write_curr_frame(output_filename);
      string banner="Exported annotated image file to "+output_filename;
      outputfunc::write_banner(banner);

      delete texture_rectangle_ptr;
   } // loop over image_index 

   delete RGB_analyzer_ptr;

}

