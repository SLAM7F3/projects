// ==========================================================================
// Program APD
// ==========================================================================
// Last updated on 3/28/04
// ==========================================================================

#include <iostream>
#include <new>
#include "math/basic_math.h"
#include "filter/filterfuncs.h"
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
   
   double alpha,sigma;
   cout << "Enter APD rise-time parameter alpha:" << endl;
   cin >> alpha;
   cout << "Enter APD decay time constant sigma:" << endl;
   cin >> sigma;
   double raw_kinky_value[waveform::N];
   for (int n=0; n<waveform::N; n++)
   {
      double t=w.get_tlo()+n*dt;
      raw_kinky_value[n]=0;
      if (t >= 0)
      {
         raw_kinky_value[n]=pow(t,alpha)*exp(-0.5*sqr(t/sigma));
      }
   }

// Smooth modeled APD waveform to remove artificial kinks:

   int nhbins=100;
   double small_h[nhbins];
   double sigma_smooth=0.5;	// sec
   filterfunc::gaussian_filter(nhbins,dt,sigma_smooth,small_h);
   double raw_value[waveform::N];
   filterfunc::brute_force_filter(
      waveform::N,nhbins,raw_kinky_value,small_h,raw_value);
   for (int n=0; n<waveform::N; n++)
   {
      w.set_value(n,raw_value[n]);
   }
   
   w.init_fftw();
   w.fourier_transform(w.get_value(),w.get_tilde());
   double energy=w.compute_energy();

// Trial high-pass filter:

   waveform h(dt,t_lo);
   
   double tau,beta;
   cout << "Enter filter's tau = 1/beta parameter:" << endl;
   cin >> tau;
   beta=1/tau;
//   cout << "Enter filter's beta parameter:" << endl;
//   cin >> beta;
   for (int n=0; n<waveform::N; n++)
   {
      double freq=h.get_freqlo()+n*h.get_deltafreq();
//      double tilde=sqr(freq);
      double tilde=-sqr(freq)/(sqr(freq)+sqr(beta));
      h.set_tilde(n,tilde);
   }

   h.init_fftw();
   h.inverse_fourier_transform(h.get_tilde(),h.get_value());

// Closed-form high-pass filter in time domain:

   waveform hclose(dt,t_lo);
   for (int n=0; n<waveform::N; n++)
   {
      double t=w.get_tlo()+n*dt;
      double arg=-2*PI*beta*fabs(t);
      double prefactor=PI*beta;
      hclose.set_value(n,prefactor*exp(arg));
   }

// Multiply together input signal and filter in frequency domain:

   waveform g(dt,t_lo);
   for (int n=0; n<waveform::N; n++)
   {
      complex curr_prod=h.get_tilde(n)*w.get_tilde(n);
      g.set_tilde(n,curr_prod);
   }
   g.init_fftw();
   g.inverse_fourier_transform(g.get_tilde(),g.get_value());

// Renormalize filtered output so that its energy equals that of the
// raw input signal:

   double altered_energy=g.compute_energy();
   g.renormalize_function(sqrt(energy/altered_energy));
   double energy_final=g.compute_energy();

// Apply threshold to filtered output:

   for (int n=0; n<waveform::N; n++)
   {
      if (g.get_value(n).getreal() < 0) g.set_value(n,0);
   }

// Plot raw and filtered APD response functions:

   const double display_time_frac=0.03;

   metafile* metafile_ptr=new metafile;
   plotfunc::plot_time_func_real(
      w,metafile_ptr,"raw_APD_time",display_time_frac);
   plotfunc::plot_freq_func_magnitude(w,metafile_ptr,"raw_APD_freq");

   plotfunc::plot_freq_func_magnitude(h,metafile_ptr,"filter_freq");
   plotfunc::plot_time_func_real(
      h,metafile_ptr,"filter_time",display_time_frac);
   plotfunc::plot_time_func_real(
      hclose,metafile_ptr,"closed_filter_time",display_time_frac);

   plotfunc::plot_freq_func_magnitude(g,metafile_ptr,"filtered_APD_freq");    
   plotfunc::plot_time_func_real(
      g,metafile_ptr,"filtered_APD_time_real",display_time_frac);
   plotfunc::plot_time_func_magnitude(
      g,metafile_ptr,"filtered_APD_time_mag",display_time_frac);
  plotfunc::plot_time_func_phase(
      g,metafile_ptr,"filtered_APD_time_phase",display_time_frac);
   delete metafile_ptr;

   cout << "Raw signal's energy = " << energy << endl;
   cout << "Filtered signal's energy = " << energy_final << endl;
}





