// ==========================================================================
// Program CROP_L1 crops level-1 ALIRT Puerto Rico imagery.
// ==========================================================================
// Last updated on 11/3/11; 11/9/11; 2/28/13
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/linesegment.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string tdp_filename="raw.tdp";
   vector<double> X,Y,Z;
   tdpfunc::read_XYZ_points_from_tdpfile(
      tdp_filename,X,Y,Z);
   
   cout << "X.size() = " << X.size() << endl;

   const int istep=1000;
   int n_points=X.size();
   vector<double> Xreduced,Yreduced,Zreduced;
   for (int i=0; i<n_points; i += istep)
   {
      Xreduced.push_back(X[i]);
      Yreduced.push_back(Y[i]);
      Zreduced.push_back(Z[i]);
   } // loop over index i 

   mypolynomial XZ_poly(1),YZ_poly(1);
   
   double chisq_XZ,chisq_YZ;
   XZ_poly.fit_coeffs_using_residuals(Zreduced,Xreduced,chisq_XZ);
   YZ_poly.fit_coeffs_using_residuals(Zreduced,Yreduced,chisq_YZ);
   
   cout << "XZ_poly = " << XZ_poly << endl;
   cout << "YZ_poly = " << YZ_poly << endl;
   threevector origin(XZ_poly.get_coeff(0),YZ_poly.get_coeff(1),0);
   threevector n_hat(XZ_poly.get_coeff(1),YZ_poly.get_coeff(1),1);
   n_hat=n_hat.unitvector();
   cout << "n_hat = " << n_hat << endl;

   outputfunc::enter_continue_char();

   templatefunc::Quicksort(Z,X,Y);
   double Zmin=Z.front();
   double Zmax=Z.back();
   cout << "Zmin = " << Zmin << " Zmax = " << Zmax << endl;

   threevector l_start(XZ_poly.value(Zmin),YZ_poly.value(Zmin),Zmin);
   threevector l_stop(XZ_poly.value(Zmax),YZ_poly.value(Zmax),Zmax);
   linesegment l(l_start,l_stop);

// Project all raw points onto fitted line:

   string banner="Projecting raw points onto fitted line:";
   outputfunc::write_big_banner(banner);
   
   vector<double> rho,n;
   cout << "Number of raw points = " << Z.size() << endl;
   for (unsigned int i=0; i<Z.size(); i++)
   {
      if (i%1000000==0) cout << double(i)/Z.size() << " " << flush;
      threevector currpoint(X[i],Y[i],Z[i]);
      rho.push_back(l.point_to_line_distance(currpoint));

      threevector curr_r=currpoint-origin;
      n.push_back(curr_r.dot(n_hat));
   }
   cout << endl;

   int n_output_bins=10000;
   prob_distribution prob_n(n,n_output_bins);
   prob_n.writeprobdists(false);

   int n_max_bin=-1;
   double peak_density=prob_n.peak_density_value(n_max_bin);
   double n_peak=prob_n.get_x(n_max_bin);
   cout << "n_peak = " << n_peak << " peak_density = " << peak_density << endl;

// Crop away points which do not lie within +/- delta_n meters of n_peak:

   const double delta_n=100;	// meters
   vector<threevector> cropped_points;
   for (unsigned int i=0; i<Z.size(); i++)
   {
      if (i%1000000==0) cout << double(i)/Z.size() << " " << flush;
      threevector currpoint(X[i],Y[i],Z[i]);
      threevector curr_r=currpoint-origin;
      double curr_n=curr_r.dot(n_hat);
      if (curr_n < n_peak-delta_n || curr_n > n_peak+delta_n) continue;
      cropped_points.push_back(currpoint);
   }
   cout << endl;

/*
   banner="Generating cropped TDP file";
   outputfunc::write_big_banner(banner);

   string cropped_tdp_filename="cropped_points.tdp";
   tdpfunc::write_relative_xyz_data(cropped_tdp_filename,cropped_points);
   string unix_cmd="lodtree "+cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);
*/

// Project all cropped and noise points onto fitted line:

   banner="Projecting cropped and noise points onto fitted line:";
   outputfunc::write_big_banner(banner);
   
   cout << "Number of cropped points = " << cropped_points.size() << endl;
   vector<double> n_cropped,n_noise;
   for (unsigned int i=0; i<cropped_points.size(); i++)
   {
      threevector curr_r=cropped_points[i]-origin;
      n_cropped.push_back(curr_r.dot(n_hat));
   }

   n_output_bins=1000;
   prob_distribution prob_cropped_n(n_cropped,n_output_bins);
   prob_cropped_n.writeprobdists(false);

   n_max_bin=-1;
   peak_density=prob_cropped_n.peak_density_value(n_max_bin);
   n_peak=prob_cropped_n.get_x(n_max_bin);
   cout << "n_peak = " << n_peak << " peak_density = " << peak_density << endl;

// Search for characteristic prob densities for noise above and below ground:

   vector<double> pnoise_below_ground,pnoise_above_ground;
   for (int i=1; i<51; i++)
   {
      pnoise_below_ground.push_back(prob_cropped_n.get_p(i));
   }
   for (int i=n_output_bins-2; i>n_output_bins-52; i--)
   {
      pnoise_above_ground.push_back(prob_cropped_n.get_p(i));
   }
   double pnoise_below_ground_mu=mathfunc::mean(pnoise_below_ground);
   double pnoise_below_ground_sigma=mathfunc::std_dev(pnoise_below_ground);

   double pnoise_above_ground_mu=mathfunc::mean(pnoise_above_ground);
   double pnoise_above_ground_sigma=mathfunc::std_dev(pnoise_above_ground);
   
   cout << "pnoise_below_ground = " << pnoise_below_ground_mu << " +/- "
        << pnoise_below_ground_sigma << endl;
   cout << "pnoise_above_ground = " << pnoise_above_ground_mu << " +/- "
        << pnoise_above_ground_sigma << endl;
   
   double n_min;
   for (int i=0; i<n_max_bin; i++)
   {
      double curr_p=prob_cropped_n.get_p(i);
      if (curr_p > pnoise_below_ground_mu + 3*pnoise_below_ground_sigma)
      {
         n_min=prob_cropped_n.get_x(i);
         break;
      }
   }
   cout << "n_min = " << n_min << endl;   
   n_min -= 10;	// meters

   double n_max = 0;
   for (int i=n_output_bins-1; i>n_max_bin; i--)
   {
      double curr_p=prob_cropped_n.get_p(i);
      if (curr_p > pnoise_above_ground_mu + 3*pnoise_above_ground_sigma)
      {
         n_max=prob_cropped_n.get_x(i);
         break;
      }
   }
   cout << "n_max = " << n_max << endl;

// Crop away points which do not lie within n interval [n_min,n_max] :

   vector<threevector> more_cropped_points;
   for (unsigned int i=0; i<cropped_points.size(); i++)
   {
      threevector curr_r=cropped_points[i]-origin;
      double curr_n=curr_r.dot(n_hat);
      if (curr_n < n_min || curr_n > n_max) continue;
      more_cropped_points.push_back(cropped_points[i]);
   }
   cout << endl;
   cout << "more_cropped_points.size() = " << more_cropped_points.size()
        << endl;

// Write out more cropped points to TDP file:

   banner="Generating more cropped points TDP file";
   outputfunc::write_big_banner(banner);

   string more_cropped_tdp_filename="more_cropped_points.tdp";
   tdpfunc::write_relative_xyz_data(
      more_cropped_tdp_filename,more_cropped_points);
   string unix_cmd="lodtree "+more_cropped_tdp_filename;
   sysfunc::unix_command(unix_cmd);

   exit(-1);

   n_output_bins=100;
   prob_distribution prob_rho(rho,n_output_bins,0);
   cout << "rho = " << prob_rho.median() << " +/- "
        << prob_rho.quartile_width() << endl;

//   rotation R;
//   R=R.rotation_taking_u_to_v(n_hat,z_hat);

// Generate TDP & OSGA files displaying fitted line:

   Zmin -= 200;
   Zmax += 200;
   double z=Zmin;
   double dz=1;
   vector<threevector> line_XYZ;
   while (z < Zmax)
   {
      double x=XZ_poly.value(z);
      double y=YZ_poly.value(z);
      line_XYZ.push_back(threevector(x,y,z));
      z += dz;
   }
   
   string line_tdp_filename="fitted_line.tdp";
   tdpfunc::write_relative_xyz_data(line_tdp_filename,line_XYZ);

//   unix_cmd="lodtree "+line_tdp_filename;
//   sysfunc::unix_command(unix_cmd);

   

   
}
