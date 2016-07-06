// ========================================================================
// Program FUSE_DTED_TRUEARTH is a specialized fusion program written
// to merge Afghanistan DTED p files with color TRUEARTH imagery
// read in from GEOTIF files.

//   fuse_dted_truearth tdp_tile_latitude tdp_tile_longitude UTM_zonenumber

//   fuse_dted_truearth 30 58 12

// ========================================================================
// Last updated on 3/29/10; 3/30/10; 4/1/10; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "gdal_priv.h"
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
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

   bool northern_hemisphere_flag=true;
   geofunc::print_recommended_UTM_zonenumbers();
//   cout << "Enter specified UTM zonenumber for output 3D map:" << endl;
//   cout << endl;
//   cin >> specified_UTM_zonenumber;

   int input_latitude=stringfunc::string_to_number(input_arguments[1]);
   int input_longitude=stringfunc::string_to_number(input_arguments[2]);
   int specified_UTM_zonenumber=stringfunc::string_to_number(
      input_arguments[3]);
   cout << "input_long = " << input_longitude 
        << " input_lat = " << input_latitude << endl;
   cout << "specified UTM zonenumber = " << specified_UTM_zonenumber
        << endl;
   
   string extra_lon_char="";
   if (mathfunc::ndigits_before_decimal_point(input_longitude)==2)
   {
      extra_lon_char="0";
   }

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   PassesGroup passes_group(&arguments);
//   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

//   string subdir_basename;
//   cout << "Enter basename for subdirectories containing input TDP files:" 
//        << endl;
//   cin >> subdir_basename;
   string subdir_basename="/media/disk/tdp_files/Overlap/";

   string tdp_subdir,tdp_filename;
//   string tdp_subdir="/data/DTED/Afghanistan/tdp/e0"
//   string tdp_subdir="/media/disk/Afghanistan/tdp/dted/e0"
//   string tdp_subdir="/media/TruEarth_01/DTED/tdp/e"+extra_lon_char
//      +stringfunc::number_to_string(input_longitude)+"/";

   string dted_filename_prefix=
      "n"+stringfunc::integer_to_string(input_latitude,2);

   if (input_longitude < 0)
   {
      tdp_subdir=subdir_basename+"w"+extra_lon_char+
         stringfunc::number_to_string(fabs(input_longitude))+"/";
      tdp_filename=tdp_subdir+
         "w"+extra_lon_char+stringfunc::number_to_string(
            fabs(input_longitude))+dted_filename_prefix+".tdp";
   }
   else
   {
      tdp_subdir=subdir_basename+"e"+extra_lon_char+
         stringfunc::number_to_string(input_longitude)+"/";
      tdp_filename=tdp_subdir+
         "e"+extra_lon_char+stringfunc::number_to_string(input_longitude)+
         dted_filename_prefix+".tdp";
   }
   cout << "tdp_filename = " << tdp_filename << endl;
//   int n_points=tdpfunc::npoints_in_tdpfile(tdp_filename);

// Store XYZ info within STL vectors:

// For corrupted DTED tiles e39n28 and e39n29, approximate terrain
// with constant height of alt=676 meters:

   double delta_x=35;	// meters
   double delta_y=35;	// meters
   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);
   twoDarray* ptwoDarray_ptr=new twoDarray(*ztwoDarray_ptr);
   ptwoDarray_ptr->initialize_values(-1);

   double xlo=ztwoDarray_ptr->get_xlo();
   double xhi=ztwoDarray_ptr->get_xhi();
   double ylo=ztwoDarray_ptr->get_ylo();
   double yhi=ztwoDarray_ptr->get_yhi();
   
   cout << "xlo = " << ztwoDarray_ptr->get_xlo() << endl;
   cout << "xhi = " << ztwoDarray_ptr->get_xhi() << endl;
   cout << "xdim = " << ztwoDarray_ptr->get_xdim() << endl;

   cout << "ylo = " << ztwoDarray_ptr->get_ylo() << endl;
   cout << "yhi = " << ztwoDarray_ptr->get_yhi() << endl;
   cout << "ydim = " << ztwoDarray_ptr->get_ydim() << endl;

// Next read in TruEarth EO GEOTIF files:

   string extra_lat_char="";
   if (mathfunc::ndigits_before_decimal_point(input_latitude+1)==1)
   {
      extra_lat_char="0";
   }
   cout << "extra_lat_char = " << extra_lat_char << endl;

   string geotif_subdir="/media/TruEarth_01/";
   string geotif_filename;
   if (input_longitude < 0)
   {
      geotif_filename=geotif_subdir+"N"+extra_lat_char+
         stringfunc::number_to_string(input_latitude+1)+
         "W"+extra_lon_char+stringfunc::number_to_string(
            fabs(input_longitude))+"_TruEarth15(c)2008.tif";
   }
   else
   {
      geotif_filename=geotif_subdir+"N"+extra_lat_char+
         stringfunc::number_to_string(input_latitude+1)+
         "E"+extra_lon_char+stringfunc::number_to_string(input_longitude)+
         "_TruEarth15(c)2008.tif";
   }

   raster_parser RasterParser;
   RasterParser.open_image_file(geotif_filename);

   int n_channels=RasterParser.get_n_channels();
   cout << "n_channels = " << n_channels << endl;

   twoDarray *RtwoDarray_ptr,*GtwoDarray_ptr,*BtwoDarray_ptr;
   RtwoDarray_ptr=GtwoDarray_ptr=BtwoDarray_ptr=NULL;
   double longitude_lo,longitude_hi,latitude_lo,latitude_hi;
   for (int channel_ID=0; channel_ID<n_channels; channel_ID++)
   {
      RasterParser.fetch_raster_band(channel_ID);

      if (n_channels==3)
      {
         cout << "channel_ID = " << channel_ID << endl;
         if (channel_ID==0)
         {
            RtwoDarray_ptr=RasterParser.get_RtwoDarray_ptr();
//            cout << "RtwoDarray_ptr = " << RtwoDarray_ptr << endl;
            RasterParser.read_raster_data(RtwoDarray_ptr);

            longitude_lo=RtwoDarray_ptr->get_xlo();
            longitude_hi=RtwoDarray_ptr->get_xhi();
            latitude_lo=RtwoDarray_ptr->get_ylo();
            latitude_hi=RtwoDarray_ptr->get_yhi();

            cout << "longitude_lo = " << longitude_lo
                 << " longitude_hi = " << longitude_hi << endl;
            cout << "latitude_lo = " << latitude_lo
                 << " latitude_hi = " << latitude_hi << endl;
            
         }
         else if (channel_ID==1)
         {
            GtwoDarray_ptr=RasterParser.get_GtwoDarray_ptr();
//            cout << "GtwoDarray_ptr = " << GtwoDarray_ptr << endl;
            RasterParser.read_raster_data(GtwoDarray_ptr);
         }
         else if (channel_ID==2)
         {
            BtwoDarray_ptr=RasterParser.get_BtwoDarray_ptr();
//            cout << "BtwoDarray_ptr = " << BtwoDarray_ptr << endl;
            RasterParser.read_raster_data(BtwoDarray_ptr);
         }
      } // n_channels conditional
   } // loop over channel_ID labeling RGB channels
   
   geopoint lower_left_corner(
      longitude_lo,latitude_lo,0,specified_UTM_zonenumber);
   geopoint lower_right_corner(
      longitude_hi,latitude_lo,0,specified_UTM_zonenumber);
   geopoint upper_left_corner(
      longitude_lo,latitude_hi,0,specified_UTM_zonenumber);
   geopoint upper_right_corner(
      longitude_hi,latitude_hi,0,specified_UTM_zonenumber);

   xlo=basic_math::min(xlo,lower_left_corner.get_UTM_posn().get(0),
           upper_left_corner.get_UTM_posn().get(0));
   ylo=basic_math::min(ylo,lower_left_corner.get_UTM_posn().get(1),
           lower_right_corner.get_UTM_posn().get(1));
   xhi=basic_math::max(xhi,lower_right_corner.get_UTM_posn().get(0),
           upper_right_corner.get_UTM_posn().get(0));
   yhi=basic_math::max(yhi,upper_left_corner.get_UTM_posn().get(1),
           upper_right_corner.get_UTM_posn().get(1));

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
      
//   cout << "lower_left_corner = " << lower_left_corner << endl;
//   cout << "lower_right_corner = " << lower_right_corner << endl;
//   cout << "upper_left_corner = " << upper_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;
//      cout << "px_lo = " << px_lo << " px_hi = " << px_hi
//           << " xlo = " << xlo << " xhi = " << xhi << endl;
//      cout << "py_lo = " << py_lo << " py_hi = " << py_hi
//           << " ylo = " << ylo << " yhi = " << yhi << endl;

// As of 5/7/09, we collapse RGB TruEarth values to a single intensity
// which we store within *ptwoDarray_ptr.  We'll someday return to
// extracting the full RGB color content from the TruEarth Geotif files:

   double x,y,curr_longitude,curr_latitude;
   for (unsigned int px=px_lo; px<=px_hi; px++)
   {
      ztwoDarray_ptr->px_to_x(px,x);
      for (unsigned int py=py_lo; py<=py_hi; py++)
      {
         ztwoDarray_ptr->py_to_y(py,y);
         latlongfunc::UTMtoLL(
            specified_UTM_zonenumber,northern_hemisphere_flag,
            y,x,curr_latitude,curr_longitude);
         unsigned int qx,qy;
         if (RtwoDarray_ptr->point_to_pixel(
            curr_longitude,curr_latitude,qx,qy))
         {
            double new_R=RtwoDarray_ptr->get(qx,qy);
            double new_G=GtwoDarray_ptr->get(qx,qy);
            double new_B=BtwoDarray_ptr->get(qx,qy);

            double r=new_R/255.0;
            double g=new_G/255.0;
            double b=new_B/255.0;
            double h,s,v;
            colorfunc::RGB_to_hsv(r,g,b,h,s,v);
//            new_RtwoDarray_ptr->put(px,py,new_R);
//            new_GtwoDarray_ptr->put(px,py,new_G);
//            new_BtwoDarray_ptr->put(px,py,new_B);

//            double curr_p=RtwoDarray_ptr->get(qx,qy)/255.0;
            double curr_p=v;
//            cout << "px = " << px << " py = " << py
//                 << " curr_p = " << curr_p << endl;
            ptwoDarray_ptr->put(px,py,curr_p);
         }
      } // loop over py index
   } // loop over px index

// On 5/7/09, we observed noticeable seams between 1 deg x 1 deg tiles
// where ptwoDarray = initial -1 value.  To minimize such seams, we
// replace the -1 null values with averages of non-null valued
// neighbors within *ptwoDarray_ptr:

   int n_replacements=0;
   int mdim=ptwoDarray_ptr->get_mdim();
   int ndim=ptwoDarray_ptr->get_ndim();
   
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         double curr_p=ptwoDarray_ptr->get(px,py);
         if (curr_p < 0)
         {
            vector<double> neighbor_p_values;
            for (int qx=-1; qx<=1; qx++)
            {
               if (px+qx >=0 && px+qx <mdim)
               {
                  for (int qy=-1; qy<=1; qy++)
                  {
                     if (py+qy >= 0 && py+qy < ndim)
                     {
                        double neighbor_p=ptwoDarray_ptr->get(px+qx,py+qy);
                        const double SMALL=0.01;
                        if (neighbor_p > SMALL)
                        {
                           neighbor_p_values.push_back(neighbor_p);
                        } // neighbor_p > SMALL conditional
                     } 
                  } // loop over qy
               }
            } // loop over qx

            double numer=0;
            double denom=0;
            for (unsigned int r=0; r<neighbor_p_values.size(); r++)
            {
               numer += neighbor_p_values[r];
               denom++;
            }
            double replacement_p=numer/denom;
            ptwoDarray_ptr->put(px,py,replacement_p);
            n_replacements++;
//                           cout << "n_repl=" << n_replacements 
//                                << " px=" << px << " py=" << py 
//                                << " curr_p=" << curr_p
//                                << " repl_p=" << replacement_p << endl;

         } // curr_p < 0 conditional
      } // loop over py index
   } // loop over px index

   cout << "mdim*ndim = " << mdim*ndim 
        << " n_replacements = " << n_replacements << endl;

// Write colored XYZ points to output tdp file:
   
   string UTMzone=stringfunc::number_to_string(specified_UTM_zonenumber);
   string prefix=stringfunc::prefix(tdp_filename);
   string output_tdp_filename=prefix+"_zp.tdp";
   string output_osga_filename=prefix+"_zp.osga";
//   string output_tdp_filename=prefix+"_rgb.tdp";
   cout << "Output tdp_filename = " << output_tdp_filename << endl;

//   tdpfunc::write_relative_xyzrgba_data(
//      output_tdp_filename,UTMzone,
//      ztwoDarray_ptr,new_RtwoDarray_ptr,
//      new_GtwoDarray_ptr,new_BtwoDarray_ptr);

   tdpfunc::write_zp_twoDarrays(
      output_tdp_filename,UTMzone,
      ztwoDarray_ptr,ptwoDarray_ptr,false);

//   string lodtree_cmd="/usr/local/bin/lodtree "+output_tdp_filename;
//   string unix_command=lodtree_cmd+" -o "+tdp_subdir+" "+output_tdp_filename;
//   string unix_command=lodtree_cmd+output_tdp_filename;
//   cout << "lodtree command = " << lodtree_cmd << endl;
//   sysfunc::unix_command(lodtree_cmd);

/*
   string final_osga_subdir="/media/disk/osga_files/fused_DTED_EO/";
   string unix_command="mv "+output_osga_filename+" "+final_osga_subdir;
   cout << unix_command << endl;
   sysfunc::unix_command(unix_command);
*/

//   unix_command="/bin/rm "+output_tdp_filename;
//   cout << unix_command << endl;
//   sysfunc::unix_command(unix_command);
}

