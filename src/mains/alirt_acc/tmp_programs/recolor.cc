// ==========================================================================
// Program RECOLOR
// ==========================================================================
// Last updated on 6/23/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "connectfuncs.h"
#include "draw3Dfuncs.h"
#include "featurefuncs.h"
#include "general/filefuncs.h"
#include "groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "imagefuncs.h"
#include "ladarfuncs.h"
#include "general/outputfuncs.h"
#include "recursivefuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/twoDarray.h"
#include "urbanfuncs.h"
#include "urbanimage.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_cplusplusrootdir()+"alirt/recolor.logfile";

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// ==========================================================================
// Raw image initialization
// ==========================================================================

// Read contents of binary xyzp file into 1D x, y, z and p arrays:

   cout << "Enter filename for refined features map:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(false,false,false);
   cityimage.compute_data_bbox(cityimage.z2Darray_ptr,false);

   featurefunc::remove_feature_holes_from_feature_map(
      urbanimage::building_sentinel_value,cityimage.get_p2Darray_ptr(),
      cityimage.z2Darray_ptr,"building",cityimage.imagedir);
   featurefunc::remove_feature_holes_from_feature_map(
      urbanimage::grass_sentinel_value,cityimage.get_p2Darray_ptr(),
      cityimage.z2Darray_ptr,"grass",cityimage.imagedir);
   featurefunc::remove_isolated_height_outliers_from_feature_map(
      cityimage.get_data_bbox_ptr(),
      cityimage.z2Darray_ptr,cityimage.get_p2Darray_ptr());

   featurefunc::recolor_feature_pixels(
      urbanimage::tree_sentinel_value,urbanimage::annot2_value,
      cityimage.get_p2Darray_ptr());

   string recolored_features_filenamestr=cityimage.imagedir+
      "recolored_features.xyzp";
   ladarfunc::write_xyzp_data(
      cityimage.z2Darray_ptr,cityimage.get_p2Darray_ptr(),
      recolored_features_filenamestr,false);
   
//   filefunc::gunzip_file(recolored_features_filenamestr);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      cityimage.get_p2Darray_ptr(),recolored_features_filenamestr);
//   filefunc::gzip_file(recolored_features_filenamestr);
}


