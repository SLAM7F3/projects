// ==========================================================================
// Program WAVETEST
// ==========================================================================
// Last updated on 3/26/04
// ==========================================================================

#include <iostream>
#include <new>
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
   waveform w(dt,t_lo);
   waveform w2(dt,t_lo);
   cout << "t_lo = " << w.get_tlo() << endl;
   cout << "t_hi = " << w.get_thi() << endl;
   cout << "dt = " << w.get_deltat() << endl;
   cout << "freq_lo = " << w.get_freqlo() << endl;
   cout << "freq_hi = " << w.get_freqhi() << endl;
   cout << "dfreq = " << w.get_deltafreq() << endl;

// Modeled APD waveform:
   
   double sigma;
   cout << "Enter sigma:" << endl;
   cin >> sigma;
   for (int n=0; n<waveform::N; n++)
   {
      double t=w.get_tlo()+n*dt;
      double value=(sqr(t)-sqr(sigma))/(sqr(sqr(sigma)))
         *exp(-0.5*sqr(t/sigma));
      w.set_value(n,value);
   }

   w.init_fftw();
   w.fourier_transform(w.get_value(),w.get_tilde());

   metafile* metafile_ptr=new metafile;
   double display_time_frac=0.05;
   plotfunc::plot_time_func_real(
      w,metafile_ptr,"filter_time",display_time_frac);
   plotfunc::plot_freq_func_magnitude(w,metafile_ptr,"filter_freq");

   delete metafile_ptr;
}


