// ==========================================================================
// Program DRAPE_EO imports a TDP file along with an overhead Google Earth
// image which has been registered with the point cloud via program
// FUSION.  We assume a package file containing a --projection_matrix
// entry based upon FUSION output has been created for the aerial EO
// image DRAPE_EO iterates over every point in the cloud and colors it
// to indicate height or RGB information.  Fused data is written to
// TDP/OSGA file output.

/*

./drape_EO \
--region_filename /data_third_disk/DIME/panoramas/Feb2013_DeerIsland/GE_ladar/packages/DeerIsland.pkg \
--region_filename /data_third_disk/DIME/panoramas/Feb2013_DeerIsland/GE_ladar/packages/DeerIsland_EO.pkg 

*/

// ==========================================================================
// Last updated on 3/13/13; 4/25/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camera.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   vector<double> min_U,min_V;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   Pass* cloudpass_ptr=passes_group.get_pass_ptr(cloudpass_ID);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   Pass* videopass_ptr=passes_group.get_pass_ptr(videopass_ID);

   string photo_filename=videopass_ptr->get_first_filename();
   cout << "Input photo_filename = " << photo_filename << endl;
   photogroup* photogroup_ptr=new photogroup();
   photograph* photo_ptr=photogroup_ptr->generate_single_photograph(
      photo_filename);
   camera* curr_camera_ptr=photo_ptr->get_camera_ptr();
   texture_rectangle* texture_rectangle_ptr=
      new texture_rectangle(photo_filename,NULL);

   string osga_filename=cloudpass_ptr->get_first_filename();
   string subdir=filefunc::getdirname(osga_filename);
   string basename=filefunc::getbasename(osga_filename);
   string tdp_filename=subdir+stringfunc::prefix(basename)+".tdp";
   cout << "Input tdp_filename = " << tdp_filename << endl;
   
   vector<double> X,Y,Z;   
   tdpfunc::read_XYZ_points_from_tdpfile(tdp_filename,X,Y,Z);
   int n_points=X.size();
   cout << "Number of inputs 3D points = " << n_points << endl;

   double z_min=mathfunc::minimal_value(Z);
   double z_max=mathfunc::maximal_value(Z);
   
// Retrieve 3x4 projection matrix calculated via program FUSION:

   genmatrix* projmatrix_ptr=
      passes_group.get_pass_ptr(videopass_ID)->get_PassInfo_ptr()->
      get_projection_matrix_ptr();
   cout << "*projmatrix_ptr = " << *projmatrix_ptr << endl;
   curr_camera_ptr->set_projection_matrix(*projmatrix_ptr);

   double U,V;
   double u_min=0;
   double u_max=texture_rectangle_ptr->getWidth()/
      texture_rectangle_ptr->getHeight();
   u_max=1310.0/1400.0;
   double v_min=0;
   double v_max=1;
   cout << "min_U = " << u_min << " max_U = " << u_max << endl;
   cout << "min_V = " << v_min << " max_V = " << v_max << endl;

   ColorMap color_map;
   color_map.set_mapnumber(14);	// reverse_large_hue_value_sans_white

   double cyclic_frac_offset;
   cout << "Enter cylic frac offset:" << endl;
   cin >> cyclic_frac_offset;

   color_map.set_cyclic_frac_offset(cyclic_frac_offset);
   color_map.set_max_value(2,z_max);
   color_map.set_min_value(2,z_min);
   color_map.set_max_threshold(2,z_max);
   color_map.set_min_threshold(2,z_min);

// Loop over all points within input cloud.  Project each one into UV
// image plane.  If projection lies within aerial image, transfer RGB
// from EO image to 3D point.  Otherwise, color point according to its
// height:

   int curr_R,curr_G,curr_B;
   vector<int> R,G,B;
   for (int i=0; i<n_points; i++)
   {
      curr_camera_ptr->project_XYZ_to_UV_coordinates(X[i],Y[i],Z[i],U,V);
//      cout << "i = " << i << " U = " << U << " V = " << V << endl;
      if (U < u_min || U > u_max || V < v_min || V > v_max)
      {
         colorfunc::RGBA curr_rgba=color_map.retrieve_curr_RGBA(Z[i]);
         curr_R=255*curr_rgba.first;
         curr_G=255*curr_rgba.second;
         curr_B=255*curr_rgba.third;
//         cout << "i = " << i << " Z = " << Z[i] 
//              << " curr_R = " << curr_R << " curr_G = " << curr_G
//              << " curr_B = " << curr_B << endl;
      }
      else
      {
         texture_rectangle_ptr->get_RGB_values(U,V,curr_R,curr_G,curr_B);
//         cout << "U = " << U << " V = " << V 
//              << " R = " << curr_R << " G = " << curr_G
//              << " B = " << curr_B << endl;
      }
      R.push_back(curr_R);
      G.push_back(curr_G);
      B.push_back(curr_B);
   } // loop over index i labeling 3D points
   cout << "R.size() = " << R.size() << endl;

   delete photogroup_ptr;
   delete texture_rectangle_ptr;

   string fused_tdp_filename=subdir+stringfunc::prefix(basename)+"_fused.tdp";
   string UTMzone="";
   tdpfunc::write_relative_xyzrgba_data(
      UTMzone,fused_tdp_filename,X,Y,Z,R,G,B);
   string banner="Exported fused TDP file to "+fused_tdp_filename;
   outputfunc::write_big_banner(banner);

   string unix_cmd="lodtree "+fused_tdp_filename;
   sysfunc::unix_command(unix_cmd);
   string fused_osga_filename=stringfunc::prefix(basename)+"_fused.osga";
   unix_cmd="mv "+fused_osga_filename+" "+subdir;
   sysfunc::unix_command(unix_cmd);

   banner="Exported fused OSGA file to "+fused_osga_filename;
   outputfunc::write_big_banner(banner);

} 

