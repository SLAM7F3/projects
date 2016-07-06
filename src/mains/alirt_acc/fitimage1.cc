// ==========================================================================
// Program FITIMAGE attempts to simulate the generation, analysis and
// displaying of ladar images
// ==========================================================================
// Last updated on 7/15/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "datastructures/Linkedlist.h"
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
   using std::ostream;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================
   
   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;


/*
   int npoints=4;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(2,0);
   vertex[2]=myvector(2,1);
   vertex[3]=myvector(0,1);
*/

   int npoints=6;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(1,-1);
   vertex[2]=myvector(2,0);
   vertex[3]=myvector(2,1);
   vertex[4]=myvector(2,2);
   vertex[5]=myvector(0,2);

/*
   int npoints=6;
   myvector vertex[npoints];
   vertex[0]=myvector(-35.94754971,-11.67203946);
   vertex[1]=myvector(-36.18850346,-11.71356635);
   vertex[2]=myvector(-36.63992065,-9.094283648);
   vertex[3]=myvector(74.4345829,10.04872078);
   vertex[4]=myvector(72.58747036,20.7663231);
   vertex[5]=myvector(-38.24607945,1.664845571);
*/

   polygon p(npoints,vertex);
   p.translate(-p.vertex_average());
   double min_x,min_y,max_x,max_y;
   p.locate_extremal_xy_points(min_x,min_y,max_x,max_y);
   max_x=max(max_x,-min_x)+1;
   max_y=max(max_y,-min_y)+1;

   const int nxbins=501;
   const int nybins=501;
//   const double max_x=3;  // meters
//   const double max_y=3;
	
   ladarimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);

// Initialize image parameters:

   xyzimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   string dirname="/home/cho/programs/c++/svn/projects/src/mains/alirt_acc/colortables/";
   xyzimage.colortablefilename=dirname+"colortable.image";
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

   drawfunc::draw_polygon(p,10,ztwoDarray_ptr);
   drawfunc::color_polygon_interior(p,60,ztwoDarray_ptr);
   xyzimage.writeimage("polygon",ztwoDarray_ptr);
}
