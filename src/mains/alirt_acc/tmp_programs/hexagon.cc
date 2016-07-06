// ==========================================================================
// Program HOUSES is a testing program (as of late Mar 04) which takes
// in a refined features map.  It attempts to extract useful
// orientation information about individual buildings as well as
// topological properties of the road network.
// ==========================================================================
// Last updated on 4/6/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/sysfuncs.h"
#include "genfuncs.h"
#include "general/filefuncs.h"
#include "featurefuncs.h"
#include "connectfuncs.h"
#include "recursivefuncs.h"
#include "general/outputfuncs.h"
#include "imagefuncs.h"
#include "datastructures/twoDarray.h"
#include "urbanimage.h"
#include "ladarfuncs.h"
#include "voronoifuncs.h"
#include "groundfuncs.h"
#include "datastructures/Hashtable.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_cplusplusrootdir()+
      "alirt/buildings.logfile";

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read contents of binary xyzp file into 1D x, y, z and p arrays:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(true,false,false);
//   cityimage.compute_data_bbox(cityimage.z2Darray_ptr,false);

   twoDarray* ztwoDarray_ptr=cityimage.z2Darray_ptr;
   twoDarray* features_twoDarray_ptr=cityimage.get_p2Darray_ptr();
   twoDarray* ftwoDarray_ptr=new twoDarray(features_twoDarray_ptr);

   string features_filename=cityimage.imagedir+"features.xyzp";   
//   ladarfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_twoDarray_ptr,features_filename);

   while(true)
   {
      double radius=5;

      double x,y;
      cout << "Enter x location for hexagon center:" << endl;
      cin >> x;
      cout << "Enter y location for hexagon center:" << endl;
      cin >> y;
      myvector posn(x,y);
      double feature_value=urbanimage::road_sentinel_value;

      features_twoDarray_ptr->copy(ftwoDarray_ptr);
      bool feature_inside_hexagon=featurefunc::feature_nearby(
         radius,posn,feature_value,ftwoDarray_ptr);
      cout << "feature_inside_hexagon = " << feature_inside_hexagon
           << endl;
      cityimage.writeimage(
         "features",ftwoDarray_ptr,false,ladarimage::p_data);
   }
   
   delete ftwoDarray_ptr;

}


