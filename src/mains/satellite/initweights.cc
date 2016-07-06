// ==========================================================================
// Program INITWEIGHTS reads in a satellite model XYZP file in which
// the p values are irrelevant.  This program simply generates an
// output binary file "weight.init" containing as many unity valued
// floats as there are XYZP points.  This trivial weight file is
// needed as initial input by program COMPSAT.
// ==========================================================================
// Last updated on 3/28/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// First read in satellite model surface normal information from a
// specialized XYZP point file:

   const double null_weight=-1;

   string xyzp_filename;
   cout << "Enter initial XYZP file containing irrelevant p values:"
        << endl;
   cin >> xyzp_filename;
   string suffix=stringfunc::suffix(xyzp_filename);
   vector<fourvector>* XYZP_ptr;
   if (suffix=="xyzp" || suffix=="fxyz")
   {
      XYZP_ptr=xyzpfunc::read_xyzp_float_data(xyzp_filename);
   }

// Instantiate STL vector to hold new weight information:

   vector<float>* weights_ptr=new vector<float>;
   for (int i=0; i<XYZP_ptr->size(); i++)
   {
      weights_ptr->push_back(null_weight);
   } // loop over index i labeling XYZP points
   
   string weights_filename="weights.null";
   filefunc::deletefile(weights_filename);
   xyzpfunc::write_p_data(weights_filename,weights_ptr,false);

   delete XYZP_ptr;
   delete weights_ptr;
}
