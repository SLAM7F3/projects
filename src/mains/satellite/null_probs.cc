// ==========================================================================
// Program NULL_PROBS resets every p value within an input XYZP point
// cloud equal to some user specified nominal value.  We wrote this
// little utility as a prelude to multi-imagery composition.
// ==========================================================================
// Last updated on 3/5/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
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
   outputfunc::newline();

   string xyzp_filename;
   cout << "Enter satellite model XYZP filename:" << endl;
   cin >> xyzp_filename;
   vector<fourvector>* XYZP_ptr=xyzpfunc::read_xyzp_float_data(xyzp_filename);

   double null_value=-1;
//   cout << "Enter null probability value:" << endl;
//   cin >> null_value;

   xyzpfunc::reset_null_p_values(2*POSITIVEINFINITY,null_value,XYZP_ptr);
   
   string nulled_xyzp_filename="null_probs.xyzp";
   filefunc::deletefile(nulled_xyzp_filename);
   xyzpfunc::write_xyzp_data(nulled_xyzp_filename,XYZP_ptr,false);
   delete XYZP_ptr;
}
