// ==========================================================================
// FFT functions 
// ==========================================================================
// Last updated on 5/10/05; 3/31/12; 10/29/13
// ==========================================================================

#include "math/basic_math.h"
#include "math/complex.h"
#include "math/constants.h"
#include "fourier.h"
#include "general/genfuncs_complex.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void fourier::allocate_member_objects()
{
}		       

void fourier::initialize_member_objects()
{
}

fourier::fourier(int nbins)
{
   nxbins=nbins;
   allocate_member_objects();
   initialize_member_objects();
}

fourier::fourier(int nbins,double x_hi,double x_lo)
{
   nxbins=nbins;
   xhi=x_hi;
   xlo=x_lo;
   deltax=(xhi-xlo)/(nxbins-1);
   delta_kx=1.0/(nxbins*deltax);
   kx_hi=(nxbins-1)*delta_kx/2.0;
   kx_lo=-kx_hi;

   allocate_member_objects();
   initialize_member_objects();
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

fourier::fourier(const fourier& f)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(f);
}

// ---------------------------------------------------------------------
void fourier::docopy(const fourier& f)
{
   nxbins=f.nxbins;
   xhi=f.xhi;
   xlo=f.xlo;
   deltax=f.deltax;
   kx_hi=f.kx_hi;
   kx_lo=f.kx_lo;
   delta_kx=f.delta_kx;
   for (int n=0; n<nxbins; n++)
   {
      value[n]=f.value[n];
      tilde[n]=f.tilde[n];
   }
}

// ---------------------------------------------------------------------
fourier::~fourier()
{
   delete [] value;
   delete [] tilde;
}

// Overload = operator:

fourier& fourier::operator= (const fourier& f)
{
   docopy(f);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const fourier& f)
{
   outstream << endl;
   outstream << "nxbins = " << f.nxbins << endl;
   outstream << "xhi = " << f.xhi << endl;
   outstream << "xlo = " << f.xlo << endl;
   outstream << "deltax = " << f.deltax << endl;
   outstream << "kx_hi = " << f.kx_hi << endl;
   outstream << "kx_lo = " << f.kx_lo << endl;
   outstream << "delta_kx = " << f.delta_kx << endl;
   outstream << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Initialize FFTW algorithm by either calculating FFT weights from
// scratch or else reading in previously computed values from files
// "fftw.forward" and "fftw.backward" within cplusplusrootdir/classes:

// Note: The senses of "theforward" and "thebackward" are adjusted
// here so as to be compatible with Numerical Recipes' Fourier
// transform conventions.

void fourier::init_fftw()
{
   bool save_forward_wisdom=false;
   bool save_backward_wisdom=false;

   string prefix=sysfunc::get_projectsrootdir()+"src/math/";
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

   theforward_1d=fftw_create_plan(nxbins,FFTW_BACKWARD,FFTW_MEASURE | 
                                  FFTW_USE_WISDOM);
   thebackward_1d=fftw_create_plan(nxbins,FFTW_FORWARD,FFTW_MEASURE | 
                                   FFTW_USE_WISDOM);
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
// Member function fouriertransform returns the *true* momentum space
// Fourier transform H(kx) of the spatial domain signal contained
// within the complex val array.  The number of bins within the
// incoming data array must be ODD!  Recall that the *discrete*
// fourier transform H(n) is related to the continuous true Fourier
// transform H(kx) by H(kx) = dx * H(n) where dx denotes the bin size.
// We *do* include a factor of dx into the momentum space tilde_out
// array values which are computed by this routine.

void fourier::fouriertransform(const complex value_in[],complex tilde_out[])
{
   const double EXTREMELY_TINY=1.0E-10;
//   const double EXTREMELY_TINY=1.0E-15;

   int i,iskip;
   complex (*tmp_value)=new complex[nxbins];
   complex (*tmp_tilde)=new complex[nxbins];

// Introduce arrays in_fftw and out_fftw of type fftw_complex in order
// to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftw=new fftw_complex[nxbins];
   fftw_complex *out_fftw=new fftw_complex[nxbins];

// Wrap input data:

   for (i=0; i<nxbins/2+1; i++)
   {
      iskip=i+nxbins/2;
      tmp_value[i]=value_in[iskip];
   }
   for (i=0; i<nxbins/2; i++)
   {
      iskip=i+nxbins/2;
      tmp_value[iskip+1]=value_in[i];
   }

// Copy real and imaginary values into in_fftw array:

   for (i=0; i<nxbins; i++)
   {
      in_fftw[i].re=tmp_value[i].get_real();
      in_fftw[i].im=tmp_value[i].get_imag();
   }

// Take Fourier transform using fftw:

   fftw_one(theforward_1d,in_fftw,out_fftw);

   for (i=0; i<nxbins; i++)
   {
      tmp_tilde[i]=complex(out_fftw[i].re,out_fftw[i].im);
   }

// Unwrap output data.  Multiply *discrete* fourier transform results
// contained within array tmp_tilde by deltax to turn them into *true*
// continuous fourier transform values.  See eqn (12.1.8) in Numerical
// Recipes.

   for (i=0; i<nxbins/2+1; i++)
   {
      iskip=i+nxbins/2;
      tilde_out[iskip]=deltax*tmp_tilde[i];
   }

   for (i=0; i<nxbins/2; i++)
   {
      iskip=i+nxbins/2;
      tilde_out[i]=deltax*tmp_tilde[iskip+1];
   }

// To minimize build-up of machine round-off errors, we null out any
// fourier transformed value less than EXTREMELY TINY:

   for (i=0; i<nxbins; i++)
   {
      if (fabs(tilde_out[i].get_real()) < EXTREMELY_TINY) 
         tilde_out[i].set_real(0);
      if (fabs(tilde_out[i].get_imag()) < EXTREMELY_TINY) 
         tilde_out[i].set_imag(0);
   }

   delete [] tmp_value;
   delete [] tmp_tilde;
   delete [] in_fftw;
   delete [] out_fftw;
}

// ---------------------------------------------------------------------
// Member function inversefouriertransform returns the *true* spatial
// domain inverse Fourier transform h(x) of the 1D momentum space
// signal contained within the complex array tilde.  The number of
// rows and columns within the incoming data array must be ODD!
// Recall that the *discrete* inverse fourier transform H(n) is
// related to the continuous true inverse Fourier transform h(x) by
// h(x) = H(n)/nxbins.  We *do* include a factor of 1/nxbins into the
// spatial domain value_out array computed by this subroutine.

void fourier::inversefouriertransform(
   const complex tilde_in[],complex value_out[])
{
   const double EXTREMELY_TINY=1.0E-10;
//   const double EXTREMELY_TINY=1.0E-15;

   int i,iskip;
   complex (*tmp_tilde)=new complex[nxbins];
   complex (*tmp_value)=new complex[nxbins];

// Introduce arrays in_fftw and out_fftw of type fftw_complex in order
// to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftw=new fftw_complex[nxbins];
   fftw_complex *out_fftw=new fftw_complex[nxbins];

// Wrap input data.  Recall the tilde_in array contains *true*
// continuous fourier transform values.  We must first divide these by
// deltax to convert them into *discrete* fourier transform values
// before calling the inverse FFTW subroutine:

   for (i=0; i<nxbins/2+1; i++)
   {
      iskip=i+nxbins/2;
      tmp_tilde[i]=tilde_in[iskip]/deltax;
   }
   for (i=0; i<nxbins/2; i++)
   {
      iskip=i+nxbins/2;
      tmp_tilde[iskip+1]=tilde_in[i]/deltax;
   }

// Copy real and imaginary momentum space values into in_fftw array:

   for (i=0; i<nxbins; i++)
   {
      in_fftw[i].re=tmp_tilde[i].get_real();
      in_fftw[i].im=tmp_tilde[i].get_imag();
   }

// Take inverse Fourier transform using fftw:

   fftw_one(thebackward_1d,in_fftw,out_fftw);

   for (i=0; i<nxbins; i++)
   {
      tmp_value[i]=complex(out_fftw[i].re,out_fftw[i].im);
      if (fabs(tmp_value[i].get_real()) < EXTREMELY_TINY) 
         tmp_value[i].set_real(0);
      if (fabs(tmp_value[i].get_imag()) < EXTREMELY_TINY) 
         tmp_value[i].set_imag(0);
   }

// Unwrap output data.  Multiply *discrete* inverse fourier transform
// results 1/nxbins to turn them into *true* continuous inverse
// fourier transform values.  See eqn (12.1.8) in Numerical Recipes.

   for (i=0; i<nxbins/2+1; i++)
   {
      iskip=i+nxbins/2;
      value_out[iskip]=tmp_value[i]/nxbins;
   }

   for (i=0; i<nxbins/2; i++)
   {
      iskip=i+nxbins/2;
      value_out[i]=tmp_value[iskip+1]/nxbins;
   }

// To minimize build-up of machine round-off errors, we null out any 
// inverse fourier transformed value less than EXTREMELY TINY:

   for (i=0; i<nxbins; i++)
   {
      if (fabs(value_out[i].get_real()) < EXTREMELY_TINY) 
         value_out[i].set_real(0);
      if (fabs(value_out[i].get_imag()) < EXTREMELY_TINY) 
         value_out[i].set_imag(0);
   }

   delete [] tmp_value;
   delete [] tmp_tilde;
   delete [] in_fftw;
   delete [] out_fftw;
}
