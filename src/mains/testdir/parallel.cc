// ==========================================================================
// Program PARALLEL begins to implement a crude simulation of a solar
// panel arbitarily rotated in 3D being projected down into an ISAR
// image plane.
// ==========================================================================
// Last updated on 1/20/06
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
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
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
// Method compute_2D_COM

twovector compute_2D_COM(const vector<twovector>& XY)
{
   twovector COM(0,0);
   for (int n=0; n<XY.size(); n++)
   {
      COM += XY[n];
   }
   COM /= double(XY.size());
   return COM;
}

void recenter_tiepoints(vector<twovector>& XY)
{
   twovector COM(compute_2D_COM(XY));
   for (int n=0; n<XY.size(); n++)
   {
      XY[n] -= COM;
   }
}

// ==========================================================================
// Method generate_panel takes in 3 rotation angles in degrees.  It
// instantiates a canonical solar panel within the XY plane and then
// rotates it about the XYZ axes by the 3 input angles.  The rotated
// panel is returned as a polygon.

polygon generate_panel(double psi,double phi,double theta)
{
   psi *= PI/180;
   phi *= PI/180;
   theta *= PI/180;

   mymatrix R(psi,phi,theta);
   mymatrix P;
   P.e[0][0]=1;
   P.e[1][1]=1;
 
   const double L=4;
   const double W=2;
   vector<threevector> vertex,rotated_vertex;
   vertex.push_back(threevector(0.5*L,0.5*W));
   vertex.push_back(threevector(-0.5*L,0.5*W));
   vertex.push_back(threevector(-0.5*L,-0.5*W));
   vertex.push_back(threevector(0.5*L,-0.5*W));

   for (int n=0; n<vertex.size(); n++)
   {
      rotated_vertex.push_back(R*vertex[n]);
      cout << "n = " << n << " rot vertex = " << rotated_vertex.back() 
           << endl;
   }
   polygon rotated_panel(rotated_vertex);
   return rotated_panel;
}

// ==========================================================================
// Method panel_corner_projections appends the input panel's XY image
// plane projections onto STL vector proj_vertices.

void panel_corner_projections(
   const polygon& panel,vector<twovector>& proj_vertices)
{
   for (int n=0; n<panel.get_nvertices(); n++)
   {
      proj_vertices.push_back(twovector(panel.get_vertex(n)));
   } // loop over index n labeling panel vertices
}

// ==========================================================================
// Method generate_cube takes in 3 rotation angles in degrees.  It
// instantiates a 2*unit cube and then rotates it about the XYZ axes
// by the 3 input angles.  The rotated cube is returned by this method.

mybox generate_cube(double psi,double phi,double theta)
{
   mybox cube(-1,1,-1,1,-1,1);

   psi *= PI/180;
   phi *= PI/180;
   theta *= PI/180;

   mymatrix R(psi,phi,theta);
   cube.rotate(R);
   
   return cube;
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{

   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
   const double max_x=6;  // meters
   const double max_y=6;  // meters
	
   myimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   xyzimage.title="Simulated neighborhood"; 
   xyzimage.xaxislabel="X (meters)";
   xyzimage.yaxislabel="Y (meters)";
   xyzimage.xtic=1; // meters
   xyzimage.ytic=1; // meters

   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize image parameters:

   xyzimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   string dirname="/home/cho/programs/c++/svn/projects/src/mains/alirt_acc/colortables/";
   xyzimage.colortablefilename=dirname+"colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);

   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

// Enter rotation angles:

   double psi,phi,theta;
   psi=20;
   phi=40;
   theta=60;
//   cout << "Enter angle psi in degrees:" << endl;
//   cin >> psi;
//   cout << "Enter angle phi in degrees:" << endl;
//   cin >> phi;
//   cout << "Enter angle theta in degrees:" << endl;
//   cin >> theta;
//   psi *= PI/180;
//   phi *= PI/180;
//   theta *= PI/180;

   vector<polygon> panel;
//   panel.push_back(generate_panel(0,0,0));
//   panel.push_back(generate_panel(0,1,2));
   panel.push_back(generate_panel(10,20,30));
   panel.back().translate(myvector(3,2));
//   panel.push_back(generate_panel(0,0,0));
//   panel.push_back(generate_panel(0,-1,-2));
   panel.push_back(generate_panel(-10,20,-30));
   panel.back().translate(myvector(-3,-2));

//   drawfunc::draw_polygon(panel[0],60,ztwoDarray_ptr);
//   drawfunc::draw_polygon(panel[1],30,ztwoDarray_ptr);

   cout << "panel[0] = " << panel[0] << endl;
   cout << "panel[1] = " << panel[1] << endl;

   mybox cube=generate_cube(0,0,0);
   draw3Dfunc::draw_parallelepiped(cube,colorfunc::red,colorfunc::blue,
                                   ztwoDarray_ptr);

   vector<twovector> XY;
   panel_corner_projections(panel[0],XY);
   panel_corner_projections(panel[1],XY);

// Rotate both panels about z axis by common angle:

//   mymatrix R(0,0,0);
   mymatrix R(0,0,15*PI/180);
   panel[0].rotate(R);
   panel[1].rotate(R);
   
   vector<twovector> UV;
   panel_corner_projections(panel[0],UV);
   panel_corner_projections(panel[1],UV);

//   drawfunc::draw_polygon(panel[0],55,ztwoDarray_ptr);
//   drawfunc::draw_polygon(panel[1],25,ztwoDarray_ptr);

   cout << "panel[0] = " << panel[0] << endl;
   cout << "panel[1] = " << panel[1] << endl;

// Rotate both panels about z axis by common angle:

//   mymatrix R(0,0,0);
//   mymatrix R2(0,0,15*PI/180);
   panel[0].rotate(R);
   panel[1].rotate(R);
   
/*
   vector<twovector> AB;
   panel_corner_projections(panel[0],AB);
   panel_corner_projections(panel[1],AB);

   drawfunc::draw_polygon(panel[0],50,ztwoDarray_ptr);
   drawfunc::draw_polygon(panel[1],20,ztwoDarray_ptr);
*/

//   xyzimage.adjust_x_scale=false;
   xyzimage.writeimage("parallel",ztwoDarray_ptr);
  
// Compute affine fundamental matrix based upon tiepoint information
// stored in XY and UV STL vectors:

   fundamental F;
   F.parse_affine_fundamental_inputs(XY,UV);
   F.compute_affine_fundamental_matrix();
   F.check_fundamental_matrix(XY,UV);

// Translate tiepoints so that their centers-of-mass equal zero:

   recenter_tiepoints(XY);
   recenter_tiepoints(UV);
//   recenter_tiepoints(AB);

   twovector XY_COM=compute_2D_COM(XY);
   cout << "XY COM = " << XY_COM << endl;
   twovector UV_COM=compute_2D_COM(UV);
   cout << "UV COM = " << UV_COM << endl;
//   twovector AB_COM=compute_2D_COM(AB);
//   cout << "AB COM = " << AB_COM << endl;

   vector<vector<twovector> > tiepoints;
   tiepoints.push_back(XY);
   tiepoints.push_back(UV);
//   tiepoints.push_back(AB);

   F.parse_tiepoint_inputs(tiepoints);
   F.compute_affine_reconstruction();

}
