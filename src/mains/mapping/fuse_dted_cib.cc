// ========================================================================
// Program FUSE_DTED_CIB is a specialized fusion program written to
// merge DTED tdp files with CIB imagery read in from NTIF files.

//	    fuse_dted_cib tdp_tile_longitude tdp_tile_latitude


// ========================================================================
// Last updated on 3/11/09; 12/4/10; 10/19/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "gdal_priv.h"
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;

   vector<string> input_arguments;
   for (int a=0; a<argc; a++)
   {
      string curr_arg=argv[a];
      input_arguments.push_back(curr_arg);
//      cout << "a = " << a << " argv[a] = " << argv[a] 
//           << " curr_arg = " << input_arguments.back() << endl;
   }

   int curr_longitude=stringfunc::string_to_number(input_arguments[1]);
   int curr_latitude=stringfunc::string_to_number(input_arguments[2]);
   cout << "curr_long = " << curr_longitude 
        << " curr_lat = " << curr_latitude << endl;
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   string tdp_subdir="/data/DTED/Afghanistan/tdp/e0"
//      +stringfunc::number_to_string(curr_longitude)+"/";
//   string tdp_filename="e"+stringfunc::number_to_string(curr_longitude)+
//      "n"+stringfunc::number_to_string(curr_latitude)+".tdp";
   string tdp_subdir="/media/disk/Thunderstorm/MA_SRTM/tdp_files/";
   string tdp_filename="w0"+stringfunc::number_to_string(curr_longitude)+
      "n"+stringfunc::number_to_string(curr_latitude)+".tdp";
   tdp_filename = tdp_subdir+tdp_filename;
   cout << "tdp_filename = " << tdp_filename << endl;
   int n_points=tdpfunc::npoints_in_tdpfile(tdp_filename);

// Store XYZ info within STL vectors:

   bool northern_hemisphere_flag=true;
   int UTM_zonenumber=19;	// Boston
//   int UTM_zonenumber=42;	// Afghanistan

   double delta_x=35;	// meters
   double delta_y=35;	// meters
   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);
   twoDarray* ptwoDarray_ptr=new twoDarray(*ztwoDarray_ptr);

   cout << "xlo = " << ztwoDarray_ptr->get_xlo() << endl;
   cout << "xhi = " << ztwoDarray_ptr->get_xhi() << endl;
   cout << "xdim = " << ztwoDarray_ptr->get_xdim() << endl;

   cout << "ylo = " << ztwoDarray_ptr->get_ylo() << endl;
   cout << "yhi = " << ztwoDarray_ptr->get_yhi() << endl;
   cout << "ydim = " << ztwoDarray_ptr->get_ydim() << endl;

// Next read in EO NTIF files:

//   string CIB_subdir="/data/DTED/Afghanistan/CIB/RPF/N"+
//      stringfunc::number_to_string(curr_latitude)+
//      "E0"+stringfunc::number_to_string(curr_longitude)+"/";
   string CIB_subdir="/home/cho/programs/c++/svn/projects/src/mains/mapping/MA_EO/CIB/RPF/N"+
      stringfunc::number_to_string(curr_latitude)+
      "W0"+stringfunc::number_to_string(curr_longitude)+"/";
   string ntifs_filename=CIB_subdir+"ntif_files.dat";

   string unix_command;
   if (filefunc::fileexist(ntifs_filename))
   {
      unix_command="/bin/rm "+ntifs_filename;
      sysfunc::unix_command(unix_command);
   }
   
   unix_command="find "+CIB_subdir+" -name \"*.I2*\" -print ";
   unix_command += "> "+ntifs_filename;
   cout << "unix find command = " << unix_command << endl;
   sysfunc::unix_command(unix_command);

   filefunc::ReadInfile(ntifs_filename);

   vector<string> ntif_filename;
   int n_EO_subtiles=filefunc::text_line.size();
   
   for (int i=0; i<n_EO_subtiles; i++)
   {
      ntif_filename.push_back(filefunc::text_line[i]);
//      cout << i << "  " << ntif_filename.back() << endl;
      cout << i << " " << flush;

      raster_parser RasterParser;
      RasterParser.open_image_file(ntif_filename.back());

      int channel_ID=0;
      RasterParser.fetch_raster_band(channel_ID);

      twoDarray* EO_twoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
      RasterParser.read_raster_data(EO_twoDarray_ptr);

      double longitude_lo=EO_twoDarray_ptr->get_xlo();
      double longitude_hi=EO_twoDarray_ptr->get_xhi();
      double latitude_lo=EO_twoDarray_ptr->get_ylo();
      double latitude_hi=EO_twoDarray_ptr->get_yhi();

      geopoint lower_left_corner(
         longitude_lo,latitude_lo,0,UTM_zonenumber);
      geopoint upper_right_corner(
         longitude_hi,latitude_hi,0,UTM_zonenumber);

      double xlo=lower_left_corner.get_UTM_posn().get(0);
      double ylo=lower_left_corner.get_UTM_posn().get(1);
      double xhi=upper_right_corner.get_UTM_posn().get(0);
      double yhi=upper_right_corner.get_UTM_posn().get(1);

      unsigned int px_lo,py_lo,px_hi,py_hi;
      ztwoDarray_ptr->point_to_pixel(xlo,yhi,px_lo,py_lo);
      ztwoDarray_ptr->point_to_pixel(xhi,ylo,px_hi,py_hi);

      px_lo -= 30;
      py_lo -= 30;
      px_hi += 30;
      py_hi += 30;

      px_lo=basic_math::max(Unsigned_Zero,px_lo);
      py_lo=basic_math::max(Unsigned_Zero,py_lo);
      px_hi=basic_math::min(ztwoDarray_ptr->get_xdim()-1,px_hi);
      py_hi=basic_math::min(ztwoDarray_ptr->get_ydim()-1,py_hi);
      
//      cout << "lower_left_corner = " << lower_left_corner << endl;
//      cout << "upper_right_corner = " << upper_right_corner << endl;
//      cout << "px_lo = " << px_lo << " px_hi = " << px_hi
//           << " xlo = " << xlo << " xhi = " << xhi << endl;
//      cout << "py_lo = " << py_lo << " py_hi = " << py_hi
//           << " ylo = " << ylo << " yhi = " << yhi << endl;

      double x,y,curr_longitude,curr_latitude;
      for (int px=px_lo; px<=px_hi; px++)
      {
         ztwoDarray_ptr->px_to_x(px,x);
         for (int py=py_lo; py<=py_hi; py++)
         {
            ztwoDarray_ptr->py_to_y(py,y);
            latlongfunc::UTMtoLL(
               UTM_zonenumber,northern_hemisphere_flag,
               y,x,curr_latitude,curr_longitude);
            unsigned int qx,qy;
            if (EO_twoDarray_ptr->point_to_pixel(
               curr_longitude,curr_latitude,qx,qy))
            {
               double curr_p=EO_twoDarray_ptr->get(qx,qy)/255.0;
//               cout << "px = " << px << " py = " << py
//                    << " curr_p = " << curr_p << endl;

// CIB imagery for at least Massachusetts is very dark.  So we amplify
// p-values in order to brighten up fused ZP imagery:

               curr_p *= 1.5;
//               curr_p *= 2.0;
//               curr_p *= 2.5;
//               curr_p *= 3.0;
               curr_p=basic_math::min(curr_p,1.0);

               ptwoDarray_ptr->put(px,py,curr_p);
            }
         } // loop over py index
      } // loop over px index
   } // loop over index i labeling NTIF subtile files
   cout << endl;

// Write colored XYZ points to output tdp file:
   
   string UTMzone=stringfunc::number_to_string(UTM_zonenumber);
   string prefix=stringfunc::prefix(tdp_filename);
   string output_tdp_filename=prefix+"_zp.tdp";
   cout << "Output dp_filename = " << output_tdp_filename << endl;

   tdpfunc::write_zp_twoDarrays(
      output_tdp_filename,UTMzone,
      ztwoDarray_ptr,ptwoDarray_ptr,false);

   unix_command="lodtree -o "+tdp_subdir+" "+output_tdp_filename;
   cout << "lodtree command = " << unix_command << endl;
   sysfunc::unix_command(unix_command);
}
