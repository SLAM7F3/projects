// ==========================================================================
// Program GRIDALIRTDATA reads in some user-specified ALIRT XYZP file.
// It places the data into regularized ztwoDarray and ptwoDarray.  The
// program then writes out the point clouds corresponding to the
// gridded information to an output "regular.xyzp" file.  Because
// floats rather than doubles are written to the binary output file,
// roundoff error can lead to nontrivial deviations of X and Y value
// separations from deltaX and deltaY.
// ==========================================================================
// Last updated on 12/23/05
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

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
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// ==========================================================================
// Raw image initialization
// ==========================================================================

// Read contents of raw binary xyzp file:
   
   ladarimage xyzpimage;

// Chimney footprint dimensions:

   const double delta_x=0.15;	// meters
   const double delta_y=0.15;	// meters
//   const double delta_x=0.3;	// meters
//   const double delta_y=0.3;	// meters
   xyzpimage.initialize_image(input_param_file,inputline,currlinenumber);

   xyzpimage.parse_and_store_input_data(
      delta_x,delta_y,false,false,false,false,0);

   string xyzp_filename="regular.xyzp";
   xyzpfunc::write_xyzp_data(
      xyzpimage.z2Darray_ptr,xyzpimage.get_p2Darray_ptr(),
      xyzp_filename,true,false);

}


