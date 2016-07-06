// ==========================================================================
// Program BUNDLER_GEOREGISTER reads in a set of features manually
// selected from Noah Snavely's raw bundler XYZ output and a
// counterpart feature set selected from our georegistered NYC ladar
// map.  This program computes the absolute scale, translation and
// rotation needed to map the former sparse point cloud onto the
// latter.
// ==========================================================================
// Last updated on 3/15/09; 6/22/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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

//   string bundler_features_filename="fbundler.dat";
   string bundler_features_filename="fbundler_1012_3D.dat";
   string nyc_features_filename="fnyc.dat";

// First read in and store features manually selected from bundler
// point cloud:

   filefunc::ReadInfile(bundler_features_filename);
   int n_lines=filefunc::text_line.size();
   cout << "n_lines = " << n_lines << endl;
   
   vector<threevector> bundler_pnts;
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

      const double fitted_nyc_to_bundler_distance_ratio=16.5671099346348;
      bundler_xyz *= fitted_nyc_to_bundler_distance_ratio;

      const threevector fitted_bundler_trans(
         583887.626010023,
         4509462.20835707,
         507.348354146674);

      bundler_xyz += fitted_bundler_trans;

      threevector fitted_bundler_feature_COM(
         583299.091845454,
         4506309.89236364,
         101.864543137272);

      double global_az = 135.226534476547*PI/180;
      double global_el = 2.14044689264945*PI/180;
      double global_roll = -9.6634040587416*PI/180;

      rotation global_R;
      global_R=global_R.rotation_from_az_el_roll(
         global_az,global_el,global_roll);
//      cout << "global_R = " << global_R << endl;


      threevector rel_bundler_xyz=bundler_xyz-fitted_bundler_feature_COM;
      rel_bundler_xyz = global_R * rel_bundler_xyz;
      bundler_xyz=rel_bundler_xyz+fitted_bundler_feature_COM;


/*
//  nyc_to_bundler_distance_ratio = 0.46269106820359 +/- 0.0147353391779605
      const double fitted_nyc_to_bundler_distance_ratio=0.463381327643963;
      bundler_xyz *= fitted_nyc_to_bundler_distance_ratio;

      const threevector fitted_bundler_trans
         (583558.678495729 , 4508707.62273733 , 131.773459314555);

      bundler_xyz += fitted_bundler_trans;

      threevector fitted_bundler_feature_COM(
         583299.023754546 , 4506310.19790909 , 101.864543137272);

//      rotation global_R;
// -0.333312276770959	-0.942788485369844	-0.00726622376443221	
//  0.942772777961321	-0.333361135531646	0.00705991870669699	
// -0.00907828666945919	-0.00449724038573528	0.999948678453079	

      double global_az = 109.470775746297 * PI/180;
      double global_el = -0.520154656320816 * PI/180;
      double global_roll = -0.257684381003458 * PI/180;

      rotation global_R;
      global_R=global_R.rotation_from_az_el_roll(
         global_az,global_el,global_roll);
//      cout << "global_R = " << global_R << endl;

      threevector rel_bundler_xyz=bundler_xyz-fitted_bundler_feature_COM;
      rel_bundler_xyz = global_R * rel_bundler_xyz;
      bundler_xyz=rel_bundler_xyz+fitted_bundler_feature_COM;
*/

      bundler_pnts.push_back(bundler_xyz);
      cout << "i = " << i << " bundler_pnt = " << bundler_pnts.back() << endl;
   } // loop over index i 

// Next read in and store counterpart features manually selected from
// NYC point cloud:

   filefunc::ReadInfile(nyc_features_filename);
//   n_lines=filefunc::text_line.size();
   cout << "n_lines = " << n_lines << endl;
   
   vector<threevector> nyc_pnts;
   for (int i=0; i<n_lines; i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> nyc_triple=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
//      cout << nyc_triple[3] << " "
//           << nyc_triple[4] << " "
//           << nyc_triple[5] << endl;
      threevector nyc_xyz(
         nyc_triple[3],nyc_triple[4],nyc_triple[5]);
      nyc_pnts.push_back(nyc_xyz);
      cout << "i = " << i << " nyc_pnt = " << nyc_pnts.back() << endl;
   } // loop over index i 

// Compute scale factor relating bundler and NYC point clouds:

   int n_features=nyc_pnts.size();
   vector<double> nyc_to_bundler_distance_ratio;
   for (int i=0; i<n_features; i++)
   {
      threevector curr_bundler_feature=bundler_pnts[i];
      threevector curr_nyc_feature=nyc_pnts[i];
      for (int j=i+1; j<n_features; j++)
      {
         threevector next_bundler_feature=bundler_pnts[j];
         threevector next_nyc_feature=nyc_pnts[j];
         
         double bundler_distance=(next_bundler_feature-curr_bundler_feature).
            magnitude();
         double nyc_distance=(next_nyc_feature-curr_nyc_feature).
            magnitude();
         double curr_nyc_to_bundler_distance_ratio=
            nyc_distance/bundler_distance;
         cout << "i = " << i << " j = " << j
              <<  " curr_nyc_to_bundler_distance_ratio = "
              << curr_nyc_to_bundler_distance_ratio << endl;
         nyc_to_bundler_distance_ratio.push_back(
            curr_nyc_to_bundler_distance_ratio);
      } // loop over index j labeling features
   } // loop over index i labeling features
   
   double mean_ratio=mathfunc::mean(nyc_to_bundler_distance_ratio);
   double std_dev_ratio=mathfunc::std_dev(nyc_to_bundler_distance_ratio);
   double median_ratio=mathfunc::median_value(nyc_to_bundler_distance_ratio);

   cout << "nyc_to_bundler_distance_ratio = " << mean_ratio << " +/- " 
        << std_dev_ratio << endl;
   cout << "median ratio = " << median_ratio << endl;

//  nyc_to_bundler_distance_ratio = 0.46269106820359 +/- 0.0147353391779605
//  median ratio = 0.463381327643963

// Compute center-of-mass of bundler and NYC features:

   threevector bundler_feature_COM,nyc_feature_COM;
   for (int i=0; i<n_features; i++)
   {
      bundler_feature_COM += bundler_pnts[i];
      nyc_feature_COM += nyc_pnts[i];
   }
   bundler_feature_COM /= n_features;
   nyc_feature_COM /= n_features;

   cout << "bundler_feature_COM = " << bundler_feature_COM << endl;
   cout << "nyc_feature_COM = " << nyc_feature_COM << endl;
   
   threevector bundler_trans=nyc_feature_COM-bundler_feature_COM;

   cout << "bundler_trans = " << bundler_trans << endl;
//   threevector fitted_bundler_trans(
//      583558.678495729 , 4508707.62273733 , 131.773459314555);

   cout << "bundler_feature_COM = " << bundler_feature_COM << endl;
   cout << "nyc_feature_COM = " << nyc_feature_COM << endl;

// Compute bundler and nyc feature rays relative to their COMs:

   vector<threevector> bundler_feature_rays,nyc_feature_rays;
   for (int i=0; i<n_features; i++)
   {
      bundler_feature_rays.push_back(bundler_pnts[i]-bundler_feature_COM);
      nyc_feature_rays.push_back(nyc_pnts[i]-nyc_feature_COM);
   }

   rotation R;
   R.rotation_between_ray_bundles(nyc_feature_rays,bundler_feature_rays);
   double az,el,roll;
   R.az_el_roll_from_rotation(az,el,roll);
   
   cout << "R = " << R << endl;
   cout << "az = " << az*180/PI << endl;
   cout << "el = " << el*180/PI << endl;
   cout << "roll = " << roll*180/PI << endl;
   
}

