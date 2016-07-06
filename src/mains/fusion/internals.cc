// ==========================================================================
// Program INTERNALS is a smaller version of program WANDER.  It is
// meant to compute internal camera parameters using manually
// extracted 2D and 3D tiepoints for just a single video image.
// Internal parameter information is written to output file
// camera_params_imagenumber.txt in subdirectory
// ./manual_camera_params/.
// ==========================================================================
// Last updated on 12/13/05; 9/15/06; 9/26/07; 12/4/10
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "video/camera.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "numerical/param_range.h"
#include "general/stringfuncs.h"
#include "math/fourvector.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "math/twovector.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();

// First read in and parse camera position & attitude information:

   string TPA_filtered_filename="TPA_filtered.txt";
   filefunc::ReadInfile(TPA_filtered_filename);
   int n_fields=8;
   double X[n_fields];

   vector<threevector> rel_camera_XYZ;
   vector<threevector> camera_rpy;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      rel_camera_XYZ.push_back(threevector(X[2],X[3],X[4]));
      camera_rpy.push_back(threevector(X[5],X[6],X[7]));
   }

   string output_camera_params_subdir="./manual_camera_params/";
   filefunc::dircreate(output_camera_params_subdir);

// For speed purposes, instantiate repeatedly used variables outside
// largest imagenumber loop:

   int starting_imagenumber;
   cout << "Enter starting imagenumber:" << endl;
   cin >> starting_imagenumber;
   int stopping_imagenumber=starting_imagenumber+1;
   int delta_imagenumber=1;
   
   double U,V;
   camera curr_camera;

   double f_lo=2.75;
   double f_hi=2.95;
//   double f_lo=2.8;
//   double f_hi=3.0;
   int n_fbins=5;
//   int n_fbins=8;

   double alpha_lo=-2*PI/180;
   double alpha_hi=2*PI/180;
   int n_alphabins=4;
//   int n_alphabins=5;

   double beta_lo=-3*PI/180;
   double beta_hi=3*PI/180;
//   double beta_lo=-2*PI/180;
//   double beta_hi=2*PI/180;
   int n_betabins=4;
//   int n_betabins=5;

//   double kappa2_lo=-0.05;
//   double kappa2_hi=0.05;
//   double kappa2_lo=-0.1;
//   double kappa2_hi=0.1;
   double kappa2_lo=-0.2;
   double kappa2_hi=0.2;
//   int n_kappabins=3;
   int n_kappabins=4;
//   int n_kappabins=5;
   
   double u0_lo=0.65;
   double u0_hi=1.1;
//   double u0_lo=0.92;
//   double u0_hi=0.99;
   int n_u0bins=7;

   double v0_lo=0.2;
   double v0_hi=0.6;
//   double v0_lo=0.23;
//   double v0_hi=0.41;
//   int n_v0bins=9;
   int n_v0bins=8;

   double phi_lo=122*PI/180;
   double phi_hi=125*PI/180;
//   double phi_lo=122*PI/180;
//   double phi_hi=125*PI/180;
   int n_phibins=6;
//   int n_phibins=8;

   for (int imagenumber=starting_imagenumber; imagenumber  !=
           stopping_imagenumber; imagenumber += delta_imagenumber)
   {
      string banner="Processing image number "+stringfunc::number_to_string(
         imagenumber);
      outputfunc::write_big_banner(banner);
//   int imagenumber;
//   cout << "Enter imagenumber to analyze:" << endl;
//   cin >> imagenumber;

// Next read in and parse XYZUV tie-point data:

      string tiepoint_dir="tiepoint_data/";
      string xyzuv_filename=tiepoint_dir+
         "XYZUV_"+stringfunc::integer_to_string(imagenumber,4)+".txt";
      cout << "xyzuv_filename = " << xyzuv_filename << endl;
      
      filefunc::ReadInfile(xyzuv_filename);

      vector<int> ID;
      vector<threevector> XYZ;
      vector<twovector> UV;
      vector<double> weight;
      double inverse_weight_sum=0;

      int n_fields=stringfunc::compute_nfields(filefunc::text_line[0]);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         stringfunc::string_to_n_numbers(
            n_fields,filefunc::text_line[i],X);
         ID.push_back(static_cast<int>(X[0]));
         XYZ.push_back(threevector(X[1],X[2],X[3]));
         UV.push_back(twovector(X[4],X[5]));
         if (n_fields >= 7)
         {
            weight.push_back(X[6]);
            cout << "ID = " << ID.back() << " X = " << X[1] 
                 << " Y = " << X[2] 
                 << " Z = " << X[3] << " U = " << X[4] << " V = " << X[5] 
                 << " wght = " << weight.back() << endl;

         }
         else
         {
            weight.push_back(1.0);
            cout << "ID = " << ID.back() << " X = " << X[1] 
                 << " Y = " << X[2] 
                 << " Z = " << X[3] << " U = " << X[4] << " V = " << X[5] 
                 << endl;
         }
         inverse_weight_sum += weight.back();
      } // loop over index i 
//         cout << "imagenumber = " << imagenumber
//              << " XYZ.size = " << XYZ.size() << endl;
      unsigned int n_tiepoints=XYZ.size();
      inverse_weight_sum = 1.0/inverse_weight_sum;

// Set up intrinsic parameter search intervals:

      param_range fu(f_lo , f_hi , n_fbins);
      cout << "n_fubins = " << fu.get_nbins() << endl;

      param_range alpha(alpha_lo , alpha_hi , n_alphabins);
      cout << "n_alphabins = " << alpha.get_nbins() << endl;

      param_range beta(beta_lo , beta_hi , n_betabins);
      cout << "n_betabins = " << beta.get_nbins() << endl;

      param_range kappa2(kappa2_lo,kappa2_hi,n_kappabins);
      cout << "n_kappa2bins = " << kappa2.get_nbins() << endl;

      param_range u0(u0_lo , u0_hi , n_u0bins);
      cout << "n_u0bins = " << u0.get_nbins() << endl;

      param_range v0(v0_lo , v0_hi , n_v0bins);
      cout << "n_v0bins = " << v0.get_nbins() << endl;

      param_range phi(phi_lo , phi_hi , n_phibins);
      cout << "n_phibins = " << phi.get_nbins() << endl;

// Work with extrinsic camera parameters obtained from regularized and
// filtered GPS/IMU measurements.  Recall first image in
// HAFB_overlap_corrected_grey.vid actually corresponds to imagenumber
// 300 in uncut HAFB.vid file:

      double orig_X=rel_camera_XYZ[imagenumber+300].get(0);
      double orig_Y=rel_camera_XYZ[imagenumber+300].get(1);
      double orig_Z=rel_camera_XYZ[imagenumber+300].get(2);
      double orig_roll=(camera_rpy[imagenumber+300]).get(0);
      double orig_pitch=(camera_rpy[imagenumber+300]).get(1);
      double orig_yaw=(camera_rpy[imagenumber+300]).get(2);

      cout << "orig_X = " << orig_X << " orig_Y = " << orig_Y << endl;

//      double X_lo=orig_X-1;
//      double X_hi=orig_X+1;
//      int n_Xbins=3;

//      double Y_lo=orig_Y-1;
//      double Y_hi=orig_Y+1;
//      int n_Ybins=3;

      double roll_lo=(orig_roll-0.75)*PI/180;
      double roll_hi=(orig_roll+0.75)*PI/180;
//         double roll_lo=(orig_roll-0.5)*PI/180;
//         double roll_hi=(orig_roll+0.5)*PI/180;
      int n_rollbins=7;

      double pitch_lo=(orig_pitch-0.35)*PI/180;
      double pitch_hi=(orig_pitch+0.35)*PI/180;
//         double pitch_lo=(orig_pitch-0.25)*PI/180;
//         double pitch_hi=(orig_pitch+0.25)*PI/180;
      int n_pitchbins=7;

      double yaw_lo=(orig_yaw-0.35)*PI/180;
      double yaw_hi=(orig_yaw+0.35)*PI/180;
//         double yaw_lo=(orig_yaw-0.25)*PI/180;
//         double yaw_hi=(orig_yaw+0.25)*PI/180;
      int n_yawbins=7;

      param_range roll(roll_lo , roll_hi , n_rollbins);
      cout << "n_rollbins = " << roll.get_nbins() << endl;
      param_range pitch(pitch_lo , pitch_hi , n_pitchbins);
      cout << "n_pitchbins = " << pitch.get_nbins() << endl;
      param_range yaw(yaw_lo , yaw_hi , n_yawbins);
      cout << "n_yawbins = " << yaw.get_nbins() << endl;

//         param_range X(X_lo , X_hi , n_Xbins);
//         cout << "n_Xbins = " << X.get_nbins() << endl;
//         param_range Y(Y_lo , Y_hi , n_Ybins);
//         cout << "n_Ybins = " << Y.get_nbins() << endl;

      int product1=fu.get_nbins()*
         alpha.get_nbins()*beta.get_nbins()*phi.get_nbins();
      int product2=u0.get_nbins()*v0.get_nbins()*kappa2.get_nbins();
      int product3=roll.get_nbins()*pitch.get_nbins()*yaw.get_nbins();
//         int product4=X.get_nbins()*Y.get_nbins();
      int product=product1*product2*product3;
      cout << "Number of chisq function evaluations to perform = "
           << product << endl;
   
      double min_chisq=POSITIVEINFINITY;
      double minimal_max_sqr_diff=POSITIVEINFINITY;
      double RMS_chisq;
//         const int n_iters=1;
      const int n_iters=8;
      for (int iter=0; iter<n_iters; iter++)
      {
         outputfunc::newline();
         cout << "Iteration = " << iter << endl;

         cout << "f search values = " << fu.get_start() << " to "
              << fu.get_stop() << endl;
         cout << "u0 search values = " << u0.get_start() << " to "
              << u0.get_stop() << endl;
         cout << "v0 search values = " << v0.get_start() << " to "
              << v0.get_stop() << endl;
         cout << "kappa2 search values = " << kappa2.get_start() << " to " 
              << kappa2.get_stop() << endl;
         cout << "alpha search values = " << alpha.get_start()*180/PI 
              << " to " << alpha.get_stop()*180/PI << endl;
         cout << "beta search values = " << beta.get_start()*180/PI 
              << " to " << beta.get_stop()*180/PI << endl;
         cout << "phi search values = " << phi.get_start()*180/PI << " to "
              << phi.get_stop()*180/PI << endl;
            
         cout << "roll search values = " << roll.get_start()*180/PI 
              << " to " << roll.get_stop()*180/PI << endl;
         cout << "pitch search values = " << pitch.get_start()*180/PI 
              << " to " << pitch.get_stop()*180/PI << endl;
         cout << "yaw search values = " << yaw.get_start()*180/PI 
              << " to " << yaw.get_stop()*180/PI << endl << endl;

//            cout << "X search values = " << X.get_start()
//                 << " to " << X.get_stop() << endl << endl;
//            cout << "Y search values = " << Y.get_start()
//                 << " to " << Y.get_stop() << endl << endl;
         curr_camera.set_world_posn(threevector(orig_X,orig_Y,orig_Z));

// =======================================================================
// Nested loops over entire parameter space starts here:
// =======================================================================

         while (roll.prepare_next_value())
         {
//               cout << roll.get_counter() << " " << flush;
            while (pitch.prepare_next_value())
            {
               cout << (roll.get_counter()-1)*pitch.get_nbins()
                  +pitch.get_counter() << " " << flush;
               while (yaw.prepare_next_value())
               {
                  curr_camera.set_aircraft_rotation_angles(
                     pitch.get_value(),roll.get_value(),
                     yaw.get_value());
               
//                     while (X.prepare_next_value())
//                     {
//                        while (Y.prepare_next_value())
//                        {
                           
                  while (alpha.prepare_next_value())
                  {
                     while (beta.prepare_next_value())
                     {
                        while (phi.prepare_next_value())
                        {
                           curr_camera.set_mount_rotation_angles(
                              alpha.get_value(),beta.get_value(),
                              phi.get_value());
                           curr_camera.compute_rotation_matrix();

                           while (fu.prepare_next_value())
                           {
                              while (kappa2.prepare_next_value())
                              {
                                 while (u0.prepare_next_value())
                                 {
                                    while (v0.prepare_next_value())
                                    {

                                       curr_camera.
                                          set_internal_params(
                                             fu.get_value(),
                                             fu.get_value(),
                                             u0.get_value(),
                                             v0.get_value(),PI/2.0,
                                             kappa2.get_value());

                                       double chisq=0;
                                       double max_sqr_diff=0;
                                       for (unsigned int i=0; 
                                            i<n_tiepoints; i++)
                                       {
                                          curr_camera.
                                             project_world_to_image_coords_with_radial_correction(
                                                XYZ[i].get(0),
                                                XYZ[i].get(1),
                                                XYZ[i].get(2),U,V);
                                          double sqr_diff = 
                                             sqr(U-UV[i].get(0))
                                             +sqr(V-UV[i].get(1));
                                          chisq += sqr_diff;
//                                          chisq += weight[i]*sqr_diff;
                                          max_sqr_diff=basic_math::max(
                                             max_sqr_diff,sqr_diff);

                                       } // loop over XYZ values 
//                                       chisq *= inverse_weight_sum;

//                                       if (chisq < min_chisq)
                                       if (max_sqr_diff < 
                                           minimal_max_sqr_diff)
                                       {
                                          minimal_max_sqr_diff=max_sqr_diff;
                                          min_chisq=chisq;
                                          fu.set_best_value();
                                          u0.set_best_value();
                                          v0.set_best_value();
                                          kappa2.set_best_value();
                                          alpha.set_best_value();
                                          beta.set_best_value();
                                          phi.set_best_value();
                                          roll.set_best_value();
                                          pitch.set_best_value();
                                          yaw.set_best_value();
//                                                   X.set_best_value();
//                                                   Y.set_best_value();
                                       }
                                    } // v0 while loop
                                 } // u0 while loop
                              } // kappa2 while loop
                           } // fu while loop
                        } // phi while loop
                     } // beta while loop
                  } // alpha while loop
//                        } // Y while loop
//                     } // X while loop
               } // yaw while loop
            } // pitch while loop
         } // roll while loop
               
         outputfunc::newline();
         cout << "best f = " << fu.get_best_value() << endl;
         cout << "best u0 = " << u0.get_best_value() << endl;
         cout << "best v0 = " << v0.get_best_value() << endl;
         cout << "best kappa2 = " << kappa2.get_best_value() << endl;
         cout << "best alpha = " << alpha.get_best_value()*180/PI 
              << endl;
         cout << "best beta = " << beta.get_best_value()*180/PI << endl;
         cout << "best phi = " << phi.get_best_value()*180/PI << endl;

         cout << "best roll = " << roll.get_best_value()*180/PI 
              << " original roll = " << orig_roll << endl;
         cout << "best pitch = " << pitch.get_best_value()*180/PI 
              << " original pitch = " << orig_pitch << endl;
         cout << "best yaw = " << yaw.get_best_value()*180/PI 
              << " original yaw = " << orig_yaw << endl;

/*
  cout << "best X = " << X.get_best_value() 
  << " original X = " << orig_X << endl;
  cout << "best Y = " << Y.get_best_value() 
  << " original Y = " << orig_Y << endl;
*/

         RMS_chisq=sqrt(min_chisq/XYZ.size());
         cout << "min_chisq = " << min_chisq << endl;
         cout << "sqrt(min_chisq) = " << sqrt(min_chisq) << endl;
         cout << "# XYZ-UV tiepoints = " << XYZ.size() << endl;
         cout << "RMS chisq = " << RMS_chisq << endl;
         cout << "sqrt(minimal_max_sqr_diff) = " 
              << sqrt(minimal_max_sqr_diff) << endl;
         outputfunc::newline();

         double frac=0.7;
//            double frac=0.5;
//            double frac=0.25;
         fu.shrink_search_interval(fu.get_best_value(),frac);
         alpha.shrink_search_interval(alpha.get_best_value(),frac);
         beta.shrink_search_interval(beta.get_best_value(),frac);
         phi.shrink_search_interval(phi.get_best_value(),frac);
         u0.shrink_search_interval(u0.get_best_value(),frac);
         v0.shrink_search_interval(v0.get_best_value(),frac);
         kappa2.shrink_search_interval(kappa2.get_best_value(),frac);

         roll.shrink_search_interval(roll.get_best_value(),frac);
         pitch.shrink_search_interval(pitch.get_best_value(),frac);
         yaw.shrink_search_interval(yaw.get_best_value(),frac);

//            X.shrink_search_interval(X.get_best_value(),frac);
//            Y.shrink_search_interval(Y.get_best_value(),frac);

      } // loop over iter index

// Write best parameter values to text output file:

      string camera_params_filename=output_camera_params_subdir+
         "camera_params_"+stringfunc::integer_to_string(imagenumber,4)
         +".txt";
      filefunc::deletefile(camera_params_filename);
      ofstream camera_stream;
      filefunc::openfile(camera_params_filename,camera_stream);
      camera_stream << "# Img	f     u0	v0	k2      alpha  beta      phi  roll pitch yaw chisq_RMS" << endl;
      camera_stream << endl;
      camera_stream << imagenumber << "  "
                    << fu.get_best_value() << "  "
                    << fu.get_best_value() << "  "
                    << u0.get_best_value() << "  "
                    << v0.get_best_value() << "  "
                    << kappa2.get_best_value() << "  "
                    << alpha.get_best_value() << "  "
                    << beta.get_best_value() << "  "
                    << phi.get_best_value() << "  "
                    << roll.get_best_value()*180/PI << "  "
                    << pitch.get_best_value()*180/PI << "  "
                    << yaw.get_best_value()*180/PI << "  "
                    << RMS_chisq << endl;
      filefunc::closefile(camera_params_filename,camera_stream);

// Reset camera's intrinsic and extrinsic parameters to their optimal
// values:

//         curr_camera.set_world_posn(threevector(orig_X,orig_Y,orig_Z));
      curr_camera.set_aircraft_rotation_angles(
         pitch.get_best_value(),roll.get_best_value(),
         yaw.get_best_value());

      curr_camera.set_mount_rotation_angles(
         alpha.get_best_value(),beta.get_best_value(),
         phi.get_best_value());
      curr_camera.compute_rotation_matrix();
      curr_camera.set_internal_params(
         fu.get_best_value(),fu.get_best_value(),
         u0.get_best_value(),v0.get_best_value(),PI/2.0,
         kappa2.get_best_value());

// Explicitly compare input and output UV values sorted according to
// increasing discrepancy:

      vector<double> sqr_diff;
      vector<twovector> UV_proj;
      for (unsigned int i=0; i<XYZ.size(); i++)
      {
         curr_camera.project_world_to_image_coords_with_radial_correction(
            XYZ[i].get(0),XYZ[i].get(1),XYZ[i].get(2),U,V);
         UV_proj.push_back(twovector(U,V));
         sqr_diff.push_back(sqr(U-UV[i].get(0))+sqr(V-UV[i].get(1)));
      }

      templatefunc::Quicksort(sqr_diff,ID,XYZ,UV,UV_proj);

      cout.precision(4);
      for (unsigned int i=0; i<XYZ.size(); i++)
      {
         cout << "i = " << i << " ID = " << ID[i] 
              << " sqrt(sqr_diff) = " << sqrt(sqr_diff[i]) << endl;
         cout << "    XYZ = " << XYZ[i].get(0) << " " << XYZ[i].get(1)
              << " " << XYZ[i].get(2) 
              << " UV = " << UV[i].get(0)
              << " " << UV[i].get(1) << " proj UV = " 
              << UV_proj[i].get(0) << " " << UV_proj[i].get(1) << endl;
      } // loop over XYZ values 

// Center parameter space search for subsequent image about current
// image's best parameter space point.  To enforce crude continuity,
// significantly restrict parameter space search volume for 2nd, 3rd,
// etc images compared to 1st:

      f_lo=fu.get_best_value()-0.016;
      f_hi=fu.get_best_value()+0.016;
      n_fbins=5;

      alpha_lo=alpha.get_best_value()-0.08*PI/180;
      alpha_hi=alpha.get_best_value()+0.08*PI/180;
      n_alphabins=4;

      beta_lo=beta.get_best_value()-0.08*PI/180;
      beta_hi=beta.get_best_value()+0.08*PI/180;
      n_betabins=4;

      kappa2_lo=kappa2.get_best_value()-0.008;
      kappa2_hi=kappa2.get_best_value()+0.008;
      n_kappabins=4;
   
      u0_lo=u0.get_best_value()-0.004;
      u0_hi=u0.get_best_value()+0.004;
      n_u0bins=5;

      v0_lo=v0.get_best_value()-0.004;
      v0_hi=v0.get_best_value()+0.004;
      n_v0bins=6;

      phi_lo=phi.get_best_value()-0.08*PI/180;
      phi_hi=phi.get_best_value()+0.08*PI/180;
      n_phibins=5;

/*
  f_lo=fu.get_best_value()-0.02;
  f_hi=fu.get_best_value()+0.02;
  n_fbins=4;

  alpha_lo=alpha.get_best_value()-0.1*PI/180;
  alpha_hi=alpha.get_best_value()+0.1*PI/180;
  n_alphabins=3;

  beta_lo=beta.get_best_value()-0.1*PI/180;
  beta_hi=beta.get_best_value()+0.1*PI/180;
  n_betabins=3;

  kappa2_lo=kappa2.get_best_value()-0.01;
  kappa2_hi=kappa2.get_best_value()+0.01;
  n_kappabins=3;
   
  u0_lo=u0.get_best_value()-0.005;
  u0_hi=u0.get_best_value()+0.005;
  n_u0bins=4;

  v0_lo=v0.get_best_value()-0.005;
  v0_hi=v0.get_best_value()+0.005;
  n_v0bins=5;

  phi_lo=phi.get_best_value()-0.1*PI/180;
  phi_hi=phi.get_best_value()+0.1*PI/180;
  n_phibins=4;
*/
      
   } // loop over imagenumber

   string banner="Elapsed time = "+
      stringfunc::number_to_string(timefunc::elapsed_timeofday_time()/60.0)
      +" minutes";
   outputfunc::write_big_banner(banner);
}
   

   
   
