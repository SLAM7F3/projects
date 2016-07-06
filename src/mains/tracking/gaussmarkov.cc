// ==========================================================================
// Program GAUSSMARKOV
// ==========================================================================
// Last updated on 3/22/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "filter/filterfuncs.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();

   double betainv=1.0;
//   cout << "Enter time constant betainv" << endl;
//   cin >> betainv;
   double beta=1.0/betainv;

   double noise_amp=1;
   cout << "Enter noise amplitude" << endl;
   cin >> noise_amp;

   const double dt=0.02;
   double prefactor=exp(-beta*dt);
   const double sigma=1;
   const double T_expt=1.0;
   
   int nbins=T_expt/dt;
   vector<double> sample_time,x_true;
   sample_time.push_back(0);
   x_true.push_back(0);
   
   for (int n=1; n<nbins; n++)
   {
      double t=n*dt;
      sample_time.push_back(t);
      double W=sigma*nrfunc::gasdev();
      x_true.push_back(prefactor*x_true.back()+noise_amp*W);
   }

// Generate metafile plot outputs:

   metafile x_meta;
   string metafile_name="gauss_markov";
   x_meta.set_parameters(
      metafile_name,"Gauss-Markov output vs time",
      "Time","Gauss-Markov output",
      0,sample_time.back(),0.1,0.02,
      -4,4,1,1);
   x_meta.add_extrainfo(
      "Time constant betainv = "+stringfunc::number_to_string(betainv));
   x_meta.add_extrainfo(
      "Gaussian sigma = "+stringfunc::number_to_string(sigma));
   x_meta.add_extrainfo(
      "Noise amplitude = "+stringfunc::number_to_string(noise_amp));

   x_meta.openmetafile();
   x_meta.write_header();
   x_meta.write_curve(sample_time,x_true,colorfunc::red);
   x_meta.closemetafile();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);

}
