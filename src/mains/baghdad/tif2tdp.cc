// ==========================================================================
// Program TIF2TDP takes in a some registered geotiff height map and
// generates a corresponding point cloud in TDP output format.

// 				tif2tdp


// ==========================================================================
// Last updated on 3/29/07; 2/10/08; 8/31/08
// ==========================================================================

#include <iostream>
#include <string>
#include "gdal_priv.h"
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

   raster_parser RasterParser;

   string image_filename;
   cout << "Enter input geotiff file name:" << endl;
   cin >> image_filename;
//   string image_filename="copley.jpg";
//   image_filename="baghdad37.tif";
//   string image_filename="cats.jp2";
//   string image_filename="clewiston.jp2";

   string filename_prefix=stringfunc::prefix(image_filename);

   RasterParser.open_image_file(image_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);

   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
   RasterParser.read_raster_data(ztwoDarray_ptr);

// Write out TDP file:

   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();

   double x,y;
   osg::Vec3 XYZ;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
   vertices_ptr->reserve(mdim*ndim);
   for (int px=0; px<mdim; px++)
   {
      if (px%100==0) cout << px << " " << flush;
      for (int py=0; py<ndim; py++)
      {
         ztwoDarray_ptr->pixel_to_point(px,py,x,y);
         XYZ[0]=x;
         XYZ[1]=y;
         XYZ[2]=ztwoDarray_ptr->get(px,py);
         vertices_ptr->push_back(XYZ);
      }
   }
   cout << endl;

   string tdp_filename=filename_prefix+".tdp";
   cout << "Output tdp_filename = " << tdp_filename << endl;
//   string UTMzone="38T";	// Baghdad
   string UTMzone="19";		// Boston


   tdpfunc::write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
} 

