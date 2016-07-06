// ==========================================================================
// Program MINI_CONVERT is a stripped-down version of BUNDLER_CONVERT
// which runs significantly faster.  It generates TDP and OSGA files
// for the sparse 3D point cloud output by BUNDLER.

// /home/cho/programs/c++/svn/projects/src/mains/photosynth/mini_convert
//    --region_filename ./bundler/kermit/packages/peter_inputs.pkg 

// ==========================================================================
// Last updated on 11/25/13; 12/3/13; 12/4/13
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/ltthreevector.h"
#include "math/lttwovector.h"
#include "templates/mytemplates.h"
#include "passes/PassesGroup.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   bool VSFM_flag=false;	// bundler output
//   bool VSFM_flag=true;		// VSFM output
   cout << "Inside mini_convert, VSFM_flag = " << VSFM_flag << endl;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   double fitted_world_to_bundler_distance_ratio=
      passes_group.get_fitted_world_to_bundler_distance_ratio();
   cout << "world_to_bundler_ratio = " 
        << fitted_world_to_bundler_distance_ratio << endl;
   threevector fitted_bundler_trans=passes_group.get_bundler_translation();
   cout << "fitted_bundler_trans = " << fitted_bundler_trans << endl;

   double global_az=passes_group.get_global_az();
   double global_el=passes_group.get_global_el();
   double global_roll=passes_group.get_global_roll();
   cout << "global_az = " << global_az*180/PI << endl;
   cout << "global_el = " << global_el*180/PI << endl;
   cout << "global_roll = " << global_roll*180/PI << endl;
   
   rotation global_R;
   global_R=global_R.rotation_from_az_el_roll(
      global_az,global_el,global_roll);
//      cout << "global_R = " << global_R << endl;
   
   threevector bundler_rotation_origin=
      passes_group.get_bundler_rotation_origin();
   cout << "bundler_rotation_origin = " << bundler_rotation_origin << endl;

// --------------------------------------------------------------------------
// Import and start to parse bundle.out file:

   filefunc::ReadInfile(bundle_filename);
   int n_lines=filefunc::text_line.size();

// Extract number of cameras and number of reconstructed XYZ points
// from first uncommented line in BUNDLER output file:

   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      filefunc::text_line[0]);
   int n_cameras=stringfunc::string_to_number(substrings[0]);
   int n_points=stringfunc::string_to_number(substrings[1]);
   cout << "n_cameras = " << n_cameras << " n_points = " << n_points
        << endl;
	
// Extract XYZ points which start at line 5*n_cameras in BUNDLER
// output file:

   int i_start=5*n_cameras+1;

   vector<double> X_pnts,Y_pnts,Z_pnts,P_pnts;

   double x_min=POSITIVEINFINITY;
   double y_min=POSITIVEINFINITY;
   double z_min=POSITIVEINFINITY;

   double x_max=NEGATIVEINFINITY;
   double y_max=NEGATIVEINFINITY;
   double z_max=NEGATIVEINFINITY;

   osg::Vec3Array* bundler_vertices_ptr=new osg::Vec3Array;
   osg::Vec4ubArray* colors_ptr=new osg::Vec4ubArray;

   typedef pair<osg::Vec4ub,vector<fourvector> > RGBA_PNTS;
   typedef map<threevector,RGBA_PNTS,ltthreevector > XYZ_RGBA_MAP;
   XYZ_RGBA_MAP* xyz_rgba_map_ptr=new XYZ_RGBA_MAP;
   
// Independent variable: point cloud point index
// Dependent variables:

//	Physical raw XYZ geocoordinates
//      vector<fourvector> camera_views

   camera* camera_ptr=new camera();
//   camera_ptr->set_georegistered_flag(false);		// default
   camera_ptr->set_georegistered_flag(true);  	// GEO,Puma VSFM

// On 12/3/13, we empirically found that VSFM reconstructions can
// sometimes contain wild outliers.  So we first scan through all raw
// reconstructed 3D points and compute their medians + quartile
// widths.  We then reject any reconstructed point which lies too far
// away from these median raw values:

   vector<double> Xraw,Yraw,Zraw;
   for (int i=i_start; i<n_lines; i++)
   {
      outputfunc::update_progress_fraction(i,10000,n_lines);
      if ((i-i_start)%3 != 0) continue;

      vector<double> curr_triple=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      Xraw.push_back(curr_triple[0]);
      Yraw.push_back(curr_triple[1]);
      Zraw.push_back(curr_triple[2]);
   }
   cout << endl;

   double Xraw_median,Yraw_median,Zraw_median;
   double Xraw_quartile_width,Yraw_quartile_width,Zraw_quartile_width;
   mathfunc::median_value_and_quartile_width(
      Xraw,Xraw_median,Xraw_quartile_width);
   mathfunc::median_value_and_quartile_width(
      Yraw,Yraw_median,Yraw_quartile_width);
   mathfunc::median_value_and_quartile_width(
      Zraw,Zraw_median,Zraw_quartile_width);

   for (int i=i_start; i<n_lines; i++)
   {
      outputfunc::update_progress_fraction(i,10000,n_lines);
      if ((i-i_start)%3 != 0) continue;

      vector<double> curr_triple=stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      double curr_x=curr_triple[0];
      double curr_y=curr_triple[1];
      double curr_z=curr_triple[2];
      double rx=fabs(curr_x-Xraw_median)/Xraw_quartile_width;
      double ry=fabs(curr_y-Yraw_median)/Yraw_quartile_width;
      double rz=fabs(curr_z-Zraw_median)/Zraw_quartile_width;

// Ignore any reconstructed point which lies ridiculously far away
// from median values:

      const double max_deviation_ratio=25;
      if (rx > max_deviation_ratio || ry > max_deviation_ratio ||
      rz > max_deviation_ratio) continue;

      bundler_vertices_ptr->push_back(osg::Vec3(curr_x,curr_y,curr_z));

      vector<double> unrenormalized_colors=stringfunc::string_to_numbers(
         filefunc::text_line[i+1]);
      vector<double> viewlist=stringfunc::string_to_numbers(
         filefunc::text_line[i+2]);

// Recall Noah's initial coordinate system needs to be grossly
// manipulated so that ground scenes lie approximately in XY plane:

// 	X_Peter = -X_Noah
// 	Y_Peter = Z_Noah
// 	Z_Peter = Y_Noah

// 	X_Peter = X_VSFM
// 	Y_Peter = Z_VSFM
//	Z_Peter = -Y_VSFM 

      threevector curr_xyz(curr_x,curr_y,curr_z);

      curr_xyz = *(camera_ptr->get_R_noah_to_peter_ptr(VSFM_flag)) * curr_xyz;
//      cout << "curr_xyz = " << curr_xyz << endl;

      curr_x=curr_xyz.get(0);
      curr_y=curr_xyz.get(1);
      curr_z=curr_xyz.get(2);
      
      X_pnts.push_back(curr_x);
      Y_pnts.push_back(curr_y);
      Z_pnts.push_back(curr_z);
         
      x_min=basic_math::min(x_min,curr_x);
      x_max=basic_math::max(x_max,curr_x);
      y_min=basic_math::min(y_min,curr_y);
      y_max=basic_math::max(y_max,curr_y);
      z_min=basic_math::min(z_min,curr_z);
      z_max=basic_math::max(z_max,curr_z);

// In January 2011, we discovered that bundler sometimes incorrectly
// colors reconstructed points as pure blue.  Until Noah explains why
// this occurs in bundler, we will reset such pure blue points to a
// mid-grey color:

      if ( nearly_equal(unrenormalized_colors[0],0) &&
      nearly_equal(unrenormalized_colors[1],0) &&
      nearly_equal(unrenormalized_colors[2],255))
      {
         unrenormalized_colors[0]=unrenormalized_colors[1]=
            unrenormalized_colors[2]=192;
      }
         
      colorfunc::RGBA curr_RGBA(unrenormalized_colors[0],
      unrenormalized_colors[1],
      unrenormalized_colors[2],255);
      bool normalized_input_RGBA_values=false;
      colorfunc::RGBA_bytes curr_RGBA_bytes=
         colorfunc::RGBA_to_bytes(curr_RGBA,normalized_input_RGBA_values);

      osg::Vec4ub rgba_ub(curr_RGBA_bytes.first,curr_RGBA_bytes.second,
      curr_RGBA_bytes.third,curr_RGBA_bytes.fourth);
      colors_ptr->push_back(rgba_ub);
      
      int n_views=viewlist[0];
      vector<fourvector> camera_views;
      for (int v=0; v<n_views; v++)
      {
         int k=v*4+1;
         int curr_camera_ID=viewlist[k];
         int sift_feature_ID=viewlist[k+1];
         double u_sift=viewlist[k+2];
         double v_sift=viewlist[k+3];
         camera_views.push_back(
            fourvector(curr_camera_ID,sift_feature_ID,u_sift,v_sift));
      } // loop over index v labeling number of cameras which can view
      //  current SIFT feature

      (*xyz_rgba_map_ptr)[curr_xyz]=RGBA_PNTS(rgba_ub,camera_views);

      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         curr_xyz,fitted_world_to_bundler_distance_ratio,
         fitted_bundler_trans,global_R,bundler_rotation_origin);

   } // loop over index i labeling lines in bundle.out file
   cout << endl;

   Xraw.clear();
   Yraw.clear();
   Zraw.clear();

//   cout << "Number raw XYZ points = " << X_pnts.size() << endl;

// --------------------------------------------------------------------------
// Write output TDP file for all raw BUNDLER XYZ points:

   string bundler_tdp_filename=bundler_IO_subdir+"raw_bundler_xyz_points.tdp";
   string UTMzone="";
   tdpfunc::write_relative_xyzrgba_data(
      bundler_tdp_filename,UTMzone,
      Zero_vector,bundler_vertices_ptr,colors_ptr);
//   tdpfunc::write_relative_xyzrgba_data(
//      bundler_tdp_filename,UTMzone,
//      bundler_rotation_origin,bundler_vertices_ptr,colors_ptr);
   string banner="Raw Bundler XYZ point cloud written to output file "
      +bundler_tdp_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// Construct probability distributions for X, Y and Z coordinates of
// reconstructed XYZ points:

   cout << "x_min = " << x_min << " x_max = " << x_max << endl;
   cout << "y_min = " << y_min << " y_max = " << y_max << endl;
   cout << "z_min = " << z_min << " z_max = " << z_max << endl;

//   cout << "X_pnts.size() = " << X_pnts.size() << endl;
//   cout << "Y_pnts.size() = " << Y_pnts.size() << endl;
//   cout << "Z_pnts.size() = " << Z_pnts.size() << endl;

   int n_output_bins=100;
   prob_distribution X_prob(X_pnts,n_output_bins,x_min);
   prob_distribution Y_prob(Y_pnts,n_output_bins,y_min);
   prob_distribution Z_prob(Z_pnts,n_output_bins,z_min);

   double cumprob=0.05;
//   cout << "Enter cumulative z probability:" << endl;
//   cin >> cumprob;
   double z_cum=Z_prob.find_x_corresponding_to_pcum(cumprob);
   cout << "z_cum = " << z_cum << endl;

   double eps_x=0.001;
//   double eps_x=0.01;
   double x_lo_cum=X_prob.find_x_corresponding_to_pcum(eps_x);
   double x_hi_cum=X_prob.find_x_corresponding_to_pcum(1-eps_x);
   x_hi_cum=x_max;

   double eps_y=0.001;
//   double eps_y=0.01;
   double y_lo_cum=Y_prob.find_x_corresponding_to_pcum(eps_y);
   double y_hi_cum=Y_prob.find_x_corresponding_to_pcum(1-eps_y);

   cout << "xlo_cum = " << x_lo_cum << " xhi_cum = " << x_hi_cum << endl;
   cout << "ylo_cum = " << y_lo_cum << " yhi_cum = " << y_hi_cum << endl;

// --------------------------------------------------------------------------
// Retain reconstructed points whose XYZ coordinates are all inliers.
// Recall that we need to eliminate RGBA values from *colors_ptr
// whenever we drop a corresponding XYZ point from xyz_pnts.  So
// introduce thresholded_colors_ptr to hold RGB counterparts to
// entries in thresholded_xyz_pnts:

   osg::Vec4ubArray* thresholded_colors_ptr=new osg::Vec4ubArray;

//   cout << "xyz_rgba_map_ptr->size() = "
//        << xyz_rgba_map_ptr->size() << endl;

   typedef map<threevector,vector<fourvector>,ltthreevector > XYZ_MAP;
   XYZ_MAP* thresholded_xyz_map_ptr=new XYZ_MAP;

   for (XYZ_RGBA_MAP::iterator itr=xyz_rgba_map_ptr->begin();
        itr != xyz_rgba_map_ptr->end(); ++itr)
   {
      threevector curr_xyz=itr->first;
      if (curr_xyz.get(0) > x_lo_cum && curr_xyz.get(0) < x_hi_cum &&
          curr_xyz.get(1) > y_lo_cum && curr_xyz.get(1) < y_hi_cum &&
          curr_xyz.get(2) > z_cum)
      {
         RGBA_PNTS rgb_pnts=itr->second;
         thresholded_colors_ptr->push_back(rgb_pnts.first);

         vector<fourvector> curr_thresholded_camera_views;
         for (unsigned int j=0; j<rgb_pnts.second.size(); j++)
         {
            curr_thresholded_camera_views.push_back(rgb_pnts.second.at(j));
         } // loop over index j labeling camera views
         P_pnts.push_back(0);
         (*thresholded_xyz_map_ptr)[curr_xyz]=curr_thresholded_camera_views;
      }
   } // iterator loop over *xyz_rgba_map_ptr
   delete xyz_rgba_map_ptr;

//   cout << "thresholded_colors_ptr->size() = " 
//        << thresholded_colors_ptr->size() << endl;

// --------------------------------------------------------------------------
// On 12/29/10, we relearned the hard and painful way that that
// osg::Vec3Array can only hold 4-byte floats and NOT 8-byte doubles.
// So we must store RELATIVE and not ABSOLUTE XYZ information within
// osg::Vec3Arrays!
   
   vector<threevector>* thresholded_vertices_ptr=new
      vector<threevector>;

   const double min_rel_X=NEGATIVEINFINITY;
   const double max_rel_X=POSITIVEINFINITY;
   const double min_rel_Y=NEGATIVEINFINITY;
   const double max_rel_Y=POSITIVEINFINITY;
   const double min_rel_Z=NEGATIVEINFINITY;
   const double max_rel_Z=POSITIVEINFINITY;

   threevector bundler_xyz,rel_bundler_xyz,zeroth_bundler_xyz;

   vector<double> relative_X,relative_Y,relative_Z;
   for (XYZ_MAP::iterator itr=thresholded_xyz_map_ptr->begin();
        itr != thresholded_xyz_map_ptr->end(); ++itr)
   {
      bundler_xyz=itr->first;

      bundlerfunc::rotate_scale_translate_bundler_XYZ(
         bundler_xyz,bundler_rotation_origin,
         fitted_world_to_bundler_distance_ratio,
         fitted_bundler_trans,global_R);

      if (itr==thresholded_xyz_map_ptr->begin())
      {
         zeroth_bundler_xyz=bundler_xyz;
      }
      rel_bundler_xyz=bundler_xyz-zeroth_bundler_xyz;

      double rel_X=rel_bundler_xyz.get(0);
      double rel_Y=rel_bundler_xyz.get(1);
      double rel_Z=rel_bundler_xyz.get(2);
      relative_X.push_back(rel_X);
      relative_Y.push_back(rel_Y);
      relative_Z.push_back(rel_Z);

      if (rel_X < min_rel_X || rel_X > max_rel_X) continue;
      if (rel_Y < min_rel_Y || rel_Y > max_rel_Y) continue;
      if (rel_Z < min_rel_Z || rel_Z > max_rel_Z) continue;

      thresholded_vertices_ptr->push_back(threevector(
         bundler_xyz.get(0),bundler_xyz.get(1),bundler_xyz.get(2)));

   } // iterator loop over *thresholded_xyz_map_ptr

// --------------------------------------------------------------------------
// Write output TDP file for all thresholded XYZ points:

//   cout << "Before exporting thresholded XYZ points" << endl;
//   cout << "zeroth_bundler_xyz = " << zeroth_bundler_xyz << endl;
   string tdp_filename=bundler_IO_subdir+"thresholded_xyz_points.tdp";
//   cout << "thresholded_vertices_ptr->size() = "
//        << thresholded_vertices_ptr->size() << endl;
//   tdpfunc::write_relative_xyzrgba_data(
//      tdp_filename,UTMzone,zeroth_bundler_xyz,
//      rel_thresholded_vertices_ptr,thresholded_colors_ptr);
   tdpfunc::write_relative_xyz_data(tdp_filename,*thresholded_vertices_ptr);
   delete thresholded_vertices_ptr;

   banner="Thresholded XYZ point cloud written to output file "+tdp_filename;
   outputfunc::write_big_banner(banner);

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);

   string osga_filename="thresholded_xyz_points.osga";
   unix_cmd="mv "+osga_filename+" "+bundler_IO_subdir;
   sysfunc::unix_command(unix_cmd);
}
