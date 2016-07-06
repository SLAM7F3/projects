// ==========================================================================
// Program NONLINEAR
// ==========================================================================
// Last updated on 1/27/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/fundamental.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "numerical/newton.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/twoDarray.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

//   const int dim=2;
   const int dim=3;
   const int mdim=sqr(dim);
   newton Newton(dim,mdim);

   double theta_start=0;
   double theta_stop=90;
   double dtheta=(theta_stop-theta_start)/(mdim-1);
   for (int m=0; m<mdim; m++)
   {
      double theta=PI/180*(theta_start+m*dtheta);
      double cos_theta=cos(theta);
      double sin_theta=sin(theta);
      Newton.get_Delta().push_back(threevector(cos_theta,sin_theta,0));
      Newton.get_Deltap().push_back(threevector(cos_theta,sin_theta,0));
//      Newton.get_Deltap().push_back(threevector(-sin_theta,cos_theta,0));
      Newton.get_N().push_back(1);

      if (dim==3)
      {
         Newton.get_Delta().push_back(threevector(cos_theta,0,sin_theta));
         Newton.get_Deltap().push_back(threevector(-sin_theta,0,cos_theta));
         Newton.get_N().push_back(0);
         Newton.get_Delta().push_back(threevector(0,cos_theta,sin_theta));
         Newton.get_Deltap().push_back(threevector(0,-sin_theta,cos_theta));
         Newton.get_N().push_back(0);
      }
   }

   Newton.fill_data_matrix();

// Fill matrix A with initial guess for its values:

//   Newton.get_Aptr()->identity();

/*
   Newton.get_Aptr()->put(0,0,5.5);
   Newton.get_Aptr()->put(0,1,1.5);
   Newton.get_Aptr()->put(1,0,12.5);
   Newton.get_Aptr()->put(1,1,-33.5);
*/

   Newton.get_Aptr()->put(0,0,7.5);
   Newton.get_Aptr()->put(0,1,1.3712);
   Newton.get_Aptr()->put(0,2,-1.3712);

   Newton.get_Aptr()->put(1,0,1.526);
   Newton.get_Aptr()->put(1,1,-3.325);
   Newton.get_Aptr()->put(1,2,4.325);

   Newton.get_Aptr()->put(2,0,0.2);
   Newton.get_Aptr()->put(2,1,-3.325);
   Newton.get_Aptr()->put(2,2,7.325);

   Newton.transfer_A_to_Avec();

   int n_iters;
   cout << "Enter number of iterations:" << endl;
   cin >> n_iters;
   Newton.refine_A(n_iters);
      
}
