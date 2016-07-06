// ==========================================================================
// Program XYZP2ASCII reads in a point cloud from some input XYZP
// file.  It writes out an ascii file containing XYZ points on each
// line.  We wrote this special utility in April 2009 in order to send
// NYC points to Noah Snavely.

// 			xyzp2ascii  x0y0_fused_output.xyzp

// ==========================================================================
// Last updated on 4/10/09; 4/24/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Read input ladar point cloud file:
   
   string input_cloud_filename=argv[1];
   vector<fourvector>* xyzp_pnt_ptr=xyzpfunc::read_xyzp_float_data(
      input_cloud_filename,0);

   string xyz_filename=stringfunc::prefix(input_cloud_filename)+"_ascii.xyz";
   cout << "Output xyz_filename = " << xyz_filename << endl;

   xyzpfunc::write_xyz_ascii_data(xyzp_pnt_ptr,xyz_filename);
}
