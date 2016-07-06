// ==========================================================================
// BUNDLERFUNCS stand-alone methods
// ==========================================================================
// Last modified on 9/10/13; 9/15/13; 12/3/13; 3/22/14
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "video/camera.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"

using std::cout;
using std::cin;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

namespace bundlerfunc
{

// Method generate_list_tmp_file() searches BUNDLER_IO_SUBDIR/images
// for any jpg or png images.  It creates a list_tmp.txt file from
// whatever images it finds in this subdirectory.

   string generate_list_tmp_file(string bundler_IO_subdir)
   {
//      cout << "inside bundlerfunc::generate_trivial_bundle_dot_out_file()"
//           << endl;
//      cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
      string images_subdir=bundler_IO_subdir+"images/";

      vector<string> allowed_suffixes;
      allowed_suffixes.push_back("jpg");
      allowed_suffixes.push_back("JPG");
      allowed_suffixes.push_back("jpeg");
      allowed_suffixes.push_back("JPEG");
      allowed_suffixes.push_back("png");
      allowed_suffixes.push_back("PNG");
      allowed_suffixes.push_back("pgm");
      
      vector<string> image_filenames=
         filefunc::files_in_subdir_matching_specified_suffixes(
            allowed_suffixes,images_subdir);

      string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
      ofstream outstream;
      filefunc::openfile(list_tmp_filename,outstream);
      for (unsigned int n=0; n<image_filenames.size(); n++)
      {
         string curr_image_filename="images/"+
            filefunc::getbasename(image_filenames[n]);
         outstream << curr_image_filename << endl;
//         cout << "image = " << curr_image_filename << endl;
      }
      filefunc::closefile(list_tmp_filename,outstream);

      return list_tmp_filename;
   }

// ---------------------------------------------------------------------
// Method generate_trivial_bundle_dot_out_file() extracts the number
// of images from input list_tmp_file.  It exports a new
// bundle_trivial.out file to bundler_IO_subdir which contains just
// this n_image information.  

   string generate_trivial_bundle_dot_out_file(string list_tmp_file)
   {
//      cout << "inside bundlerfunc::generate_trivial_bundle_dot_out_file()"
//           << endl;
      
      filefunc::ReadInfile(list_tmp_file);
      int n_images=filefunc::text_line.size();

      string bundler_IO_subdir=filefunc::getdirname(list_tmp_file);
//      cout << "bundler_IO_subdir="+bundler_IO_subdir << endl;
      string bundle_filename=bundler_IO_subdir+"bundle_trivial.out";
      ofstream bundlestream;
      filefunc::openfile(bundle_filename,bundlestream);
      bundlestream << "# Bundle file v0.3" << endl;
      bundlestream << n_images << "  0" << endl;
      filefunc::closefile(bundle_filename,bundlestream);

      string substring="bundler";
      int posn=stringfunc::first_substring_location(
         bundler_IO_subdir,substring);
      string path=bundler_IO_subdir.substr(
         posn+8,bundler_IO_subdir.length()-8);
      path="~/programs/c++/svn/projects/src/mains/photosynth/bundler/"+path;
//      cout << "path = " << path << endl;

      string unix_cmd="ln -s "+path+"bundle_trivial.out "+path+"bundle.out";
      sysfunc::unix_command(unix_cmd);

      return bundle_filename;
   }

// ---------------------------------------------------------------------
// Method read_in_pointcloud_and_photos() takes in the name of some
// subdirectory of mains/photosynth/bundler/ along with an input
// PassesGroup.  It fills and returns strings bundler_IO_subdir,
// image_sizes_filename and xyz_points_filenmae.  This method also
// reads in a reconstructed point cloud which is assumed to be either an
// OSGA or TDP file with prefix thresholded_xyz_points.  It also reads
// in packages for reconstructed photos and generates a new pass for
// each one.

   void read_in_pointcloud_and_photos(
      string subdir,PassesGroup& passes_group,
      int photo_number_step,int cloudpass_ID,
      string& bundler_IO_subdir,string& image_sizes_filename,
      string& xyz_points_filename)
      {
         bundler_IO_subdir="./bundler/"+subdir;
         image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";

// First read in point cloud data.  We assume cloudpass_filename
// either equals thresholded_xyz_points.osga or
// thresholded_xyz_points.tdp:

         string cloudpass_filename=bundler_IO_subdir+
            "thresholded_xyz_points.osga";
         if (!filefunc::fileexist(cloudpass_filename))
         {
            cloudpass_filename=bundler_IO_subdir+"thresholded_xyz_points.tdp";
         }
//         cout << "cloudpass_filename = " << cloudpass_filename << endl;
         xyz_points_filename=bundler_IO_subdir+"thresholded_xyz_points.dat";
         passes_group.generate_new_pass(cloudpass_filename,cloudpass_ID);

// Next read in reconstructed photos:

         int start_photo_number=0;
         filefunc::ReadInfile(image_sizes_filename);
//         cout << "filefunc::text_line.size() = " 
//              << filefunc::text_line.size() << endl;
         int stop_photo_number=filefunc::text_line.size()-1;

         vector<string> ext_filenames;
         for (int photo_number=start_photo_number; 
              photo_number <= stop_photo_number; photo_number += 
                 photo_number_step)
         {
            string curr_package_filename=
               bundler_IO_subdir+"packages/photo_"+
               stringfunc::integer_to_string(photo_number,4)+".pkg";
//            cout << "curr_package_filename = "
//                 << curr_package_filename << endl;
            if (filefunc::fileexist(curr_package_filename))
            {
               ext_filenames.push_back(curr_package_filename);
            }
         } // loop over photo_number 

         passes_group.interpret_arguments(ext_filenames);

         int photo_pass_counter=0;
         int n_passes=passes_group.get_n_passes();
         for (unsigned int i=0; i<ext_filenames.size(); i++)
         {
            string curr_package_filename=ext_filenames[i];
//            cout << "curr_package_filename = " << curr_package_filename 
//                 << endl;
            filefunc::ReadInfile(curr_package_filename);
            string pass_filename=filefunc::text_line[0];
            int specified_pass_ID=n_passes+photo_pass_counter;
            passes_group.generate_new_pass(pass_filename,specified_pass_ID);
            photo_pass_counter++;
         }
      }

// ---------------------------------------------------------------------
// Method scale_translate_rotate_bundler_XYZ() transforms input
// bundler_xyz by the input scaling, translation and rotation
// parameters.  We wrote this method in order to georegister
// reconstructed BUNDLER point clouds with ladar imagery.

// Deprecated pre-sailplane reconstruction (Dec 2010) method for
// converting bundler to world coordinates:

   void scale_translate_rotate_bundler_XYZ(
      threevector& bundler_xyz,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,
      double global_az,double global_el,double global_roll,
      const threevector& rotation_origin)
      {
         rotation global_R;
         global_R=global_R.rotation_from_az_el_roll(
            global_az,global_el,global_roll);
//      cout << "global_R = " << global_R << endl;

         scale_translate_rotate_bundler_XYZ(
            bundler_xyz,fitted_world_to_bundler_distance_ratio,
            fitted_bundler_trans,global_R,rotation_origin);
      }

// Deprecated pre-sailplane reconstruction (Dec 2010) method for
// converting bundler to world coordinates:

   void scale_translate_rotate_bundler_XYZ(
      threevector& bundler_xyz,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,const rotation& global_R,
      const threevector& rotation_origin)
      {
         bundler_xyz *= fitted_world_to_bundler_distance_ratio;
         bundler_xyz += fitted_bundler_trans;
         threevector rel_bundler_xyz=bundler_xyz-rotation_origin;
         rel_bundler_xyz = global_R * rel_bundler_xyz;
         bundler_xyz=rel_bundler_xyz+rotation_origin;
      }

// ---------------------------------------------------------------------
// Use this next method to convert bundler XYZs into georegistered
// points for all reconstructions generated after Dec 2010:

   void rotate_scale_translate_bundler_XYZ(
      threevector& bundler_xyz,const threevector& bundler_COM,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,const rotation& global_R)
      {
         bundler_xyz -= bundler_COM;
         bundler_xyz= global_R * bundler_xyz;
         bundler_xyz *= fitted_world_to_bundler_distance_ratio;
         bundler_xyz += fitted_bundler_trans;
      }

// ==========================================================================
// GPS georegistration methods
// ========================================================================== 

// Method read_left_right_points() parses corresponding 3D points
// within two input text files.  Following B.K. Horn, "Closed-form
// solution of absolute orientation using unit quaterions", Journal of
// Optical Society A, vol 4 (1987) pp 629-642, we refer to the 2 sets
// of points as "left" and "right" coordinate systems.

   void read_left_right_points(
      string left_points_filename,string right_points_filename,
      vector<threevector>& left_points,vector<threevector>& right_points)
   {
//      cout << "inside bundlerfunc::read_left_right_points()" << endl;
//      cout.precision(10);

      vector<threevector> raw_left_points,raw_right_points;

      filefunc::ReadInfile(left_points_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> column_values=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         double curr_x=stringfunc::string_to_number(column_values[2]);
         double curr_y=stringfunc::string_to_number(column_values[3]);
         double curr_z=stringfunc::string_to_number(column_values[4]);
         raw_left_points.push_back(threevector(curr_x,curr_y,curr_z));
//         cout << i << "  " << raw_left_points.back() << endl;
      }
//      cout << "left_points_filename = " << left_points_filename << endl;

      filefunc::ReadInfile(right_points_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> column_values=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         double curr_x=stringfunc::string_to_number(column_values[3]);
         double curr_y=stringfunc::string_to_number(column_values[4]);
         double curr_z=stringfunc::string_to_number(column_values[5]);
         raw_right_points.push_back(threevector(curr_x,curr_y,curr_z));
//         cout << i << "  " << raw_right_points.back() << endl;
      }
//      cout << "right_points_filename = " << right_points_filename << endl;

      int n_left_points=raw_left_points.size();
      int n_right_points=raw_right_points.size();
      if (n_left_points != n_right_points)
      {
         cout << "ERROR!" << endl;
         cout << "Number of points in " << left_points_filename
              << " = " << n_left_points << endl;
         cout << "Number of points in " << right_points_filename
              << " = " << n_right_points << endl;
         cout << "These numbers of points must be equal!" << endl;
         exit(-1);
      }

// As of 7/8/11, we assume that some small percentage of reconstructed 
// camera locations at the beginning and end of the camera's point are
// generally untrustworthy.  So we exclude these initial and final
// points from the results returned in left_points and right_points:

      double frac_to_ignore=0.0;
//      double frac_to_ignore=0.05;
      unsigned int n_raw_points=raw_left_points.size();
      for (unsigned int n=0; n<n_raw_points; n++)
      {
         if (n >= frac_to_ignore*n_raw_points && 
             n <= (1-frac_to_ignore)*n_raw_points)
         {
            left_points.push_back(raw_left_points[n]);
            right_points.push_back(raw_right_points[n]);
         }
      } // loop over index n labeling all raw input points

//      int n_points=left_points.size();
//      cout << "n_raw_points = " << n_raw_points
//           << " n_points = " << n_points << endl;
   }
   
// ---------------------------------------------------------------------
// Method subtract_left_right_COMS() takes in corresponding "left" and
// "right" coordinates system 3D points.  It computes and subtracts
// their COMS from the point lists.

   void subtract_left_right_COMs(
      vector<threevector>& left_points,vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM)
   {
//      cout << "inside bundlerfunc::subtract_left_right_COMs()" << endl;
      unsigned int n_points=left_points.size();

      left_COM=Zero_vector;
      right_COM=Zero_vector;

      for (unsigned int i=0; i<n_points; i++)
      {
         left_COM += left_points[i];
         right_COM += right_points[i];
//         cout << "i = " << i << " left_COM = " << left_COM
//              << " right_COM = " << right_COM << endl;
      }
      left_COM /= n_points;
      right_COM /= n_points;
   
      for (unsigned int i=0; i<n_points; i++)
      {
         left_points[i]=left_points[i]-left_COM;
         right_points[i]=right_points[i]-right_COM;
      }
   }

// ---------------------------------------------------------------------
// Method symmetric_N_matrix()

   genmatrix* form_symmetric_N_matrix(
      vector<threevector>& left_points,vector<threevector>& right_points)
   {
      unsigned int n_points=left_points.size();
   
      double Sxx,Sxy,Sxz;
      double Syx,Syy,Syz;
      double Szx,Szy,Szz;
      Sxx=Sxy=Sxz=Syx=Syy=Syz=Szx=Szy=Szz=0;

      for (unsigned int i=0; i<n_points; i++)
      {
         double xl=left_points[i].get(0);
         double yl=left_points[i].get(1);
         double zl=left_points[i].get(2);

         double xr=right_points[i].get(0);
         double yr=right_points[i].get(1);
         double zr=right_points[i].get(2);
      
         Sxx += xl*xr;
         Sxy += xl*yr;
         Sxz += xl*zr;

         Syx += yl*xr;
         Syy += yl*yr;
         Syz += yl*zr;

         Szx += zl*xr;
         Szy += zl*yr;
         Szz += zl*zr;
      } // loop over index i labeling points

// Form symmetrix 4x4 matrix N:

      genmatrix* N_ptr=new genmatrix(4,4);
      N_ptr->put(0,0,Sxx+Syy+Szz);
      N_ptr->put(0,1,Syz-Szy);
      N_ptr->put(0,2,Szx-Sxz);
      N_ptr->put(0,3,Sxy-Syx);
   
      N_ptr->put(1,0,Syz-Szy);
      N_ptr->put(1,1,Sxx-Syy-Szz);
      N_ptr->put(1,2,Sxy+Syx);
      N_ptr->put(1,3,Szx+Sxz);

      N_ptr->put(2,0,Szx-Sxz);
      N_ptr->put(2,1,Sxy+Syx);
      N_ptr->put(2,2,-Sxx+Syy-Szz);
      N_ptr->put(2,3,Syz+Szy);
   
      N_ptr->put(3,0,Sxy-Syx);
      N_ptr->put(3,1,Szx+Sxz);
      N_ptr->put(3,2,Syz+Szy);
      N_ptr->put(3,3,-Sxx-Syy+Szz);

//      cout << "N = " << *N_ptr << endl;
//      cout << "N_ptr->transpose = " << N_ptr->transpose() << endl;
//      cout << "N-NT = " << *N_ptr-N_ptr->transpose() << endl;
      
      return N_ptr;
   }

// ---------------------------------------------------------------------
// Method fit_rotation_angles()
   
   void fit_rotation_angles(
      genmatrix* N_ptr,double& az,double& el,double& roll)
   {
//      cout << "inside bundlerfunc::fit_rotation_angles()" << endl;
      
      genmatrix D(4,4),U(4,4);
//       bool flag=
         N_ptr->sym_eigen_decomposition(D,U);
//      cout << "flag = " << flag << endl;
//      cout << "D = " << D << endl;
//      cout << "U = " << U << endl;
//      cout << "N-U*D*Utrans = " 
//           << *N_ptr-U*D*U.transpose() << endl;
//      cout << "U*Utrans = " << U*U.transpose() << endl;

      vector<double> eigenvalues;
      vector<fourvector> eigenvectors;
      for (int i=0; i<4; i++)
      {
         eigenvalues.push_back(D.get(i,i));
         fourvector curr_eigenvector;
         U.get_column(i,curr_eigenvector);
         eigenvectors.push_back(curr_eigenvector);
      }
      templatefunc::Quicksort_descending(eigenvalues,eigenvectors);
  
//      for (int i=0; i<4; i++)
//      {
//         cout << "i = " << i
//              << " eigenvalue = " << eigenvalues[i]
//              << " eigenvector = " << eigenvectors[i] << endl;
//      }
//      cout << "N*e-lambda*e = "
//           << *N_ptr*eigenvectors[0]-eigenvalues[0]*eigenvectors[0] << endl;
      fourvector q=eigenvectors[0].unitvector();
//      cout << "q = " << q << endl;

      rotation R;
      R=R.rotation_corresponding_to_quaternion(q);
//      cout << "R = " << R << endl;
      R.az_el_roll_from_rotation(az,el,roll);
//      cout << "az = " << az*180/PI
//           << " el = " << el*180/PI
//           << " roll = " << roll*180/PI << endl;
   }

// ---------------------------------------------------------------------
// Method fit_scale_factor()
   
   double fit_scale_factor(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points)
   {
      unsigned int n_points=left_points.size();

      double numer=0;
      double denom=0;
      for (unsigned int i=0; i<n_points; i++)
      {
         numer += right_points[i].sqrd_magnitude();
         denom += left_points[i].sqrd_magnitude();
      }
      double scale=sqrt(numer/denom);
//      cout << "scale = " << scale << endl;

      return scale;
   }

// ---------------------------------------------------------------------
// Method fit_translation_and_reset_left_right_points()
   
   threevector fit_translation_and_reset_left_right_points(
      vector<threevector>& left_points,
      vector<threevector>& right_points,
      const threevector& right_COM,const threevector& left_COM,
      const rotation& R,double scale)
   {
      unsigned int n_points=left_points.size();

      threevector trans=right_COM-scale*(R*left_COM);
//      cout << "trans = " << trans << endl;

// Add COMs back to left and right points:

      for (unsigned int i=0; i<n_points; i++)
      {
         left_points[i] += left_COM;
         right_points[i] += right_COM;
      }

      return trans;
   }
   
// ---------------------------------------------------------------------
// Method RANSAC_fit_rotation_translation_scale() takes in a set of
// "left" points which need to be rotated, translated and scaled so
// that they optimally match a corresponding set of "right" points.
// Since the input "left" points may contain significant outliers, we
// perform RANSAC determination of inlier transformed left-right
// tiepoint pairs.  The rotation, translation and scale parameters
// which yield the largest number of inlier tiepoint pairs are printed
// out by this method.  
   
   void RANSAC_fit_rotation_translation_scale(
      vector<threevector>& left_points,vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM,
      double& az,double& el,double& roll,double& scale,threevector& trans_Horn,
      threevector& trans_Peter,
      double& median_residual_dist,double& quartile_width)
   {
//      cout << "inside bundlerfunc::RANSAC_fit_rotation_translation_scale()" 
//           << endl;
//      cout << "left_points.size() = " << left_points.size() << endl;
//      cout << "right_points.size() = " << right_points.size() << endl;

      unsigned int n_points=left_points.size();
//       double max_inlier_frac=0;
      double min_median_residual_dist=POSITIVEINFINITY;

      int n_iters=0;
//      int max_n_RANSAC_iters=100;
      int max_n_RANSAC_iters=1000;

      double best_scale,best_az,best_el,best_roll;
      best_scale = best_az = best_el = best_roll = 0;
      threevector best_left_COM,best_trans_Horn,best_trans_Peter;
      vector<threevector> subsampled_left_points,subsampled_right_points;
      while (n_iters < max_n_RANSAC_iters)
      {
         vector<int> tiepoint_indices=mathfunc::random_sequence(
            0,n_points-1,8);

         subsampled_left_points.clear();
         subsampled_right_points.clear();
         for (unsigned int q=0; q<tiepoint_indices.size(); q++)
         {
            int random_index=tiepoint_indices[q];
//            cout << "q = " << q << " random_index = " << random_index
//                 << endl;
            subsampled_left_points.push_back(left_points[random_index]);
            subsampled_right_points.push_back(right_points[random_index]);
         }
         
         fit_rot_trans_scale_params(
            subsampled_left_points,subsampled_right_points,
            left_COM,right_COM,az,el,roll,scale,trans_Horn,trans_Peter);

// FAKE FAKE:  Sun Jul 28, 2013 at 10:20 am
// Experiment with using median left-right residual distance as score
// for RANSAC rather than n_inliers which depends upon a pre-defined
// max_residual_dist:

         median_left_right_residual_distance(
            left_points,right_points,az,el,roll,scale,trans_Horn,
            median_residual_dist,quartile_width);
         if (median_residual_dist < min_median_residual_dist)
         {
            min_median_residual_dist=median_residual_dist;
            best_scale=scale;
            best_az=az;
            best_el=el;
            best_roll=roll;
            best_left_COM=left_COM;
            best_trans_Horn=trans_Horn;
            best_trans_Peter=trans_Peter;
            cout << "n_iter = " << n_iters 
                 << " min_median_residual_dist = " << min_median_residual_dist
                 << endl;
         }
         
/*
         double inlier_frac=inlier_left_right_pair_fraction(
            left_points,right_points,az,el,roll,scale,trans_Horn,
            max_residual_dist);
         if (inlier_frac > max_inlier_frac)
         {
            max_inlier_frac=inlier_frac;
            best_scale=scale;
            best_az=az;
            best_el=el;
            best_roll=roll;
            best_left_COM=left_COM;
            best_trans_Peter=trans_Peter;
            cout << "n_iter = " << n_iters 
                 << " max_inlier_frac = " << max_inlier_frac << endl;
         }
*/

         n_iters++;
      } // RANSAC while loop

      scale=best_scale;
      az=best_az;
      el=best_el;
      roll=best_roll;
      left_COM=best_left_COM;
      trans_Horn=best_trans_Horn;
      trans_Peter=best_trans_Peter;

// Write out 7 fitted parameter values:

      cout << endl;
      cout << "Global parameters that convert Bundler to GPS coordinates:"
           << endl;
      cout << endl;
      cout << "=====================================================" << endl;
      cout << endl;
//      cout << "double scale_0 = " << scale << ";" << endl;
//      cout << "double xtrans_0 = " << trans_Peter.get(0) << ";" << endl;
//      cout << "double ytrans_0 = " << trans_Peter.get(1) << ";" << endl;
//      cout << "double ztrans_0 = " << trans_Peter.get(2) << ";" << endl;
//      cout << "double az_0 = " << az*180/PI << ";" << endl;
//      cout << "double el_0 = " << el*180/PI << ";" << endl;
//      cout << "double roll_0 = " << roll*180/PI << ";" << endl << endl;
      
//      cout << "threevector bundler_rotation_origin" 
//           << "( " 
//           << left_COM.get(0) << ", " 
//           << left_COM.get(1) << ", " 
//           << left_COM.get(2) << " ); " << endl;
//      cout << endl;
//      cout << "=====================================================" << end//l;
      cout << endl;

      cout << "--fitted_world_to_bundler_distance_ratio " << scale << endl;
      cout << "--bundler_translation_X " << trans_Peter.get(0) << endl;
      cout << "--bundler_translation_Y " << trans_Peter.get(1) << endl;
      cout << "--bundler_translation_Z " << trans_Peter.get(2) << endl;
      cout << "--global_az " << az*180/PI << endl;
      cout << "--global_el " << el*180/PI << endl;
      cout << "--global_roll " << roll*180/PI << endl;
      cout << "--bundler_rotation_origin_X " << left_COM.get(0) << endl;
      cout << "--bundler_rotation_origin_Y " << left_COM.get(1) << endl;
      cout << "--bundler_rotation_origin_Z " << left_COM.get(2) << endl;
      cout << endl;
   }

// ---------------------------------------------------------------------
// Method fit_rot_trans_scale_params() takes in a set of "left"
// points which need to be rotated, translated and scaled so that they
// optimally match a corresponding set of "right" points.  It returns
// the best fit values for the rotation, translation and scale
// parameters.
   
   void fit_rot_trans_scale_params(
      vector<threevector>& left_points,vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM,
      double& az,double& el,double& roll,double& scale,threevector& trans_Horn,
      threevector& trans_Peter)
   {
//      cout << "inside bundlerfunc::fit_rot_trans_scale_params()" 
//           << endl;
//      cout << "left_points.size() = " << left_points.size() << endl;
//      cout << "right_points.size() = " << right_points.size() << endl;

      subtract_left_right_COMs(
         left_points,right_points,left_COM,right_COM);
//      cout << "left_COM = " << left_COM << endl;
//      cout << "right_COM = " << right_COM << endl;

      genmatrix* N_ptr=form_symmetric_N_matrix(left_points,right_points);

      fit_rotation_angles(N_ptr,az,el,roll);
      delete N_ptr;

      rotation R;
      R=R.rotation_from_az_el_roll(az,el,roll);
//      cout << "R = " << R << endl;

// Compute scale factor:

      scale=fit_scale_factor(left_points,right_points);
//      cout << "Scale = " << scale << endl;

// Compute translation vector:

      trans_Horn=fit_translation_and_reset_left_right_points(
         left_points,right_points,right_COM,left_COM,R,scale);
//      cout << "trans_Horn = " << trans_Horn << endl;

      trans_Peter=trans_Horn+scale*(R*left_COM);
   }
   
// ---------------------------------------------------------------------
// Method compute_avg_residual()
   
   double compute_avg_residual(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points,
      vector<threevector>& transformed_left_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn)
   {
      unsigned int n_points=left_points.size();

      rotation R;
      R=R.rotation_from_az_el_roll(az,el,roll);

      vector<double> residuals;
      for (unsigned int i=0; i<n_points; i++)
      {
         threevector transformed_left_point=scale*(R*left_points[i])
            +trans_Horn;
         transformed_left_points.push_back(transformed_left_point);
//      cout << "i = " << i
//           << " right_point = " << right_points[i]
//           << " transformed left point = "
//           << transformed_left_point << endl;
         double curr_residual = 
            (transformed_left_point-right_points[i]).magnitude();
//         cout << "i = " << i << " curr_residual = " << curr_residual << endl;
         residuals.push_back(curr_residual);
      }
      double mu_residual=mathfunc::mean(residuals);
      double sigma_residual=mathfunc::std_dev(residuals);
      cout << "Residual between transformed left and raw right points = " 
           << endl << endl;
      cout << mu_residual << " +/- " << sigma_residual << endl << endl;

      return mu_residual;
   }
   
// ---------------------------------------------------------------------
// Method identify_left_right_pair_inliers()
   
   vector<int> identify_left_right_pair_inliers(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist)
   {
//      cout << "inside bundlerfunc::identify_left_right_inliers()"
//           << endl;
      
      rotation R;
      R=R.rotation_from_az_el_roll(az,el,roll);

      vector<int> inlier_indices;
      for (unsigned int i=0; i<left_points.size(); i++)
      {
         threevector transformed_left_point=scale*(R*left_points[i])
            +trans_Horn;
         double curr_residual_dist = 
            (transformed_left_point-right_points[i]).magnitude();
//         cout << "i = " << i << " curr_residual_dist = "
//              << curr_residual_dist << endl;
         if (curr_residual_dist < max_residual_dist) 
            inlier_indices.push_back(i);
      }
      return inlier_indices;
   }
   
// ---------------------------------------------------------------------
// Method identify_left_right_pair_outliers()
   
   vector<int> identify_left_right_pair_outliers(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist)
   {
      cout << "inside bundlerfunc::identify_left_right_outliers()"
           << endl;
      
      rotation R;
      R=R.rotation_from_az_el_roll(az,el,roll);

      vector<int> outlier_indices;
      for (unsigned int i=0; i<left_points.size(); i++)
      {
         threevector transformed_left_point=scale*(R*left_points[i])
            +trans_Horn;
         double curr_residual_dist = 
            (transformed_left_point-right_points[i]).magnitude();
         cout << "i = " << i << " curr_residual_dist = "
              << curr_residual_dist << endl;
         if (curr_residual_dist > max_residual_dist) 
            outlier_indices.push_back(i);
      }
      return outlier_indices;
   }
   
// ---------------------------------------------------------------------
// Method inlier_left_right_pair_fraction()
   
   double inlier_left_right_pair_fraction(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist)
   {
//      cout << "inside bundlerfunc::inlier_left_right_pair_fraction()"
//           << endl;
      
      vector<int> left_right_pair_inliers=identify_left_right_pair_inliers(
         left_points,right_points,az,el,roll,scale,
         trans_Horn,max_residual_dist);
      unsigned int n_inliers=left_right_pair_inliers.size();
      double inlier_frac=double(n_inliers)/left_points.size();
      return inlier_frac;
   }

// ---------------------------------------------------------------------
// Method median_left_right_residual_distance() computes residual
// distances between transformed left points and their right point
// counterparts.  It returns the median of these residual distances.
   
   void median_left_right_residual_distance(
      const vector<threevector>& left_points,
      const vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,
      double& median_distance,double& quartile_width)
   {
//      cout << "inside bundlerfunc::median_left_right_residual_distance()"
//           << endl;

//      cout << "az = " << az*180/PI 
//           << " el = " << el*180/PI
//           << " roll = " << roll*180/PI
//           << " scale = " << scale
//           << " trans_Horn = " << trans_Horn << endl;
      
      rotation R;
      R=R.rotation_from_az_el_roll(az,el,roll);

      vector<double> residual_distances;
      for (unsigned int i=0; i<left_points.size(); i++)
      {
         threevector transformed_left_point=scale*(R*left_points[i])
            +trans_Horn;
         double curr_residual_dist = 
            (transformed_left_point-right_points[i]).magnitude();
//         cout << "i = " << i << " curr_residual_dist = "
//              << curr_residual_dist << endl;
         residual_distances.push_back(curr_residual_dist);
      }

      mathfunc::median_value_and_quartile_width(
         residual_distances,median_distance,quartile_width);
   }

// ==========================================================================
// Radial undistortion methods
// ==========================================================================
   
   string radial_undistort(
      photograph* photo_ptr,string undistorted_images_subdir)
   {
//      cout << "inside bundlerfunc::radial_undistort()" << endl;

      camera* camera_ptr=photo_ptr->get_camera_ptr();
      return radial_undistort(
         camera_ptr,photo_ptr->get_filename(),undistorted_images_subdir);
   }

// ---------------------------------------------------------------------   
// Method radial_undistort() resamples the image specified by input
// photo_filename using the radial distortion parameters encoded
// within input camera *camera_ptr.  It generates a new output image
// for which the kappa2 and kappa4 radial distortion parameters equal
// 0.  This method returns the name of the new, radially undistorted
// image file.

   string radial_undistort(
      camera* camera_ptr,string photo_filename,
      string undistorted_images_subdir)
   {
      cout << "inside bundlerfunc::radial_undistort()" << endl;
      cout << "Undistorting " << photo_filename << endl;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         photo_filename,NULL);

      texture_rectangle* undistorted_texture_rectangle_ptr=
         new texture_rectangle(photo_filename,NULL);
      undistorted_texture_rectangle_ptr->clear_all_RGB_values();

      int w=texture_rectangle_ptr->getWidth();
      int h=texture_rectangle_ptr->getHeight();

      double f=camera_ptr->get_fu();		// indep of h
      double kappa2=camera_ptr->get_kappa2();	//
      double kappa4=camera_ptr->get_kappa4();	// 

      double f_noah=fabs(f)*h;
      for (int py=0; py<h; py++)
      {
         for (int px=0; px<w; px++)
         {
            double px_c = px - 0.5 * w;
            double py_c = py - 0.5 * h;

            double r2 = (px_c*px_c + py_c*py_c)/sqr(f_noah);
            double factor = 1.0 + kappa2*r2 + kappa4*r2*r2;

            px_c *= factor;
            py_c *= factor;
            px_c += 0.5 * w;
            py_c += 0.5 * h;

            int R=0;
            int G=0;
            int B=0;
            if (px_c >= 0 && px_c < w && py_c >= 0 && py_c < h) 
            {
               texture_rectangle_ptr->get_pixel_RGB_values(px_c,py_c,R,G,B);
            }

/*
            if (px > 0.5*w-2 && px < 0.5*w+2 &&
                noah_py > 0.5*h-5 && noah_py < 0.5*h+5)
            {
               cout.precision(10);
               cout << "px = " << px 
//                    << " py = " << py 
                    << " npy = " << noah_py
                    << " px_c = " << px_c
                    << " py_c = " << py_c 
//                    << " k2 = " << kappa2
//                    << " k4 = " << kappa4
                    << " r2 = " << r2
                    << " factor = " << factor 
                    << " R = " << R 
                    << " G = " << G 
                    << " B = " << B 
                    << endl;
            }
*/

            undistorted_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,R,G,B);

         } // loop over px index
      } // loop over py index

      outputfunc::enter_continue_char();

      delete texture_rectangle_ptr;

      filefunc::dircreate(undistorted_images_subdir);

      string basename=filefunc::getbasename(photo_filename);
      string prefix=stringfunc::prefix(basename);
      string suffix=stringfunc::suffix(basename);
      string undistorted_filename=
         undistorted_images_subdir+prefix+"."+suffix;
      undistorted_texture_rectangle_ptr->write_curr_frame(
         undistorted_filename);

      string banner="Wrote radially undistorted image "+undistorted_filename;
      outputfunc::write_banner(banner);

      delete undistorted_texture_rectangle_ptr;
      return undistorted_filename;
   }

/*
// ---------------------------------------------------------------------   
// Method radial_undistort() resamples the image specified by input
// photo_filename using the radial distortion parameters encoded
// within input camera *camera_ptr.  It generates a new output image
// for which the kappa2 and kappa4 radial distortion parameters equal
// 0.  This method returns the name of the new, radially undistorted
// image file.

   string radial_undistort(
      camera* camera_ptr,string photo_filename,
      string undistorted_images_subdir)
   {
      cout << "inside bundlerfunc::radial_undistort()" << endl;
      cout << "Undistorting " << photo_filename << endl;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         photo_filename,NULL);

      texture_rectangle* undistorted_texture_rectangle_ptr=
         new texture_rectangle(photo_filename,NULL);
      undistorted_texture_rectangle_ptr->clear_all_RGB_values();

      int w=texture_rectangle_ptr->getWidth();
      int h=texture_rectangle_ptr->getHeight();

      double f=camera_ptr->get_fu();		// indep of h
      double kappa2=camera_ptr->get_kappa2();	// depends on h !!
      double kappa4=camera_ptr->get_kappa4();	// depends on h !!

      double f_noah=fabs(f)*h;
      for (int py=0; py<h; py++)
      {
         for (int px=0; px<w; px++)
         {
            double px_c = px - 0.5 * w;
            double py_c = py - 0.5 * h;

            double r2 = (px_c*px_c + py_c*py_c)/sqr(f_noah);
            double factor = 1.0 + kappa2*r2 + kappa4*r2*r2;

            px_c *= factor;
            py_c *= factor;
            px_c += 0.5 * w;
            py_c += 0.5 * h;

            int R=0;
            int G=0;
            int B=0;
            if (px_c >= 0 && px_c < w && py_c >= 0 && py_c < h) 
            {
               texture_rectangle_ptr->get_pixel_RGB_values(px_c,py_c,R,G,B);
            }

//            if (px > 0.5*w-2 && px < 0.5*w+2 &&
//                noah_py > 0.5*h-5 && noah_py < 0.5*h+5)
//            {
//               cout.precision(10);
//               cout << "px = " << px 
//                    << " py = " << py 
//                    << " npy = " << noah_py
//                    << " px_c = " << px_c
//                    << " py_c = " << py_c 
//                    << " k2 = " << kappa2
//                    << " k4 = " << kappa4
//                    << " r2 = " << r2
//                    << " factor = " << factor 
//                    << " R = " << R 
//                    << " G = " << G 
//                    << " B = " << B 
//                    << endl;
//            }

            undistorted_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,R,G,B);

         } // loop over px index
      } // loop over py index

      outputfunc::enter_continue_char();

      delete texture_rectangle_ptr;

      filefunc::dircreate(undistorted_images_subdir);

      string basename=filefunc::getbasename(photo_filename);
      string prefix=stringfunc::prefix(basename);
      string suffix=stringfunc::suffix(basename);
      string undistorted_filename=
         undistorted_images_subdir+prefix+"."+suffix;
      undistorted_texture_rectangle_ptr->write_curr_frame(
         undistorted_filename);

      string banner="Wrote radially undistorted image "+undistorted_filename;
      outputfunc::write_banner(banner);

      delete undistorted_texture_rectangle_ptr;
      return undistorted_filename;
   }
*/
   
// ---------------------------------------------------------------------
// Method generate_undistorted_bundle_file()
   
   void generate_undistorted_bundle_file(
      string bundle_filename,string undistorted_images_subdir)
   {
      cout << "inside bundlerfunc::generate_undistorted_bundle_file()" << endl;
      filefunc::ReadInfile(bundle_filename);
      
      string new_bundle_filename=undistorted_images_subdir+"bundle.rd.out";
      ofstream outstream;
      filefunc::openfile(new_bundle_filename,outstream);
      outstream << "# Bundle file v0.3" << endl;

      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[0]);
      unsigned int n_photos=column_values[0];
      cout << "n_photos = " << n_photos << endl;

      outstream.precision(10);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         if ((i-1)%5==0 && i < 5*n_photos)
         {
            column_values.clear();
            column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            outstream << column_values[0] << " 0 0" << endl;
         }
         else
         {
            outstream << filefunc::text_line[i] << endl;
         }
      }
   }

// ==========================================================================
// Bundle.out file export methods
// ========================================================================== 
   
// Method export_bundle_file() takes in a set of cameras within
// *photogroup_ptr and a set of 3D points plus their 2D image plane
// projections in *FeaturesGroup_ptr.  It exports a bundle.out file in
// Noah's bundler (as opposed to world) coordinate system.
   
   void export_bundle_file(
      string bundle_filename,photogroup* photogroup_ptr,
      FeaturesGroup* FeaturesGroup_ptr)
   {
      export_bundle_file(
         bundle_filename,photogroup_ptr,FeaturesGroup_ptr,NULL);
   }

   void export_bundle_file(
      string bundle_filename,photogroup* photogroup_ptr,
      FeaturesGroup* FeaturesGroup_ptr,
      map<int,bool>* feature_ids_to_ignore_map_ptr)
   {
//      cout << "inside bundlerfunc::export_bundle_file()" << endl;
      
      int n_cameras=photogroup_ptr->get_n_photos();
      unsigned int n_features=FeaturesGroup_ptr->get_n_Graphicals();

      int n_features_to_export=n_features;
      if (feature_ids_to_ignore_map_ptr != NULL)
         n_features_to_export -= feature_ids_to_ignore_map_ptr->size();
  
      ofstream bundlestream;
      filefunc::openfile(bundle_filename,bundlestream);
      bundlestream << "# Bundle file v0.3" << endl;
      bundlestream << n_cameras << " " << n_features_to_export << endl;

// First export intrinsic and extrinsic camera parameters:

      vector<double> xdims,ydims;
      vector<texture_rectangle*> texture_rectangle_ptrs;
      for (int n=0; n<n_cameras; n++)
      {
         photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
         xdims.push_back(photo_ptr->get_xdim());
         ydims.push_back(photo_ptr->get_ydim());

         string image_filename=photo_ptr->get_filename();
         texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
            image_filename,NULL);
         texture_rectangle_ptrs.push_back(texture_rectangle_ptr);

         camera* camera_ptr=photo_ptr->get_camera_ptr();
         double f=camera_ptr->get_fu();
         double f_noah=fabs(f)*ydims.back();
         double k1=0;
         double k2=0;
         bundlestream << stringfunc::scinumber_to_string(f_noah)
                      << " " << k1 << " " << k2 << endl;

//      double FOV_u,FOV_v;
//      camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
//         f,aspect_ratio,FOV_u,FOV_v);
//      cout << "FOV_u = " << FOV_u*180/PI << " FOV_v = " << FOV_v*180/PI
//           << endl;

         rotation R_noah;
         threevector t_noah;
         camera_ptr->convert_world_to_bundler_coords(R_noah,t_noah);

         for (int r=0; r<3; r++)
         {
            for (int c=0; c<3; c++)
            {
               bundlestream << stringfunc::scinumber_to_string(
                  R_noah.get(r,c),9)
                            << " ";
            }
            bundlestream << endl;
         }
         bundlestream << stringfunc::scinumber_to_string(t_noah.get(0)) << " "
                      << stringfunc::scinumber_to_string(t_noah.get(1)) << " "
                      << stringfunc::scinumber_to_string(t_noah.get(2)) 
                      << endl;

//         cout <<  "R_noah = " << R_noah << endl;
//         cout << "t_noah = " << t_noah << endl;
      } // loop over index n labeling cameras

// Next export points in bundler (rather than world) coordinates:

//      cout << "Exporting points in bundler coords" << endl;
      
      rotation* R_noah_to_peter_ptr=new rotation();

      R_noah_to_peter_ptr->put(0,0,-1);
      R_noah_to_peter_ptr->put(0,1,0);
      R_noah_to_peter_ptr->put(0,2,0);
   
      R_noah_to_peter_ptr->put(1,0,0);
      R_noah_to_peter_ptr->put(1,1,0);
      R_noah_to_peter_ptr->put(1,2,1);
   
      R_noah_to_peter_ptr->put(2,0,0);
      R_noah_to_peter_ptr->put(2,1,1);
      R_noah_to_peter_ptr->put(2,2,0);

      double curr_t=FeaturesGroup_ptr->get_curr_t();
      threevector XYZ,UVW;

      int n_features_exported=0;
      for (unsigned int f=0; f<n_features; f++)
      {
         Feature* Feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(f);
         int feature_ID=Feature_ptr->get_ID();

// Ignore current feature if its ID is included within
// *feature_ids_to_ignore_map_ptr:

         if (feature_ids_to_ignore_map_ptr != NULL)
         {
            if (feature_ids_to_ignore_map_ptr->find(feature_ID) != 
            feature_ids_to_ignore_map_ptr->end())
            {
               continue;
            }
         }
         n_features_exported++;

         Feature_ptr->get_UVW_coords(curr_t,n_cameras,XYZ);

         threevector bundler_XYZ= R_noah_to_peter_ptr->transpose() * XYZ;
         bundlestream << bundler_XYZ.get(0) << " "
                      << bundler_XYZ.get(1) << " "
                      << bundler_XYZ.get(2) << endl;

         fourvector curr_camera_key_U_V;
         vector<fourvector> camera_key_U_V;

         int R,G,B;
         R=G=B=0;
         int feature_index;
         for (int p=0; p<n_cameras; p++)
         {
            int camera_ID=p;

            Feature_ptr->get_index(curr_t,p,feature_index);
            if (feature_index < 0) continue;

            Feature_ptr->get_UVW_coords(curr_t,p,UVW);
            double U=UVW.get(0);
            double V=UVW.get(1);

            int curr_R,curr_G,curr_B;
            texture_rectangle_ptrs[p]->get_RGB_values(
               U,V,curr_R,curr_G,curr_B);
            R += curr_R;
            G += curr_G;
            B += curr_B;

//         cout << "camera_ID = " << camera_ID 
//              << " feature_index = " << feature_index
//              << " U = " << U << " V = " << V << endl;
         
            double px=U*ydims[p]-0.5*xdims[p];
            double py=V*ydims[p]-0.5*ydims[p];
         
            curr_camera_key_U_V.put(0,camera_ID);
            curr_camera_key_U_V.put(1,feature_index);
            curr_camera_key_U_V.put(2,px);
            curr_camera_key_U_V.put(3,py);
            camera_key_U_V.push_back(curr_camera_key_U_V);
         } // loop over index p labeling cameras
         unsigned int n_views=camera_key_U_V.size();

         if (n_views > 0)
         {
            R /= n_views;
            G /= n_views;
            B /= n_views;
         }
         bundlestream << R << " " << G << " " << B << endl;

         bundlestream << n_views << " ";
         for (unsigned int c=0; c<n_views; c++)
         {
            bundlestream 
               << camera_key_U_V[c].get(0) << " "
               << camera_key_U_V[c].get(1) << " "
               << camera_key_U_V[c].get(2) << " "
               << camera_key_U_V[c].get(3) << " ";
         }
         bundlestream << endl;

      } // loop over index f labeling reconstructed 3D points

      cout << "n_features = " << n_features
           << " n_features exported to bundle.out = " << n_features_exported
           << endl;

      for (int n=0; n<n_cameras; n++)
      {
         delete texture_rectangle_ptrs[n];
      }
      texture_rectangle_ptrs.clear();

      filefunc::closefile(bundle_filename,bundlestream);
      string banner="Exported "+bundle_filename;
      outputfunc::write_big_banner(banner);
   }

// ---------------------------------------------------------------------   
// This next overloaded version of export_bundle_file() takes in a set
// of cameras within *photogroup_ptr and a set of 3D points in XYZ.
// It exports a bundle.out file in Noah's bundler (as opposed to
// world) coordinate system.  UV image plane projections of XYZ points
// are ignored by this method.
   
   void export_bundle_file(
      string bundle_filename,photogroup* photogroup_ptr,
      const vector<threevector>& XYZ)
   {
//      cout << "inside bundlerfunc::export_bundle_file()" << endl;
      
      int n_cameras=photogroup_ptr->get_n_photos();
      unsigned int n_features=XYZ.size();
      
      ofstream bundlestream;
      bundlestream.precision(12);
      
      filefunc::openfile(bundle_filename,bundlestream);
      bundlestream << "# Bundle file v0.3" << endl;
      bundlestream << n_cameras << " " << n_features << endl;

// First export intrinsic and extrinsic camera parameters:

      vector<double> xdims,ydims;
      vector<texture_rectangle*> texture_rectangle_ptrs;
      for (int n=0; n<n_cameras; n++)
      {
         photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
         xdims.push_back(photo_ptr->get_xdim());
         ydims.push_back(photo_ptr->get_ydim());

         camera* camera_ptr=photo_ptr->get_camera_ptr();
         double f=camera_ptr->get_fu();
         double f_noah=fabs(f)*ydims.back();
         double k1=0;
         double k2=0;
         bundlestream << stringfunc::scinumber_to_string(f_noah)
                      << " " << k1 << " " << k2 << endl;

//      double FOV_u,FOV_v;
//      camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
//         f,aspect_ratio,FOV_u,FOV_v);
//      cout << "FOV_u = " << FOV_u*180/PI << " FOV_v = " << FOV_v*180/PI
//           << endl;

         rotation R_noah;
         threevector t_noah;
         camera_ptr->convert_world_to_bundler_coords(R_noah,t_noah);

         for (int r=0; r<3; r++)
         {
            for (int c=0; c<3; c++)
            {
               bundlestream << stringfunc::scinumber_to_string(
                  R_noah.get(r,c),9)
                            << " ";
            }
            bundlestream << endl;
         }
         bundlestream << stringfunc::scinumber_to_string(t_noah.get(0)) << " "
                      << stringfunc::scinumber_to_string(t_noah.get(1)) << " "
                      << stringfunc::scinumber_to_string(t_noah.get(2)) 
                      << endl;

//         cout <<  "R_noah = " << R_noah << endl;
//         cout << "t_noah = " << t_noah << endl;
      } // loop over index n labeling cameras

// Next export points in bundler (rather than world) coordinates:

      rotation* R_noah_to_peter_ptr=new rotation();

      R_noah_to_peter_ptr->put(0,0,-1);
      R_noah_to_peter_ptr->put(0,1,0);
      R_noah_to_peter_ptr->put(0,2,0);
   
      R_noah_to_peter_ptr->put(1,0,0);
      R_noah_to_peter_ptr->put(1,1,0);
      R_noah_to_peter_ptr->put(1,2,1);
   
      R_noah_to_peter_ptr->put(2,0,0);
      R_noah_to_peter_ptr->put(2,1,1);
      R_noah_to_peter_ptr->put(2,2,0);

      int R,G,B;
      R=G=B=0;
      int n_views=0;
      for (unsigned int f=0; f<n_features; f++)
      {
         threevector bundler_XYZ= R_noah_to_peter_ptr->transpose() * XYZ[f];
         bundlestream << bundler_XYZ.get(0) << " "
                      << bundler_XYZ.get(1) << " "
                      << bundler_XYZ.get(2) << endl;
         bundlestream << R << " " << G << " " << B << endl;
         bundlestream << n_views << endl;
      } // loop over index f labeling reconstructed 3D points

      filefunc::closefile(bundle_filename,bundlestream);
      string banner="Exported "+bundle_filename;
      outputfunc::write_big_banner(banner);
   }

// ==========================================================================
// Z_ground finding methods
// ==========================================================================

// Method extract_Zground() imports a thresholded XYZ point cloud
// generated by bundler which we assume is georegistered.  It forms
// the cloud's Z distribution and searches for peaks.  The lowest peak
// is assumed to correspond to a flat, ground plane.  The Z value for
// the ground plane is returned by this method.  The Z distribution is
// also exported to Zground.jpg.

   double extract_Zground(string bundler_IO_subdir)
   {
      cout << "inside bundlerfunc::extract_Zground()" << endl;
//      string thresholded_xyz_tdp_filename=bundler_IO_subdir+
//         "thresholded_xyz_points.tdp";
      string thresholded_xyz_tdp_filename=bundler_IO_subdir+
         "pmvs/models/pmvs_options.txt.tdp";
//      cout << "thresholded_xyz_tdp_filename = " << thresholded_xyz_tdp_filena//me
//           << endl;

      vector<double> X,Y,Z;
      tdpfunc::read_XYZ_points_from_tdpfile(
         thresholded_xyz_tdp_filename,X,Y,Z);
      double Zlo=mathfunc::minimal_value(Z);
      cout << "Zlo = " << Zlo << endl;

      prob_distribution Z_prob(Z,100,Zlo);
      Z_prob.writeprobdists(false);
      string unix_cmd="mv prob_density.jpg "+bundler_IO_subdir+"Zground.jpg";
      sysfunc::unix_command(unix_cmd);
      
      double peak_width=2.5;	// meters
      int n_max_peaks=3;
      vector<double> Zpeaks=mathfunc::find_local_peaks(
         Z,peak_width,n_max_peaks);

      std::sort(Zpeaks.begin(),Zpeaks.end());
//      for (unsigned int p=0; p<Zpeaks.size(); p++)
//      {
//         cout << "p = " << p << " Zpeak = " << Zpeaks[p] << endl;
//      }
      double Zground=Zpeaks[0];
      cout << "Zground = " << Zground << endl;
      return Zground;
   }
   

} // bundlerfunc namespace
