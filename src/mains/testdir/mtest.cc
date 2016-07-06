// ==========================================================================
// Program MTEST
// ==========================================================================
// Last updated on 2/20/06
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
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/plane.h"
#include "geometry/projective.h"
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

   mybox box(1,2,3);
   mybox box_orig(box);

   double alpha=0*PI/180;
//   double alpha=20*PI/180;
//   double beta=0*PI/180;
   double beta=-60*PI/180;
   double gamma=0*PI/180;
//   double gamma=30*PI/180;
   mymatrix R(alpha,beta,gamma);
   threevector trans(3,-4,5.5);

   box.rotate(R);
   box.translate(trans);

   cout << "Original box = " << box_orig << endl;
   cout << "Rotated box = " << box << endl;

   const int N_vertices=8;

   genmatrix P(N_vertices,4),Q(N_vertices,4);
   
   double noise_mag=0;
   cout << "Enter noise magnitude:" << endl;
   cin >> noise_mag;
   vector<double> noise;

   vector<threevector> pvec,qvec;

   for (int n=0; n<4; n++)
   {
      threevector top_p(box_orig.top_face.vertex[n]);
      threevector bottom_p(box_orig.bottom_face.vertex[n]);
      threevector top_q(box.top_face.vertex[n]);
      threevector bottom_q(box.bottom_face.vertex[n]);

      noise.clear();
      for (int m=0; m<3*4; m++)
      {
         noise.push_back(noise_mag*nrfunc::gasdev());
      }

      pvec.push_back(top_p+threevector(noise[0],noise[1],noise[2]));
      pvec.push_back(bottom_p+threevector(noise[3],noise[4],noise[5]));
      qvec.push_back(top_q+threevector(noise[6],noise[7],noise[8]));      
      qvec.push_back(bottom_q+threevector(noise[9],noise[10],noise[11]));   
   }

   projective proj(N_vertices);
   proj.parse_projective_inputs(pvec,qvec);
   proj.compute_projective_matrix();
   proj.check_projective_matrix(pvec,qvec);

   vector<threevector> rvec;
   proj.transform_XYZs_to_UVWs(pvec,rvec);

   for (int n=0; n<pvec.size(); n++)
   {
      cout << "n = " << n << " diff = " 
           << (qvec[n]-rvec[n]).magnitude() << endl;
   }
   

}
