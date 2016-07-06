// ==========================================================================
// Program FITIMAGE attempts to simulate the generation, analysis and
// displaying of ladar images
// ==========================================================================
// Last updated on 5/22/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "geometry/mybox.h"
#include "math/threevector.h"
#include "urban/oriented_box.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   threevector u(-1,2.1415,3.14);
   threevector v(1,-17.33542,8.99803);

//   threevector u(cos(PI/6)*cos(PI/4),cos(PI/6),sin(PI/6));
//   threevector v(cos(5*PI/6.0),sin(5*PI/6.0),0);
   cout << "u = " << u << " v = " << v << endl;

}
