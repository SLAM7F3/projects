// ========================================================================
// Program GEOREG_WISP_PIXELS

//				./georeg_wisp_pixels

// ========================================================================
// Last updated on 3/27/13; 3/28/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string features_3D_filename="./features_3D_DeerIsland.txt";
   string features_2D_filename="./features_pano_coords.dat";

   int pano_width=40000;
   int pano_height=2200;
   double Umax=double(pano_width)/double(pano_height); // = 18.18182
   double IFOV=2*PI/Umax;	// = 0.34557

   double wisp_x=339106.0631;
   double wisp_y=4690476.511;
   double wisp_z=12.73415031;
   threevector wisp_XYZ(wisp_x,wisp_y,wisp_z);

   filefunc::ReadInfile(features_3D_filename);

   cout.precision(12);

   vector<int> feature_3D_ID;
   vector<threevector> rel_XYZ;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      feature_3D_ID.push_back(column_values[1]);
      threevector feature_XYZ(
         column_values[3],column_values[4],column_values[5]);
      rel_XYZ.push_back(feature_XYZ-wisp_XYZ);
//      rel_XYZ.push_back(feature_XYZ);

//      cout << "3D ID = " << feature_3D_ID.back()
//           << " XYZ = " << feature_XYZ << endl;
   }

   filefunc::ReadInfile(features_2D_filename);
   
   vector<int> feature_2D_ID,px,py;   
   vector<double> d_az,d_el;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      if (column_values.size() <= 1) continue;

      int curr_ID=column_values[0];
      feature_2D_ID.push_back(curr_ID);
      int curr_px=column_values[1];
      int curr_py=column_values[2];

      double U=double(curr_px)/pano_height;
      double V=1-double(curr_py)/pano_height;
      double wisp_az=2*PI-U*IFOV;
      double wisp_el=(V-0.5)*IFOV;

//      cout << "ID = " << curr_ID
//           << " XYZ = " << rel_XYZ[curr_ID] << endl;
//      cout << "Curr_px = " << curr_px << " curr_py = " << curr_py
//           << endl;
//      cout << endl << endl;

      threevector curr_rhat=rel_XYZ[curr_ID].unitvector();
      double azimuth=atan2(curr_rhat.get(1),curr_rhat.get(0));
      azimuth=basic_math::phase_to_canonical_interval(azimuth,0,2*PI);
      d_az.push_back(azimuth-wisp_az);

//      cout << "curr_rhat = " << curr_rhat << endl;
//      double theta=acos(curr_rhat.dot(z_hat));
      double theta=acos(curr_rhat.get(2));
      double elevation=PI/2-theta;
      d_el.push_back(elevation-wisp_el);

      cout << "feature ID = " << feature_2D_ID.back()
           << " wisp_az = " << wisp_az*180/PI
           << " true azimuth = " << azimuth*180/PI
           << " d_az = " << d_az.back()*180/PI
           << endl;
      cout << " wisp_el = " << wisp_el*180/PI
           << " true elevation = " << elevation*180/PI 
           << " d_el = " << d_el.back()*180/PI
           << endl;
      cout << endl;
   }
   
   cout << "d_az = " << mathfunc::mean(d_az)*180/PI << " +/- " 
        << mathfunc::std_dev(d_az)*180/PI << endl;
   cout << "d_el = " << mathfunc::mean(d_el)*180/PI << " +/- " 
        << mathfunc::std_dev(d_el)*180/PI << endl;
}

// Coordinate conversion from 3D world XYZ -> 2D Wisp pixel coordinates

// 1.  rel_XYZ=XYZ-WISP_XYZ

// 2.  az=atan2(rel_Y,rel_X), el = PI/2 - acos(rel_Z);

// 3.  wisp_az = az - delta_az , wisp_el = el + delta_el
//	where delta_az = -60.868 degs and delta_el = 0.067 degs

// 4.  U = (2*PI-wisp_az)/IFOV   V = 0.5+wisp_el/IFOV

// 5.  px = U*pano_height  , py = (1-V) * pano_height

// EAST:  az = 0
//    wisp_az = -60.868 degs
//    U = 3.34774  px = 7365

// NORTH: az = 90 degs
//      wisp_az = 29.132 degs
// 	wisp_az = 330.868

//    double IFOV=2*PI/Umax;	// = 0.34557
