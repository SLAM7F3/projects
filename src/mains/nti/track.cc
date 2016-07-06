// ==========================================================================
// Program ALARMS
// ==========================================================================
// Last updated on 12/30/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "ladar/featurefuncs.h"
#include "filter/filterfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "math/mypolynomial.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "track/track.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   
   track T(3);
   
   threevector V1(0,0);
   threevector V2(3,0);
   threevector V3(3,7);
   
   T.set_XYZ_coords(10,V1);
   T.set_XYZ_coords(20,V2);
   T.set_XYZ_coords(30,V3);
   
   cout << "Track T = " << T << endl;
   T.compute_segments(true);

   double ds;
   cout << "Enter ds:" << endl;
   cin >> ds;
   vector<threevector> interp_posn;
   T.interpolated_posns(ds,interp_posn);

   cout << "Interpolated positions:" << endl;
   templatefunc::printVector(interp_posn);

}
