// ==========================================================================
// Program GROUNDPLANE reads in the TDP file containing thresholded
// XYZ points.  It generates a probability distribution for the Z
// values and computes their peak plus 5%, 10% and 20% cumumlative
// distribution values.  We wrote this little utility to help
// determine the Z value for surveyed tripod camera positions which
// yields a ground plane close to Z=0.  The tripod cameras' effective
// Z value output by this program becomes input to program
// CAMERA_POSNS.
// ==========================================================================
// Last updated on 12/28/11; 4/23/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "plot/metafile.h"
#include "math/mypolynomial.h"
#include "passes/PassesGroup.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);


// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string xyz_points_filename=bundler_IO_subdir+"thresholded_xyz_points.tdp";

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;

   tdpfunc::read_XYZ_points_from_tdpfile(
      xyz_points_filename,*X_ptr,*Y_ptr,*Z_ptr);

   int n_points=Z_ptr->size();
   cout << "Number of thresholded XYZ points = " << n_points << endl;

// Generate Z distribution:

   int n_output_bins=100;
   prob_distribution prob_X(*X_ptr,n_output_bins);
   prob_distribution prob_Y(*Y_ptr,n_output_bins);
   prob_distribution prob_Z(*Z_ptr,n_output_bins);

//   prob_X.writeprobdists(false);
   prob_Y.writeprobdists(false);
//   prob_Z.writeprobdists(false);

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;

   exit(-1);

   int n_peak_bin;
   cout << "Peak density value = " << prob_Z.peak_density_value(n_peak_bin)
        << endl;
   
   double z_peak=prob_Z.get_x(n_peak_bin);
   cout << "z_peak = " << z_peak << endl;

   double Z_05=prob_Z.find_x_corresponding_to_pcum(0.05);
   double Z_10=prob_Z.find_x_corresponding_to_pcum(0.1);
   double Z_20=prob_Z.find_x_corresponding_to_pcum(0.2);
   
//   cout << "Z_05 = " << Z_05 << endl;
   cout << "Z_10 = " << Z_10 << endl;
//   cout << "Z_20 = " << Z_20 << endl;

// Z values for cameras = 1.422 meters (approximate actual tripod height) +
// additional z offset adjusted so that 10% value for cumulative Z
// distribution for thresholded_xyz_points.tdp basically equals 0:

   double eff_z_camera=1.422-Z_10;
   
//   eff_z_camera=3.2;	// Expt 2B
//   eff_z_camera=2.1;	// Expt 5C

   string banner="Effective Z_camera = "+stringfunc::number_to_string(
      eff_z_camera);
   outputfunc::write_big_banner(banner);

   cout << "Enter this eff_Z_camera value in program CAMERA_POSNS..." << endl;
   cout << endl;
}
