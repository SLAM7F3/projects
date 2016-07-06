// ==========================================================================
// Program POTENTIAL_2D_phiE generates a movie displaying the QFP
// potential as a parametric function of phiE.

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
#include "image/twoDarray.h"

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
   
   const double phiE_init=0;
   const double phiE_final=-180*PI/180;
//   const double dphiE=-90*PI/180;
   const double dphiE=-15*PI/180;
   
   bool input_param_file;
   int ninputlines;
   string basefilename,inputline[200];

   sysfunc::clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

   quantum_2Dwavefunction psi;
   int nphiE_bins=static_cast<int>(fabs((phiE_final-phiE_init)/dphiE)+1);
   for (int n=0; n<nphiE_bins; n++)
   {
      double phiE=phiE_init+n*dphiE;
      cout << "n = " << n << " phiE = " << phiE*180/PI << endl;
      psi.potential_param[3]=phiE;
      psi.plot_potential(n);
   }
   outputfunc::generate_animation_script(
      nphiE_bins,"potential",psi.imagedir,"show_potentials");
}



