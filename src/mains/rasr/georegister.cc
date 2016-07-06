// ==========================================================================
// Program GEOREGISTER reads in a set of building exterior features
// manually selected from some georegistered aerial EO image and a
// corresponding set of interior features selected from a G76 ladar
// map.  This program computes the absolute scale, translation and
// rotation needed to georegister the latter sparse point cloud onto
// the aerial image.
// ==========================================================================
// Last updated on 2/3/10; 2/4/10
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

   string interior_features_filename="features2.dat";
   string exterior_features_filename="features1.dat";
//   string interior_features_filename="interior_features.dat";
//   string exterior_features_filename="exterior_features.dat";

// Initialize 7 global params to trivial starting values:

   double fitted_exterior_to_interior_distance_ratio=1;
   threevector fitted_interior_trans(0,0,0);
   double global_az = 0;
   double global_el = 0;
   double global_roll = 0;
   threevector rotation_origin(0,0,0);

// Read in and store features manually selected from interior G76
// ladar point cloud:

   filefunc::ReadInfile(interior_features_filename);
   int n_lines=filefunc::text_line.size();
//   cout << "n_lines = " << n_lines << endl;
   vector<threevector> orig_interior_pnts,interior_pnts;
   for (int i=0; i<n_lines; i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> interior_coords=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector interior_xyz(interior_coords[3],interior_coords[4]);

      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         interior_xyz,
         fitted_exterior_to_interior_distance_ratio,
         fitted_interior_trans,
         global_az,global_el,global_roll,
         rotation_origin);
      
      orig_interior_pnts.push_back(interior_xyz);
      interior_pnts.push_back(interior_xyz);
      cout << "i = " << i 
           << " interior_pnt = " << interior_pnts.back() << endl;
   } // loop over index i 

// Next read in and store counterpart features manually selected from
// exterior LL aerial photo:

   filefunc::ReadInfile(exterior_features_filename);
//   n_lines=filefunc::text_line.size();
//   cout << "n_lines = " << n_lines << endl;
   
   vector<threevector> exterior_pnts;
   for (int i=0; i<n_lines; i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> exterior_coords=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector exterior_xy(exterior_coords[3],exterior_coords[4]);
      exterior_pnts.push_back(exterior_xy);
      cout << "i = " << i 
           << " exterior_pnt = " << exterior_pnts.back() << endl;
   } // loop over index i 

// Compute center-of-mass of interior and exterior features:

   int n_features=exterior_pnts.size();
   threevector interior_feature_COM,exterior_feature_COM;
   for (int i=0; i<n_features; i++)
   {
      interior_feature_COM += interior_pnts[i];
      exterior_feature_COM += exterior_pnts[i];
   }
   interior_feature_COM /= n_features;
   exterior_feature_COM /= n_features;

   cout << "interior_feature_COM = " << interior_feature_COM << endl;
   cout << "exterior_feature_COM = " << exterior_feature_COM << endl;
   rotation_origin=interior_feature_COM;
   cout << "rotation_origin = " << rotation_origin << endl;

// Compute interior and exterior feature rays relative to their COMs:

   vector<threevector> interior_feature_rays,exterior_feature_rays;
   for (int i=0; i<n_features; i++)
   {
      interior_feature_rays.push_back(interior_pnts[i]-interior_feature_COM);
      exterior_feature_rays.push_back(exterior_pnts[i]-exterior_feature_COM);
   }

   rotation R;
   R.rotation_between_ray_bundles(
      exterior_feature_rays,interior_feature_rays);
//   cout << "R = " << R << endl;

   R.az_el_roll_from_rotation(global_az,global_el,global_roll);
   cout << "Global az = " << global_az*180/PI << endl;
   cout << "Global el = " << global_el*180/PI << endl;
   cout << "Global roll = " << global_roll*180/PI << endl << endl;

   fitted_interior_trans=exterior_feature_COM-interior_feature_COM;
   cout << "fitted_interior_trans = " << fitted_interior_trans << endl;

// Compute scale factor relating interior and exterior features:

   vector<double> exterior_to_interior_distance_ratio;
   for (int i=0; i<n_features; i++)
   {
      threevector curr_interior_feature=interior_pnts[i];
      threevector curr_exterior_feature=exterior_pnts[i];
      for (int j=i+1; j<n_features; j++)
      {
         threevector next_interior_feature=interior_pnts[j];
         threevector next_exterior_feature=exterior_pnts[j];
         
         double interior_distance=
            (next_interior_feature-curr_interior_feature).magnitude();
         double exterior_distance=
            (next_exterior_feature-curr_exterior_feature).magnitude();
         double curr_exterior_to_interior_distance_ratio=
            exterior_distance/interior_distance;
//         cout << "i = " << i << " j = " << j
//              <<  " curr_exterior_to_interior_distance_ratio = "
//              << curr_exterior_to_interior_distance_ratio << endl;
         exterior_to_interior_distance_ratio.push_back(
            curr_exterior_to_interior_distance_ratio);
      } // loop over index j labeling features
   } // loop over index i labeling features
   
   double mean_ratio=mathfunc::mean(exterior_to_interior_distance_ratio);
   double std_dev_ratio=mathfunc::std_dev(
      exterior_to_interior_distance_ratio);
   double median_ratio=
      mathfunc::median_value(exterior_to_interior_distance_ratio);
   cout << "median ratio = " << median_ratio << endl;
   cout << "exterior_to_interior_distance_ratio = " << mean_ratio << " +/- " 
        << std_dev_ratio << endl;

// For G76 ladar data and aerial LL photo, 
// median ratio = 1.01099999388265
// exterior_to_interior_distance_ratio = 1.01857030524674 +/- 0.0281481851621732

   fitted_exterior_to_interior_distance_ratio=mean_ratio;

/*
// Rescale all interior XY points:

   for (int i=0; i<interior_pnts.size(); i++)
   {
      threevector interior_xyz=orig_interior_pnts[i];
      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         interior_xyz,
         fitted_exterior_to_interior_distance_ratio,
         fitted_interior_trans,
         global_az,global_el,global_roll,
         rotation_origin);
      interior_pnts[i]=interior_xyz;
   } // loop over index i 
*/

   fitted_interior_trans=exterior_feature_COM-interior_feature_COM;
//   cout << "fitted_interior_trans = " << fitted_interior_trans << endl;

   cout << "--fitted_exterior_to_interior_distance_ratio " 
        << fitted_exterior_to_interior_distance_ratio << endl;
   cout << "--interior_translation_X " << fitted_interior_trans.get(0) << endl;
   cout << "--interior_translation_Y " << fitted_interior_trans.get(1) << endl;
   cout << "--interior_translation_Z " << fitted_interior_trans.get(2) << endl;
   cout << "--global_az " << global_az*180/PI << endl;
   cout << "--global_el " << global_el*180/PI << endl;
   cout << "--global_roll " << global_roll*180/PI << endl;
   cout << "--interior_rotation_origin_X " << rotation_origin.get(0) << endl;
   cout << "--interior_rotation_origin_Y " << rotation_origin.get(1) << endl;
   cout << "--interior_rotation_origin_Z " << rotation_origin.get(2) << endl;
   cout << endl;

// Transform all rescaled interior points:

   for (int i=0; i<interior_pnts.size(); i++)
   {
      threevector interior_xyz=orig_interior_pnts[i];
      bundlerfunc::scale_translate_rotate_bundler_XYZ(
         interior_xyz,
         fitted_exterior_to_interior_distance_ratio,
         fitted_interior_trans,
         global_az,global_el,global_roll,
         rotation_origin);
      interior_pnts[i]=interior_xyz;
      threevector delta=exterior_pnts[i]-interior_pnts[i];

      cout << "i = " << i << " interior_pnt = " << interior_pnts[i]
           << " exterior_pnt = " << exterior_pnts[i] << endl;
      cout << "delta = " << delta << endl;
      cout << endl;
      
   } // loop over index i 


}

/*
--fitted_exterior_to_interior_distance_ratio 1.01857030524674
--interior_translation_X 313746.411261324
--interior_translation_Y 4703319.43927706
--interior_translation_Z 0
--global_az 167.015831190556
--global_el -0
--global_roll 0
--interior_rotation_origin_X 313690.8705342
--interior_rotation_origin_Y 4703275.83418
--interior_rotation_origin_Z 0
*/
