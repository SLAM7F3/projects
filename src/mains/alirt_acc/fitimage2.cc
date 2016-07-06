// ==========================================================================
// Program FITIMAGE attempts to simulate the generation, analysis and
// displaying of ladar images
// ==========================================================================
// Last updated on 10/8/04
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
#include "ladar/oriented_box.h"
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
   using std::flush;
   using std::ios;
   using std::ostream;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=301;
   const int nybins=301;
   const double max_x=3;  // meters
   const double max_y=3;  // meters
	
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

   xyzimage.imagedir=sysfunc::get_cplusplusrootdir()+"alirt/images/fitimage/";
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
   myvector vertex[4];
   vertex[0]=myvector(1,0,0);
   vertex[1]=myvector(0,1,0);
   vertex[2]=myvector(-1,0,0);
   vertex[3]=myvector(0,-1,0);
   polygon square(4,vertex);

   myvector ray_basepoint(5,0,3);
   myvector ray_hat(-1,0,-1);
   myvector pnt_in_plane(square.ray_projected_into_poly_plane(
      ray_basepoint,ray_hat,square.vertex_average()));
   cout << "projection point = " << pnt_in_plane << endl;
   bool ray_pierce_poly=square.point_inside_polygon(pnt_in_plane);

   cout << "ray_pierce_poly = " << ray_pierce_poly << endl;
*/

   oriented_box box(1,3,1);
   box.translate(myvector(0,0,0.5));
   cout << "box = " << box << endl;

   double elevation,azimuth;
   cout << "Enter elevation angle in degs:" << endl;
   cin >> elevation;
   elevation *= PI/180;
   cout << "Enter azimuth angle in degs:" << endl;
   cin >> azimuth;
   azimuth *= PI/180;
   double tan_elevation=tan(elevation);

   const myvector ray_hat(
      cos(elevation)*cos(azimuth),cos(elevation)*sin(azimuth),sin(elevation));

/*
   for (int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      cout << px << " "  << flush;
      for (int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         myvector ground_point;
         ztwoDarray_ptr->pixel_to_point(px,py,ground_point);

         if (box.ray_pierces_me(tan_elevation,ground_point,ray_hat))
         {
            ztwoDarray_ptr->put(px,py,0);
         }
      } // loop over py index
   } // loop over px index
   
   drawfunc::draw_parallelepiped(
      box,colorfunc::red,colorfunc::blue,ztwoDarray_ptr);
   xyzimage.writeimage("shadow",ztwoDarray_ptr);
*/

   const int nvertices=4;
   myvector vertex[nvertices];
   vertex[0]=myvector(0,0,0);
   vertex[1]=myvector(0,1,0);
   vertex[2]=myvector(0,1,1);
   vertex[3]=myvector(0,0,1);
   polygon xz_square(nvertices,vertex);
   

/*
   vertex[0]=myvector(1,0,0);
   vertex[1]=myvector(0,1,0);
   vertex[2]=myvector(-1,0,0);
   vertex[3]=myvector(0,-1,0);
   polygon xy_square(nvertices,vertex);
   myvector xy_origin(xy_square.vertex_average());

   myvector projected_vertex_in_xy_plane[nvertices];

   for (int i=0; i<nvertices; i++)
   {
      vertex[i]=xz_square.vertex[i];
      if (xy_square.ray_projected_into_poly_plane(
         vertex[i],ray_hat,xy_origin,projected_vertex_in_xy_plane[i]))
      {
         cout << "i = " << i << " xz square vertex = " << vertex[i]
              << " projected vertex in xy plane = "
              << projected_vertex_in_xy_plane[i] << endl;
      }
   }
*/

   polygon xz_square_projection(xz_square);
   if (xz_square.projection_into_xy_plane_along_ray(
      ray_hat,xz_square_projection))
   {
      cout << "xz_square_projection = "
           << xz_square_projection << endl;
   }

}
