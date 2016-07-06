// ==========================================================================
// Program WANDER iteratively performs a sequence of brute-force
// minimizations of a chisq function that measures the difference
// between input and output UV pairs for a particular video image.
// Interpolated and temporally filtered aircraft GPS/IMU information
// is read in from ascii file "TPA_filtered.txt".  Input UV members of
// 3D/2D tie-point pairs are next read in from XYZUV_imagenumber.txt
// files stored within the tiepoint subdirectory.  Output UV values
// are calculated by projecting XYZ members of 3D/2D tie-point pairs
// onto the image plane.  As of 8/30/05, we equate lengths f=fu=fv and
// ignore any skew within CCD pixels.  But we attempt to compensate
// for some radial distortion.

// We initiate the search for the camera's internal parameters within
// reasonable ranges that we determined empirically.  The machine
// minimizes chisq over a coarse grid defined in the initial search
// space.  It subsequently generates a finer grid centered upon the
// previous iteration's optimal location in the parameter space and
// searches again.  This iterative approach to function minimization
// over a multi-dimensional space appears to yield RMS chisq values
// that look reasonable for at least the HAFB death pass.

// Inevitable errors in GPS/IMU readings and filtering as well as
// 3D/2D tie-point selection make it impossible to find a set of truly
// constant internal camera parameters.  In principle, we need to
// perturb the tie point information, the extrinsic and intrinsic
// parameters in such a way as to optimize a chisq function.  But we
// are willing to settle for adjusting only the internal parameters by
// small amounts for different images.  The best fit values for image
// n are taken as seeds for the parameter space search for image n+1.
// Moreover, the volume of the parameter search space is taken to be
// several orders of magnitude smaller for the second, third, fourth,
// etc images compared to the first.  This restricts the path through
// the intrinsic parameter space to not jerk wildly from image to
// image.  

// The best fit intrinsic parameter values are written out to text
// file "camera_params_imagenumber.txt".

// ==========================================================================
// Last updated on 10/4/05; 9/15/06; 9/26/07
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

   string output_camera_params_subdir="./KLT_camera_params/";
   filefunc::dircreate(output_camera_params_subdir);

// For speed purposes, instantiate repeatedly used variables outside
// of largest imagenumber loop:

   int starting_imagenumber,stopping_imagenumber;
   cout << "Enter starting imagenumber:" << endl;
   cin >> starting_imagenumber;
   cout << "Enter stopping imagenumber:" << endl;
   cin >> stopping_imagenumber;
   int delta_imagenumber=1;
   if (stopping_imagenumber < starting_imagenumber)
   {
      delta_imagenumber=-1;
   }
   
   double U,V;
   camera curr_camera;

   double f_lo=2.8;
   double f_hi=3.0;
   int n_fbins=8;

   double alpha_lo=-2*PI/180;
   double alpha_hi=2*PI/180;
   int n_alphabins=5;

   double beta_lo=-2*PI/180;
   double beta_hi=2*PI/180;
   int n_betabins=5;

   double kappa2_lo=-0.1;
   double kappa2_hi=0.1;
   int n_kappabins=5;
   
   double u0_lo=0.92;
   double u0_hi=0.99;
   int n_u0bins=8;

   double v0_lo=0.23;
   double v0_hi=0.41;
   int n_v0bins=10;

   double phi_lo=122*PI/180;
   double phi_hi=125*PI/180;
   int n_phibins=8;

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

      string tiepoint_dir="KLT_tiepoint_data/";
      string xyzuv_filename=tiepoint_dir+
         "XYZUV_"+stringfunc::integer_to_string(imagenumber,4)+".txt";
      cout << "xyzuv_filename = " << xyzuv_filename << endl;
      
      if (filefunc::ReadInfile(xyzuv_filename))
      {
         n_fields=6;
         vector<int> ID;
         vector<threevector> XYZ;
         vector<twovector> UV;
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            stringfunc::string_to_n_numbers(
               n_fields,filefunc::text_line[i],X);
            ID.push_back(static_cast<int>(X[0]));
            XYZ.push_back(threevector(X[1],X[2],X[3]));
            UV.push_back(twovector(X[4],X[5]));
//            cout << "ID = " << ID.back() << " X = " << X[0] 
//                 << " Y = " << X[1] 
//                 << " Z = " << X[2] << " U = " << X[3] << " V = " << X[4] 
//                 << endl;
         }
//         cout << "imagenumber = " << imagenumber
//              << " XYZ.size = " << XYZ.size() << endl;

//   char distortion_char;
//   cout << "Include radial distortion in analysis (y/n)?" << endl;
//   cin >> distortion_char;
//      bool include_radial_distortion_flag=true;
//   if (distortion_char=='n' || distortion_char=='N') 
//      include_radial_distortion_flag=false;

// Set up parameter search intervals:

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

         int product1=fu.get_nbins()*
            alpha.get_nbins()*beta.get_nbins()*phi.get_nbins();
         int product2=u0.get_nbins()*v0.get_nbins()*kappa2.get_nbins();
         int product=product1*product2;
         cout << "Number of chisq function evaluations to perform = "
              << product << endl;

// Work with extrinsic camera parameters obtained from regularized and
// filtered GPS/IMU measurements.  Recall first image in
// HAFB_overlap_corrected_grey.vid actually corresponds to imagenumber
// 300 in uncut HAFB.vid file:

         curr_camera.set_world_posn(rel_camera_XYZ[imagenumber+300]);
         double roll=(camera_rpy[imagenumber+300]).get(0)*PI/180;
         double pitch=(camera_rpy[imagenumber+300]).get(1)*PI/180;
         double yaw=(camera_rpy[imagenumber+300]).get(2)*PI/180;
         curr_camera.set_aircraft_rotation_angles(pitch,roll,yaw);
   
         double min_chisq=POSITIVEINFINITY;
         double RMS_chisq;
//   const int n_iters=1;
         const int n_iters=5;
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
                 << phi.get_stop()*180/PI << endl << endl;

            while (alpha.prepare_next_value())
            {
               cout << alpha.get_counter() << " " << flush;
               while (beta.prepare_next_value())
               {
                  while (phi.prepare_next_value())
                  {
                     curr_camera.set_mount_rotation_angles(
                        alpha.get_value(),beta.get_value(),phi.get_value());
                     curr_camera.compute_rotation_matrix();

                     while (fu.prepare_next_value())
                     {
                        while (kappa2.prepare_next_value())
                        {
                           while (u0.prepare_next_value())
                           {
                              while (v0.prepare_next_value())
                              {

                                 curr_camera.set_internal_params(
                                    fu.get_value(),fu.get_value(),
                                    u0.get_value(),v0.get_value(),PI/2.0,
                                    kappa2.get_value());

                                 double chisq=0;
                                 for (unsigned int i=0; i<XYZ.size(); i++)
                                 {
                                    curr_camera.
                                       project_world_to_image_coords_with_radial_correction(
                                          XYZ[i].get(0),XYZ[i].get(1),
                                          XYZ[i].get(2),U,V);
                                    chisq += sqr(U-UV[i].get(0))
                                       +sqr(V-UV[i].get(1));
                                 } // loop over XYZ values 

                                 if (chisq < min_chisq)
                                 {
                                    min_chisq=chisq;
                                    fu.set_best_value();
                                    alpha.set_best_value();
                                    beta.set_best_value();
                                    phi.set_best_value();
                                    u0.set_best_value();
                                    v0.set_best_value();
                                    kappa2.set_best_value();
                                 }

                              } // v0 while loop
                           } // u0 while loop
                        } // kappa2 while loop
                     } // fu while loop
                  } // phi while loop
               } // beta while looop
            } // alpha while loop

            outputfunc::newline();
            cout << "best f = " << fu.get_best_value() << endl;
            cout << "best u0 = " << u0.get_best_value() << endl;
            cout << "best v0 = " << v0.get_best_value() << endl;
            cout << "best kappa2 = " << kappa2.get_best_value() << endl;
            cout << "best alpha = " << alpha.get_best_value()*180/PI << endl;
            cout << "best beta = " << beta.get_best_value()*180/PI << endl;
            cout << "best phi = " << phi.get_best_value()*180/PI << endl;

            RMS_chisq=sqrt(min_chisq/XYZ.size());
            cout << "min_chisq = " << min_chisq << endl;
            cout << "sqrt(min_chisq) = " << sqrt(min_chisq) << endl;
            cout << "# XYZ-UV tiepoints = " << XYZ.size() << endl;
            cout << "RMS chisq = " << RMS_chisq << endl;
            outputfunc::newline();

            double frac=0.25;
            fu.shrink_search_interval(fu.get_best_value(),frac);
            alpha.shrink_search_interval(alpha.get_best_value(),frac);
            beta.shrink_search_interval(beta.get_best_value(),frac);
            phi.shrink_search_interval(phi.get_best_value(),frac);
            u0.shrink_search_interval(u0.get_best_value(),frac);
            v0.shrink_search_interval(v0.get_best_value(),frac);
            kappa2.shrink_search_interval(kappa2.get_best_value(),frac);
         } // loop over iter index

// Write best parameter values to text output file:

         string camera_params_filename=output_camera_params_subdir+
            "camera_params_"+stringfunc::integer_to_string(imagenumber,4)
            +".txt";
         filefunc::deletefile(camera_params_filename);
         ofstream camera_stream;
         filefunc::openfile(camera_params_filename,camera_stream);
//      camera_stream << "# Img	f     u0	v0	k2      alpha  beta      phi  chisq_RMS" << endl;
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
                       << RMS_chisq << endl;
         filefunc::closefile(camera_params_filename,camera_stream);

// Reset camera's intrinsic and extrinsic parameters to their optimal
// values:

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
                 << " sqr_diff = " << sqr_diff[i] << endl;
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
      
      } // filefunc::ReadInfile(xyzuv_filename) conditional
   } // loop over imagenumber
}
