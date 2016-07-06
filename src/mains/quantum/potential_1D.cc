// ==========================================================================
// Program POTENTIAL_1D generates metafile output displaying the 1D
// potential as a function of some independent variable.

// NOTE: Be sure to make necessary alterations within
// quantum_1Dwavefunction::potential() involving potential_param
// member variables !!

// ==========================================================================
// Last updated on 3/23/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "quantum/quantum_1Dwavefunction.h"
#include "general/sysfuncs.h"

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
   int ninputlines,currlinenumber;
   string inputline[200];

//   sysfunc::clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

   quantum_1Dwavefunction psi;
//   psi.viewgraph_mode=true;
   psi.plot_potential("potential");
}






