// ==========================================================================
// Program VORONOI
// ==========================================================================
// Last updated on 3/4/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <new>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "genfuncs.h"
#include "math/myvector.h"
#include "delaunay.h"
#include "general/sysfuncs.h"
#include "genfuncs.h"
#include "datastructures/twoDarray.h"
#include "regular_polygon.h"
#include "parallelogram.h"
#include "drawfuncs.h"
#include "imagefuncs.h"
#include "recursivefuncs.h"
#include "urbanimage.h"
#include "ladarfuncs.h"
#include "building.h"
#include "graphicsfuncs.h"
#include "voronoifuncs.h"
#include "numrec/nrfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::ostream;
   using std::ofstream;
   using std::cin;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
//   const double max_x=7;  // meters
//   const double max_y=7;
   const double max_x=10;  // meters
   const double max_y=10;
//   const double max_x=15;  // meters
//   const double max_y=15;
//   const double max_x=20;  // meters
//   const double max_y=20;
	
   urbanimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   
// Initialize image parameters:

   xyzimage.imagedir=sysfunc::get_cplusplusrootdir()+"alirt/images/voronoi/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   xyzimage.colortablefilename=sysfunc::get_cplusplusrootdir()
      +"alirt/colortables/colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(NEGATIVEINFINITY);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;
   
//   int nsites=6;
//   int nsites=10;
   int nsites=20;
//   cout << "Enter number of sites:" << endl;
//   cin >> nsites;
   myvector site[nsites];

/*
   string filename="sites.txt";
   ofstream sitestream;
   openfile(filename,sitestream);
   numrec::init_time_based_seed();
   double scale=10;	// meters
   for (int i=0; i<nsites; i++)
   {
      double random_number1=scale*2*(nrfunc::ran1()-0.5);
      double random_number2=scale*2*(nrfunc::ran1()-0.5);
      site[i]=myvector(random_number1,random_number2);
      sitestream << "(" << random_number1 << "," << random_number2 
                 << ")" << endl;
   }
   closefile(filename,sitestream);
*/

/*
   site[0]=myvector(-10.3,9.2);
   site[1]=myvector(0.2,7.3);
   site[2]=myvector(10.11,10.33);
   site[3]=myvector(-9.23,-8.45);
   site[4]=myvector(1.33,-7.83);
   site[5]=myvector(11.011,-11.01);
*/

/*
   site[0]=myvector(-1.45093,2.24683);
   site[1]=myvector(2.88523,2.55114);
   site[2]=myvector(-3.06403,3.21962);
   site[3]=myvector(-1.77054,2.41086);
   site[4]=myvector(-0.176528,0.288212);
   site[5]=myvector(-3.0856,4.13784);
   site[6]=myvector(-3.96284,-3.50267);
   site[7]=myvector(-4.67862,2.70835);
   site[8]=myvector(0.328086,4.41524);
   site[9]=myvector(-3.54043,-4.11237);
*/

   site[0]=myvector(3.89082,7.50446);
   site[1]=myvector(9.86439,8.26385);
   site[2]=myvector(0.933532,-5.61408);
   site[3]=myvector(1.56889,-0.0503132);
   site[4]=myvector(-9.57009,-4.30894);
   site[5]=myvector(-0.299827,8.76095);
   site[6]=myvector(-9.48171,5.20348);
   site[7]=myvector(7.44078,-9.22343);
   site[8]=myvector(3.43998,7.5054);
   site[9]=myvector(0.589812,4.22743);
   site[10]=myvector(0.799702,3.44581);
   site[11]=myvector(5.49851,6.5087);
   site[12]=myvector(1.80091,-2.97681);
   site[13]=myvector(-9.76876,-1.53231);
   site[14]=myvector(0.180447,-4.92939);
   site[15]=myvector(-7.04221,4.43398);
   site[16]=myvector(-2.84123,9.3725);
   site[17]=myvector(-3.60454,-3.50188);
   site[18]=myvector(8.35617,4.35434);
   site[19]=myvector(-8.27695,-5.55224);

// Generate and draw Delaunay triangles:

   Linkedlist<polygon>* delaunay_triangle_list_ptr=
      voronoifunc::generate_Delaunay_triangle_list(nsites,site);

   Mynode<polygon>* currnode_ptr=delaunay_triangle_list_ptr->get_start_ptr();
   int ntriangle=1;
   while (currnode_ptr != NULL)
   {
      double curr_intensity=0.1*modulo(1+3*ntriangle,11);
      polygon curr_triangle=currnode_ptr->get_data();
      cout << "ntriangle = " << ntriangle << endl;
      cout << "curr_triangle = " << curr_triangle << endl;
      cout << "=================================================" << endl;
      drawfunc::color_polygon_interior(
         curr_triangle,curr_intensity,ztwoDarray_ptr);
      drawfunc::draw_polygon(curr_triangle,colorfunc::white,ztwoDarray_ptr);
      currnode_ptr=currnode_ptr->get_nextptr();
      ntriangle++;
   }
   delete delaunay_triangle_list_ptr;

// Draw original site locations:
  
//   double radius=0.4;
//   double radius=0.3;
   double radius=0.2;
//   double radius=0.1;
   for (int i=0; i<nsites; i++)
   {
      drawfunc::draw_hugepoint(site[i],radius,1000,ztwoDarray_ptr);
   }

   drawfunc::draw_axes(colorfunc::red,ztwoDarray_ptr);
   xyzimage.writeimage(
      "delaunay_triangles",ztwoDarray_ptr,false,ladarimage::p_data);
}
