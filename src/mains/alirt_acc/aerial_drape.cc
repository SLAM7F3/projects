// ==========================================================================
// Program AERIAL_DRAPE is a simpler version of program DRAPE_BLDG.
// In this program, we simply drape RGB values read from a PNG file
// onto individual points within an XYZ point cloud.  We cooked up
// this main program for Fort Devens parachute tower draping purposes.
// ==========================================================================
// Last updated on 3/6/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "math/mymatrix.h"
#include "templates/mytemplates.h"
#include "geometry/parallelepiped.h"
#include "geometry/polygon.h"
#include "space/spasefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

// Open output XYZP file:

   string input_subdir="./2D_images/";
   string output_subdir="./xyzp_files/";
   filefunc::dircreate(output_subdir);
   string input_xyzp_filename=input_subdir+"parachute_tower.xyzp";
   string output_xyzp_filename=output_subdir+"draped_parachute_tower.xyzp";

// Drape parachute tower visible PNG photo #1:

   filefunc::deletefile(output_xyzp_filename);

   double missing_data_value=colorfunc::rgb_colormap_value(colorfunc::grey);
   draw3Dfunc::drape_PNG_image_onto_point_cloud(
      input_subdir,"parachute_tower1.png","parachute_tower1_xyz_uv.txt",
      input_xyzp_filename,output_xyzp_filename,missing_data_value);

// Add fake color map points into final XYZP file:

   threevector tower_ground_corner(-72794,14199,-12.7);
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      output_xyzp_filename,tower_ground_corner);
   filefunc::gunzip_file_if_gzipped(output_xyzp_filename);
}
