// ==========================================================================
// Program PLOT_LAYOUT
// ==========================================================================
// Last updated on 8/30/09
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
#include "threeDgraphics/xyzpfuncs.h"

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

   string input_filename="layout.output";
   cout << "Enter name of input layout file:" << endl;
   cin >> input_filename;
   filefunc::ReadInfile(input_filename);

   vector<double> X,Y,Z;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      
      vector<double> NXY=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      X.push_back(NXY[1]);
      Y.push_back(NXY[2]);
      Z.push_back(0);
   }

   string xyz_filename="layout.xyz";
   xyzpfunc::write_xyz_data(X,Y,Z,xyz_filename);

   

}
