// ==========================================================================
// Program CROPTDP reads in some initial Baghdad geotiff tile.  It
// then queries the user for computing the upper or lower halves in
// the horizontal and vertical directions of this tile.  CROPTDP then
// converts the selected tile quarter into TDP file output.  We wrote
// this somewhat specialized utility in order to generate manageably
// sized Baghdad TDP files.
// ==========================================================================
// Last updated on 4/2/07; 4/3/07; 4/23/07; 2/10/08
// ==========================================================================

#include <iostream>
#include <string>
#include "gdal_priv.h"
#include "astro_geo/latlong2utmfuncs.h"
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
//   image_filename="baghdad39.tif";
//   string image_filename="cats.jp2";
//   string image_filename="clewiston.jp2";

   string tile_half;
   cout << "Enter 'l' ('h') to process xlo-xmid (xmid-xhi) half of tile:"
        << endl;
   cin >> tile_half;

   bool xlo_flag=true;
   if (tile_half=="h")
   {
      xlo_flag=false;
   }

   cout << "Enter 'l' ('h') to process ylo-ymid (ymid-yhi) half of tile:"
        << endl;
   cin >> tile_half;

   bool ylo_flag=true;
   if (tile_half=="h")
   {
      ylo_flag=false;
   }

   cout << "xlo_flag = " << xlo_flag 
        << " ylo_flag = " << ylo_flag << endl;

   string filename_prefix=stringfunc::prefix(image_filename);
   RasterParser.open_image_file(image_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);
   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
   RasterParser.read_raster_data(ztwoDarray_ptr);

// Write out TDP file:

   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();
   double xlo=ztwoDarray_ptr->get_xlo();
   double xhi=ztwoDarray_ptr->get_xhi();
   double ylo=ztwoDarray_ptr->get_ylo();
   double yhi=ztwoDarray_ptr->get_yhi();

   double xmid=0.5*(xlo+xhi);
   double ymid=0.5*(ylo+yhi);

   cout << "xlo = " << xlo
        << " xmid = " << xmid
        << " xhi = " << xhi
        << endl;
   cout << "ylo = " << ylo
        << " ymid = " << ymid
        << " yhi = " << yhi
        << endl;

   double x,y;
   osg::Vec3 XYZ;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
   vertices_ptr->reserve(mdim*ndim);

   double delta_x=2.0;	// meters
   double delta_y=2.0;  // meters
   for (int px=0; px<mdim; px++)
   {
      if (px%1000==0) cout << px << " " << flush;
      for (int py=0; py<ndim; py++)
      {
         ztwoDarray_ptr->pixel_to_point(px,py,x,y);
         XYZ[0]=x;
         XYZ[1]=y;

         if ( 
            ( (xlo_flag && x < xmid+delta_x) || 
              (!xlo_flag && x >= xmid-delta_x) ) &&
            ( (ylo_flag && y < ymid+delta_y) || 
              (!ylo_flag && y >= ymid-delta_y) ) )
         {
            XYZ[2]=ztwoDarray_ptr->get(px,py);
            vertices_ptr->push_back(XYZ);
         }
      }
   }
   cout << endl;

   cout << "Number valid points = " << vertices_ptr->size() << endl;

   string suffix;
   if (xlo_flag && ylo_flag)
   {
      suffix="_lo_lo.tdp";
   }
   else if (xlo_flag && !ylo_flag)
   {
      suffix="_lo_hi.tdp";
   }
   else if (!xlo_flag && ylo_flag)
   {
      suffix="_hi_lo.tdp";
   }
   else if (!xlo_flag && !ylo_flag)
   {
      suffix="_hi_hi.tdp";
   }

   string tdp_filename=filename_prefix+suffix;
   cout << "Output tdp_filename = " << tdp_filename << endl;
   string UTMzone="38T";

   tdpfunc::write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
} 

