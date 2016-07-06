// ==========================================================================
// Program PERTURB

// ==========================================================================
// Last updated on 11/21/04
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "ladar/roadfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

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
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   bool input_param_file;
   vector<string> filename;
   filefunc::parameter_input(argc,argv,input_param_file,filename);

   myvector e_hat(1,0);
   myvector origin(0,0);
   double displacement_dist=10;	// meters
   vector<myvector> testpnts=roadfunc::generate_intersection_testpnts(
      displacement_dist,origin,e_hat);
   templatefunc::printVector(testpnts);
  
}
