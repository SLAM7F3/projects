// ==========================================================================
// Program HUE_SAT_PLANE 
// ==========================================================================
// Last updated on 7/5/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "datastructures/twoDarray.h"
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

   const int Nh=64;
   const int Ns=48;
   const double max_x=30;  // meters
   const double max_y=30;
	
   urbanimage xyzimage(Nh,Ns);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(Nh,Ns);
   xyzimage.z2Darray_ptr=new twoDarray(Nh,Ns);
   
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

   twoDarray* ptwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   
   double h_lo=0;
   double h_hi=300;
   double dh=(h_hi-h_lo)/double(Nh-1);
   double s_lo=0.0;
   double s_hi=1.0;
   double ds=(s_hi-s_lo)/double(Ns-1);

   double fprev=0;
   
   for (int h=0; h<Nh; h++)
   {
      double hue=h_lo+h*dh;
      for (int s=0; s<Ns; s++)
      {
         double saturation=s_lo+s*ds;


         double r,g,b;
         double v=1;
         colorfunc::hsv_to_RGB(hue,saturation,v,r,g,b);
         double magnitude=sqrt(sqr(r)+sqr(g)+sqr(b));

         double f;
         ladarfunc::convert_hue_and_saturation_to_f(hue,saturation,f);

         if (s==10)
         {
            cout << "s = " << saturation << " h = " << hue 
                 << " f = " << f 
                 << " f-fprev = " << f-fprev << endl;
            cout << "R = " << r << " G = " << g << " B = " << b << endl;
            cout << "sqrt(R**2+G**2+B**2) = " << magnitude << endl;
            outputfunc::newline();
            fprev=f;
         }

         ztwoDarray_ptr->put(h,s,0.0);
         ptwoDarray_ptr->put(h,s,f);
      }
   }
   ztwoDarray_ptr->put(0,0,1);
   string color_filenamestr=xyzimage.imagedir+"hue_sat_plane.xyzp";
   ladarfunc::write_xyzp_data(
      ztwoDarray_ptr,ptwoDarray_ptr,color_filenamestr,false);

   delete ptwoDarray_ptr;
   
}
