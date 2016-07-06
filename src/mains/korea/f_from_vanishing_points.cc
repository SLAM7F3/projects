// ========================================================================
// Program F_FROM_VANISHING_POINTS follows the discussion in example
// 8.28 "Determining the focal length when the other internal
// parameters are known" in Hartley and Zisserman, 2nd edition (pg
// 228).  Given UV coordinates for the vanishing points of two sets of
// orthogonal parallel 3D world-lines, we can simply compute a single
// camera's focal parameter.
// ========================================================================
// Last updated on 9/17/13; 9/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Ground_photo1.jpg

//   threevector v1(-0.88028 , 0.280371 , 1);
//   threevector v2(13.7399 , 0.225872 , 1);

// Ground_photo2.jpg

   threevector v1(-5.00662 , 2.40054 , 1);
   threevector v2( 6.88321 , 3.75507 , 1);
   
   double f_sqr=-(v1.get(0)*v2.get(0)+v1.get(1)*v2.get(1));
   cout << "f_sqr = " << f_sqr << endl;
   if (f_sqr < 0) exit(-1);
   
   double f=-sqrt(f_sqr);
   cout << "f = " << f << endl;

   double aspect_ratio = 1000.0/564.0;	// ground_photo1.jpg

   double FOV_u,FOV_v;
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      f,aspect_ratio,FOV_u,FOV_v);

   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;

/*

ground_photo1.jpg

f_sqr = 12.0316
f = -3.46866
FOV_u = 28.6736
FOV_v = 16.4051

ground_photo2.jpg

f_sqr = 25.4474
f = -5.04454
FOV_u = 19.9347
FOV_v = 11.321


*/


}

