// ==========================================================================
// Program FITIMAGE attempts to simulate the generation, analysis and
// displaying of ladar images
// ==========================================================================
// Last updated on 8/4/04
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
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/TwoDarray.h"
#include "ladar/urbanimage.h"

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

   const int nxbins=301;
   const int nybins=301;
   const double max_x=30;  // meters
   const double max_y=30;  // meters
	
   urbanimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   
   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize image parameters:

   xyzimage.imagedir=sysfunc::get_cplusplusrootdir()
      +"alirt/images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   xyzimage.colortablefilename=sysfunc::get_cplusplusrootdir()
      +"alirt/colortables/colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(ladarimage::null_value);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

/*
   int npoints=4;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(2,0);
   vertex[2]=myvector(2,1);
   vertex[3]=myvector(0,1);
*/

// L contour:

/*
   int npoints=6;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(3,0);
   vertex[2]=myvector(3,1);
   vertex[3]=myvector(1,1);
   vertex[4]=myvector(1,2);
   vertex[5]=myvector(0,2);
*/

/*
// 2nd L contour:

   int npoints=6;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(-3,0);
   vertex[2]=myvector(-3,1);
   vertex[3]=myvector(-1,1);
   vertex[4]=myvector(-1,2);
   vertex[5]=myvector(0,2);
*/

/*
// Stairs contour

   int npoints=10;
   myvector vertex[npoints];
   vertex[0]=myvector(1,0);
   vertex[1]=myvector(3,0);
   vertex[2]=myvector(3,1);
   vertex[3]=myvector(2,1);
   vertex[4]=myvector(2,2);
   vertex[5]=myvector(1,2);
   vertex[6]=myvector(1,3);
   vertex[7]=myvector(0,3);
   vertex[8]=myvector(0,1);
   vertex[9]=myvector(1,1);
*/

/*
// Backward C contour:

   int npoints=8;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(-1,0);
   vertex[2]=myvector(-1,-1);
   vertex[3]=myvector(1,-1);
   vertex[4]=myvector(1,2);
   vertex[5]=myvector(-1,2);
   vertex[6]=myvector(-1,1);
   vertex[7]=myvector(0,1);
*/

/*
// Upside-down T contour:

   int npoints=8;
   myvector vertex[npoints];
   vertex[0]=myvector(-1,0);
   vertex[1]=myvector(2,0);
   vertex[2]=myvector(2,1);
   vertex[3]=myvector(1,1);
   vertex[4]=myvector(1,5);
   vertex[5]=myvector(0,5);
   vertex[6]=myvector(0,1);
   vertex[7]=myvector(-1,1);
*/

/*
// Factory contour:

   int npoints=14;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(6,0);
   vertex[2]=myvector(6,6);
   vertex[3]=myvector(5,6);
   vertex[4]=myvector(5,1);
   vertex[5]=myvector(4,1);
   vertex[6]=myvector(4,5);
   vertex[7]=myvector(3,5);
   vertex[8]=myvector(3,1);
   vertex[9]=myvector(2,1);
   vertex[10]=myvector(2,2);
   vertex[11]=myvector(1,2);
   vertex[12]=myvector(1,1);
   vertex[13]=myvector(0,1);
*/

/*
// Fake house #1 contour:
   int npoints=10;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(1,0);
   vertex[2]=myvector(1,-0.1);
   vertex[3]=myvector(1.1,-0.1);
   vertex[4]=myvector(1.1,0);
   vertex[5]=myvector(2,0);
   vertex[6]=myvector(2,1);
   vertex[7]=myvector(1,1);
   vertex[8]=myvector(1,1.2);
   vertex[9]=myvector(0,1.2);
*/

/*
// Fake house #2 contour:
   int npoints=10;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(2,0);
   vertex[2]=myvector(2,0.5);
   vertex[3]=myvector(1.8,0.5);
   vertex[4]=myvector(1.8,0.7);
   vertex[5]=myvector(2,0.7);
   vertex[6]=myvector(2,1);
   vertex[7]=myvector(-0.2,1);
   vertex[8]=myvector(-0.2,0.2);
   vertex[9]=myvector(0,0.2);
*/

/*
   int npoints=8;
   myvector vertex[npoints];
   vertex[0]=myvector(306.4315326,309.8365546);
   vertex[1]=myvector(309.027216,310.3834377);
   vertex[2]=myvector(310.5267022,303.2663929);
   vertex[3]=myvector(323.2577329,305.9486869);
   vertex[4]=myvector(321.6837609,313.419265);
   vertex[5]=myvector(314.0670993,311.8145147);
   vertex[6]=myvector(313.5990172,314.0361835);
   vertex[7]=myvector(305.8889647,312.4117567);
*/


/*
   int npoints=7;
   myvector vertex[npoints];
   vertex[0]=myvector(291.0055123,445.640411);
   vertex[1]=myvector(282.0867216,442.6109928);
   vertex[2]=myvector(281.1293334,444.5011044);
   vertex[3]=myvector(279.0292593,444.1286994);
   vertex[4]=myvector(280.5899803,437.6851008);
   vertex[5]=myvector(292.9723829,440.3560909);
   vertex[6]=myvector(291.017756,446.7207479);
*/

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

// "Pythagoerus" contour

/*
   int npoints=8;
   myvector vertex[npoints];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(2,0);
   vertex[2]=myvector(2,1);
   vertex[3]=myvector(1,1);
   vertex[4]=myvector(1,2.5);
   vertex[5]=myvector(-2,2.5);
   vertex[6]=myvector(-2,1.5);
   vertex[7]=myvector(0,1.5);
*/

// Apartment complex contour:

   int npoints=12;
   myvector vertex[npoints];

   vertex[0]=myvector(5.809220492,30.8922539);
   vertex[1]=myvector(22.3732285,33.37057222);
   vertex[2]=myvector(24.4779842,19.30329524);
   vertex[3]=myvector(8.535575536,16.91798105);
   vertex[4]=myvector(10.86802834,1.328875018);
   vertex[5]=myvector(28.65249873,3.989799758);
   vertex[6]=myvector(26.49672479,18.39806058);
   vertex[7]=myvector(43.34335257,20.91866466);
   vertex[8]=myvector(40.75064311,38.24721356);
   vertex[9]=myvector(24.76892063,35.8560172);
   vertex[10]=myvector(22.44081724,51.41605359);
   vertex[11]=myvector(3.169842222,48.532717);

/*
   vertex[0]=myvector(-18.6374,21.9351);
   vertex[1]=myvector(-15.998,4.29463);
   vertex[2]=myvector(0.565992,6.77295);
   vertex[3]=myvector(2.67075,-7.29433);
   vertex[4]=myvector(-13.2717,-9.67964);
   vertex[5]=myvector(-10.9392,-25.2688);
   vertex[6]=myvector(6.84526,-22.6078);
   vertex[7]=myvector(4.68949,-8.19956);
   vertex[8]=myvector(21.5361,-5.67896);
   vertex[9]=myvector(18.9434,11.6496);
   vertex[10]=myvector(2.96168,9.25839);
   vertex[11]=myvector(0.633581,24.8184);

   vertex[0]=myvector(-15.998,4.29463);
   vertex[1]=myvector(0.565992,6.77295);
   vertex[2]=myvector(2.67075,-7.29433);
   vertex[3]=myvector(-13.2717,-9.67964);
   vertex[4]=myvector(-10.9392,-25.2688);
   vertex[5]=myvector(6.84526,-22.6078);
   vertex[6]=myvector(4.68949,-8.19956);
   vertex[7]=myvector(21.5361,-5.67896);
   vertex[8]=myvector(18.9434,11.6496);
   vertex[9]=myvector(2.96168,9.25839);
   vertex[10]=myvector(0.633581,24.8184);
   vertex[11]=myvector(-18.6374,21.9351);
*/

/*
// L-shaped house

   int npoints=8;
   myvector vertex[npoints];
   vertex[0]=myvector(42.69670663,10.53270833);
   vertex[1]=myvector(45.84149752,10.64645805);
   vertex[2]=myvector(46.10411576,3.385961167);
   vertex[3]=myvector(60.3121304,3.899876946);
   vertex[4]=myvector(60.03659372,11.51752443);
   vertex[5]=myvector(49.85014078,11.1490719);
   vertex[6]=myvector(49.33539087,25.38014742);
   vertex[7]=myvector(42.16903829,25.12093445);
*/

   contour* cityblock_ptr=new contour(npoints,vertex);
   cityblock_ptr->translate(-(cityblock_ptr->vertex_average()));
//   cityblock_ptr->scale(2.85);
   const double z_null=ladarimage::null_value;
   const double z_boundary=4;
   drawfunc::draw_thick_contour(
      *cityblock_ptr,z_boundary,0.15,ztwoDarray_ptr);

   xyzimage.writeimage("contour",ztwoDarray_ptr);

   double delta_s=1.0;	// meter
//   myvector origin(cityblock_ptr->get_origin());
   myvector origin(10,0);
   contour* c_ptr=graphicsfunc::contour_surrounding_enclosed_region(
      z_null,z_boundary,delta_s,origin,ztwoDarray_ptr);
   drawfunc::draw_thick_contour(*c_ptr,10,0.3,ztwoDarray_ptr);
   xyzimage.writeimage("mask",ztwoDarray_ptr);
   delete c_ptr;

   delete cityblock_ptr;

}
