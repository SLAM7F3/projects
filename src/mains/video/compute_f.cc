// ==========================================================================
// Program COMPUTE_F takes in the number of horizontal and vertical
// pixels for a photo along with its horizontal field of view FOV_u.
// It returns the vertical field of view FOV_v along with
// dimensionless focal parameter f.
// ==========================================================================
// Last updated on 1/26/11; 5/29/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "math/mathfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   double f,aspect_ratio,FOV_u,FOV_v;

   int n_horiz_pixels=4000;	// WISP panel
   int n_vert_pixels=2200;	// WISP panel
   FOV_u=36*PI/180.0;		// WISP panel
   
//   int n_horiz_pixels=2400;	// lighthawk
//   int n_vert_pixels=1599;	// lighthawk
//   FOV_u=33.4*PI/180.0;		// lighthawk

//   int n_horiz_pixels=703;	// FLIR
//   int n_vert_pixels=358;	// FLIR
//   FOV_u=29.79*PI/180.0;	// FLIR

   aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);

   camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
      FOV_u,aspect_ratio,f,FOV_v);

   cout << "n_horiz_pixels = " << n_horiz_pixels << endl;
   cout << "n_vert_pixels = " << n_vert_pixels << endl;
   cout << "n_horiz_pixels/n_vert_pixels = "
        << double(n_horiz_pixels)/double(n_vert_pixels) << endl;

   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;

   cout << " f = " << f << endl;
   cout << "aspect_ratio = " << aspect_ratio << endl;

   double f_n_horiz_pixels=fabs(f)*n_horiz_pixels;
   double f_n_vert_pixels=fabs(f)*n_vert_pixels;
   cout << "f_n_horiz_pixels = " << f_n_horiz_pixels << endl;
//   cout << "f_noah = f_n_vert_pixels = " << f_n_vert_pixels << endl;


}
