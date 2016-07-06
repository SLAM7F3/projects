// ==========================================================================
// Program SINGLESPECTRUM computes the lowest lying energy eigenvalues
// for some user specified potential.  It then generates metafile
// output which displays the energy eigenvalues as horizontal bars
// lying across the potential function.
// ==========================================================================
// Last updated on 5/22/05
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/quantum_1Dwavefunction.h"
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

   bool input_param_file;
   int ninputlines;
   std::string inputline[200];

//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   int currlinenumber=0;

   quantum_1Dwavefunction psi;
//   psi.viewgraph_mode=true;

// Calculate low lying energy eigenstates:

/*
   cout << "Enter Ej:" << endl;
   cin >> psi.potential_param1;
   cout << "Enter BetaL coefficient:" << endl;
   cin >> psi.potential_param2;
*/
//   double phi0;
//   cout << "Enter phi0 in degs:" << endl;
//   cin >> phi0;
//   psi.potential_param3=phi0*PI/180;

   psi.start_processing_time=time(NULL);
   int n_energystates=psi.select_n_energystates(
      input_param_file,inputline,currlinenumber);
   psi.project_low_energy_states(n_energystates);

   twoDarray spectrum_twoDarray(n_energystates+1,psi.nxbins);
   twoDarray efunc_twoDarray(n_energystates,psi.nxbins);
   psi.prepare_spectrum_data(n_energystates,spectrum_twoDarray,
                             efunc_twoDarray);

/*
   psi.complex_plot_type="energy_spectrum";
   quantumarray spectrum_dataarray=psi.plot_spectrum(n_energystates,spectrum_twoDarray);
   meta_to_jpeg(spectrum_dataarray.datafilenamestr);
   gzip_file(spectrum_dataarray.datafilenamestr+".meta");

   psi.complex_plot_type="real_imag";
   quantumarray efunc_dataarray=psi.plot_eigenfunctions(n_energystates,efunc_twoDarray);
   meta_to_jpeg(efunc_dataarray.datafilenamestr);
   gzip_file(efunc_dataarray.datafilenamestr+".meta");
   psi.summarize_results(0);
*/

   quantumarray spectrum_efunc_dataarray=psi.plot_spectrum_and_efuncs(
      n_energystates,spectrum_twoDarray,efunc_twoDarray,0);
   filefunc::meta_to_jpeg(spectrum_efunc_dataarray.datafilenamestr);
   filefunc::gzip_file(spectrum_efunc_dataarray.datafilenamestr+".meta");
   psi.summarize_results(0);
}


