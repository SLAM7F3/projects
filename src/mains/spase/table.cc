// ==========================================================================
// Program TABLE generates a wireframe model of the table on which the
// SPASE satellite was imaged in the ladar lab in Dec 04 and Jan 05.
// This table model is useful for JIGSAW sensor calibration.
// ==========================================================================
// Last updated on 5/22/05; 4/23/06
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
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ostream;
   using std::pair;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
   const double max_x=0.5;  // meters
   const double max_y=0.5;  // meters
	
   myimage xyzimage(nxbins,nybins);   
   xyzimage.set_z2Darray_orig_ptr(new twoDarray(nxbins,nybins));
   xyzimage.set_z2Darray_ptr(new twoDarray(nxbins,nybins));
   
   bool input_param_file;
   unsigned int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize image parameters:

   xyzimage.set_imagedir(filefunc::get_pwd()+"images/fitimage/");
   filefunc::dircreate(xyzimage.get_imagedir());
   xyzimage.set_classified(false);
   xyzimage.set_title("Effective APD Response");
   xyzimage.set_colortable_filename(filefunc::get_pwd()
      +"colortables/colortable.image");
   xyzimage.get_z2Darray_orig_ptr()->init_coord_system(max_x,max_y);
   xyzimage.get_z2Darray_ptr()->init_coord_system(max_x,max_y);
   xyzimage.get_z2Darray_ptr()->initialize_values(0);

   const double inches_to_meters=0.0254;

   mybox box0(60*inches_to_meters,34*inches_to_meters,1.5*inches_to_meters);
   mybox box1(55*inches_to_meters,29*inches_to_meters,4.25*inches_to_meters);
   box1.translate(threevector(0,0,-2.875*inches_to_meters));

   mybox leg1(2.5*inches_to_meters,2.5*inches_to_meters,
              24.75*inches_to_meters);
   leg1.translate(threevector(0,0,-17.375*inches_to_meters));   
   mybox leg2(leg1);
   mybox leg3(leg1);
   mybox leg4(leg1);

   leg1.translate(threevector(26.25*inches_to_meters,13.25*inches_to_meters));
   leg2.translate(threevector(-26.25*inches_to_meters,13.25*inches_to_meters));
   leg3.translate(threevector(26.25*inches_to_meters,-13.25*inches_to_meters));
   leg4.translate(threevector(-26.25*inches_to_meters,-13.25*inches_to_meters));

   string xyzp_filename="table.xyzp";
   draw3Dfunc::ds=0.002;
   draw3Dfunc::draw_parallelepiped(
      box0,xyzp_filename,draw3Dfunc::annotation_value1);
   draw3Dfunc::draw_parallelepiped(
      box1,xyzp_filename,draw3Dfunc::annotation_value2);
   draw3Dfunc::draw_parallelepiped(
      leg1,xyzp_filename,draw3Dfunc::annotation_value3);
   draw3Dfunc::draw_parallelepiped(
      leg2,xyzp_filename,draw3Dfunc::annotation_value3);
   draw3Dfunc::draw_parallelepiped(
      leg3,xyzp_filename,draw3Dfunc::annotation_value3);
   draw3Dfunc::draw_parallelepiped(
      leg4,xyzp_filename,draw3Dfunc::annotation_value3);

}
