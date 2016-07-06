// ==========================================================================
// Waveform class member function definitions
// ==========================================================================
// Last modified on 5/23/05; 12/4/10; 3/31/12
// ==========================================================================

#include <string>
#include "math/basic_math.h"
#include "math/constants.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "waveform/waveform.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// Number N of time and frequency domain bins must be odd !

// const int waveform::N=2025; // 2025 = 3^4 * 5^2
const int waveform::N=4095; // 4095 = 3^2 * 5 * 7 * 13

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void waveform::initialize_member_objects()
{
}

void waveform::allocate_member_objects()
{
   T=new double[N];
   Freq=new double[N+1];
   value=new complex[N];
   tilde=new complex[N+1];
}		       

waveform::waveform(void)
{
   initialize_member_objects();
   allocate_member_objects();
}

waveform::waveform(double dt,double tlo)
{
   initialize_member_objects();
   allocate_member_objects();
   initialize_time_and_frequency_domains(dt,tlo);
}

// Copy constructor:

waveform::waveform(const waveform& d)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(d);
}

waveform::~waveform()
{
   delete [] T;
   delete [] Freq;
   delete [] value;
   delete [] tilde;
}

// ---------------------------------------------------------------------
void waveform::docopy(const waveform& d)
{
}	

// Overload = operator:

waveform& waveform::operator= (const waveform& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const waveform& d)
{
   return(outstream);
}

// ---------------------------------------------------------------------
void waveform::initialize_time_and_frequency_domains(double dt,double tlo)
{
   delta_t=dt;
   delta_freq=1.0/double(N*delta_t);
   t_lo=tlo;
   t_hi=t_lo+(N-1)*delta_t;
   freq_lo=-N/2*delta_freq;
   freq_hi=N/2*delta_freq;
   
   for (int i=0; i<N; i++)
   {
      T[i]=t_lo+i*delta_t;
      Freq[i]=freq_lo+i*delta_freq;
      value[i]=0;
      tilde[i]=0;
   }
}

// ==========================================================================
// Fourier transform member functions
// ==========================================================================

// Initialize FFTW algorithm by either calculating FFT weights from
// scratch or else reading in previously computed values from files
// "fftw.forward" and "fftw.backward" within cplusplusrootdir/classes:

// Note: The senses of "theforward" and "thebackward" are adjusted
// here so as to be compatible with Numerical Recipes' Fourier
// transform conventions.

void waveform::init_fftw()
{
   bool save_forward_wisdom=false;
   bool save_backward_wisdom=false;

   string prefix=sysfunc::get_projectsrootdir()+"src/datastructures/";
   string forwardfilenamestr=prefix+"fftw.forward";
   string backwardfilenamestr=prefix+"fftw.backward";
   
// fopen must take a C-style char* string argument rather than a C++
// string class object:

   FILE* forward_wisdomfile=fopen(forwardfilenamestr.c_str(),"r");
   FILE* backward_wisdomfile=fopen(backwardfilenamestr.c_str(),"r");

   if (FFTW_FAILURE==fftw_import_wisdom_from_file(forward_wisdomfile))
   {
      cout << "Forward FFT wisdom file not found." << endl;
      cout << "New wisdom will be generated and saved." << endl;
      outputfunc::newline();
      save_forward_wisdom=true;
   } 
   else 
   {
      fclose(forward_wisdomfile);
   }

   if (FFTW_FAILURE==fftw_import_wisdom_from_file(backward_wisdomfile))
   {
      cout << "Backward FFT wisdom file not found." << endl;
      cout << "New wisdom file will be generated and saved." << endl;
      outputfunc::newline();
      save_backward_wisdom=true;
   } 
   else 
   {
      fclose(backward_wisdomfile);
   }

   cout << "Initializing FFTW arrays:" << endl;
   outputfunc::newline();
   theforward=fftw_create_plan(
      N,FFTW_BACKWARD,FFTW_MEASURE | FFTW_USE_WISDOM);
   thebackward=fftw_create_plan(
      N,FFTW_FORWARD,FFTW_MEASURE | FFTW_USE_WISDOM);

   if (save_forward_wisdom)
   {
      forward_wisdomfile = fopen(forwardfilenamestr.c_str(),"w");
      fftw_export_wisdom_to_file(forward_wisdomfile);
      fclose(forward_wisdomfile);
   }

   if (save_backward_wisdom)
   {
      backward_wisdomfile = fopen(backwardfilenamestr.c_str(),"w");
      fftw_export_wisdom_to_file(backward_wisdomfile);
      fclose(backward_wisdomfile);
   }
}

// ---------------------------------------------------------------------
// Function fourier_transform returns the *true* Fourier transform
// H(f_n) of the time domain signal contained within the complex val
// array.  Recall that the *discrete* fourier transform H_n is related
// to the continuous true Fourier transform H(f_n) by H(f_n) = delta_t
// * H_n where delta_t denotes the sampling time interval.  (See eqn
// (12.1.8) in Numerical Recipes.)  We *do* include a factor of delta
// into the tilde array values which are computed by this routine.
// See eqn (12.1.8) in Numerical Recipes.

void waveform::fourier_transform(
   const complex value_in[],complex tilde_out[])
{
   int i,iskip;

// Wrap input data:

   complex* tmp_value=new complex[N];
   for (i=0; i<N/2+1; i++)
   {
      iskip=i+N/2;
      tmp_value[i]=value_in[iskip];
   }
   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      tmp_value[iskip+1]=value_in[i];
   }

// Introduce arrays in_fftw (and later out_fftw) of type fftw_complex
// in order to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex* in_fftw=new fftw_complex[N];

// Copy real and imaginary time domain values into in_fftw array:

   for (i=0; i<N; i++)
   {
      in_fftw[i].re=tmp_value[i].get_real();
      in_fftw[i].im=tmp_value[i].get_imag();
   }
   delete [] tmp_value;

// Take Fourier transform using fftw:

   fftw_complex* out_fftw=new fftw_complex[N];
   fftw_one(theforward,in_fftw,out_fftw);
   delete [] in_fftw;

   complex* tmp_tilde=new complex[N];
   for (i=0; i<N; i++)
   {
      tmp_tilde[i]=complex(out_fftw[i].re,out_fftw[i].im);
   }
   delete [] out_fftw;

// Unwrap output data.  Multiply *discrete* fourier transform results
// contained within array tmp_tilde by delta_t to turn them into
// *true* continuous fourier transform values.  See eqn (12.1.8) in
// Numerical Recipes.

   for (i=0; i<N/2+1; i++)
   {
      iskip=i+N/2;
      tilde_out[iskip]=delta_t*tmp_tilde[i];
   }
   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      tilde_out[i]=delta_t*tmp_tilde[iskip+1];
   }
   delete [] tmp_tilde;

// To minimize build-up of machine round-off errors, we null out any
// fourier transformed value less than EXTREMELY TINY:

   const double EXTREMELY_TINY=1.0E-10;
   for (i=0; i<N; i++)
   {
      if (fabs(tilde_out[i].get_real()) < EXTREMELY_TINY) 
         tilde_out[i].set_real(0);
      if (fabs(tilde_out[i].get_imag()) < EXTREMELY_TINY) 
         tilde_out[i].set_imag(0);
   }
}

// ---------------------------------------------------------------------
// Function inverse_fourier_transform returns the *true* spatial domain
// inverse Fourier transform h(x) of the 1D momentum space signal
// contained within the complex array tilde.  The number of rows and
// columns within the incoming data array must be ODD!  Recall that
// the *discrete* inverse fourier transform H(n) is related to the
// continuous true inverse Fourier transform h(x) by h(x) =
// H(n)/nxbins.  We *do* include a factor of 1/nxbins into the spatial
// domain value_out array computed by this subroutine.

void waveform::inverse_fourier_transform(
   const complex tilde_in[],complex value_out[])
{
   int i,iskip;

// Wrap input data.  Recall the tilde_in array contains *true*
// continuous fourier transform values.  We must first divide these by
// deltax to convert them into *discrete* fourier transform values
// before calling the inverse FFTW subroutine:

   complex *tmp_tilde=new complex[N];
   for (i=0; i<N/2+1; i++)
   {
      iskip=i+N/2;
      tmp_tilde[i]=tilde_in[iskip]/delta_t;
   }
   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      tmp_tilde[iskip+1]=tilde_in[i]/delta_t;
   }

// Copy real and imaginary momentum space values into in_fftw array:

   fftw_complex *in_fftw=new fftw_complex[N];
   for (i=0; i<N; i++)
   {
      in_fftw[i].re=tmp_tilde[i].get_real();
      in_fftw[i].im=tmp_tilde[i].get_imag();
   }
   delete [] tmp_tilde;

// Take inverse Fourier transform using fftw:

   fftw_complex* out_fftw=new fftw_complex[N];
   fftw_one(thebackward,in_fftw,out_fftw);
   delete [] in_fftw;

   complex* tmp_value=new complex[N];
   for (i=0; i<N; i++)
   {
      tmp_value[i]=complex(out_fftw[i].re,out_fftw[i].im);
   }
   delete [] out_fftw;

// Unwrap output data.  Multiply *discrete* inverse fourier transform
// results 1/N to turn them into *true* continuous inverse fourier
// transform values.  See eqn (12.1.8) in Numerical Recipes.

   for (i=0; i<N/2+1; i++)
   {
      iskip=i+N/2;
      value_out[iskip]=tmp_value[i]/N;
   }
   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      value_out[i]=tmp_value[iskip+1]/N;
   }

// To minimize build-up of machine round-off errors, we null out any 
// inverse fourier transformed value less than EXTREMELY TINY:

   const double EXTREMELY_TINY=1.0E-10;
   for (i=0; i<N; i++)
   {
      if (fabs(value_out[i].get_real()) < EXTREMELY_TINY) 
         value_out[i].set_real(0);
      if (fabs(value_out[i].get_imag()) < EXTREMELY_TINY) 
         value_out[i].set_imag(0);
   }
}

// ==========================================================================
void waveform::extremal_time_domain_function_values(
   double& max_value_real,double& min_value_real)
{
   max_value_real=NEGATIVEINFINITY;
   min_value_real=POSITIVEINFINITY;

   for (int i=0; i<N; i++)
   {
      double value_real=value[i].get_real();
      max_value_real=basic_math::max(max_value_real,value_real);
      min_value_real=basic_math::min(min_value_real,value_real);
   }
}

double waveform::max_time_domain_function_magnitude()
{
   double max_value_mag=0;
   for (int i=0; i<N; i++)
   {
      double value_mag=value[i].get_mod();
      max_value_mag=basic_math::max(max_value_mag,value_mag);
   }
   return max_value_mag;
}

double waveform::max_freq_domain_function_magnitude()
{
   double max_tilde_mag=0;
   for (int i=0; i<N; i++)
   {
      double tilde_mag=tilde[i].get_mod();
      max_tilde_mag=basic_math::max(max_tilde_mag,tilde_mag);
   }
   return max_tilde_mag;
}

void waveform::renormalize_function(const double a)
{
   for (int i=0; i<N; i++)
   {
      value[i] *= a;
      tilde[i] *= a;
   }
}

// ---------------------------------------------------------------------
// Member function compute_energy calculates the *true* total energies
// over the total intervals in the time and frequency domain signals.
// See eqn (12.1.10) in Numerical Recipes.  The delta_t and delta_freq
// prefactors effectively render the energy results independent of the
// number of bins N.

// Note: In the time domain, E_true = sum_i |h_i|**2 delta_t.  In the
// frequency domain, E_true = sum_i |Htildetrue_i|**2 delta_freq.

double waveform::compute_energy()
{
   double E=0;
   double Etilde=0;
   for (int i=0; i<N; i++)
   {
      E += sqr(value[i].get_mod())*delta_t;
      Etilde += sqr(tilde[i].get_mod())*delta_freq;
   }
   cout << "Time domain energy = " << E
        << " Freq domain energy = " << Etilde << endl;
   return E;
}
