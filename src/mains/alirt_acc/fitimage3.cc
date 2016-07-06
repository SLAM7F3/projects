// ==========================================================================
// Program FITIMAGE attempts to simulate the generation, analysis and
// displaying of ladar images
// ==========================================================================
// Last updated on 11/22/04
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
#include "general/outputfuncs.h"
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

   xyzimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   xyzimage.colortablefilename=sysfunc::get_cplusplusrootdir()
      +"alirt/colortables/colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

   const int nvertices=4;
   myvector vertex[nvertices];
   vertex[0]=myvector(0,0);
   vertex[1]=myvector(1,0);
   vertex[2]=myvector(1,1);
   vertex[3]=myvector(0,1);
   polygon square(nvertices,vertex);
   square.scale(3);

   double zfill=10;
   double zboundary=1;
   drawfunc::draw_thick_polygon(square,zboundary,0.05,ztwoDarray_ptr);

   bool recursion_limit_exceeded=false;
   int npixels_filled,max_empty_neighbors,n_recursion;
   int px,py,new_px,new_py;

   myvector origin(0,0);
   ztwoDarray_ptr->point_to_pixel(origin,px,py);

   do  
   {
      recursion_limit_exceeded=false;
      npixels_filled=n_recursion=max_empty_neighbors=0;

      recursivefunc::boundaryFill(
         recursion_limit_exceeded,npixels_filled,n_recursion,
         px,py,zfill,zboundary,max_empty_neighbors,new_px,new_py,
         ztwoDarray_ptr);

      cout << "npixels_filled = " << npixels_filled << endl;
      cout << "n_recursion = " << n_recursion << endl;
      cout << "recursion_limit_exceeded = " << recursion_limit_exceeded
           << endl;
      cout << "max_empty_neighbors = " << max_empty_neighbors << endl;
      cout << "px = " << px << " py = " << py << endl;
      cout << "new_px = " << new_px << " new_py = " << new_py << endl;
      outputfunc::newline();
      
      px=new_px;
      py=new_py;
   }
   while (recursion_limit_exceeded);
   
/*
   recursivefunc::boundaryFill(
      recursion_limit_exceeded,npixels_filled,n_recursion,
      px,py,zfill,zboundary,max_empty_neighbors,new_px,new_py,
      ztwoDarray_ptr);

   
   cout << "npixels_filled = " << npixels_filled << endl;
   cout << "n_recursion = " << n_recursion << endl;
   cout << "recursion_limit_exceeded = " << recursion_limit_exceeded
        << endl;
   cout << "max_empty_neighbors = " << max_empty_neighbors << endl;
   cout << "px = " << px << " py = " << py << endl;
   cout << "new_px = " << new_px << " new_py = " << new_py << endl;
*/

   xyzimage.writeimage("square",ztwoDarray_ptr);

}
