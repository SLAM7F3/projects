// ==========================================================================
// Program FLIGHT_PATH_ORIENTATION reads in raw ALIRT xyzp data.  It
// fits a data bounding box parallelogram to the data and extracts its
// dominant symmetry direction.  We take this direction to correspond
// to the flight path orientation relative to a north-east coordinate
// system.

// We cluged together this little utility program in Aug 04 for IED
// data reduction purposes.  IED data tends to be in long, narrow
// strips.  In order to cut down on wasted storage space in twoDarrays
// such as z2Darray and p2Darray, we want to reorient the canonical x
// and y axes so that they are respectively aligned with the flight
// path's along-track and cross-track directions.  This program
// supplies the angle by which the x and y coordinates in raw ALIRT
// IED data need to be COUNTER-rotated so that they'll line up with
// the flight path.

// ==========================================================================
// Last updated on 8/11/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "ladar/urbanfuncs.h"
#include "ladar/urbanimage.h"

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

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read contents of raw binary xyzp file:
   
   urbanimage cityimage;
//    cityimage.set_public_software(true);

// Chimney footprint dimensions:
//   const double delta_x=0.3;	// meters
//   const double delta_y=0.3;	// meters
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(
      delta_x,delta_y,false,true,false,false,0);
   cityimage.compute_data_bbox(cityimage.z2Darray_ptr,false,false);
//   cout << "data bbox = " << *cityimage.get_data_bbox_ptr() << endl;

   threevector e_hat;
   if (cityimage.get_data_bbox_ptr()->get_width() < 
       cityimage.get_data_bbox_ptr()->get_length())
   {
      e_hat=cityimage.get_data_bbox_ptr()->get_lhat();
   }
   else
   {
      e_hat=cityimage.get_data_bbox_ptr()->get_what();
   }
   double theta=atan2(e_hat.e[1],e_hat.e[0]);
   cout << "Flight path direction vector = " << e_hat << endl;
   cout << "Flight path orientation angle (degs) = " << theta*180/PI << endl;
}


