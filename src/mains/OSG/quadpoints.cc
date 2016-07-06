// ==========================================================================
// Test program QUADPOINTS writes out (0,0,0), (1,0,0), (0,2,0) and
// (0,0,3) to an XYZP file for testing purposes.
// ==========================================================================
// Last updated on 7/14/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/threevector.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   bool input_param_file;
   unsigned int ninputlines;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

// Initialize image parameters:
 
   vector<fourvector> vertex;
//   vertex.push_back(fourvector(0,0,0,0.2));
//   vertex.push_back(fourvector(1,0,0,0.4));
//   vertex.push_back(fourvector(0,2,0,0.6));
//   vertex.push_back(fourvector(0,0,3,0.8));

   vertex.push_back(fourvector(0,0,0,0.2));
   vertex.push_back(fourvector(1,0,1,0.4));
   vertex.push_back(fourvector(1,1,2,0.6));
   vertex.push_back(fourvector(0,1,3,0.8));

   string xyzp_filename="quadpoints.xyzp";
   filefunc::deletefile(xyzp_filename);
   xyzpfunc::write_xyzp_data(xyzp_filename,&vertex,false);

   
   threevector origin(0,0,0);
   threevector X(1,0,0);
   threevector Y(0,2,0);
   threevector Z(0,0,3);


//   draw3Dfunc::draw_line(origin,X,xyzp_filename,0.3);
//   draw3Dfunc::draw_line(origin,Y,xyzp_filename,0.6);
//   draw3Dfunc::draw_line(origin,Z,xyzp_filename,0.9);

}
