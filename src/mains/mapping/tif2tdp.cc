// ==========================================================================
// Program TIF2TDP takes in a some registered geotiff height map and
// generates a corresponding point cloud in TDP output format.  The
// UTM zone for the output TDP file can differ from that within the
// input geotiff file via setting integer TDP_specified_UTM_zonenumber.

// 			    tif2tdp w109n30.tif


// ==========================================================================
// Last updated on 4/13/11; 10/2/11; 11/15/11
// ==========================================================================

#include <iostream>
#include <string>
#include "gdal_priv.h"
#include "astro_geo/geopoint.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   Pass* pass_ptr=passes_group.get_pass_ptr(0);
   string image_filename=pass_ptr->get_first_filename();
   cout << "image_filename = " << image_filename << endl;

/*
   string image_filename;
   cout << "Enter input geotiff file name:" << endl;
   cin >> image_filename;
//   string image_filename="copley.jpg";
//   image_filename="baghdad37.tif";
//   string image_filename="cats.jp2";
//   string image_filename="clewiston.jp2";
*/

   string filename_prefix=stringfunc::prefix(image_filename);

   raster_parser RasterParser;
   RasterParser.open_image_file(image_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);

   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
//   cout << "ztwoDarray_ptr = " << ztwoDarray_ptr << endl;
   RasterParser.read_raster_data(ztwoDarray_ptr);

// Compute geocoordinates for raster image's four corners:

   bool geotif_northern_hemisphere_flag=
      RasterParser.get_northern_hemisphere_flag();
   int geotif_specified_UTM_zonenumber=
      RasterParser.get_specified_UTM_zonenumber();

// Easting,northing values from geotif's UTM zone may be reset to new
// values for UTM zone for output TDP file:

   int TDP_specified_UTM_zonenumber=geotif_specified_UTM_zonenumber;

   geopoint lower_left_corner(
      geotif_northern_hemisphere_flag,geotif_specified_UTM_zonenumber,
      ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_ylo());

   geopoint upper_left_corner(
      geotif_northern_hemisphere_flag,geotif_specified_UTM_zonenumber,
      ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_yhi());
   
   geopoint lower_right_corner(
      geotif_northern_hemisphere_flag,geotif_specified_UTM_zonenumber,
      ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_ylo());

   geopoint upper_right_corner(
      geotif_northern_hemisphere_flag,geotif_specified_UTM_zonenumber,
      ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_yhi());

   cout << "lower_left_corner = " << lower_left_corner << endl;
   cout << "upper_left_corner = " << upper_left_corner << endl;
   cout << "lower_right_corner = " << lower_right_corner << endl;
   cout << "upper_right_corner = " << upper_right_corner << endl;


// Write out TDP file:

   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   double easting,northing;

/*
   double min_easting=312137;
   double max_easting=312297;
   double min_northing=4703727.5;
   double max_northing=4703868;
   double min_Z=0;
   double max_Z=23.729;
*/

   double min_Z=0;
 

   osg::Vec3 XYZ;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
   vertices_ptr->reserve(mdim*ndim);
   for (int px=0; px<mdim; px++)
   {
      if (px%100==0) cout << px << " " << flush;
      for (int py=0; py<ndim; py++)
      {
         ztwoDarray_ptr->pixel_to_point(px,py,easting,northing);

//         if (easting < min_easting || easting > max_easting) continue;
//         if (northing < min_northing || northing > max_northing) continue;

         double altitude=ztwoDarray_ptr->get(px,py);
//         if (altitude < min_Z || altitude > max_Z) continue;

         if (TDP_specified_UTM_zonenumber != geotif_specified_UTM_zonenumber)
         {
            geopoint curr_point(
               geotif_northern_hemisphere_flag,geotif_specified_UTM_zonenumber,
               easting,northing);
            curr_point.recompute_UTM_coords(TDP_specified_UTM_zonenumber);
            easting=curr_point.get_UTM_easting();
            northing=curr_point.get_UTM_northing();
         }

         XYZ[0]=easting;
         XYZ[1]=northing;
         XYZ[2]=altitude;

// Ignore any XYZ point whose Z value lies below some minimal
// threshold:

         if (XYZ[2] < min_Z) continue;
         
         vertices_ptr->push_back(XYZ);
      }
   }
   cout << endl;

   string tdp_filename=filename_prefix+".tdp";
   cout << "Output tdp_filename = " << tdp_filename << endl;
   string UTMzone="";
   tdpfunc::write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
} 

