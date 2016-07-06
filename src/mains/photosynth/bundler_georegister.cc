// ==========================================================================
// Program BUNDLER_GEOREGISTER reads in a set of features manually
// selected from Noah Snavely's raw bundler XYZ output and a
// counterpart feature set selected from some georegistered LADAR ladar
// map.  This program computes the absolute scale, translation and
// rotation needed to map the former sparse point cloud onto the
// ladar data.
// ==========================================================================
// Last updated on 1/20/10; 1/21/10; 1/22/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

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
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundle_filename=passes_group.get_bundle_filename();
   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

//   string bundler_features_filename=bundler_IO_subdir+"features_bundler.dat";
   string bundler_features_filename="./features_3D_bundler.txt";
//   string ladar_features_filename=
//      bundler_IO_subdir+"features_reduced_ladar.dat";
   string ladar_features_filename="./features_3D_GPS.txt";
//   string ladar_features_filename=bundler_IO_subdir+"features_ladar.dat";

// Initialize 7 global params to trivial starting values:

   double fitted_ladar_to_bundler_distance_ratio=1;
   threevector fitted_bundler_trans(0,0,0);
   double global_az = 0;
   double global_el = 0;
   double global_roll = 0;
   threevector rotation_origin(0,0,0);

// Read in and store features manually selected from bundler point
// cloud:

   filefunc::ReadInfile(bundler_features_filename);
   int n_lines=filefunc::text_line.size();
//   cout << "n_lines = " << n_lines << endl;
   vector<threevector> orig_bundler_pnts,bundler_pnts;
   for (int i=0; i<n_lines; i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> bundler_triple=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
//      cout << bundler_triple[3] << " "
//           << bundler_triple[4] << " "
//           << bundler_triple[5] << endl;
      threevector bundler_xyz(
         bundler_triple[3],bundler_triple[4],bundler_triple[5]);

      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         bundler_xyz,
         fitted_ladar_to_bundler_distance_ratio,
         fitted_bundler_trans,
         global_az,global_el,global_roll,
         rotation_origin);

      orig_bundler_pnts.push_back(bundler_xyz);
      bundler_pnts.push_back(bundler_xyz);
      cout << "i = " << i << " bundler_pnt = " << bundler_pnts.back() << endl;
   } // loop over index i 

// Next read in and store counterpart features manually selected from
// LADAR point cloud:

   filefunc::ReadInfile(ladar_features_filename);
//   n_lines=filefunc::text_line.size();
//   cout << "n_lines = " << n_lines << endl;
   
   vector<threevector> ladar_pnts;
   for (int i=0; i<n_lines; i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> ladar_triple=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
//      cout << ladar_triple[3] << " "
//           << ladar_triple[4] << " "
//           << ladar_triple[5] << endl;
      threevector ladar_xyz(
         ladar_triple[3],ladar_triple[4],ladar_triple[5]);
      ladar_pnts.push_back(ladar_xyz);
      cout << "i = " << i << " ladar_pnt = " << ladar_pnts.back() << endl;
   } // loop over index i 

// Compute scale factor relating bundler and LADAR point clouds:

   int n_features=ladar_pnts.size();
   vector<double> ladar_to_bundler_distance_ratio;
   for (int i=0; i<n_features; i++)
   {
      threevector curr_bundler_feature=bundler_pnts[i];
      threevector curr_ladar_feature=ladar_pnts[i];
      for (int j=i+1; j<n_features; j++)
      {
         threevector next_bundler_feature=bundler_pnts[j];
         threevector next_ladar_feature=ladar_pnts[j];
         
         double bundler_distance=(next_bundler_feature-curr_bundler_feature).
            magnitude();
         double ladar_distance=(next_ladar_feature-curr_ladar_feature).
            magnitude();
         double curr_ladar_to_bundler_distance_ratio=
            ladar_distance/bundler_distance;
//         cout << "i = " << i << " j = " << j
//              <<  " curr_ladar_to_bundler_distance_ratio = "
//              << curr_ladar_to_bundler_distance_ratio << endl;
         ladar_to_bundler_distance_ratio.push_back(
            curr_ladar_to_bundler_distance_ratio);
      } // loop over index j labeling features
   } // loop over index i labeling features
   
   double mean_ratio=mathfunc::mean(ladar_to_bundler_distance_ratio);
   double std_dev_ratio=mathfunc::std_dev(ladar_to_bundler_distance_ratio);
//   double median_ratio=
//      mathfunc::median_value(ladar_to_bundler_distance_ratio);
//   cout << "median ratio = " << median_ratio << endl;

   cout << "ladar_to_bundler_distance_ratio = " << mean_ratio << " +/- " 
        << std_dev_ratio << endl;

// For fused Boston/Cambridge ladar point cloud and MIT2317
// reconstructed bundler set:

// ladar_to_bundler_distance_ratio = 11.3511650263346 +/- 0.405143385563342
// median ratio = 11.3652017578834
   fitted_ladar_to_bundler_distance_ratio=mean_ratio;

// Rescale all BUNDLER XYZ points:

   for (unsigned int i=0; i<bundler_pnts.size(); i++)
   {
      threevector bundler_xyz=orig_bundler_pnts[i];
      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         bundler_xyz,
         fitted_ladar_to_bundler_distance_ratio,
         fitted_bundler_trans,
         global_az,global_el,global_roll,
         rotation_origin);
      bundler_pnts[i]=bundler_xyz;
   } // loop over index i 

// Compute center-of-mass of bundler and LADAR features:

   threevector bundler_feature_COM,ladar_feature_COM;
   for (int i=0; i<n_features; i++)
   {
      bundler_feature_COM += bundler_pnts[i];
      ladar_feature_COM += ladar_pnts[i];
   }
   bundler_feature_COM /= n_features;
   ladar_feature_COM /= n_features;

   cout << "bundler_feature_COM = " << bundler_feature_COM << endl;
   cout << "ladar_feature_COM = " << ladar_feature_COM << endl;
   rotation_origin=ladar_feature_COM;
   cout << "rotation_origin = " << rotation_origin << endl;
   fitted_bundler_trans=ladar_feature_COM-bundler_feature_COM;
   cout << "fitted_bundler_trans = " << fitted_bundler_trans << endl;

// Translate all rescaled BUNDLER XYZ points:

   for (unsigned int i=0; i<bundler_pnts.size(); i++)
   {
      threevector bundler_xyz=orig_bundler_pnts[i];
      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         bundler_xyz,
         fitted_ladar_to_bundler_distance_ratio,
         fitted_bundler_trans,
         global_az,global_el,global_roll,
         rotation_origin);
      bundler_pnts[i]=bundler_xyz;
   } // loop over index i 

// Compute bundler and ladar feature rays relative to their COMs:

   vector<threevector> bundler_feature_rays,ladar_feature_rays;
   for (int i=0; i<n_features; i++)
   {
      bundler_feature_rays.push_back(bundler_pnts[i]-bundler_feature_COM);
      ladar_feature_rays.push_back(ladar_pnts[i]-ladar_feature_COM);
   }

   rotation R;
   R.rotation_between_ray_bundles(ladar_feature_rays,bundler_feature_rays);
//   cout << "R = " << R << endl;

   R.az_el_roll_from_rotation(global_az,global_el,global_roll);
   cout << "Global az = " << global_az*180/PI << endl;
   cout << "Global el = " << global_el*180/PI << endl;
   cout << "Global roll = " << global_roll*180/PI << endl << endl;

   string peter_inputs_filename=bundler_IO_subdir+
      "packages/peter_inputs.pkg";
   cout << "======================================================" << endl;
   cout << "Replace trivial scaling, translation and rotation parameter values" << endl;
   cout << "within "+peter_inputs_filename << endl;
   cout << "with the following georegistered values:" << endl;
   cout << "======================================================" << endl
        << endl;

   cout << "--fitted_world_to_bundler_distance_ratio " 
        << fitted_ladar_to_bundler_distance_ratio << endl;
   cout << "--bundler_translation_X " << fitted_bundler_trans.get(0) << endl;
   cout << "--bundler_translation_Y " << fitted_bundler_trans.get(1) << endl;
   cout << "--bundler_translation_Z " << fitted_bundler_trans.get(2) << endl;
   cout << "--global_az " << global_az*180/PI << endl;
   cout << "--global_el " << global_el*180/PI << endl;
   cout << "--global_roll " << global_roll*180/PI << endl;
   cout << "--bundler_rotation_origin_X " << rotation_origin.get(0) << endl;
   cout << "--bundler_rotation_origin_Y " << rotation_origin.get(1) << endl;
   cout << "--bundler_rotation_origin_Z " << rotation_origin.get(2) << endl;

   cout << endl;
}

