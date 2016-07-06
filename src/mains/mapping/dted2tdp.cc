// ==========================================================================
// Program DTED2TDP takes in a level 0, 1 or 2 dted/SRTM height maps
// obtained from LL's library and generates a corresponding point
// cloud in TDP output format.

// 				dted2tdp

// ==========================================================================
// Last updated on 9/30/11; 2/22/13; 4/5/13
// ==========================================================================

#include <iostream>
#include <string>
#include "gdal_priv.h"
#include "astro_geo/geopoint.h"
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

   int specified_UTM_zonenumber=5;	// Hawaii Big Island
//   int specified_UTM_zonenumber=16;	// Florida panhandle
//   int specified_UTM_zonenumber=38;	// Iraq/Iran
//   int specified_UTM_zonenumber=42;	// Afghanistan
//   int specified_UTM_zonenumber=52;	// Korea

   cout << endl;
   cout << "Recommended UTM zone numbers for various world regions:" << endl;
   cout << "Hawaii: 5" << endl;
   cout << "California/Arizona: 12" << endl;
   cout << "New Mexico/Texas: 13" << endl;
   cout << "Florida panhandle: 16" << endl;
   cout << "Massachusetts: 19" << endl;
   cout << "Iraq, Yeman, Somalia: 38" << endl;
   cout << "Afghanistan/Pakistan: 42" << endl;
   cout << "Korea: 52" << endl;
   cout << endl;

   cout << "Enter specified UTM zonenumber for output 3D map:" << endl;
   cout << endl;

   cin >> specified_UTM_zonenumber;

// FL panhandle:  -80 --> -95 
// n: 25 --> 36


   int dirnumber_start=-179;
//   int dirnumber_start=-95;
//   int dirnumber_start=37;
   cout << "Enter starting directory number:" << endl;
   cout << "(Use negative value to indicate longitudes WEST of Greenwich)"
        << endl;
//   cin >> dirnumber_start;

   int dirnumber_stop=-103;
//   int dirnumber_stop=-88;
//   int dirnumber_stop=-80;
//   int dirnumber_stop=49;
   cout << "Enter stopping directory number:" << endl;
   cout << "(Use negative value to indicate longitudes WEST of Greenwich)"
        << endl;
//   cin >> dirnumber_stop;

   string extra_char="";
   if (mathfunc::ndigits_before_decimal_point(dirnumber_start)==2)
   {
      extra_char="0";
   }

   string subdir_basename;
   subdir_basename="/data_second_disk/DTED/Hawaii/dted/";
//   subdir_basename="/data_third_disk/DIME/DTED/Florida/dted/";
   cout << "Enter full path for subdirectory containing input DTED files:" 
        << endl;
//   cin >> subdir_basename;

   string tdpdir_basename;
   tdpdir_basename="/data_second_disk/DTED/Hawaii/tdp/";
//   tdpdir_basename="/data_third_disk/DIME/DTED/Florida/tdp/";

   int tilenumber_start=17;
   int tilenumber_stop=29;
//   int tilenumber_start=16;
//   int tilenumber_stop=30;
//   int tilenumber_start=25;
//   int tilenumber_stop=36;
//   int tilenumber_start=28;
//   int tilenumber_stop=39;

   cout << "Enter starting tilenumber:" << endl;
//   cin >> tilenumber_start;
   cout << "Enter stopping tilenumber:" << endl;
//   cin >> tilenumber_stop;
   
   raster_parser RasterParser;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array;

   cout << "dirnumber_start = " << dirnumber_start << endl;
   cout << "dirnumber_stop = " << dirnumber_stop << endl;
   
   for (int dirnumber=dirnumber_start; 
        dirnumber <= dirnumber_stop; dirnumber++)
   {
      string input_subdir,output_subdir;
      if (dirnumber < 0)
      {
         input_subdir=subdir_basename+"w"+extra_char+
            stringfunc::number_to_string(fabs(dirnumber))+"/";
         output_subdir=tdpdir_basename+"w"+extra_char+
            stringfunc::number_to_string(fabs(dirnumber))+"/";
      }
      else
      {
         input_subdir=subdir_basename+"e"+extra_char+
            stringfunc::number_to_string(dirnumber)+"/";
         output_subdir=tdpdir_basename+"e"+extra_char+
            stringfunc::number_to_string(dirnumber)+"/";
      }
      filefunc::dircreate(output_subdir);

      for (int tilenumber=tilenumber_start; tilenumber <= tilenumber_stop;
           tilenumber++)
      {
         string image_filename="n"+stringfunc::integer_to_string(tilenumber,2)
            +".dt2";
         cout << "image_filename = " << input_subdir+image_filename << endl;

         if (!filefunc::fileexist(input_subdir+image_filename)) continue;

//      string image_filename;
//      cout << "Enter input dted file name:" << endl;
//      cin >> image_filename;

         string filename_prefix=stringfunc::prefix(image_filename);

         RasterParser.open_image_file(input_subdir+image_filename);

         int channel_ID=0;
         RasterParser.fetch_raster_band(channel_ID);

         twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
         RasterParser.read_raster_data(ztwoDarray_ptr);
         RasterParser.close_image_file();

         cout << "xlo = " << ztwoDarray_ptr->get_xlo() << endl;
         cout << "xhi = " << ztwoDarray_ptr->get_xhi() << endl;
         cout << "ylo = " << ztwoDarray_ptr->get_ylo() << endl;
         cout << "yhi = " << ztwoDarray_ptr->get_yhi() << endl;

// Write out TDP file:

         int mdim=ztwoDarray_ptr->get_mdim();
         int ndim=ztwoDarray_ptr->get_ndim();

         geopoint curr_point;
         double longitude,latitude;
         double min_z=POSITIVEINFINITY;
         double max_z=NEGATIVEINFINITY;
         osg::Vec3 XYZ;

         vertices_ptr->clear();
         vertices_ptr->reserve(mdim*ndim);
         for (int px=0; px<mdim; px++)
         {
            if (px%100==0) cout << px << " " << flush;
            for (int py=0; py<ndim; py++)
            {
               double altitude=ztwoDarray_ptr->get(px,py);

// GDAL returns height values from DTED level 0, 1 and 2 maps as
// 16-bit integers.  As of 3/8/09, we believe that dummy -32767 height
// values probably indicate missing data.  Otherwise, the 16-bit
// integer values appear to represent genuine heights measured in
// meters:

               if (altitude < -32767+1)
               {
//            cout << " lon = " << longitude << " lat = " << latitude
//                 << " zone = " << curr_point.get_UTM_zonenumber()
//                 << " E = " << XYZ[0] << " N = " << XYZ[1]
//                 << " z = " << XYZ[2] << endl;
               }
               else
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,longitude,latitude);
                  curr_point.set_longitude(longitude);
                  curr_point.set_latitude(latitude);
                  curr_point.recompute_UTM_coords(specified_UTM_zonenumber);

                  XYZ[0]=curr_point.get_UTM_easting();
                  XYZ[1]=curr_point.get_UTM_northing();
                  XYZ[2]=altitude;

                  max_z=basic_math::max(max_z,altitude);
                  min_z=basic_math::min(min_z,altitude);
                  vertices_ptr->push_back(XYZ);
               }
            } // loop over py index
         } // loop over px index
         
//          delete ztwoDarray_ptr;

         cout << endl;
         cout << "max_z = " << max_z << " min_z = " << min_z << endl;

         string tdp_filename;
         if (dirnumber < 0)
         {
            tdp_filename=output_subdir+
               "w"+extra_char+stringfunc::number_to_string(fabs(dirnumber))+
               filename_prefix+".tdp";
         }
         else
         {
            tdp_filename=output_subdir+
               "e"+extra_char+stringfunc::number_to_string(dirnumber)+
               filename_prefix+".tdp";
         }

         cout << "Output tdp_filename = " << tdp_filename << endl;
         string UTMzone=stringfunc::number_to_string(specified_UTM_zonenumber);
         cout << "UTMzone = " << UTMzone << endl;

         tdpfunc::write_relative_xyz_data(
            tdp_filename,UTMzone,vertices_ptr);
      } // loop over tilenumber index
   } // loop over dirnumber index   
} 

