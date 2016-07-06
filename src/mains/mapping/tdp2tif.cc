// ==========================================================================
// Program TDP2TIF reads in a TDP file [ containing XYZ points with or
// without P information ] and maps it onto a ztwoDarray.  It then
// outputs the ztwoDarray's contents into a geotiff file.  

//			tdp2tif baghdad36_lfit.tdp

// ~los/programs/c++/svn/projects/src/mains/mapping/tdp2tif e040n00_zp.tdp

// Use gdal_merge.py in order to combine together multiple geotif
// files into a large mosaic.  See README.GDAL for details.

// ==========================================================================
// Last updated on 10/1/11; 10/2/11; 11/15/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
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
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   string cloud_filename=
      passes_group.get_pass_ptr(cloudpass_ID)->get_first_filename();
   cout << "cloud_filename = " << cloud_filename << endl;
   string prefix=cloud_filename.substr(0,cloud_filename.length()-4);
//   cout << "prefix = " << prefix << endl;
   string geotiff_filename=prefix+".tif";
   cout << "Output geotiff_filename = " << geotiff_filename << endl;

//   double delta_x=0.3;	// meters	
   double delta_x=0.5;		// meters      
//   double delta_x=1.0;		// meters	NYC, Boston
//   double delta_x=10.0;
//   double delta_x=100.0;	// meters	Afghanistan SRTM

//   double delta_y=0.3;	// meters	
   double delta_y=0.5;		// meters      
//   double delta_y=1.0;		// meters	NYC,Boston
//   double delta_y=10.0;
//   double delta_y=100.0;	// meters	Afghanistan SRTM

   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      cloud_filename,delta_x,delta_y);

/*
   pair<twoDarray*,twoDarray*> P=
      tdpfunc::generate_ztwoDarray_and_ptwoDarray_from_tdpfile(
         cloud_filename,delta_x,delta_y);

   twoDarray* ztwoDarray_ptr=P.first;
   twoDarray* ptwoDarray_ptr=P.second;
*/

//    outputfunc::enter_continue_char();

/*
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      stringfunc::prefix(cloud_filename),"z");
   cout << "substrings[0] = " << substrings[0] << endl;

// Added following hacked logic in order to generate output geotiff
// filename based upon input TDP filename for Horn of Africa maps
// generated in April 2010:

   string filename=substrings[0];
   string basename=filefunc::getbasename(filename);
   string dirname=filefunc::getdirname(filename);
//   cout << "basename = " << basename << endl;
//   cout << "dirname = " << dirname << endl;
   string modified_basename=basename.substr(0,basename.length()-1);
//   cout << "modified_basename = " << modified_basename << endl;

//   string geotiff_filename=stringfunc::prefix(cloud_filename)+".tif";
   string geotiff_filename=dirname+modified_basename+".tif";
   cout << "Output geotiff_filename = " << geotiff_filename << endl;
*/

   bool northern_hemisphere_flag=true;
   geofunc::print_recommended_UTM_zonenumbers();
//   int output_UTM_zonenumber=12;	// CA/AZ
   int output_UTM_zonenumber=18;	// Haiti, NYC
//   int output_UTM_zonenumber=19;	// Boston
//   int output_UTM_zonenumber=38;	// Horn of Africa
//   int output_UTM_zonenumber=42;	// Afghanistan
   bool output_northern_hemisphere_flag=true;

   bool output_floats_flag=true;
//   bool output_floats_flag=false;

   raster_parser RasterParser;
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

//      double z_min = -100; // meters
//      double z_max = 8000; // meters

//      double z_min = 1270; // meters
//      double z_max = 1460; // meters


      double z_min = 0; // meters
      double z_max = 50; // meters

      RasterParser.write_raster_data(
         output_floats_flag,geotiff_filename,
         output_UTM_zonenumber,output_northern_hemisphere_flag,ztwoDarray_ptr,
         z_min,z_max);
   }


}
