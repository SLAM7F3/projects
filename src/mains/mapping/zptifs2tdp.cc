// ==========================================================================
// Program ZPTIFS2TDP takes in registered geotiff height and
// probability maps and generates a corresponding point cloud in TDP
// output format.

// 				 zptifs2tdp


// ==========================================================================
// Last updated on 4/19/11; 4/22/11; 4/23/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
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
using std::vector;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   raster_parser ZRasterParser,PRasterParser;
   
// Use an ArgumentParser object to manage the program arguments:
   
   string subdir=
      "/media/66368D22368CF3F9/TOC11/FOB_blessing/FOB_Blessing_tiles/";
   string ztiles_subdir=subdir+"z_tif_tiles/tiles/";
   string ptiles_subdir=subdir+"p_tif_tiles/tiles/";
   string fused_subdir=subdir+"fused_tiles/";
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("tif");
   vector<string> ztile_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,ztiles_subdir);

   osg::Vec3 XYZ;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
   osg::FloatArray* probs_ptr=new osg::FloatArray;

   vector<string> ptile_filenames;
   for (int i=0; i<ztile_filenames.size(); i++)
   {
      cout << "i = " << i
           << " ztile_filename = " << ztile_filenames[i] << endl;
      string zimage_filename=ztile_filenames[i];
      string basename=filefunc::getbasename(ztile_filenames[i]);
      string pimage_filename=ptiles_subdir+basename;

      int channel_ID=0;
      ZRasterParser.open_image_file(zimage_filename);
      ZRasterParser.fetch_raster_band(channel_ID);
      twoDarray* ztwoDarray_ptr=ZRasterParser.get_ztwoDarray_ptr();
      ZRasterParser.read_raster_data(ztwoDarray_ptr);

      PRasterParser.open_image_file(pimage_filename);
      PRasterParser.fetch_raster_band(channel_ID);
      twoDarray* ptwoDarray_ptr=PRasterParser.get_ztwoDarray_ptr();
      PRasterParser.read_raster_data(ptwoDarray_ptr);

// Compute geocoordinates for raster image's four corners:

      geopoint lower_left_corner(
         ZRasterParser.get_northern_hemisphere_flag(),
         ZRasterParser.get_specified_UTM_zonenumber(),
         ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_ylo());

      geopoint upper_left_corner(
         ZRasterParser.get_northern_hemisphere_flag(),
         ZRasterParser.get_specified_UTM_zonenumber(),
         ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_yhi());
   
      geopoint lower_right_corner(
         ZRasterParser.get_northern_hemisphere_flag(),
         ZRasterParser.get_specified_UTM_zonenumber(),
         ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_ylo());

      geopoint upper_right_corner(
         ZRasterParser.get_northern_hemisphere_flag(),
         ZRasterParser.get_specified_UTM_zonenumber(),
         ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_yhi());

      cout << "lower_left_corner = " << lower_left_corner << endl;
      cout << "upper_left_corner = " << upper_left_corner << endl;
      cout << "lower_right_corner = " << lower_right_corner << endl;
      cout << "upper_right_corner = " << upper_right_corner << endl;

      ZRasterParser.close_image_file();
      PRasterParser.close_image_file();

      int mdim=ztwoDarray_ptr->get_mdim();
      int ndim=ztwoDarray_ptr->get_ndim();
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;

// Find min/max prob values:
   
      double pmin=POSITIVEINFINITY;
      double pmax=NEGATIVEINFINITY;
      for (int px=0; px<mdim; px++)
      {
         for (int py=0; py<ndim; py++)
         {
            double curr_p=ptwoDarray_ptr->get(px,py);
            pmin=basic_math::min(pmin,curr_p);
            pmax=basic_math::max(pmax,curr_p);
         }  // py loop
      }	// px loop

      double x,y;
//      const double min_Z=0;
      const double min_Z=1000;	// meters
   
      vertices_ptr->clear();
      probs_ptr->clear();
      vertices_ptr->reserve(mdim*ndim);
      probs_ptr->reserve(mdim*ndim);
      for (int px=0; px<mdim; px++)
      {
         if (px%100==0) cout << px << " " << flush;
         for (int py=0; py<ndim; py++)
         {
            ztwoDarray_ptr->pixel_to_point(px,py,x,y);
            XYZ[0]=x;
            XYZ[1]=y;
            XYZ[2]=ztwoDarray_ptr->get(px,py);

// Ignore any XYZ point whose Z value lies below some minimal
// threshold:

            if (XYZ[2] < min_Z) continue;
         
            vertices_ptr->push_back(XYZ);

            float curr_prob=(ptwoDarray_ptr->get(px,py)-pmin)/(pmax-pmin);
//         cout << "px = " << px << " py = " << py 
//              << " prob = " << curr_prob << endl;
            probs_ptr->push_back(curr_prob);
         
         } // loop over py index
      } // loop over px index
      cout << endl;

// Write out TDP file:

      string filename_prefix="zp"+stringfunc::prefix(basename);
      string tdp_filename=fused_subdir+filename_prefix+".tdp";
      string osga_filename="./"+filename_prefix+".osga";
      cout << "Output tdp_filename = " << tdp_filename << endl;

      string UTMzone="";		
      tdpfunc::write_relative_xyzp_data(
         tdp_filename,UTMzone,vertices_ptr,probs_ptr);

      string unix_cmd=
         "/home/cho/programs/c++/svn/projects/src/mains/mapping/lodtree "
         +tdp_filename;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="mv "+tdp_filename+" "+fused_subdir+"tdp/";
      sysfunc::unix_command(unix_cmd);
      unix_cmd="mv "+osga_filename+" "+fused_subdir+"osga/";
      sysfunc::unix_command(unix_cmd);
   } // loop over index i labeling Z tiles
} 

