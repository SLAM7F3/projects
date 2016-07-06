// ==========================================================================
// Program RGB
// ==========================================================================
// Last updated on 1/13/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
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
#include "numrec/nrfuncs.h"
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
   using std::ofstream;
   using std::pair;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   mybox unitcube(1,1,1);
   unitcube.translate(threevector(0.5,0.5,0.5));
   
   string xyzp_filename="box.xyzp";
   if (filefunc::fileexist("./"+xyzp_filename))
   {
      filefunc::deletefile("./"+xyzp_filename);
   }
   
//   draw3Dfunc::draw_parallelepiped(
//      unitcube,xyzp_filename,draw3Dfunc::annotation_value1,0.002);

   ofstream binary_outstream;
   binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

//   const int nbins=10;
//   const int nbins=20;
//   const int nbins=25;
   const int nbins=50;
   const double delta=1.0/double(nbins-1);

   {
      double r,g,b;
      
      cout << "Enter r:" << endl;
      cin >> r;
      cout << "Enter g:" << endl;
      cin >> g;
      cout << "Enter b:" << endl;
      cin >> b;
      double rgb_colormap_value=colorfunc::rgb_colormap_value(r,g,b);

      threevector point1(-1,-1,-2);
      threevector point2(-1,-1,2);
      draw3Dfunc::draw_thick_line(
         point1,point2,xyzp_filename,rgb_colormap_value,30,0.001);
   }

/*
   int counter=0;
   int ncolors=nbins*nbins*nbins;
   for (int r=0; r<nbins; r++)
   {
      double rfrac=min(1.0,double(r*delta));
      for (int g=0; g<nbins; g++)
      {
         double gfrac=min(1.0,double(g*delta));
         for (int b=0; b<nbins; b++)
         {
            double bfrac=min(1.0,double(b*delta));
//            cout << "map[" << counter << "]=" << rfrac << "; \t"
//                 << "map[" << counter+1 << "]=" << gfrac << "; \t"
//                 << "map[" << counter+2 << "]=" << bfrac << ";" << endl;

            threevector curr_point(rfrac,gfrac,bfrac);
            double annotation_value=double(counter)/double(ncolors-1);

//            cout << "r = " << rfrac << " g = " << gfrac 
//                 << " b = " << bfrac 
//                 << " annot = " << annotation_value << endl;
            xyzpfunc::write_single_xyzp_point(
               binary_outstream,curr_point,annotation_value);
//            cout << "counter = " << counter << " annot_value = "
//                 << annotation_value << endl;
            counter++;
         }
      }
   }
*/

   binary_outstream.close();  

   threevector origin(0,0,0);
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      xyzp_filename,origin);
   filefunc::gunzip_file_if_gzipped(xyzp_filename);
}
