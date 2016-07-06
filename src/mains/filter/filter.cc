// ==========================================================================
// Program WAVETEST
// ==========================================================================
// Last updated on 3/26/04
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "plot/metafile.h"
#include "myconstants.h"
#include "plot/plotfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/waveform.h"

using std::cin;
using std::cout;
using std::endl;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
  
   double dt=0.1;	// sec
   double t_lo=-waveform::N/2*dt;
   waveform h(dt,t_lo);
   
   double alpha;
   cout << "Enter alpha:" << endl;
   cin >> alpha;
   for (int n=0; n<waveform::N; n++)
   {
      double freq=h.get_freqlo()+n*h.get_deltafreq();
      double tilde=sqr(freq)/(sqr(freq)+sqr(alpha));
//      double tilde=1/(sqr(freq)-1);
      h.set_tilde(n,tilde);
   }

   h.init_fftw();
   h.inverse_fourier_transform(h.get_tilde(),h.get_value());


   const double display_time_frac=0.03;

   metafile* metafile_ptr=new metafile;
   plotfunc::plot_time_func(h,metafile_ptr,"filter_time",display_time_frac);
   plotfunc::plot_freq_func_magnitude(h,metafile_ptr,"filter_freq_mag");
   plotfunc::plot_freq_func_phase(h,metafile_ptr,"filter_freq_phase");

   delete metafile_ptr;

   h.compute_energy();
}


