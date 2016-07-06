// ==========================================================================
// Program POTENTIAL_2D generates metafile output displaying the 2D
// potential as a function of some independent variable.

// NOTE: Be sure to make necessary alterations within
// quantum_2Dwavefunction::potential() involving potential_param
// member variables !!

// ==========================================================================
// Last updated on 3/23/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "quantum/quantum_2Dwavefunction.h"
#include "general/sysfuncs.h"
#include "datastructures/twoDarray.h"

// ==========================================================================
// Constants
// ==========================================================================

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::cout;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);

   bool input_param_file;
   int ninputlines;
   string basefilename,inputline[200];
   sysfunc::clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   quantum_2Dwavefunction psi;
   psi.plot_potential("potential");
}







