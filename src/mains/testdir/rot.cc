// ==========================================================================
// Program ROT
// ==========================================================================
// Last updated on 2/8/06; 5/28/09; 6/15/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <osg/Matrixd>

#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "numerical/euler.h"
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
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/plane.h"
#include "math/rotation.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
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

// Matrix R0 maps a vertically downward oriented OBSFRUSTUM onto a
// horizontal one with Uhat = -y_hat, Vhat = +z_hat and pointing
// direction -What = +x_hat:

   rotation R0;
   R0.clear_values();
   R0.put(0,2,-1);
   R0.put(1,0,-1);
   R0.put(2,1,1);

//curr_R = 
//0.3812412548    0.05824652021   -0.9226388505
//-0.9165996925   -0.1061795069   -0.3854489798
//-0.1204164      0.9926395394    0.01290873406

   osg::Quat q_OSG(
      0.608249540781,-0.354078442943,-0.43026971992,0.565268892212);

   fourvector q(q_OSG._v[0], q_OSG._v[1], q_OSG._v[2], q_OSG._v[3]);

   cout << "q_OSG = " << endl;
   osgfunc::print_quaternion(q_OSG);

   cout << "OSG matrix corresponding to q_OSG: " << endl;

   osg::Matrix R_OSG_matrix;
   R_OSG_matrix.set(q_OSG);
   osgfunc::print_matrix(R_OSG_matrix);

//   osg::Quat q_OSG_recovered;
//   R_OSG_matrix.get(q_OSG_recovered);

//   cout << "q_OSG_recovered = " << endl;
//   osgfunc::print_quaternion(q_OSG_recovered);

// On 6/15/09, we explicitly verified that OSG quaternions have zeroth
// and third components which need to be flipped in order to match
// onto our fourvector quaternion representation!

   rotation Rcamera;

   fourvector q_peter=Rcamera.quat_corresponding_to_OSG_quat(q);
   cout << "q_peter = " << q_peter << endl;

//   Rcamera=Rcamera.rotation_corresponding_to_quaternion(q_peter);
//   cout << "Rcamera.transpose() = " << Rcamera.transpose() << endl;

   Rcamera=Rcamera.rotation_corresponding_to_OSG_quat(q);
   cout << "Rcamera.transpose() = " << Rcamera.transpose() << endl;

   rotation R_OSG=Rcamera.OSG_rotation_corresponding_to_rotation();

   osg::Matrixd Rfinal,Rfinal2;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         Rfinal(i,j)=R_OSG.get(i,j);
      }
   }

   cout << "OSG Rfinal = " << endl;
   osgfunc::print_matrix(Rfinal);

   fourvector q_peter_recovered=
      Rcamera.quaternion_corresponding_to_rotation();
   cout << "q_peter_recovered = " << q_peter_recovered << endl;
   

   fourvector q_OSG_recovered=Rcamera.OSG_quat_corresponding_to_rotation();
   cout << "q_OSG_recovered = " << q_OSG_recovered << endl;

 


}
