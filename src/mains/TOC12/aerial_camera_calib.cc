// ==========================================================================
// Program AERIAL_CAMERA_CALIB
// ==========================================================================
// Last updated on 10/24/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/homography.h"
#include "math/rotation.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   homography H;

   vector<threevector> XY,UV;

   const double inches_per_meter=39.37;

   XY.push_back(threevector(4.25,2.8125)/inches_per_meter);
   XY.push_back(threevector(11.625,2.8125)/inches_per_meter);
   XY.push_back(threevector(7.875,9.25)/inches_per_meter);
   XY.push_back(threevector(4.25,12.75)/inches_per_meter);
   XY.push_back(threevector(11.625,12.75)/inches_per_meter);
   XY.push_back(threevector(7.875,19.25)/inches_per_meter);

   UV.push_back(threevector(0.7797,0.4815));
   UV.push_back(threevector(0.8425,0.4818));
   UV.push_back(threevector(0.8093,0.5355));
   UV.push_back(threevector(0.7771,0.5645));
   UV.push_back(threevector(0.8402,0.5642));
   UV.push_back(threevector(0.8067,0.6159));

   H.parse_homography_inputs(XY,UV);
//   H.parse_homography_inputs(UV,XY);
   H.compute_homography_matrix();
   H.check_homography_matrix(XY,UV);


   double f=fabs(-0.66);
   rotation R;
   threevector camera_posn;
 
   H.compute_extrinsic_params(f,R,camera_posn);


   cout << "camera_posn = " << camera_posn << endl;
   cout << "R = " << R << endl;
   

}
