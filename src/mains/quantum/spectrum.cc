// ==========================================================================
// Program SPECTRUM computes the lowest lying energy eigenvalues for
// some user specified potential as a function of some potential
// parameter.  It then generates metafile output which displays the
// energy eigenvalues as horizontal bars lying across the potential
// function.

// NOTE: Be sure to make necessary alterations within
// quantum_1Dwavefunction::potential() involving potential_param
// member variables !!
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
   string inputline[200];
   quantumarray spectrum_dataarray,efunc_dataarray,spectrum_efunc_dataarray;

   sysfunc::clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   int currlinenumber=0;

   quantum_1Dwavefunction psi;
//   psi.viewgraph_mode=true;

   int n_energystates=psi.select_n_energystates(
      input_param_file,inputline,currlinenumber);
   twoDarray spectrum_twoDarray(n_energystates+1,psi.nxbins);
   twoDarray efunc_twoDarray(n_energystates,psi.nxbins);

   const int n_param_steps=5;
//   const int n_param_steps=21;
//   const int n_param_steps=10;
   twoDarray evalues_twoDarray(n_energystates,n_param_steps);

// ------------------------------------------------------------------------
// Loop over potential parameter starts here:

// Calculate low lying energy eigenstates:

   const double param_lo=179.25*PI/180;
   const double param_hi=180*PI/180;
   const double TINY_NEGATIVE=-0.001;
  
   psi.start_processing_time=time(NULL);
   double dparam,dlog_param;
   if (n_param_steps > 1)
   {
      dparam=(param_hi-param_lo)/(n_param_steps-1);
//      dlog_param=(log10(param_hi)-log10(param_lo))/(n_param_steps-1);
   }
   else
   {
      dparam=0;
//      dlog_param=0;
   }

   psi.save_eigenfunction=false;
   for (int n=0; n<n_param_steps; n++)
   {
      double curr_parameter=param_lo+n*dparam;
//      curr_parameter=pow(10,log10(param_lo)+n*dlog_param);
      psi.potential_param[2]=curr_parameter;
//      psi.potential_param[0]=curr_parameter;

      outputfunc::newline();
      cout << "Parameter iteration = " << n+1 << " out of " << n_param_steps
           << endl;
//      cout << "Current parameter value = " << curr_parameter << endl;
      cout << "Current parameter value = " << curr_parameter*180/PI << endl;
      outputfunc::newline();

      psi.project_low_energy_states(n_energystates);
      psi.prepare_spectrum_data(n_energystates,spectrum_twoDarray,
                                efunc_twoDarray);

      for (int m=0; m<n_energystates; m++)
      {
         evalues_twoDarray.put(m,n,spectrum_twoDarray.get(m+1,0));
      }

//      spectrum_dataarray=psi.plot_spectrum(n_energystates,spectrum_twoDarray);
//      meta_to_jpeg(spectrum_dataarray.datafilenamestr);
//      gzip_file(spectrum_dataarray.datafilenamestr+".meta");

      spectrum_efunc_dataarray=psi.plot_spectrum_and_efuncs(
         n_energystates,spectrum_twoDarray,efunc_twoDarray,n);
      filefunc::meta_to_jpeg(spectrum_efunc_dataarray.datafilenamestr);
      filefunc::gzip_file(spectrum_efunc_dataarray.datafilenamestr+".meta");

   } // loop over index n labeling potential parameter value

// Write out energy eigenvalues as a function of potential parameter
// on logarithmic scale:

//   dataarray evalues(log10(param_lo),dlog_param,evalues_twoDarray);
   dataarray evalues(param_lo*180/PI,dparam*180/PI,evalues_twoDarray);
   double maxval,minval;
   evalues.find_max_min_vals(maxval,minval);

   evalues.datafilenamestr=psi.imagedir+"spectrum_vs_param";
   evalues.title="SQUID energy spectra";
//   evalues.log_xaxis=true;
   evalues.xmin=param_lo*180/PI;
   evalues.xmax=param_hi*180/PI;
//   evalues.xlabel="E^-j^n";
   evalues.xlabel="Phase shift ^g\152^u^-0^n";
   evalues.xtic=10;
   evalues.xsubtic=2;
   evalues.yplotminval=TINY_NEGATIVE;
   evalues.yplotmaxval=1.2*maxval;
   evalues.ylabel="Energy eigenvalue/[(2e)^+2^n/(2C)]";
   evalues.ytic=2000;
   evalues.ysubtic=2000;

   filefunc::openfile(evalues.datafilenamestr+".meta",evalues.datastream);
   evalues.singletfile_header();
   evalues.writedataarray();
   filefunc::closefile(evalues.datafilenamestr+".meta",evalues.datastream);
   filefunc::meta_to_jpeg(evalues.datafilenamestr);
   filefunc::gzip_file(evalues.datafilenamestr+".meta");

   outputfunc::generate_animation_script(
      n_param_steps,"spectrum_efunc",psi.imagedir,"show_spectra_efuncs");
   psi.summarize_results(0);
}






