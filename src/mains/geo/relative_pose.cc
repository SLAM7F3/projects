// ========================================================================
// Program RELATIVE_POSE imports two aerial video frames which we
// label as "current" and "next".  It works with the camera metadata
// for the two input frames except their orientation angles.  Instead,
// we assume both frames were shot at a constant depression angle
// (which is hardwired below) and with zero roll angle.  The relative
// azimuthal angle between the two video frames' cameras is
// guestimated based upon their average altitude.  Working with just
// the first few frames from GEO pass 20120105_1402, we empirically
// find that the guestimatzed relative az, el and roll angles between
// the two frames are within a few degrees of the bundle adjusted
// "true" values computed via our entire GEO reconstruction pipeline.

// RELATIVE_POSE also attempts to improve upon the initial guestimated
// relative camera orientation angles by brute force searching for
// values which minimize a score function based upon ray intersection
// errors.  But after many experiments, we could not find any set of
// input parameters for the brute force search which yielded better
// results than the original guestimated angles.  So as of 9/20/13, we
// suspect that feeding the guestimates as starting points to bundle
// adjustment may be the best way to operate when initial camera
// pointing metadata is unavailable.

// ========================================================================
// Last updated on 9/19/13; 9/20/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "math/genmatrix.h"
#include "geometry/homography.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();      
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string packages_subdir=bundler_IO_subdir+"packages/";

   int curr_reconstructed_photo_ID=0;
   int next_reconstructed_photo_ID=1;
   string curr_reconstructed_photo_ID_str=
      stringfunc::integer_to_string(curr_reconstructed_photo_ID,4);
   string next_reconstructed_photo_ID_str=
      stringfunc::integer_to_string(next_reconstructed_photo_ID,4);

   string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
   tiepoints_subdir += "tiepoints_"+
      curr_reconstructed_photo_ID_str+"_"+
      next_reconstructed_photo_ID_str+"/";
   cout << "tiepoints_subdir = " << tiepoints_subdir << endl;

// First import bundle adjusted az, el, roll angles as truth data for
// 'current' and 'next' images:

   string curr_photo_package_filename=packages_subdir+
      "photo_"+curr_reconstructed_photo_ID_str+".pkg";
   string next_photo_package_filename=packages_subdir+
      "photo_"+next_reconstructed_photo_ID_str+".pkg";

   vector<vector<string> > curr_package_substrings=
      filefunc::ReadInSubstrings(curr_photo_package_filename);
   double curr_f=stringfunc::string_to_number(
      curr_package_substrings[2].at(1));
   double curr_U0=stringfunc::string_to_number(
      curr_package_substrings[4].at(1));
   double curr_V0=stringfunc::string_to_number(
      curr_package_substrings[5].at(1));
   double curr_az=stringfunc::string_to_number(
      curr_package_substrings[6].at(1))*PI/180;
   double curr_el=stringfunc::string_to_number(
      curr_package_substrings[7].at(1))*PI/180;
   double curr_roll=stringfunc::string_to_number(
      curr_package_substrings[8].at(1))*PI/180;
   cout << "curr_az = " << curr_az*180/PI << endl;
   cout << "curr_el = " << curr_el*180/PI << endl;
   cout << "curr_roll = " << curr_roll*180/PI << endl;
   
   vector<vector<string> > next_package_substrings=
      filefunc::ReadInSubstrings(next_photo_package_filename);
   double next_f=stringfunc::string_to_number(
      next_package_substrings[2].at(1));
   double next_U0=stringfunc::string_to_number(
      next_package_substrings[4].at(1));
   double next_V0=stringfunc::string_to_number(
      next_package_substrings[5].at(1));
   double next_az=stringfunc::string_to_number(
      next_package_substrings[6].at(1))*PI/180;
   double next_el=stringfunc::string_to_number(
      next_package_substrings[7].at(1))*PI/180;
   double next_roll=stringfunc::string_to_number(
      next_package_substrings[8].at(1))*PI/180;
   cout << "next_az = " << next_az*180/PI << endl;
   cout << "next_el = " << next_el*180/PI << endl;
   cout << "next_roll = " << next_roll*180/PI << endl;

// Compute relative rotation between current and next images:

   rotation Rcurr,Rnext,Rrel;
   Rcurr=Rcurr.rotation_from_az_el_roll(curr_az,curr_el,curr_roll);
   Rnext=Rnext.rotation_from_az_el_roll(next_az,next_el,next_roll);
   Rrel=Rnext*Rcurr.transpose();

   double rel_az,rel_el,rel_roll;
   Rrel.az_el_roll_from_rotation(rel_az,rel_el,rel_roll);
   cout << "Bundle adjusted rel_az = " << rel_az*180/PI << endl;
   cout << "Bundle adjusted rel_el = " << rel_el*180/PI << endl;
   cout << "Bundle adjusted rel_roll = " << rel_roll*180/PI << endl;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> feature_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,tiepoints_subdir);

// Read current and next photographs and their hardware-derived
// metadata:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
   }

   camera* curr_camera_ptr=camera_ptrs.at(0);
   camera* next_camera_ptr=camera_ptrs.at(1);
//   camera* curr_camera_ptr=camera_ptrs.at(1);
//   camera* next_camera_ptr=camera_ptrs.at(0);

   threevector curr_world_posn=curr_camera_ptr->get_world_posn();
   threevector next_world_posn=next_camera_ptr->get_world_posn();
   threevector rel_world_posn=next_world_posn-curr_world_posn;
   cout << "rel_world_posn = " << rel_world_posn << endl;
   
   genmatrix* curr_K_ptr=curr_camera_ptr->get_K_ptr();
   genmatrix* next_K_ptr=next_camera_ptr->get_K_ptr();

// We now specialize to working with video frames from an aerial
// camera whose spin we assume is relatively mild.  In particular, we
// guestimate that the relative elevation and roll angles between and
// two video frames is close to zero.  On the other hand, we
// guestimate an initial delta_azimuth angle between two video frames
// based upon a typical camera depression angle, average altitude and
// known air vehicle displacement between the two frames:

   double avg_altitude=0.5*(curr_world_posn.get(2)+next_world_posn.get(2));
   cout << "Avg altitude = " << avg_altitude << endl;
   double depression_angle=15*PI/180;	// typical depression angle for GEO
   double range=avg_altitude/sin(depression_angle);
   cout << "range = " << range << endl;

   threevector s=range*x_hat-rel_world_posn;
   threevector s_hat=s.unitvector();
   double rel_az_guess=acos(s_hat.dot(x_hat));
   int sgn_rel_az=sgn(z_hat.dot(rel_world_posn.cross(x_hat)));
   rel_az_guess *= sgn_rel_az;
   double rel_el_guess=0;
   double rel_roll_guess=0;

   cout << "rel_az_guess = " << rel_az_guess*180/PI << endl;
   cout << "rel_el_guess = " << rel_el_guess*180/PI << endl;
   cout << "rel_roll_guess = " << rel_roll_guess*180/PI << endl;

   curr_camera_ptr->set_f(curr_f);
   next_camera_ptr->set_f(next_f);
   curr_camera_ptr->set_u0(curr_U0);
   next_camera_ptr->set_u0(next_U0);

//   cout << "curr_world_posn = " << curr_world_posn << endl;
//   cout << "next_world_posn = " << next_world_posn << endl;
//   cout << "curr_camera = " << *curr_camera_ptr << endl;
//   cout << "next_camera = " << *next_camera_ptr << endl;

// Reset orientation of current camera to canonical pose pointing
// along +xhat axis and its world position to the origin:

   curr_camera_ptr->set_Rcamera(0,-depression_angle,0);
   curr_world_posn=Zero_vector;
   next_world_posn=rel_world_posn;
   curr_camera_ptr->set_world_posn(curr_world_posn);
   next_camera_ptr->set_world_posn(next_world_posn);

   curr_camera_ptr->construct_seven_param_projection_matrix();
   next_camera_ptr->construct_seven_param_projection_matrix();
   genmatrix* curr_P_ptr=curr_camera_ptr->get_P_ptr();
   genmatrix* next_P_ptr=next_camera_ptr->get_P_ptr();

//   threevector curr_pointing_dir=curr_camera_ptr->get_pointing_dir();
//   cout << "curr_pointing_dir = "
//        << curr_pointing_dir << endl;
      
// Import 2D tiepoint feature image-plane coordinates for image #1.
// Backproject 2D feature coordinates into 3D world-space via
// *curr_P_ptr:

   filefunc::ReadInfile(feature_filenames.front());

   vector<twovector> currUV;
   vector<threevector> curr_rhats;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      currUV.push_back(twovector(u,v));
      curr_rhats.push_back(curr_camera_ptr->pixel_ray_direction(u,v));
   }
   int n_tiepoints=curr_rhats.size();

// Import 2D tiepoint feature image-plane coordinates for image #2:

   filefunc::ReadInfile(feature_filenames.back());

   vector<twovector> nextUV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      nextUV.push_back(twovector(u,v));
   }

/*
// Compute homography:

   homography* H_ptr=new homography();

   H_ptr->parse_homography_inputs(currUV,nextUV);
   H_ptr->compute_homography_matrix();
   H_ptr->check_homography_matrix(currUV,nextUV);
   cout << "curr_UV.size() = " << currUV.size() << endl;

   threevector camera_posn;
   H_ptr->compute_extrinsic_params(curr_f,curr_U0,curr_V0,Rrel,camera_posn);

   double homography_rel_az,homography_rel_el,homography_rel_roll;
   Rrel.az_el_roll_from_rotation(
      homography_rel_az,homography_rel_el,homography_rel_roll);
   cout << "homography_rel_az = " << homography_rel_az*180/PI
        << " homography_rel_el = " << homography_rel_el*180/PI
        << " homography_rel_roll = " << homography_rel_roll*180/PI << endl;
   delete H_ptr;
   outputfunc::enter_continue_char();
*/

/*
// Compute fundamental matrix:

   fundamental* fundamental_ptr=new fundamental();
   fundamental_ptr->parse_fundamental_inputs(currUV,nextUV);
   fundamental_ptr->compute_fundamental_matrix();
   bool print_flag=true;
   fundamental_ptr->check_fundamental_matrix(currUV,nextUV,print_flag);

   double max_scalar_product=0.01;
   vector<twovector> currUV_reduced,nextUV_reduced;
   for (int i=0; i<currUV.size(); i++)
   {
      if (fabs(fundamental_ptr->scalar_product(currUV[i],nextUV[i])) < 
      max_scalar_product)
      {
         currUV_reduced.push_back(currUV[i]);
         nextUV_reduced.push_back(nextUV[i]);
      }
   }
   
   fundamental_ptr->parse_fundamental_inputs(currUV_reduced,nextUV_reduced);
   fundamental_ptr->compute_fundamental_matrix();
   fundamental_ptr->check_fundamental_matrix(
      currUV_reduced,nextUV_reduced,print_flag);
   
   genmatrix* E_ptr=fundamental_ptr->generate_essential_matrix(
      curr_K_ptr,next_K_ptr);

   vector<genmatrix*> projection_matrix_ptrs;
   fundamental_ptr->compute_four_relative_projection_matrix_candidates(
      projection_matrix_ptrs);

   delete fundamental_ptr;

   outputfunc::enter_continue_char();
*/

// Perform brute-force search over 3 relative camera angle parameters:

   double min_score=POSITIVEINFINITY;
  
   double az_start=rel_az_guess-5*PI/180;
   double az_stop=rel_az_guess+5*PI/180;
   if (sgn_rel_az > 0)
   {
      az_start=basic_math::max(0.0,az_start);
      az_stop=basic_math::max(0.0,az_stop);
   }
   else
   {
      az_start=basic_math::min(0.0,az_start);
      az_stop=basic_math::min(0.0,az_stop);
   }
   cout << "rel_az_guess = " << rel_az_guess*180/PI << endl;
   cout << "az_start = " << az_start*180/PI
        << " az_stop = " << az_stop*180/PI << endl;
   param_range az(az_start,az_stop,21);
//   outputfunc::enter_continue_char();

   double el_start=rel_el_guess-10*PI/180;
   double el_stop=rel_el_guess+10*PI/180;
   param_range el(el_start,el_stop,11);

   double roll_start=rel_roll_guess-5*PI/180;
   double roll_stop=rel_roll_guess+5*PI/180;
   param_range roll(roll_start,roll_stop,9);

//   int n_iters=3;
   int n_iters=5;
//   int n_iters=7;
//   int n_iters=8;
//   int n_iters=10;
//   int n_iters=15;

   double best_az,best_el,best_roll,best_phi,best_theta;
   rotation R;
   vector<double> feature_scores;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "=========================================================="
           << endl;
      cout << "Iteration = " << iter << " of " << n_iters << endl;

      while (az.prepare_next_value())
      {
         double curr_az=az.get_value();
//         cout << "curr_az = " << curr_az*180/PI << endl;
         while (el.prepare_next_value())
         {
            double curr_el=el.get_value();
            if (curr_el < -PI/2 || curr_el > PI/2) continue;
            while (roll.prepare_next_value())
            {
               double curr_roll=roll.get_value();
               next_camera_ptr->set_Rcamera(curr_az,curr_el,curr_roll);

//                  cout << "Input az = " << curr_az*180/PI
//                       << " input el = " << curr_el*180/PI
//                       << " input roll = " << curr_roll*180/PI
//                       << endl;

               next_camera_ptr->construct_seven_param_projection_matrix();

               genmatrix* next_P_ptr=next_camera_ptr->get_P_ptr();
                     
// Given projection matrices curr_P and next_P, backproject 2D
// tiepoint UV coordinates into 3D world space.  Compute 3D
// intersection points for all tiepoint pairs.  Calculate median
// distance of backprojected rays to intersection points.  Set score
// equal to sum over all median distances.

               feature_scores.clear();
               for (int f=0; f<n_tiepoints; f++)
               {
                  threevector curr_rhat=curr_rhats[f];
                  threevector next_rhat=
                     next_camera_ptr->pixel_ray_direction(
                        nextUV[f].get(0),nextUV[f].get(1));
                  double dotprod=curr_rhat.dot(next_rhat);
                  threevector term=rel_world_posn/(1-dotprod*dotprod);

// Compute points along curr and next rays which lie closest to each
// other:

                  threevector curr_p=curr_world_posn+
                     term.dot(curr_rhat-dotprod*next_rhat)*curr_rhat;
                  threevector next_p=next_world_posn-
                     term.dot(next_rhat-dotprod*curr_rhat)*next_rhat;

//                  cout << "(curr_p-curr_world_posn).mag = "
//                       << (curr_p-curr_world_posn).magnitude() << endl;

//                  threevector delta_p=next_p-curr_p;
//                  cout << "delta_p.curr_rhat = " << delta_p.dot(curr_rhat)
//                       << endl;
//                  cout << "delta_p.next_rhat = " << delta_p.dot(next_rhat)
//                       << endl;
                  
                  feature_scores.push_back((curr_p-next_p).magnitude());
               }

// Try to minimize outlier impact by taking score to equal median of
// impact parameters computed for all tiepoint pairs:

               double curr_score=mathfunc::median_value(feature_scores);
                   
               if (curr_score < min_score)
               {
                  min_score=curr_score;
                  az.set_best_value();
                  el.set_best_value();
                  roll.set_best_value();
                  cout << "min_score = " << min_score << endl;
               }
            } // roll while loop
         } // el while loop
      } // az while loop

      double frac=0.35;
//      double frac=0.55;
//      double frac=0.75;
      az.shrink_search_interval(az.get_best_value(),frac);
      el.shrink_search_interval(el.get_best_value(),frac);
      roll.shrink_search_interval(roll.get_best_value(),frac);

      best_az=az.get_best_value();
      best_el=el.get_best_value();
      best_roll=roll.get_best_value();

      cout << endl;
      cout << "min_score = " << min_score << endl;
      cout << "best_az = " << best_az*180/PI << endl;
      cout << "best_el = " << best_el*180/PI << endl;
      cout << "best_roll = " << best_roll*180/PI << endl;

   } // loop over iter index

   cout << endl;
   cout << "Bundle adjusted rel_az = " << rel_az*180/PI << endl;
   cout << "Bundle adjusted rel_el = " << rel_el*180/PI << endl;
   cout << "Bundle adjusted rel_roll = " << rel_roll*180/PI << endl;

   cout << "rel_az_guess = " << rel_az_guess*180/PI << endl;
   cout << "rel_el_guess = " << rel_el_guess*180/PI << endl;
   cout << "rel_roll_guess = " << rel_roll_guess*180/PI << endl;

   delete photogroup_ptr;

   cout << "At end of program RELATIVE_POSE" << endl;
   outputfunc::print_elapsed_time();
}
