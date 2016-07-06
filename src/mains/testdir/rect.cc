// ==========================================================================
// Program RECT
// ==========================================================================
// Last updated on 10/7/05
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "image/myimage.h"
#include "math/myvector.h"
#include "numrec/nrfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "general/sysfuncs.h"
#include "image/twoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();

   myvector corner[4];
   double sgnx,sgny;
   for (int c=0; c<4; c++)
   {
      switch(c)
      {
         case 0: 
            sgnx=1;
            sgny=1;
            break;
         case 1: 
            sgnx=-1;
            sgny=1;
            break;
         case 2: 
            sgnx=-1;
            sgny=-1;
            break;
         case 3: 
            sgnx=1;
            sgny=-1;
            break;
            
      }
      double rx=sgnx*10.0*nrfunc::ran1();
      double ry=sgny*10.0*nrfunc::ran1();
      corner[c]=myvector(rx,ry);
   }
   
//   corner[0]=myvector(3.5,3);
//   corner[1]=myvector(-3,4);
//   corner[2]=myvector(-4,-2);
//   corner[3]=myvector(5,-4);
   polygon quad(4,corner);

   const int nxbins=501;
   const int nybins=501;
   double max_x=11;
   double max_y=11;
   myimage xyimage(nxbins,nybins);   
   xyimage.z2Darray_ptr=new twoDarray(nxbins,nybins);

// Initialize image parameters:

   xyimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyimage.imagedir);
   xyimage.classified=false;
   xyimage.title="Simulated Video Image";
   string projects_dir="/home/cho/programs/c++/svn/projects/";
   xyimage.colortablefilename=projects_dir+
      "src/mains/alirt_acc/colortables/colortable.image";
   xyimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyimage.z2Darray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* ztwoDarray_ptr=xyimage.z2Darray_ptr;
   drawfunc::draw_polygon(quad,10,ztwoDarray_ptr);


   parallelogram rect;
   rect.rectangle_within_quadrilateral(corner);
   drawfunc::draw_polygon(rect,60,ztwoDarray_ptr);

   xyimage.writeimage("quad",ztwoDarray_ptr);

}
