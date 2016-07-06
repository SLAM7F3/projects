// ==========================================================================
// Program PERSPEC_ROT is a testing grounds for rotating text planes
// in 3D and then perspectively projecting into a virtual camera image
// plane.
// ==========================================================================
// Last updated on 3/7/16; 3/8/16; 3/14/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   double FOV_u = 45 * PI/180;
   double aspect_ratio = 640.0 / 480.0;
   double max_pixel_width = 3 * 321;

   string syntext_subdir="./images/syntext/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      syntext_subdir);
   string output_subdir=syntext_subdir+"output/";
   filefunc::dircreate(output_subdir);

//   int n_steps = 3;
   int n_steps = 15;
//   int n_steps = 21;

   double az_start = -60 * PI/180;
   double az_stop = 60 * PI/180;
   double daz = (az_stop - az_start) / (n_steps - 1);

   double el_start = -30 * PI/180;
   double el_stop = 30 * PI/180;
   double del = (el_stop - el_start) / (n_steps - 1);

   double roll_start = -20 * PI/180;
   double roll_stop = 20 * PI/180;
   double droll = (roll_stop - roll_start) / (n_steps - 1);
   
   int iter_start = 0;
   int iter_stop = 5;
   for(int iter = iter_start; iter <= iter_stop; iter++)
   {
      cout << "Rotating image " << iter << endl;
      string curr_image_filename=image_filenames[iter];

      for(int t = 0; t < n_steps; t++)
      {
//         double img_az = 0;
         double img_az = az_start + t * daz;
//         double img_el = 0;
         double img_el = el_start + t * del;
//         double img_roll = 0;
         double img_roll = roll_start + t * droll;
         
         string output_test_subdir=output_subdir+
            "test"+stringfunc::number_to_string(iter)+"/";
         filefunc::dircreate(output_test_subdir);
         string output_image_filename=output_test_subdir
            +stringfunc::prefix(filefunc::getbasename(curr_image_filename))
            +"_"+stringfunc::integer_to_string(t,3)+"_rot.png";

         string background_color = "black";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            curr_image_filename, background_color, output_image_filename);

      } // loop over index t labeling az steps

   } // loop over iter index labeling input images
   
}

