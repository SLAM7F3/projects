// ==========================================================================
// PROGRAM RECOLOR_TDP is a little specialized utility which we wrote
// in order to brighten the reconstructed point cloud from imagery
// gathered over Texas Tech by Lighthawk camera #2.  It uses histogram
// specification in order to brighten the black & white point cloud's
// intensity distribution.

//				recolor_tdp

// ==========================================================================
// Last updated on 2/27/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "math/adv_mathfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/lighthawk/";
   string input_tdp_filename="pmvs_options.txt.10.ladar.tdp";
   string input_full_filename=bundler_IO_subdir+input_tdp_filename;
   vector<double> X,Y,Z;
   vector<int> R,G,B;
   tdpfunc::read_XYZRGB_points_from_tdpfile(
      input_full_filename,X,Y,Z,R,G,B);

   cout << "X.size() = " << X.size() << endl;
   cout << "Y.size() = " << Y.size() << endl;
   cout << "Z.size() = " << Z.size() << endl;
   cout << "R.size() = " << R.size() << endl;
   cout << "G.size() = " << G.size() << endl;
   cout << "B.size() = " << B.size() << endl;

// Generate final, desired gaussian intensity distribution:

   const int nbins=200;
   double mu=0.5;
   double sigma=0.2;
   prob_distribution p_gaussian=advmath::generate_gaussian_density(
      nbins,mu,sigma);

// Load probabilities greater than input threshold into STL vector,
// and then compute their cumulative distribution:
   
   double intensity_threshold=-1;
//   cout << "Enter threshold below which intensities will not be equalized:"
//        << endl;
//   cin >> intensity_threshold;

   vector<double> intensity;
   for (int i=0; i<R.size(); i++)
   {
      intensity.push_back(R[i]/255.0);
   }
   prob_distribution prob(intensity,nbins);

// To perform "intensity histogram equalization", we reset each
// normalized intensity value 0 <= x <= 1 to Pcum(x).  To perform
// "intensity histogram specification" onto the desired gaussian
// distribution, we perform an inverse histogram equalization and map
// Pcum(x) onto the y value for which Pcum(x) = Pgaussian(y):

//   double intensity_magnification=2.0;
//   cout << "Enter intensity magnifcation factor:" << endl;
//   cin >> intensity_magnification;

   for (int i=0; i<intensity.size(); i++)
   {
      double old_p=intensity[i];
      double new_p=old_p;
      if (old_p > intensity_threshold)
      {
//         new_p=intensity_magnification*old_p;
//         new_p=basic_math::min(new_p,1.0);

         int n=prob.get_bin_number(old_p);
         double pcum=prob.get_pcum(n);
//         new_p=pcum;		// Histogram equalization
         new_p=p_gaussian.find_x_corresponding_to_pcum(pcum);

      }

      if (i%1000==0)
         cout << "i = " << i 
              << " old_p = " << old_p
              << " new_p = " << new_p << endl;

      intensity[i]=new_p;
      R[i]=255*intensity[i];
      G[i]=255*intensity[i];
      B[i]=255*intensity[i];
   } // loop over index i labeling points in cloud


   string UTMzone="";
   string output_tdp_filename="recolored_"+input_tdp_filename;
   string output_full_filename=bundler_IO_subdir+output_tdp_filename;
   tdpfunc::write_relative_xyzrgba_data(
      UTMzone,output_full_filename,X,Y,Z,R,G,B);

   string banner="Wrote out recolored TDP file "+output_tdp_filename;
   outputfunc::write_banner(banner);
}
