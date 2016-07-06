// ==========================================================================
// Program GEOREG reads in a set of corresponding world and
// "raytraced" 3D points.  It then utilizes Horn's approach to compute
// the global rotation, translation and scaling which transforms the
// "raytraced" points into their georegistered counterparts.  GEOREG
// writes out these parameters which can be placed into
// peter_inputs.pkg in order to georegister bundler output.

//				     georeg

// ==========================================================================
// Last updated on 1/23/12; 1/24/12; 5/16/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

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

   string features_filename="features_3D_modeled_vs_raytraced.txt";
   filefunc::ReadInfile(features_filename);
   
   cout.precision(12);
   vector<threevector> modeled_XYZ,raytraced_XYZ;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector XYZ(column_values[0],column_values[1],column_values[2]);
      cout << "i = " << i << " XYZ = " << XYZ << endl;

      if (is_even(i))
      {
         modeled_XYZ.push_back(XYZ);
      }
      else
      {
         raytraced_XYZ.push_back(XYZ);
      }
      
   } // loop over index i 
   
   threevector modeled_COM=Zero_vector;
   threevector raytraced_COM=Zero_vector;
   
   int n_features=modeled_XYZ.size();
   for (int i=0; i<n_features; i++)
   {
      cout << "i = " << i 
           << " modeled XYZ = " << modeled_XYZ[i]
           << " raytraced XYZ = " << raytraced_XYZ[i] << endl;
      modeled_COM += modeled_XYZ[i];
      raytraced_COM += raytraced_XYZ[i];
   }

   modeled_COM /= n_features;
   raytraced_COM /= n_features;
   
   cout << "modeled_COM = " << modeled_COM << endl;
   cout << "raytraced_COM = " << raytraced_COM << endl;
   cout << "|modeled_COM-raytraced_COM| = "
        << (modeled_COM-raytraced_COM).magnitude() << endl;
   
   double az,el,roll,scale;
   threevector trans_Horn,trans_Peter;
   double max_residual_dist=150;	// meters
   double median_residual_dist,quartile_width;
   bundlerfunc::RANSAC_fit_rotation_translation_scale(
      raytraced_XYZ,modeled_XYZ,raytraced_COM,modeled_COM,
      az,el,roll,scale,trans_Horn,trans_Peter,
      median_residual_dist,quartile_width);

   cout << "az = " << az*180/PI << endl;
   cout << "el = " << el*180/PI << endl;
   cout << "roll = " << roll*180/PI << endl;
   cout << "scale = " << scale << endl;
   cout << "trans_Horn = " << trans_Horn << endl;
   cout << "trans_Peter = " << trans_Peter << endl;

   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll);

//   az=el=roll=0;
//   scale=1;
//   trans_Horn=Zero_vector;

   vector<threevector> transformed_left_points;
   double avg_residual=bundlerfunc::compute_avg_residual(
      raytraced_XYZ,modeled_XYZ,transformed_left_points,
      az,el,roll,scale,trans_Horn);

/*

Uncorrected residual between raytraced and modeled 3D features= 

	8.92621640008 +/- 6.97371471559

Corrected residual between raytraced and modeled 3D features = 

	1.56083266676 +/- 0.9346622715

--fitted_world_to_bundler_distance_ratio 11.2617108722
--bundler_translation_X 328180.804496
--bundler_translation_Y 4692031.67185
--bundler_translation_Z 32.6961840629
--global_az -159.301437701
--global_el 1.13037289091
--global_roll -16.2941904802
--bundler_rotation_origin_X 8.64245166716
--bundler_rotation_origin_Y -5.00538436058
--bundler_rotation_origin_Z 0.87065684378


*/


}
