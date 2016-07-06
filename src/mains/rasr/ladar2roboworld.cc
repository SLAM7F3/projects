// ========================================================================
// Program LADAR2ROBOWORLD
// ========================================================================
// Last updated on 2/10/11; 2/11/11; 2/28/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   vector<threevector> robo_corners,ladar_corners;
//   robo_corners.push_back(threevector(13.0721,36.7841));
//   robo_corners.push_back(threevector(21.0728,36.1558));
//   robo_corners.push_back(threevector(5.93414,30.7367));
//   robo_corners.push_back(threevector(27.0703,29.0806));

   robo_corners.push_back(threevector(12.4628 , 15.8318));
   robo_corners.push_back(threevector(21.3805 , 18.0811));
   robo_corners.push_back(threevector(25.533 , 24.6244));
   robo_corners.push_back(threevector(23.3546 , 33.3488));

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/rasr/panoramas/old_auditorium/";
   string ladar_corners_filename=subdir+"features_3D_auditorium_interior.txt";
   filefunc::ReadInfile(ladar_corners_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      ladar_corners.push_back(threevector(
         column_values[3],column_values[4],column_values[5]));
      cout << "i = " << i << " ladar_corner = " << ladar_corners[i] << endl;
   }

/*
   ladar_corners.push_back(threevector(-3.624651909,22.41419411,4.255654335));
   ladar_corners.push_back(threevector(-0.3093929291,29.40193939,4.856329918));
   ladar_corners.push_back(threevector(-0.3789424896,14.05598450,3.958874702));
   ladar_corners.push_back(threevector(8.376977921,32.62770081,5.318881512));
*/
   ladar_corners.push_back(threevector(
      -0.3005647659 , 13.78893471 , 4.925774097));
   ladar_corners.push_back(threevector(
      -3.506295204 , 22.28761864 , 4.873120308));
   ladar_corners.push_back(threevector(
      -0.3763390834 ,  29.48987527 , 4.991262436));
   ladar_corners.push_back(threevector(
      8.580425262 , 32.70237732 , 4.924972057));

   threevector robo_corner_COM=Zero_vector;
   threevector ladar_corner_COM=Zero_vector;
   for (int c=0; c<robo_corners.size(); c++)
   {
      robo_corner_COM += robo_corners[c];
      ladar_corner_COM += ladar_corners[c];
   }
   robo_corner_COM /= robo_corners.size();
   ladar_corner_COM /= robo_corners.size();

   cout << "ladar_corner_COM = " << ladar_corner_COM << endl;
   cout << "robo_corner_COM = " << robo_corner_COM << endl;


   double scaling_ratio=0;
   double avg_delta_az=0;
   vector<threevector> robo_ray_bundle,ladar_ray_bundle;

   for (int c=0; c<robo_corners.size(); c++)
   {
      threevector robo_rvec=robo_corners[c]-robo_corner_COM;

      double robo_r=robo_rvec.magnitude();
      threevector robo_rhat=robo_rvec.unitvector();
      cout << "robo_rhat = " << robo_rhat << endl;
      robo_ray_bundle.push_back(robo_rhat);
      double curr_az_robo=atan2(robo_rhat.get(1),robo_rhat.get(0));

      threevector ladar_rvec=ladar_corners[c]-ladar_corner_COM;
      double ladar_r=ladar_rvec.magnitude();
      threevector ladar_rhat=ladar_rvec.unitvector();
      cout << "ladar_rhat = " << ladar_rhat << endl;
      ladar_ray_bundle.push_back(ladar_rhat);
      double curr_az_ladar=atan2(ladar_rhat.get(1),ladar_rhat.get(0));

      double curr_delta_az=curr_az_robo-curr_az_ladar;
      curr_delta_az=basic_math::phase_to_canonical_interval(
         curr_delta_az,0,2*PI);
      avg_delta_az += curr_delta_az;

      scaling_ratio += robo_r/ladar_r;
   }
   scaling_ratio /= robo_corners.size();
   avg_delta_az /= robo_corners.size();
   
   cout << "scaling_ratio = " << scaling_ratio << endl;
   cout << "avg delta az = " << avg_delta_az*180/PI << endl;
   threevector trans=robo_corner_COM-ladar_corner_COM;
   cout << "trans = robo_corner_COM - ladar_corner_COM = " << trans << endl;


   rotation R;
//   R.rotation_between_ray_bundles(robo_ray_bundle,ladar_ray_bundle);
   R=R.rotation_from_az_el_roll(avg_delta_az,0,0);

/*
   double az,el,roll;
   R.az_el_roll_from_rotation(az,el,roll);
   cout << "az = " << az*180/PI << endl;
   cout << "el = " << el*180/PI << endl;
   cout << "roll = " << roll*180/PI << endl << endl;
*/

   cout << "rotation R = " << R << endl;

// Translation, rotation and scaling which map (U,V) from floorplan
// PNG image to (X,Y) in 3D robo world:

/*
ladar_corner_COM = 
1.03313
24.6441
4.90727

robo_corner_COM = 
16.7873
33.1893
0

trans = robo_corner_COM - ladar_corner_COM = 
15.7542
8.54524
-4.90727

scaling_ratio = 1.02134
az = -120.214
el = -0.207171
roll = -179.781

rotation R = 
-0.503221	-0.864143	0.00512906	
-0.86415	0.503232	0.00119738	
-0.00361582	-0.00382973	-0.999986	
*/
   
//   az=fabs(az);
//   az=180*PI/180-az;
//   az=-az;
//   cout << "new az = " << az*180/PI << endl;
   
//   el=0;
//   roll=0;
//   R=R.rotation_from_az_el_roll(az,el,roll);

//   cout << "Rnew = " << R << endl;

// Write out new set of translated, scaled & rotated ladar points
// which should fit into robo world coordinate system:

   string tdp_filename=subdir+"auditorium_interior_3D_leveled.tdp";
   vector<double> X,Y,Z;
   tdpfunc::read_XYZ_points_from_tdpfile(tdp_filename,X,Y,Z);

   threevector grid_origin(-2.5,-2.5,0);
   double z_grid=0;
   threevector global_camera_translation(-8.4,9.3,5);

   vector<threevector> ladar_points,ladar_wo_ceiling_points;
   for (int i=0; i<X.size(); i++)
   {
      threevector curr_ladar_point(X[i],Y[i],Z[i]);
      curr_ladar_point -= ladar_corner_COM;
      curr_ladar_point *= scaling_ratio;
      curr_ladar_point = R * curr_ladar_point;
      curr_ladar_point += robo_corner_COM;
      curr_ladar_point += grid_origin+global_camera_translation;
//      curr_ladar_point += grid_origin;
      ladar_points.push_back(curr_ladar_point);

      if (curr_ladar_point.get(2) < 4 &&
          curr_ladar_point.get(2) > z_grid-0.25)
//      if (curr_ladar_point.get(2) < 0)
         ladar_wo_ceiling_points.push_back(curr_ladar_point);
      
//      cout << "i = " << i 
//           << " curr_ladar_point = " << curr_ladar_point << endl;
   }

   string output_tdp_filename=subdir+"rot_leveled_auditorium_interior_2.tdp";
   tdpfunc::write_relative_xyz_data(output_tdp_filename,ladar_points);

   output_tdp_filename=subdir+"rot_leveled_auditorium_wo_ceiling_2.tdp";
   tdpfunc::write_relative_xyz_data(
      output_tdp_filename,ladar_wo_ceiling_points);
}


