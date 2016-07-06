// ==========================================================================
// Program BASEMENT_DRAPE is a minor version of program DRAPE_BLDG.
// In this program, we drape RGB values read from 3 PNG files onto
// individual points within an XYZ point cloud.  We cooked up this
// main program for static voxels ladar lab 3D imagery draping
// purposes.
// ==========================================================================
// Last updated on 3/6/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/camerafuncs.h"
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "geometry/parallelepiped.h"
#include "geometry/polygon.h"
#include "space/spasefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "ladar/urbanfuncs.h"
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
//   string input_xyzp_filename=input_subdir+"static_voxels_sans_grid2.xyzp";
//   string input_xyzp_filename=input_subdir+"static_voxels_with_grid.xyzp";
//   string input_xyzp_filename=input_subdir+"static_voxels_wo_grid.xyzp";
   string input_xyzp_filename=input_subdir+"static_voxels_pass8_wo_grid.xyzp";
   string output_xyzp_filename=output_subdir+
      "draped_static_voxels_pass8.xyzp";

// Drape ladar lab scene PNG photo #1:

   filefunc::deletefile(output_xyzp_filename);

//   double missing_data_value=colorfunc::rgb_colormap_value(colorfunc::red);
   double missing_data_value=colorfunc::rgb_colormap_value(colorfunc::grey);

   double min_range=-20;	// meters
   double max_range=-1;		// meters
   draw3Dfunc::drape_PNG_image_onto_point_cloud(
      input_subdir,"brightened_cropped_scene.png",
      "brightened_cropped_scene_xyz_uv.txt",
      input_xyzp_filename,output_xyzp_filename,missing_data_value,
      min_range,max_range);

   min_range=-1;	// meters
   max_range=5;		// meters
   draw3Dfunc::drape_PNG_image_onto_point_cloud(
      input_subdir,"brightened_closeup.png",
      "brightened_closeup_xyz_uv.txt",
      input_xyzp_filename,output_xyzp_filename,missing_data_value,
      min_range,max_range);

   min_range=5;	// meters
   max_range=10.5;		// meters
   double min_height=-10;
   double max_height=11;
   draw3Dfunc::drape_PNG_image_onto_point_cloud(
      input_subdir,"brightened_closeup.png",
      "brightened_closeup_xyz_uv.txt",
      input_xyzp_filename,output_xyzp_filename,missing_data_value,
      min_range,max_range,min_height,max_height);

   min_range=8;	// meters
   max_range=10;		// meters
   min_height=11;
   max_height=12;
   draw3Dfunc::drape_PNG_image_onto_point_cloud(
      input_subdir,"missile_closeup#2.png","missile_xyz_uv.txt",
      input_xyzp_filename,output_xyzp_filename,missing_data_value,
      min_range,max_range,min_height,max_height);

// Add fake color map points into final XYZP file:

   threevector origin(0,0,10);
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      output_xyzp_filename,origin);
   filefunc::gunzip_file_if_gzipped(output_xyzp_filename);
}
