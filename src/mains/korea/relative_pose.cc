// ========================================================================
// Program RELATIVE_POSE imports manually selected tiepoint image
// plane coordinates for 2 images.  We also hardwire the images' focal
// parameters derived via program F_FROM_VANISHING_POINTS.  The first
// camera is assumed to have a canonical world position and pose.  The
// second camera is assumed to reside at a conventional unit distance
// from the first.  But we search over two angles phi and theta which
// describe the second camera's position relative to the first.  We
// also search of the second camera's relative az, el and roll
// compared to the first.  A score function consisting of the median
// distance between closest points of approach for corresponding
// backprojected rays is evaluated to find the best 5-parameter
// solution for the second camera's relative position and pose.
// ========================================================================
// Last updated on 9/19/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
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
   PassesGroup passes_group(&arguments);
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string features_subdir=bundler_IO_subdir+"features/";
   string features1_filename=features_subdir+"features_2D_ground_photo1.txt";
   string features2_filename=features_subdir+"features_2D_ground_photo2.txt";

// Read photographs from input video passes:

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
   
// Hardwire focal parameters for ground_photo1.jpg and ground_photo2.jpg

   curr_camera_ptr->set_f(-3.46866);
   next_camera_ptr->set_f(-5.04454);
   
// WLOG, set Rcamera and world_posn for *curr_camera_ptr at canonical
// values:

   curr_camera_ptr->set_Rcamera(0,0,0);
   curr_camera_ptr->set_world_posn(Zero_vector);
   curr_camera_ptr->construct_seven_param_projection_matrix();
   cout << "curr_camera = " << *curr_camera_ptr << endl;
      
// Also initialize Rcamera and world_posn for *next_camera_ptr at
// canonical values:

   next_camera_ptr->set_Rcamera(0,0,0);
   next_camera_ptr->set_world_posn(Zero_vector);
   next_camera_ptr->construct_seven_param_projection_matrix();
   cout << "next_camera = " << *next_camera_ptr << endl;

// Import 2D tiepoint feature image-plane coordinates for image #1.
// Backproject 2D feature coordinates into 3D world-space via
// *curr_P_ptr:

   filefunc::ReadInfile(features1_filename);

   vector<threevector> curr_rhats;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      curr_rhats.push_back(curr_camera_ptr->pixel_ray_direction(u,v));
   }
   int n_tiepoints=curr_rhats.size();

// Import 2D tiepoint feature image-plane coordinates for image #2:

   filefunc::ReadInfile(features2_filename);

   vector<double> nextU,nextV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      nextU.push_back(u);
      nextV.push_back(v);
   }

// Perform brute-force search over 5 relative camera pose and position
// parameters:

   double min_score=POSITIVEINFINITY;
      
//   double az_start=0*PI/180;
   double az_start=300*PI/180;
   double az_stop=360*PI/180;
   param_range az(az_start,az_stop,12);

//   double el_start=-80*PI/180;
   double el_start=-60*PI/180;
   double el_stop=-10*PI/180;
//   double el_stop=80*PI/180;
   param_range el(el_start,el_stop,10);

//   double roll_start=-60*PI/180;
//   double roll_stop=60*PI/180;
   double roll_start=-10*PI/180;
   double roll_stop=10*PI/180;
   param_range roll(roll_start,roll_stop,8);

//   double phi_start=0*PI/180;
   double phi_start=45*PI/180;
   double phi_stop=135*PI/180;
//   double phi_stop=360*PI/180;
   param_range phi(phi_start,phi_stop,16);

//   double theta_start=-80*PI/180;
//   double theta_start=0*PI/180;
   double theta_start=40*PI/180;
   double theta_stop=80*PI/180;
   param_range theta(theta_start,theta_stop,14);

//   int n_iters=3;
//   int n_iters=5;
//   int n_iters=7;
//   int n_iters=8;
//   int n_iters=15;
//   int n_iters=20;
   int n_iters=25;

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
         cout << "curr_az = " << curr_az*180/PI << endl;
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

               while (phi.prepare_next_value())
               {
                  double curr_phi=phi.get_value();
                  while (theta.prepare_next_value())
                  {
                     double curr_theta=theta.get_value();
                     if (curr_theta < -PI/2 || curr_theta > PI/2) continue;

                     double cos_curr_theta=cos(curr_theta);
                     double curr_X=cos_curr_theta*cos(curr_phi);
                     double curr_Y=cos_curr_theta*sin(curr_phi);
                     double curr_Z=sin(curr_theta);
                     threevector next_world_posn(curr_X,curr_Y,curr_Z);
                     next_camera_ptr->set_world_posn(next_world_posn);
//                     cout << "Next world posn = " << next_world_posn << endl;
                           
                     next_camera_ptr->
                        construct_seven_param_projection_matrix();
//                     genmatrix* next_P_ptr=next_camera_ptr->get_P_ptr();
                     
// Given projection matrices curr_P and next_P, backproject 2D
// tiepoint UV coordinates into 3D world space.  Compute 3D
// intersection points for all tiepoint pairs.  Calculate median
// distance of backprojected rays to intersection points.  Set score
// equal to sum over all median distances.

//                     double curr_score=0;
                     feature_scores.clear();
                     for (int f=0; f<n_tiepoints; f++)
                     {
                        threevector curr_rhat=curr_rhats[f];
                        threevector next_rhat=
                           next_camera_ptr->pixel_ray_direction(
                              nextU[f],nextV[f]);
                        double dotprod=curr_rhat.dot(next_rhat);
                        threevector term=next_world_posn/(1-dotprod*dotprod);

// Compute points along curr and next rays which lie closest to each
// other:

                        threevector curr_p=
                           term.dot(curr_rhat-dotprod*next_rhat)*curr_rhat;
                        threevector next_p=next_world_posn-
                           term.dot(next_rhat-dotprod*curr_rhat)*next_rhat;
                        feature_scores.push_back((curr_p-next_p).magnitude());
//                        curr_score += (curr_p-next_p).magnitude();
                     }
                     double curr_score=mathfunc::median_value(
                        feature_scores);
                   
                     if (curr_score < min_score)
                     {
                        min_score=curr_score;
                        az.set_best_value();
                        el.set_best_value();
                        roll.set_best_value();
                        phi.set_best_value();
                        theta.set_best_value();
                        cout << "min_score = " << min_score << endl;
                     }

                  } // theta while loop
               } // phi while loop
            } // roll while loop
         } // el while loop
      } // az while loop

//      double frac=0.55;
//      double frac=0.66;
      double frac=0.71;
      az.shrink_search_interval(az.get_best_value(),frac);
      el.shrink_search_interval(el.get_best_value(),frac);
      roll.shrink_search_interval(roll.get_best_value(),frac);
      phi.shrink_search_interval(phi.get_best_value(),frac);
      theta.shrink_search_interval(theta.get_best_value(),frac);

      best_az=az.get_best_value();
      best_el=el.get_best_value();
      best_roll=roll.get_best_value();
      best_phi=phi.get_best_value();
      best_theta=theta.get_best_value();

      cout << endl;
      cout << "min_score = " << min_score << endl;
      cout << "best_az = " << best_az*180/PI << endl;
      cout << "best_el = " << best_el*180/PI << endl;
      cout << "best_roll = " << best_roll*180/PI << endl;
      cout << "best_phi = " << best_phi*180/PI << endl;
      cout << "best_theta = " << best_theta*180/PI << endl;

   } // loop over iter index

   delete photogroup_ptr;

   cout << "At end of program RELATIVE_POSE" << endl;
   outputfunc::print_elapsed_time();
}

/*

Cumulative error scoring results

min_score = 0.0347275674091
best_az = 304.895535841
best_el = -67.3545079872
best_roll = -23.2148742071
best_phi = 33.4459732904
best_theta = 57.464179546

min_score = 0.0190300795466
best_az = 338.368447422
best_el = -32.7952638883
best_roll = 2.31899879033
best_phi = 108.278683607
best_theta = 58.1575340703

min_score = 0.0171571868729
best_az = 330.71570229
best_el = -45.2444715896
best_roll = -1.39830090342
best_phi = 78.4578337336
best_theta = 63.091333005

min_score = 0.0146135547661
best_az = 333.184088038
best_el = -41.6444787769
best_roll = 0.178468840935
best_phi = 87.5577954572
best_theta = 62.4768275412

min_score = 0.0118793249461
best_az = 336.241150049
best_el = -37.6345222964
best_roll = 1.92873292071
best_phi = 98.0738291978
best_theta = 61.5270254135

*/

/*

// Median error scoring results:

min_score = 0.000652879767279
best_az = 333.448363636
best_el = -43.2411111111
best_roll = -1.324
best_phi = 78.6852
best_theta = 63.5877692308

min_score = 0.000204262423256
best_az = 332.121309186
best_el = -38.4606973327
best_roll = -1.38210993637
best_phi = 94.5153852321
best_theta = 58.8392826326
Elapsed time = 546.5 secs =   9.11 minutes =   0.152 hours 

*/
