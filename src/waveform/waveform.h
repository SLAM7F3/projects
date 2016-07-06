// ==========================================================================
// Header file for WAVEFORM class 
// ==========================================================================
// Last modified on 3/26/04
// ==========================================================================

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <fftw.h>
#include <iostream>
#include "math/complex.h"

class waveform
{
  private:

   void initialize_member_objects();
   void allocate_member_objects();
   void docopy(const waveform& w);
   void initialize_time_and_frequency_domains(double dt,double tlo);

   double t_lo,t_hi,delta_t;
   double freq_lo,freq_hi,delta_freq;
   double *T,*Freq;
   complex *value,*tilde;

// theforward and thebackward are pointers to fftw_data structures:

   fftw_plan theforward,thebackward;

  public:

// Number of bins in both time and frequency domain arrays.  N must
// ODD for FFT purposes:

   static const int N;
   
// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

   waveform(void);
   waveform(double dt,double tlo=0);
   waveform(const waveform& d);
   virtual ~waveform();
   waveform& operator= (const waveform& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const waveform& d);

// Set & get member functions:
   
   void set_value(int n,const complex& v);
   void set_tilde(int n,const complex& t);
   double get_tlo() const;
   double get_thi() const;
   double get_deltat() const;
   double get_freqlo() const;
   double get_freqhi() const;
   double get_deltafreq() const;
   double get_time(int n) const;
   double get_freq(int n) const;
   complex* get_value();
   complex* get_tilde();
   complex& get_value(int n) const;
   complex& get_tilde(int n) const;

// Fourier transform member functions:

   void init_fftw();
   void fourier_transform(const complex value_in[],complex tilde_out[]);
   void inverse_fourier_transform(
      const complex tilde_in[],complex value_out[]);

   void extremal_time_domain_function_values(
      double& max_value_real,double& min_value_real);
   double max_time_domain_function_magnitude();
   double max_freq_domain_function_magnitude();
   void renormalize_function(const double a);
   double compute_energy();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void waveform::set_value(int n,const complex& v) 
{
   if (n >= 0 && n < N)
   {
      value[n]=v;
   }
   else
   {
      std::cout << "Error in waveform::set_value()" << std::endl;
      std::cout << "n = " << n << " N = " << N << std::endl;
   }
}

inline void waveform::set_tilde(int n,const complex& t) 
{
   if (n >= 0 && n < N)
   {
      tilde[n]=t;
   }
   else
   {
      std::cout << "Error in waveform::set_tilde()" << std::endl;
      std::cout << "n = " << n << " N = " << N << std::endl;
   }
}

inline double waveform::get_tlo() const
{
   return t_lo;
}

inline double waveform::get_thi() const
{
   return t_hi;
}

inline double waveform::get_deltat() const
{
   return delta_t;
}

inline double waveform::get_freqlo() const
{
   return freq_lo;
}

inline double waveform::get_freqhi() const
{
   return freq_hi;
}

inline double waveform::get_deltafreq() const
{
   return delta_freq;
}

inline double waveform::get_time(int n) const
{
   return T[n];
}

inline double waveform::get_freq(int n) const
{
   return Freq[n];
}

inline complex* waveform::get_value() 
{
   return value;
}

inline complex* waveform::get_tilde()
{
   return tilde;
}

inline complex& waveform::get_value(int n) const
{
   return value[n];
}

inline complex& waveform::get_tilde(int n) const
{
   return tilde[n];
}

#endif // datastructures/waveform.h




