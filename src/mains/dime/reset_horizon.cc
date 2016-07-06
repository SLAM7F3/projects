// ========================================================================
// Program RESET_HORIZON takes in sinusoidal fit parameter py_avg
// found in program HORIZON.  It shifts WISP panel jpgs so that py_avg
// --> 0.5*pixel height.  After this shift, the horizon separating sky
// from sea lies (on average) at the vertical mid-line of each WISP
// panel image.


//			      ./reset_horizon

// ========================================================================
// Last updated on 3/15/13; 3/16/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string panels_subdir=
      "/data_third_disk/DIME/panoramas/Feb2013_DeerIsland/panels/";
   int n_panels=10;

/*

Number of input theta,py horizon pairs = 25
Average residual = 0.246382 pixels

Best A value = 10.1289 pixels 
Best phi value = 309.848 degs
Best py_avg value = 1114.04 pixels

*/

   int measured_py_avg_value=1114;	// pixels
   int corrected_py_avg_value=1100;	// pixels
   int d_py=measured_py_avg_value-corrected_py_avg_value;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle;
   for (int p=0; p<n_panels; p++)
   {
      string input_panel_filename=panels_subdir+
         "wisp_p"+stringfunc::number_to_string(p)+"_res0_00000.png";

      cout << "Correcting average horizon within panel p = " << p << endl;
      texture_rectangle_ptr->import_photo_from_file(input_panel_filename);
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;
//      int n_channels=texture_rectangle_ptr->getNchannels();
//      cout << "n_channels = " << n_channels << endl;

      twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->
         fill_ptwoDarray_from_single_channel_byte_data();

      for (int px=0; px<width; px++)
      {
         for (int py=0; py<height; py++)
         {
            int pixel_intensity=0;
            if (py+d_py < height)
            {
               pixel_intensity=texture_rectangle_ptr->
                  get_pixel_intensity(px,py+d_py);
            }
            texture_rectangle_ptr->set_pixel_intensity_value(
               px,py,pixel_intensity);
         }
      }

      string output_panel_filename=panels_subdir+
         "horizon_p"+stringfunc::number_to_string(p)+
         "_res0_00000.png";
      texture_rectangle_ptr->write_curr_frame(output_panel_filename);
   }
   
   delete texture_rectangle_ptr;
}
