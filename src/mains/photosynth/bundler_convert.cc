// ==========================================================================
// Program BUNDLER_CONVERT parses and transforms Noah Snavely's
// BUNDLER program ascii output.  It can read in a set of global
// rotation and scaling parameters needed to transform Noah's
// relatively reconstructed XYZ points into georegistered coordinates.
// After reading in the reconstructed XYZ points, it identifies
// outlier points and thresholds them away.  This program writes out
// the thresholded XYZ points to "thresholded_xyz_points.dat".  It
// also exports to "sorted_camera_views.dat" the XYZID-UV tiepoint
// pairs as a function of sorted thresholded point ID.
// BUNDLER_CONVERT writes out the adjacency matrix for the photos'
// graph as an edge list to "edgelist.dat".  It also exports the
// reconstructed points common to pairs of photos to
// "photoids_xyzids.dat".  Camera IDs vs an STL vector of XYZIDs is
// also written to cameraID_vs_XYZIDS.dat.  Finally, this program
// generates a TDP file for the thresholded XYZ points which contains
// RGB color information.


// /home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler_convert 
//    --region_filename ./bundler/kermit/packages/peter_inputs.pkg 

// ==========================================================================
// Last updated on 12/28/10; 12/29/10; 1/25/11
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundle_filename=passes_group.get_bundle_filename();
   cout << " bundle_filename = " << bundle_filename << endl;
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

   filefunc::ReadInfile(bundle_filename);
   int n_lines=filefunc::text_line.size();
   cout << "n_lines = " << n_lines << endl;
//   cout << "filefunc::text_line[0] = "
//        << filefunc::text_line[0] << endl;

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
//   cout << "filefunc::text_line[i_start] = "
//        << filefunc::text_line[i_start] << endl;

   vector<threevector> xyz_pnts;
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

   typedef pair<threevector,vector<fourvector> > XYZ_CAMERA_VIEWS;
   vector<XYZ_CAMERA_VIEWS> raw_xyz_camera_views;

   int n_camera_views=0;
   int curr_progress=0;

   for (int i=i_start; i<n_lines; i++)
   {
      double curr_frac=(i-i_start)/double(n_lines-i_start);
      if (10*curr_frac > curr_progress+1)
      {
         curr_progress++;
         cout << 0.1*curr_progress << " " << flush;
      }
//      if ((i-i_start)%10000==0) cout << (i-i_start)/10000 << " " << flush;

      if ((i-i_start)%3==0)
      {
         vector<double> curr_triple=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         vector<double> unrenormalized_colors=stringfunc::string_to_numbers(
            filefunc::text_line[i+1]);
         vector<double> viewlist=stringfunc::string_to_numbers(
            filefunc::text_line[i+2]);

         bundler_vertices_ptr->push_back(
            osg::Vec3(curr_triple[0],curr_triple[1],curr_triple[2]));

//         cout << "i = " << i
//              << " bundler_vertex = "
//              << bundler_vertices_ptr->back().x() << "  "
//              << bundler_vertices_ptr->back().y() << "  "
//              << bundler_vertices_ptr->back().z() << endl;

// Recall Noah's initial coordinate system needs to be grossly
// manipulated so that ground scenes lie approximately in XY plane:

// 	X_Peter = -X_Noah
// 	Y_Peter= Z_Noah
// 	Z_Peter = Y_Noah

         threevector curr_xyz(-curr_triple[0],curr_triple[2],curr_triple[1]);

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
      
         xyz_pnts.push_back(curr_xyz);

         X_pnts.push_back(curr_xyz.get(0));
         Y_pnts.push_back(curr_xyz.get(1));
         Z_pnts.push_back(curr_xyz.get(2));

         x_min=basic_math::min(x_min,curr_xyz.get(0));
         x_max=basic_math::max(x_max,curr_xyz.get(0));

         y_min=basic_math::min(y_min,curr_xyz.get(1));
         y_max=basic_math::max(y_max,curr_xyz.get(1));

         z_min=basic_math::min(z_min,curr_xyz.get(2));
         z_max=basic_math::max(z_max,curr_xyz.get(2));

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

            n_camera_views++;
         } // loop over index v labeling number of cameras which can view
           //  current SIFT feature

         (*xyz_rgba_map_ptr)[curr_xyz]=RGBA_PNTS(rgba_ub,camera_views);

// Save georegistered raw XYZ point position and camera views STL
// vector as a function of raw XYZ ID within STL vector
// raw_xyz_camera_views:
         
         bundlerfunc::scale_translate_rotate_bundler_XYZ(
            curr_xyz,fitted_world_to_bundler_distance_ratio,
            fitted_bundler_trans,global_R,bundler_rotation_origin);

         XYZ_CAMERA_VIEWS curr_xyz_camera_views;
         curr_xyz_camera_views.first=curr_xyz;
         curr_xyz_camera_views.second=camera_views;

         raw_xyz_camera_views.push_back(curr_xyz_camera_views);

      } // (i-i_start)%3==0 conditional

   } // loop over index i labeling lines in bundle.out file
   cout << endl;

   cout << "xyz_pnts.size() = " << xyz_pnts.size() << endl;
   cout << "n_camera_views = " << n_camera_views << endl;
   cout << "raw_xyz_camera_views.size() = "
        << raw_xyz_camera_views.size() << endl;

// --------------------------------------------------------------------------
// Export Photo IDs as a function of raw reconstructed XYZ point ID to
// output text file:

   string raw_xyz_camera_views_filename=bundler_IO_subdir+
      "RawXYZID_vs_cameraIDs.dat";
   ofstream raw_xyz_camera_views_stream;
   filefunc::openfile(raw_xyz_camera_views_filename,
                      raw_xyz_camera_views_stream);
   raw_xyz_camera_views_stream 
      << "# Raw XYZ ID, photoID, photoID', photoID''..."
      << endl;
   raw_xyz_camera_views_stream << endl;
   
   for (unsigned int i=0; i<raw_xyz_camera_views.size(); i++)
   {
      XYZ_CAMERA_VIEWS curr_xyz_camera_views=raw_xyz_camera_views[i];
      threevector XYZ=curr_xyz_camera_views.first;
      vector<fourvector> camera_views=curr_xyz_camera_views.second;
      raw_xyz_camera_views_stream << i << "    ";
      for (unsigned int j=0; j<camera_views.size(); j++)
      {
         fourvector curr_camera_view=camera_views[j];
         int curr_camera_ID=curr_camera_view.get(0);
         raw_xyz_camera_views_stream << curr_camera_ID << " ";
      }
      raw_xyz_camera_views_stream << endl;
   } // loop over index i labeling Noah's reconstructed points

   filefunc::closefile(raw_xyz_camera_views_filename,
                       raw_xyz_camera_views_stream);

   string banner=
      "Photo IDs as a function of raw XYZ point ID written to output file "
      +raw_xyz_camera_views_filename;
   outputfunc::write_big_banner(banner);

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
   banner="Raw Bundler XYZ point cloud written to output file "
      +bundler_tdp_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// Construct probability distributions for X, Y and Z coordinates of
// reconstructed XYZ points:

   cout << "x_min = " << x_min << " x_max = " << x_max << endl;
   cout << "y_min = " << y_min << " y_max = " << y_max << endl;
   cout << "z_min = " << z_min << " z_max = " << z_max << endl;
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
//   double x_median=X_prob.find_x_corresponding_to_pcum(0.5);
   x_hi_cum=x_max;

   double eps_y=0.001;
//   double eps_y=0.01;
   double y_lo_cum=Y_prob.find_x_corresponding_to_pcum(eps_y);
   double y_hi_cum=Y_prob.find_x_corresponding_to_pcum(1-eps_y);
//   double y_median=Y_prob.find_x_corresponding_to_pcum(0.5);

//   double z_median=Z_prob.find_x_corresponding_to_pcum(0.5);

// --------------------------------------------------------------------------
// Retain reconstructed points whose XYZ coordinates are all inliers.
// Recall that we need to eliminate RGBA values from *colors_ptr
// whenever we drop a corresponding XYZ point from xyz_pnts.  So
// introduce thresholded_colors_ptr to hold RGB counterparts to
// entries in thresholded_xyz_pnts:

   osg::Vec4ubArray* thresholded_colors_ptr=new osg::Vec4ubArray;

   typedef map<threevector,vector<fourvector>,ltthreevector > XYZ_MAP;
   XYZ_MAP* thresholded_xyz_map_ptr=new XYZ_MAP;

   vector<fourvector> thresholded_camera_views;
   
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
            thresholded_camera_views.push_back(rgb_pnts.second.at(j));
            curr_thresholded_camera_views.push_back(rgb_pnts.second.at(j));
         } // loop over index j labeling camera views
         P_pnts.push_back(0);
         (*thresholded_xyz_map_ptr)[curr_xyz]=curr_thresholded_camera_views;
      }
   } // iterator loop over *xyz_rgba_map_ptr
   delete xyz_rgba_map_ptr;

   cout << "thresholded_colors_ptr->size() = " 
        << thresholded_colors_ptr->size() << endl;
   cout << "thresholded_camera_views.size() = "
        << thresholded_camera_views.size() << endl;

// --------------------------------------------------------------------------
// Export reconstucted, thresholded and georegistered XYZ points seen
// by each camera to output text files.  Also generate adjacency
// matrix for photos' graph:

   string thresholded_xyz_filename=bundler_IO_subdir+
      "thresholded_xyz_points.dat";
   ofstream points_stream;
   filefunc::openfile(thresholded_xyz_filename,points_stream);
   points_stream << "# XYZID   X         Y           Z" << endl << endl;

   string camera_views_filename=bundler_IO_subdir+"camera_views.dat";
   ofstream camera_views_stream;
   filefunc::openfile(camera_views_filename,camera_views_stream);
   camera_views_stream << "# Total number of photos  = "
      +stringfunc::number_to_string(n_cameras) << endl << endl;
   camera_views_stream << "# PhotoID XYZID U   V" << endl << endl;
   
   cout << "n_cameras = " << n_cameras << endl;
   genmatrix adjacency(n_cameras,n_cameras);
   adjacency.clear_values();

   typedef map<twovector,vector<int>,lttwovector > PHOTOIDS_XYZIDS_MAP;
   PHOTOIDS_XYZIDS_MAP* photoids_xyzids_map_ptr=new PHOTOIDS_XYZIDS_MAP;

   int thresholded_xyz_ID=0;
   threevector bundler_xyz,rel_bundler_xyz,zeroth_bundler_xyz;
   vector<int> curr_camera_IDs;

// On 12/29/10, we relearned the hard and painful way that that
// osg::Vec3Array can only hold 4-byte floats and NOT 8-byte doubles.
// So we must store RELATIVE and not ABSOLUTE XYZ information within
// osg::Vec3Arrays!

   osg::Vec3Array* rel_thresholded_vertices_ptr=new osg::Vec3Array;
   for (XYZ_MAP::iterator itr=thresholded_xyz_map_ptr->begin();
        itr != thresholded_xyz_map_ptr->end(); ++itr)
   {
      bundler_xyz=itr->first;

// Deprecated pre-sailplane reconstruction (Dec 2010) method for
// converting bundler to world coordinates:

//      bundlerfunc::scale_translate_rotate_bundler_XYZ(
//         bundler_xyz,fitted_world_to_bundler_distance_ratio,
//         fitted_bundler_trans,global_R,bundler_rotation_origin);

//      bundler_xyz *= fitted_world_to_bundler_distance_ratio;
//      bundler_xyz += fitted_bundler_trans;
//      threevector rel_bundler_xyz=bundler_xyz-bundler_rotation_origin;
//      rel_bundler_xyz = global_R * rel_bundler_xyz;
//      bundler_xyz=rel_bundler_xyz+bundler_rotation_origin;

//      thresholded_vertices_ptr->push_back(
//         osg::Vec3(rel_bundler_xyz.get(0),rel_bundler_xyz.get(1),
//                   rel_bundler_xyz.get(2)));

      bundlerfunc::rotate_scale_translate_bundler_XYZ(
         bundler_xyz,bundler_rotation_origin,
         fitted_world_to_bundler_distance_ratio,
         fitted_bundler_trans,global_R);

      if (itr==thresholded_xyz_map_ptr->begin())
      {
         zeroth_bundler_xyz=bundler_xyz;
      }
      rel_bundler_xyz=bundler_xyz-zeroth_bundler_xyz;

      rel_thresholded_vertices_ptr->push_back(
         osg::Vec3(rel_bundler_xyz.get(0),rel_bundler_xyz.get(1),
	           rel_bundler_xyz.get(2)));

      points_stream << thresholded_xyz_ID << "   "
                    << rel_bundler_xyz.get(0) << "   "
                    << rel_bundler_xyz.get(1) << "   "
                    << rel_bundler_xyz.get(2) << "   ";

// Output information about which XYZ points are visible to which
// cameras to an ascii file:

      curr_camera_IDs.clear();
      vector<fourvector> thresholded_camera_views=itr->second;
      for (unsigned int j=0; j<thresholded_camera_views.size(); j++)
      {
         fourvector curr_camera_view(thresholded_camera_views[j]);
         camera_views_stream << curr_camera_view[0] << "   "
//                             << bundler_xyz.get(0) << "   "
//                             << bundler_xyz.get(1) << "   "
//                             << bundler_xyz.get(2) << "   "
                             << thresholded_xyz_ID << "   "
                             << curr_camera_view[2] << "   "
                             << curr_camera_view[3] << endl;
         curr_camera_IDs.push_back(curr_camera_view[0]);
         points_stream << curr_camera_view[0] << "  ";
      } // loop over index j labeling thresholded camera views
      points_stream << endl;

// Increment entries within adjacency matrix to indicate which pairs
// of cameras share reconstructed XYZ points in common:

      if (curr_camera_IDs.size() > 1)
      {
         for (unsigned int k=0; k<curr_camera_IDs.size(); k++)
         {
            int a=curr_camera_IDs[k];
            for (unsigned int l=k+1; l<curr_camera_IDs.size(); l++)
            {
               int b=curr_camera_IDs[l];

               twovector photoids;
               if (a < b)
               {
                  adjacency.put(a,b,adjacency.get(a,b)+1);
                  photoids=twovector(a,b);
               }
               else
               {
                  adjacency.put(b,a,adjacency.get(b,a)+1);
                  photoids=twovector(b,a);
               }

               vector<int> xyzids=(*photoids_xyzids_map_ptr)[photoids];
               (*photoids_xyzids_map_ptr)[photoids].clear();
               xyzids.push_back(thresholded_xyz_ID);
               (*photoids_xyzids_map_ptr)[photoids]=xyzids;

            } // loop over index l
         } // loop over index k
      }

      thresholded_xyz_ID++;
   } // iterator loop over *thresholded_xyz_map_ptr

   filefunc::closefile(thresholded_xyz_filename,points_stream);
   filefunc::closefile(camera_views_filename,camera_views_stream);

   banner="Thresholded XYZ points written to output file "
      +thresholded_xyz_filename;
   outputfunc::write_big_banner(banner);

   banner="Unsorted camera views written to output file "
      +camera_views_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// Write output TDP file for all thresholded XYZ points:

   string tdp_filename=bundler_IO_subdir+"thresholded_xyz_points.tdp";
   tdpfunc::write_relative_xyzrgba_data(
      tdp_filename,UTMzone,zeroth_bundler_xyz,
      rel_thresholded_vertices_ptr,thresholded_colors_ptr);
   banner="Thresholded XYZ point cloud written to output file "+tdp_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// On 12/28/09, Noah Snavely indicated that some minimal threshold
// number of SIFT pairs should be imposed before an edge in the
// adjacency matrix/graph is created between two photo nodes:

//   const int min_SIFT_pairs_threshold=0;
   const int min_SIFT_pairs_threshold=25;

// Zero out entries within adjacency matrix which fall below threshold:

   int n_edges=0;
   for (int r=0; r<n_cameras; r++)
   {
      for (int c=0; c<n_cameras; c++)
      {
         if (adjacency.get(r,c) < min_SIFT_pairs_threshold)
         {
            adjacency.put(r,c,0);
         }
         else
         {
            n_edges++;
         }
      }
   }
   cout << "n_edges = " << n_edges << endl;
   
// Symmetrize thresholded adjacency matrix (appropriate for undirected
// SIFT graph):

   for (int r=0; r<n_cameras; r++)
   {
      for (int c=r+1; c<n_cameras; c++)
      {
         adjacency.put(c,r,adjacency.get(r,c));
      }
   }

// Set number of graph nodes to number of non-empty rows within
// symmetrized adjacency matrix:

   int n_nodes=0;
   for (int r=0; r<n_cameras; r++)
   {
      int row_sum=0;
      for (int c=0; c<n_cameras; c++)
      {
         row_sum += adjacency.get(r,c);
      }
      if (row_sum > 0) n_nodes++;
   }
   cout << "n_nodes = " << n_nodes << endl;

   int mini_n_cameras=12;
   if (n_cameras < mini_n_cameras)
   {
      cout << "Adjacency matrix = " << adjacency << endl;
   }
   else
   {
      genmatrix mini_adjacency(mini_n_cameras,mini_n_cameras);
      for (int r=0; r<mini_n_cameras; r++)
      {
         for (int c=0; c<mini_n_cameras; c++)
         {
            mini_adjacency.put(r,c,adjacency.get(r,c));
         } // loop over index column c
      } // loop over row index r
      cout << "Reduced adjacency matrix = " << mini_adjacency << endl;
//      outputfunc::enter_continue_char();
   }

// --------------------------------------------------------------------------
// Sort XYZ-UV tiepoint pair contents of camera_views_filename by
// camera ID:

   filefunc::ReadInfile(camera_views_filename);

   vector<int> photo_ID,XYZ_ID;
   vector<double> U_sift,V_sift;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> fields=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      photo_ID.push_back(fields[0]);
      XYZ_ID.push_back(fields[1]);
      U_sift.push_back(fields[2]);
      V_sift.push_back(fields[3]);
   } // loop over index i labeling lines in camera_views_filename

   templatefunc::Quicksort(photo_ID,XYZ_ID,U_sift,V_sift);

// Open text file to hold reconstucted XYZ points sorted by photo ID:

   string sorted_camera_views_filename=bundler_IO_subdir+
      "sorted_camera_views.dat";
   ofstream sorted_camera_views_stream;
   filefunc::openfile(sorted_camera_views_filename,
                      sorted_camera_views_stream);
   sorted_camera_views_stream << "# PhotoID XYZID U   V" << endl << endl;

   for (unsigned int i=0; i<photo_ID.size(); i++)
   {
      sorted_camera_views_stream << photo_ID[i] << "   "
                                 << XYZ_ID[i] << "   "
                                 << U_sift[i] << "   "
                                 << V_sift[i] << endl;
   } // loop over index i labeling photos

   filefunc::closefile(sorted_camera_views_filename,
                       sorted_camera_views_stream);

   banner="Camera visibility of XYZ points written to output file "
      +sorted_camera_views_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// Read back in contents of sorted_camera_views.dat:

   typedef map<int,vector<int> > CAMERAIDS_XYZIDS_MAP;
   CAMERAIDS_XYZIDS_MAP* cameraids_xyzids_map_ptr=new CAMERAIDS_XYZIDS_MAP;

   filefunc::ReadInfile(sorted_camera_views_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> curr_fields=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int curr_photoID=curr_fields[0];
      int curr_XYZID=curr_fields[1];

      CAMERAIDS_XYZIDS_MAP::iterator iter=
         cameraids_xyzids_map_ptr->find(curr_photoID);
      if (iter == cameraids_xyzids_map_ptr->end())
      {
         vector<int> xyzids;
         xyzids.push_back(curr_XYZID);
         (*cameraids_xyzids_map_ptr)[curr_photoID]=xyzids;
      }
      else
      {
         (iter->second).push_back(curr_XYZID);
      }
   } // loop over index i labeling lines in sorted_camera_views_filename

// Open text file to hold camera ID vs sorted STL vector of XYZ point IDs:

   string cameraID_vs_XYZIDs_filename=bundler_IO_subdir+
      "cameraID_vs_XYZIDs.dat";
   ofstream cameraID_vs_XYZIDs_stream;
   filefunc::openfile(cameraID_vs_XYZIDs_filename,
                      cameraID_vs_XYZIDs_stream);
   cameraID_vs_XYZIDs_stream << "# PhotoID XYZID XYZID' XYZID'' ..." 
                              << endl << endl;

   int prev_photo_ID=-1;   
   for (unsigned int i=0; i<photo_ID.size(); i++)
   {
      int curr_photo_ID=photo_ID[i];
      if (curr_photo_ID==prev_photo_ID)
      {
         continue;
      }
      else
      {
         prev_photo_ID=curr_photo_ID;
      }

      CAMERAIDS_XYZIDS_MAP::iterator iter=
         cameraids_xyzids_map_ptr->find(curr_photo_ID);
      if (iter != cameraids_xyzids_map_ptr->end())
      {
         vector<int> xyzids=iter->second;
         std::sort(xyzids.begin(),xyzids.end());
         cameraID_vs_XYZIDs_stream << curr_photo_ID << "     ";
         for (unsigned int j=0; j<xyzids.size(); j++)
         {
            cameraID_vs_XYZIDs_stream << xyzids[j] << " ";
         }
         cameraID_vs_XYZIDs_stream << endl;
      }
   } // loop over index i labeling photo IDs
   
   filefunc::closefile(cameraID_vs_XYZIDs_filename,
                       cameraID_vs_XYZIDs_stream);

   banner="Camera ID vs XYZIDs written to output file "
      +cameraID_vs_XYZIDs_filename;
   outputfunc::write_big_banner(banner);

// --------------------------------------------------------------------------
// Write out adjacency matrix for photos' graph as an edge list:

   string edgelist_filename=bundler_IO_subdir+"edgelist.dat";
   graph g;
   g.export_edgelist(&adjacency,edgelist_filename,min_SIFT_pairs_threshold);

// --------------------------------------------------------------------------
// Write out reconstructed points common to pairs of photos:

   string photoids_xyzids_output_filename=
      bundler_IO_subdir+"photoids_xyzids.dat";
   ofstream edgestream,xyzstream;
   filefunc::openfile(photoids_xyzids_output_filename,xyzstream);

   xyzstream << "# Total number of photos  = "
      +stringfunc::number_to_string(n_cameras) << endl << endl;
   xyzstream << "# PhotoID  PhotoID'" << endl;
   xyzstream << "# XYZID1 XYZID2 XYZID3 ..." << endl << endl;

   for (PHOTOIDS_XYZIDS_MAP::iterator itr=photoids_xyzids_map_ptr->begin();
        itr != photoids_xyzids_map_ptr->end(); ++itr)
   {
      twovector photoids=itr->first;
      xyzstream << photoids.get(0) << "  " 
                << photoids.get(1) << "       " << endl;

      vector<int> xyzids=itr->second;
      for (unsigned int i=0; i<xyzids.size(); i++)
      {
         xyzstream << xyzids[i] << "  ";
      }
      xyzstream << endl;
   } // iterator loop over *photoids_xyzids_map_ptr
   filefunc::closefile(photoids_xyzids_output_filename,xyzstream);

   banner="Reconstructed points common to photo pairs written to output file "
      +photoids_xyzids_output_filename;
   outputfunc::write_big_banner(banner);

}

