// ==========================================================================
// Program POTENTIAL_1D_PHI generates a movie displaying the SQUID
// potential with fixed Ej and BetaL parameters and variable phi0 as a
// function of phase phi.

// NOTE: Be sure to make necessary alterations within
// quantum_1Dwavefunction::potential() involving potential_param
// member variables !!
// ==========================================================================
// Last updated on 3/24/03
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
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

   const double phi0_hi=PI;
   const double phi0_lo=-PI;
//   const double dphi0=90*PI/180;
   const double dphi0=5*PI/180;
//   const double dphi0=2*PI/180;
  
   bool input_param_file;
   int ninputlines;
   string inputline[200];

   sysfunc::clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

   quantum_1Dwavefunction psi;
//   psi.viewgraph_mode=true;

   int nphi_bins=mathfunc::round((phi0_hi-phi0_lo)/dphi0+1);
   for (int n=0; n<nphi_bins; n++)
   {
      double phi0=phi0_lo+n*dphi0;
      cout << "phi0 = " << phi0*180/PI << endl;
      psi.potential_param[2]=phi0;
      psi.plot_potential(n);
   }
   outputfunc::generate_animation_script(nphi_bins,"potential",psi.imagedir,
                                         "show_potentials");
}






