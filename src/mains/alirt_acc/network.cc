// ==========================================================================
// Program NETWORK is a little algorithm development tool which we use
// to experiment with basic network searching utilities.
// ==========================================================================
// Last updated on 1/17/05
// ==========================================================================

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <new>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include "ladar/ladarfuncs.h"
#include "network/Network.h"
#include "ladar/roadfuncs.h"
#include "general/outputfuncs.h"
#include "ladar/roadpoint.h"
#include "general/sysfuncs.h"
#include "datastructures/Triple.h"
#include "image/TwoDarray.h"
#include "ladar/urbanimage.h"

// ==========================================================================
int main(int argc, char* argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
   const double max_x=7;  // meters
   const double max_y=7;
//   const double max_x=10;  // meters
//   const double max_y=10;
//   const double max_x=20;  // meters
//   const double max_y=20;
	
   urbanimage cityimage(nxbins,nybins);   
   cityimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   cityimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   
// Initialize image parameters:

   cityimage.imagedir=filefunc::get_pwd()+"images/network/";
   filefunc::dircreate(cityimage.imagedir);
   cityimage.classified=false;
   cityimage.title="Simulated Ladar Image";
   cityimage.colortablefilename=filefunc::get_pwd()+
      "colortables/colortable.image";
   cityimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   cityimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   cityimage.z2Darray_ptr->initialize_values(NEGATIVEINFINITY);
   twoDarray* ztwoDarray_ptr=cityimage.z2Darray_ptr;

   cityimage.generate_roadpoints_network();

//   roadfunc::insert_sites_at_netlink_intersections(
//      cityimage.get_roadpoints_network_ptr());

   double min_distance=0.5;	// meter
   cityimage.get_roadpoints_network_ptr()->merge_close_sites_and_links(
      min_distance);

   roadfunc::draw_road_network(
      cityimage.get_roadpoints_network_ptr(),ztwoDarray_ptr);
   cityimage.writeimage(
      "road_network",ztwoDarray_ptr,false,ladarimage::p_data);
}


