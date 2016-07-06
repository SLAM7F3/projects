// ==========================================================================
// Program FLIR_FOCAL imports 3D/2D tiepoints selected from HAFB ALIRT
// and FLIR video frames as well as corresponding GPS camera position
// measurements.  FLIR_FOCAL computes world-space angles between all
// tiepoint pairs and derives a dimensionless focal parameter from
// each angle.  We wrote this utility program in order to calibrate
// "five deg" f.
// ==========================================================================
// Last updated on 3/10/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   
// Low-resolution FLIR video frame 20120105_112600.370.jpg:

//   threevector GPS_camera_posn(312156.995492 , 4706974.85341 , 732);

// Low-resolution FLIR video frame 20120105_112943.376.jpg:

//   threevector GPS_camera_posn(313545.198886 , 4705337.60239 , 742);

// Low-resolution FLIR video frame 20120105_112723.370.jpg

   threevector GPS_camera_posn(310020.213074 , 4703565.77105 , 717);

   vector<threevector> rays;
//   string features_3D_filename="./features/features_3D_flightfacility.txt";
//   string features_3D_filename="./features/features_3D_flightfacility_b.txt";
   string features_3D_filename="./features/features_3D_flightfacility_c.txt";
   filefunc::ReadInfile(features_3D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      vector<double> column_values=stringfunc::string_to_numbers(curr_line);
      threevector curr_feature_posn(
         column_values[3],column_values[4],column_values[5]);
      threevector curr_ray=(curr_feature_posn-GPS_camera_posn).unitvector();
      rays.push_back(curr_ray);
   }
   
   vector<twovector> UVs;
   string features_2D_filename=
//      "./features/features_2D_20120105_112600.370.txt";
//      "./features/features_2D_20120105_112943.376.txt";
      "./features/features_2D_20120105_112723.370.txt";
   filefunc::ReadInfile(features_2D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      vector<double> column_values=stringfunc::string_to_numbers(curr_line);
      twovector curr_UV(column_values[3],column_values[4]);
      UVs.push_back(curr_UV);
   }

   vector<double> FOV_vs,focal_params;
   for (int i=0; i<rays.size(); i++)
   {
      for (int j=i+1; j<rays.size(); j++)
      {
         double curr_dotproduct=rays[i].dot(rays[j]);
         double delta_theta=acos(curr_dotproduct);
         twovector delta_UV=UVs[i]-UVs[j];
         double delta_s=delta_UV.magnitude();
         double FOV_v=delta_theta/delta_s;
         double f=-0.5/tan(0.5*FOV_v);
         cout << "i = " << i << " j = " << j 
              << " FOV_v = " << FOV_v*180/PI 
              << " f = " << f 
              << endl;
         FOV_vs.push_back(FOV_v);
         focal_params.push_back(f);
      } // loop over index j
   } // loop over index i 
   
   double mean_FOV_v=mathfunc::mean(FOV_vs);
   double mean_f=mathfunc::mean(focal_params);

   cout << "mean_f = " << mean_f << endl;

   double avg_f=-0.5/tan(0.5*mean_FOV_v);
   cout << "mean_FOV_v = " << mean_FOV_v*180/PI << endl;
   cout << "avg_f = " << avg_f << endl;

/*

// Low-resolution FLIR video frame 20120105_112600.370.jpg:

mean_f = -24.593107393
mean_FOV_v = 2.33477802768
avg_f = -24.5367440426

// Low-resolution FLIR video frame 20120105_112943.376.jpg:

mean_f = -24.4230156092
mean_FOV_v = 2.3515122473
avg_f = -24.3620831069

// Low-resolution FLIR video frame 20120105_112723.370.jpg

mean_f = -24.7174108877
mean_FOV_v = 2.32307779301
avg_f = -24.6603580398


Best estimate for "5 deg" FLIR focal length = -24.58


*/



}

