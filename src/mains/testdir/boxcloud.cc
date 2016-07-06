// ==========================================================================
// Program BOXCLOUD
// ==========================================================================
// Last updated on 9/25/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "math/twovector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   double min_x=0;
   double max_x=10;
   double min_y=0;
   double max_y=10;
   double min_z=0;
   double max_z=40;

   polyhedron P;
   P.generate_box(min_x,max_x,min_y,max_y,min_z,max_z);
   cout << "Box P = " << P << endl;

   double ds_frac;
   cout << "Enter ds_frac:" << endl;
   cin >> ds_frac;
   
   vector<pair<threevector,threevector> > pnts_normals=
      P.generate_point_cloud(ds_frac);
   vector<threevector> surface_points;

   for (int i=0; i<pnts_normals.size(); i++)
   {
      threevector curr_point=pnts_normals[i].first;
      surface_points.push_back(curr_point);
      cout << "i = " << i 
           << " X = " << curr_point.get(0)
           << " Y = " << curr_point.get(1)
           << " Z = " << curr_point.get(2) << endl;
   }

   string xyz_filename="cloud.xyz";
   filefunc::deletefile(xyz_filename);
   xyzpfunc::write_xyz_data(xyz_filename,&surface_points,false);
}

