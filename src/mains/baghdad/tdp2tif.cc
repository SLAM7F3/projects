// ==========================================================================
// Program TDP2TIF reads in a TDP file containing just XYZ points
// (with no p information) and maps it onto a ztwoDarray.  It then
// outputs the ztwoDarray's contents into a geotiff file.  

//			tdp2tif baghdad36_lfit.tdp

// ==========================================================================
// Last updated on 5/9/07; 12/7/07; 3/18/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   string cloud_filename=
      passes_group.get_pass_ptr(cloudpass_ID)->get_first_filename();
//   cout << "cloud_filename = " << cloud_filename << endl;

//   double delta_x=1.0;
//   double delta_x=10.0;
   double delta_x=100.0;    	// meters
//   double delta_y=1.0;
//   double delta_y=10.0;
   double delta_y=100.0;	// meters
   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      cloud_filename,delta_x,delta_y);
   raster_parser RasterParser;
   string geotiff_filename=stringfunc::prefix(cloud_filename)+".tif";

//   int output_UTM_zonenumber=14;	// Lubbock
//   int output_UTM_zonenumber=18;	// NYC
//   int output_UTM_zonenumber=19;	// Boston
//   int output_UTM_zonenumber=38;	// Baghdad
   int output_UTM_zonenumber=42;	// Afghanistan
   bool output_northern_hemisphere_flag=true;

   bool output_floats_flag=true;
//   bool output_floats_flag=false;

   if (output_floats_flag)
   {
      RasterParser.write_raster_data(
         output_floats_flag,geotiff_filename,
         output_UTM_zonenumber,output_northern_hemisphere_flag,
         ztwoDarray_ptr);
   }
   else
   {

// Lubbock z bounds:

//      double z_min = 862.37097168;
//      double z_max = 1060.62304688;

// NYC z bounds:

//      double z_min = 0;
//      double z_max = 396;

// Afghanistan z bounds:

      double z_min = -100; // meters
      double z_max = 6000; // meters

      RasterParser.write_raster_data(
         output_floats_flag,geotiff_filename,
         output_UTM_zonenumber,output_northern_hemisphere_flag,ztwoDarray_ptr,
         z_min,z_max);
   }


}
