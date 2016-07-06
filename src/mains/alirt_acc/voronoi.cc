// ==========================================================================
// Program VORONOI
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 8/5/06
// ==========================================================================

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <new>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include "urban/building.h"
#include "delaunay/delaunay.h"
#include "image/drawfuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "math/threevector.h"
#include "numrec/nrfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "urban/urbanimage.h"
#include "geometry/voronoifuncs.h"

// ==========================================================================
int main(int argc, char* argv[])
// ==========================================================================
{
   using std::ostream;
   using std::ofstream;
   using std::cin;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
//   const double max_x=10;  // meters
//   const double max_y=10;
//   const double max_x=20;  // meters
//   const double max_y=20;
   const double max_x=30;  // meters
   const double max_y=30;
	
   urbanimage xyzimage(nxbins,nybins);   
   xyzimage.set_z2Darray_orig_ptr(new twoDarray(nxbins,nybins));
   xyzimage.set_z2Darray_ptr(new twoDarray(nxbins,nybins));
   
// Initialize image parameters:

   xyzimage.set_imagedir(sysfunc::get_projectsrootdir()+
      "src/mains/alirt_acc/images/voronoi/");
   filefunc::dircreate(xyzimage.get_imagedir());
   xyzimage.set_classified(false);
   xyzimage.set_title("Simulated Ladar Image");
   xyzimage.set_colortable_filename(sysfunc::get_projectsrootdir()
      +"src/mains/alirt_acc/colortables/colortable.image");
   xyzimage.get_z2Darray_orig_ptr()->init_coord_system(max_x,max_y);
   xyzimage.get_z2Darray_ptr()->init_coord_system(max_x,max_y);
   xyzimage.get_z2Darray_ptr()->initialize_values(NEGATIVEINFINITY);
   twoDarray* ztwoDarray_ptr=xyzimage.get_z2Darray_ptr();
   
   int nsites=5;
//   int nsites=10;
//   int nsites=20;
//   cout << "Enter number of sites:" << endl;
//   cin >> nsites;
   vector<threevector> site(nsites);

/*
   string filename="sites.txt";
   ofstream sitestream;
   openfile(filename,sitestream);

   numrec::init_time_based_seed();
   double scale=5;	// meters
   for (int i=0; i<nsites; i++)
   {
      double random_number1=scale*2*(nrfunc::ran1()-0.5);
      double random_number2=scale*2*(nrfunc::ran1()-0.5);
      site[i]=threevector(random_number1,random_number2);
      sitestream << "(" << random_number1 << "," << random_number2 
                 << ")" << endl;
   }
   closefile(filename,sitestream);
*/

/*
   site[0]=threevector(-10,9);
   site[1]=threevector(0,7);
   site[2]=threevector(10,10);
   site[3]=threevector(-9,-8);
   site[4]=threevector(1,-7);
   site[5]=threevector(11,-11);
*/

/*
   site[0]=threevector(-10.3,9.2);
   site[1]=threevector(0.2,7.3);
   site[2]=threevector(10.11,10.33);
   site[3]=threevector(-9.23,-8.45);
   site[4]=threevector(1.33,-7.83);
   site[5]=threevector(11.011,-11.01);
*/

/*
   site[0]=threevector(-1.45093,2.24683);
   site[1]=threevector(2.88523,2.55114);
   site[2]=threevector(-3.06403,3.21962);
   site[3]=threevector(-1.77054,2.41086);
   site[4]=threevector(-0.176528,0.288212);
   site[5]=threevector(-3.0856,4.13784);
   site[6]=threevector(-3.96284,-3.50267);
   site[7]=threevector(-4.67862,2.70835);
   site[8]=threevector(0.328086,4.41524);
   site[9]=threevector(-3.54043,-4.11237);
*/

   site[0]=threevector(14.0075,24.6186);
   site[1]=threevector(-2.55159,14.6328);
   site[2]=threevector(-10.3838,-3.00344);
   site[3]=threevector(7.54999,-8.51816);
   site[4]=threevector(19.63,6.45371);

/*
   site[0]=threevector(3.89082,7.50446);
   site[1]=threevector(9.86439,8.26385);
   site[2]=threevector(0.933532,-5.61408);
   site[3]=threevector(1.56889,-0.0503132);
   site[4]=threevector(-9.57009,-4.30894);
   site[5]=threevector(-0.299827,8.76095);
   site[6]=threevector(-9.48171,5.20348);
   site[7]=threevector(7.44078,-9.22343);
   site[8]=threevector(3.43998,7.5054);
   site[9]=threevector(0.589812,4.22743);
   site[10]=threevector(0.799702,3.44581);
   site[11]=threevector(5.49851,6.5087);
   site[12]=threevector(1.80091,-2.97681);
   site[13]=threevector(-9.76876,-1.53231);
   site[14]=threevector(0.180447,-4.92939);
   site[15]=threevector(-7.04221,4.43398);
   site[16]=threevector(-2.84123,9.3725);
   site[17]=threevector(-3.60454,-3.50188);
   site[18]=threevector(8.35617,4.35434);
   site[19]=threevector(-8.27695,-5.55224);
*/

   Linkedlist<polygon>* voronoi_region_list_ptr=
      voronoifunc::generate_voronoi_regions(site);

   cout << "voronoi_regions = " << *voronoi_region_list_ptr << endl;

   exit(-1);
   
// Draw Voronoi regions:

   Mynode<polygon>* curr_node_ptr=voronoi_region_list_ptr->get_start_ptr();
   int npoly=1;
   while (curr_node_ptr != NULL)
   {
      double curr_intensity=0.1*modulo(1+5*npoly,11);
//      double curr_intensity=0.1*modulo(npoly,11);
      polygon curr_poly=curr_node_ptr->get_data();
      cout << "npoly = " << npoly << endl;
      cout << "curr_poly = " << curr_poly << endl;
      cout << "=================================================" << endl;
      drawfunc::color_polygon_interior(
         curr_poly,curr_intensity,ztwoDarray_ptr);
      drawfunc::draw_polygon(curr_poly,colorfunc::white,ztwoDarray_ptr);
      curr_node_ptr=curr_node_ptr->get_nextptr();
      npoly++;
   }
   delete voronoi_region_list_ptr;

// Draw original site locations:
  
   double radius=0.6;
//   double radius=0.4;
//   double radius=0.2;
   for (int i=0; i<nsites; i++)
   {
      drawfunc::draw_hugepoint(site[i],radius,1000,ztwoDarray_ptr);
   }

   drawfunc::draw_axes(colorfunc::red,ztwoDarray_ptr,threevector(0,0,0));
   xyzimage.writeimage(
      "voronoi_regions",ztwoDarray_ptr,false,ladarimage::p_data);
}

