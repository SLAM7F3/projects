// ==========================================================================
// Program FIVEPOINT attempts to use Noah Snavely's implementation of
// the 5-point algorithm in order to determine relative position and
// pose between two cameras.
// ==========================================================================
// Last updated on 9/18/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "bundler/5point.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
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

   int n_points=5;
   v2_t l_pts[n_points];
   v2_t r_pts[n_points];
   double K1[9],K2[9];
   double R_out[9],t_out[3];
   double ransac_threshold=1E-4;
   int ransac_rounds=25;

   int return_value=compute_pose_ransac(
      n_points,r_pts,l_pts,
      K1,K2,ransac_threshold,ransac_rounds,
      R_out,t_out);

   

}
