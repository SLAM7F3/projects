// ==========================================================================
// Program ROTATE_RAW_DATA reads in raw ALIRT xyzp data.  It queries
// the user to enter an angle by which all x and y coordinates should
// be rotated.  (The angle for which the x and y axes become aligned
// with the flight path is calculated in program
// FLIGHT_PATH_ORIENTATION.)  After rotating the data, this program
// also performs a "snow removal" operation.  The rotated and
// trivially cleaned data is written to a binary "no_snow" xyzp output
// file which can be used for subsequent image processing.
// ==========================================================================
// Last updated on 5/22/05
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
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"

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
   unsigned int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read contents of raw binary xyzp file:
   
   ladarimage iedimage;
//    iedimage.set_public_software(true);

// Chimney footprint dimensions:
//   const double delta_x=0.3;	// meters
//   const double delta_y=0.3;	// meters
   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
   iedimage.initialize_image(input_param_file,inputline,currlinenumber);

   double theta;
   outputfunc::newline();
   cout << "Enter angle (in degs) by which x and y coordinates" << endl;
   cout << "be rotated so that they'll be respectively aligned" << endl;
   cout << "with the along-track and cross-track flight path directions"
        << endl << endl;
   cout << "Angle equals NEGATIVE of result returned by program" << endl;
   cout << "flight_path_orientation.cc." << endl;

   cin >> theta;
   theta *= PI/180;
   
   iedimage.parse_and_store_input_data(
      delta_x,delta_y,false,true,false,true,theta);
}


