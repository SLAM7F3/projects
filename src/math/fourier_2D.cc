// ==========================================================================
// Two-dimensional FFT member functions
// ==========================================================================
// Last updated on 1/1/15; 1/4/15
// ==========================================================================

#include "math/basic_math.h"
#include "math/complex.h"
#include "math/constants.h"
#include "math/fourier_2D.h"
#include "general/genfuncs_complex.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor member functions:
// ---------------------------------------------------------------------

void fourier_2D::allocate_member_objects()
{
   value_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);
   tilde_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);
   tmp_tilde_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);
   tmp_tilde2_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);
   tmp_value_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);
   tmp_value2_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);

   expon_matrix_ptr =new flann::Matrix<complex>(
      new complex[nxbins*nybins], nxbins, nybins);

// Introduce arrays in_fftwnd and out_fftwnd of type fftw_complex in
// order to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   in_fftwnd=new fftw_complex[nxbins*nybins];
   out_fftwnd=new fftw_complex[nxbins*nybins];
}		       

void fourier_2D::initialize_member_objects()
{
   delta_x=(xhi-xlo)/(nxbins-1);
   delta_kx=1.0/(nxbins*delta_x);
   delta_y=(yhi-ylo)/(nybins-1);
   delta_ky=1.0/(nybins*delta_y);

   kx_hi=(nxbins-1)*delta_kx/2.0;
   kx_lo=-kx_hi;
   ky_hi=(nybins-1)*delta_ky/2.0;
   ky_lo=-ky_hi;

// Fill entries of *expon_matrix_ptr which is used to compute discrete
// cosine transforms:

   for (int pu = 0; pu < nxbins; pu++)
   {
      double arg_u = PI*pu/nxbins;
      complex Zu(cos(arg_u), -sin(arg_u));
      
      for (int pv = 0; pv < nybins; pv++)
      {
         double arg_v = PI*pv/nybins;
         complex Zv(cos(arg_v), -sin(arg_v));
         (*expon_matrix_ptr)[pu][pv] = Zu * Zv;
      }
   }

}

// nxbins and nybins may be either even or odd ...

fourier_2D::fourier_2D(int nx_bins, int ny_bins)
{
   nxbins = nx_bins;
   nybins = ny_bins;

   xhi = 1;
   xlo = -1;
   yhi = 1;
   ylo = -1;

   allocate_member_objects();
   initialize_member_objects();
}

fourier_2D::fourier_2D(
   int nx_bins, int ny_bins, double x_lo,double x_hi, double y_lo, double y_hi)
{
   nxbins = nx_bins;
   nybins = ny_bins;

   xhi=x_hi;
   xlo=x_lo;
   yhi=y_hi;
   ylo=y_lo;

   allocate_member_objects();
   initialize_member_objects();
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

fourier_2D::fourier_2D(const fourier_2D& f)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(f);
}

// ---------------------------------------------------------------------
void fourier_2D::docopy(const fourier_2D& f)
{
   xhi=f.xhi;
   xlo=f.xlo;
   delta_x=f.delta_x;

   yhi=f.yhi;
   ylo=f.ylo;
   delta_y=f.delta_y;

   kx_hi=f.kx_hi;
   kx_lo=f.kx_lo;
   delta_kx=f.delta_kx;

   ky_hi=f.ky_hi;
   ky_lo=f.ky_lo;
   delta_ky=f.delta_ky;
}

// ---------------------------------------------------------------------
fourier_2D::~fourier_2D()
{
   delete [] value_matrix_ptr->ptr();
   delete [] tilde_matrix_ptr->ptr();
   delete [] tmp_value_matrix_ptr->ptr();
   delete [] tmp_value2_matrix_ptr->ptr();
   delete [] tmp_tilde_matrix_ptr->ptr();
   delete [] tmp_tilde2_matrix_ptr->ptr();

   delete [] expon_matrix_ptr->ptr();

   delete [] in_fftwnd;
   delete [] out_fftwnd;
}

// Overload = operator:

fourier_2D& fourier_2D::operator= (const fourier_2D& f)
{
   docopy(f);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const fourier_2D& f)
{
   outstream << endl;
   outstream << "nxbins = " << f.nxbins << endl;
   outstream << "nybins = " << f.nybins << endl;
   outstream << "xhi = " << f.xhi << endl;
   outstream << "xlo = " << f.xlo << endl;
   outstream << "delta_x = " << f.delta_x << endl;
   outstream << "yhi = " << f.yhi << endl;
   outstream << "ylo = " << f.ylo << endl;
   outstream << "delta_y = " << f.delta_y << endl;
   outstream << "kx_hi = " << f.kx_hi << endl;
   outstream << "kx_lo = " << f.kx_lo << endl;
   outstream << "delta_kx = " << f.delta_kx << endl;
   outstream << "ky_hi = " << f.ky_hi << endl;
   outstream << "ky_lo = " << f.ky_lo << endl;
   outstream << "delta_ky = " << f.delta_ky << endl;
   outstream << endl;
   return outstream;
}

/*
// ---------------------------------------------------------------------
// Set & get methods:


void fourier_2D::set_value(int px, int py, complex curr_value)
{
   (*value_matrix_ptr)[px][py] = curr_value;
}

complex fourier_2D::get_value(int px, int py)
{
   return (*value_matrix_ptr)[px][py];
}

void fourier_2D::set_tilde(int px, int py, complex curr_tilde)
{
   (*tilde_matrix_ptr)[px][py] = curr_tilde;
}

complex fourier_2D::get_tilde(int px, int py)
{
   return (*tilde_matrix_ptr)[px][py];
}
*/

// ---------------------------------------------------------------------
// Initialize FFTW algorithm by either calculating FFT weights from
// scratch or else reading in previously computed values from files
// "fftw.forward" and "fftw.backward" within cplusplusrootdir/classes:

// Note: The senses of "theforward" and "thebackward" are adjusted
// here so as to be compatible with Numerical Recipes' Fourier
// transform conventions.

void fourier_2D::init_fftw()
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

   theforward_2d = fftw2d_create_plan(
      nxbins,nybins,FFTW_FORWARD, FFTW_MEASURE | FFTW_IN_PLACE);
   thebackward_2d = fftw2d_create_plan(
      nxbins,nybins, FFTW_BACKWARD,FFTW_MEASURE | FFTW_IN_PLACE);

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
// Method fouriertransform() returns the *true* momentum space
// Fourier transform H(kx,ky) of the 2D spatial signal contained
// within the complex value array.  Recall that the
// *discrete* fourier transform H(m,n) is related to the continuous
// true Fourier transform H(kx,ky) by H(kx,ky) = dx * dy * H(m,n)
// where dx and dy denote the sampling step sizes in the x and y
// directions.  As of 11/00, we take dx=dy.  We *do* include a factor
// of dx*dy into the momentum space tilde array values which are
// computed by this method.

void fourier_2D::fouriertransform()
{
   int iskip,jskip;

// Wrap input data columns:

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tmp_value_matrix_ptr)[i][j] = (*value_matrix_ptr)[iskip][j];
      }
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tmp_value_matrix_ptr)[iskip][j] = (*value_matrix_ptr)[i][j];
      }
   }

// Wrap input data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_value2_matrix_ptr)[i][j] = (*tmp_value_matrix_ptr)[i][jskip];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_value2_matrix_ptr)[i][jskip] = (*tmp_value_matrix_ptr)[i][j];
      }
   }

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
//         in_fftwnd[i+nxbins*j].re=(*tmp_value2_matrix_ptr)[i][j].get_real();
//         in_fftwnd[i+nxbins*j].im=(*tmp_value2_matrix_ptr)[i][j].get_imag();
         in_fftwnd[i+nxbins*j].re=(*value_matrix_ptr)[i][j].get_real();
         in_fftwnd[i+nxbins*j].im=(*value_matrix_ptr)[i][j].get_imag();
      }
   }

// Take Fourier transform using fftwnd:

   fftwnd_one(theforward_2d,in_fftwnd,NULL);
//   fftwnd_one(theforward_2d,in_fftwnd,out_fftwnd);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
//         tmp_tilde2[i][j]=complex(out_fftwnd[i+nxbins*j].re,
//                                  out_fftwnd[i+nxbins*j].im);
         (*tmp_tilde2_matrix_ptr)[i][j]=complex(in_fftwnd[i+nxbins*j].re,
                                                in_fftwnd[i+nxbins*j].im);
      }
   }

// Unwrap output data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_tilde_matrix_ptr)[i][jskip]=(*tmp_tilde2_matrix_ptr)[i][j];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_tilde_matrix_ptr)[i][j]=(*tmp_tilde2_matrix_ptr)[i][jskip];
      }
   }

// Unwrap output data columns and return results within array tilde.
// Multiply *discrete* fourier transform results by delta_x*delta_y to
// turn them into *true* continuous fourier transform values.  See eqn
// (12.1.8) in Numerical Recipes.

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tilde_matrix_ptr)[iskip][j]=
            delta_x*delta_y*(*tmp_tilde_matrix_ptr)[i][j];
      }

      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tilde_matrix_ptr)[i][j]=
            delta_x*delta_y*(*tmp_tilde_matrix_ptr)[iskip][j];
      }
   }

   null_extremely_tiny_matrix_elements(tilde_matrix_ptr);
}

// ---------------------------------------------------------------------
// Method inverse_fouriertransform() returns the *true* spatial domain
// inverse Fourier transform h(x,y) of the 2D momentum space signal
// contained within the complex array tilde.  Recall that
// the *discrete* inverse fourier transform H(m,n) is related to the
// continuous true inverse Fourier transform h(x,y) by h(x,y) =
// H(m,n)/(nxbins*nybins).  We *do* include a factor of
// 1/(nxbins*nybins) into the spatial domain value array computed by
// this subroutine.

void fourier_2D::inverse_fouriertransform()
{
   int iskip,jskip;

// Wrap input data columns.  Recall the tilde array contains *true*
// continuous fourier transform values.  We must first divide these by
// (delta_x*delta_y) to convert them into *discrete* fourier transform
// values before calling the inverse FFTW subroutine:

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tmp_tilde_matrix_ptr)[i][j]=
            (*tilde_matrix_ptr)[iskip][j]/(delta_x*delta_y);
      }
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*tmp_tilde_matrix_ptr)[iskip][j]=
            (*tilde_matrix_ptr)[i][j]/(delta_x*delta_y);
      }
   }

// Wrap input data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_tilde2_matrix_ptr)[i][j]=(*tmp_tilde_matrix_ptr)[i][jskip];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_tilde2_matrix_ptr)[i][jskip]=(*tmp_tilde_matrix_ptr)[i][j];
      }
   }

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         in_fftwnd[i+nxbins*j].re=(*tmp_tilde2_matrix_ptr)[i][j].get_real();
         in_fftwnd[i+nxbins*j].im=(*tmp_tilde2_matrix_ptr)[i][j].get_imag();
      }
   }

// Take inverse Fourier transform using fftwnd:

   fftwnd_one(thebackward_2d,in_fftwnd,NULL);
//   fftwnd_one(thebackward_2d,in_fftwnd,out_fftwnd);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
//         tmp_value2[i][j]=complex(out_fftwnd[i+nxbins*j].re,
//                                  out_fftwnd[i+nxbins*j].im);

        (*tmp_value2_matrix_ptr)[i][j] = complex(in_fftwnd[i+nxbins*j].re,
                                                 in_fftwnd[i+nxbins*j].im);
      }
   }
  
// Unwrap output data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_value_matrix_ptr)[i][jskip]=(*tmp_value2_matrix_ptr)[i][j];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         (*tmp_value_matrix_ptr)[i][j]=(*tmp_value2_matrix_ptr)[i][jskip];
      }
   }

// Unwrap output data columns and return results within array value.
// Multiply *discrete* inverse fourier transform results
// 1/(nxbins*nybins) to turn them into *true* continuous inverse
// fourier transform values.  See eqn (12.1.8) in Numerical Recipes.

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*value_matrix_ptr)[iskip][j]=
            (*tmp_value_matrix_ptr)[i][j]/(nxbins*nybins);
      }

      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         (*value_matrix_ptr)[i][j]=
            (*tmp_value_matrix_ptr)[iskip][j]/(nxbins*nybins);
      }
   }

   null_extremely_tiny_matrix_elements(value_matrix_ptr);
}

// ---------------------------------------------------------------------
// Member function null_extremly_tiny_matrix elements sets to zero any
// real or imaginary part of a matrix element whose absolute value is
// less than EXTREMELY_TINY.  This helps to minimize build-up of
// machine round-off errors.

void fourier_2D::null_extremely_tiny_matrix_elements(
   flann::Matrix<complex> *matrix_ptr)
{
   const double EXTREMELY_TINY=1.0E-15;
   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         if (fabs((*matrix_ptr)[i][j].get_real()) < EXTREMELY_TINY) 
            (*matrix_ptr)[i][j].set_real(0);
         if (fabs((*matrix_ptr)[i][j].get_imag()) < EXTREMELY_TINY) 
            (*matrix_ptr)[i][j].set_imag(0);
      }
   }
}


// ---------------------------------------------------------------------
// Method integrated_value_energy()

double fourier_2D::integrated_value_energy()
{
   double Esqrd = 0;

   for (int px = 0; px < nxbins; px++)
   {
      for (int py = 0; py < nybins; py++)
      {
         Esqrd += sqr((*value_matrix_ptr)[px][py].get_mod())*delta_x*delta_y;
      }
   }
   
   return Esqrd;
}

// ---------------------------------------------------------------------
// Function integrated_tilde_energy()

// Note added on 1/2/15: Parseval's theorem is exactly true when
// nxbins,nybins are both even.  But for reasons we don't currently
// understand, it is only approximately true when either nxbins and/or
// nybins is odd

double fourier_2D::integrated_tilde_energy()
{
   double Esqrd = 0;

   for (int px = 0; px < nxbins; px++)
   {
      for (int py = 0; py < nybins; py++)
      {
         Esqrd += sqr((*tilde_matrix_ptr)[px][py].get_mod())*delta_kx*delta_ky;
      }
   }

   return Esqrd;
}

// ---------------------------------------------------------------------
// Method discrete_cosine_transform() closely follows the DCT 
// computation from conventional FFTs presented in section II.A 
// of "A fast cosine transform in one and two dimensions" by John
// Makhoul, Feb 1980.  In particular, it generates a two-dimensional
// even extension of input *matrix_ptr which is assumed to have size
// nxbins/2 x nybins/2.  The extended image has pixel size nxbins x
// nybins, and its entries are loaced into the *value_matrix_ptr
// member of the fourier_2D class.  The discrete fourier transform is
// subsequently computed via a call to FFTW (with no further wrapping
// of columns or rows).  Entries in output *dct_matrix_ptr are
// recovered from the first nxbins/2 x nybins/2 elements of member
// *tilde_matrix_ptr.

void fourier_2D::discrete_cosine_transform(
   const flann::Matrix<float> *matrix_ptr,
   flann::Matrix<float> *dct_matrix_ptr)
{
//   cout << "inside fourier_2D::discrete_cosine_transform()" << endl;

   complex curr_value;
   
   for (int px = 0; px < nxbins; px++)
   {
      for (int py = 0; py < nybins; py++)
      {
         if (px < nxbins/2 && py < nybins/2)
         {
            curr_value = complex((*matrix_ptr)[px][py]);
         }
         else if (px >= nxbins/2 && py < nybins/2)
         {
            curr_value = complex((*matrix_ptr)[nxbins-1-px][py]);
         }
         else if (px < nxbins/2 && py >= nybins/2)
         {
            curr_value = complex((*matrix_ptr)[px][nybins-1-py]);
         }
         else if (px >= nxbins/2 && py >= nybins/2)
         {
            curr_value = complex((*matrix_ptr)[nxbins-1-px][nybins-1-py]);
         }

         set_value(px,py,curr_value);
      } // loop over py
   } // loop over px

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         in_fftwnd[i+nxbins*j].re=(*value_matrix_ptr)[i][j].get_real();
         in_fftwnd[i+nxbins*j].im=(*value_matrix_ptr)[i][j].get_imag();
      }
   }

// Take Fourier transform using fftwnd:

   fftwnd_one(theforward_2d,in_fftwnd,NULL);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         (*tilde_matrix_ptr)[i][j]=complex(in_fftwnd[i+nxbins*j].re,
                                           in_fftwnd[i+nxbins*j].im);
      }
   }

   null_extremely_tiny_matrix_elements(tilde_matrix_ptr);

   double prefactor = 4.0/( (nxbins) * (nybins));
   double inv_sqrt_two = 1.0 / sqrt(2.0);

   for (int pu = 0; pu < nxbins/2; pu++)
   {
      double alpha_u = 1;
      if (pu == 0) alpha_u = inv_sqrt_two;
      
      for (int pv = 0; pv < nybins/2; pv++)
      {
         double alpha_v = 1;
         if (pv == 0) alpha_v = inv_sqrt_two;
         
         complex Z_uv = prefactor * alpha_u * alpha_v * 
            (*expon_matrix_ptr)[pu][pv] * get_tilde(pu,pv);

         (*dct_matrix_ptr)[pu][pv] = Z_uv.get_real();
         
      } // loop over pv
   } // loop over pu
}

// ---------------------------------------------------------------------
// Method inverse_discrete_cosine_transform() closely follows the 
// IDCT computation from conventional FFTs presented in section II.B
// of "A fast cosine transform in one and two dimensions" by John
// Makhoul, Feb 1980.  Recall that the even, nxbins x nybins extension
// of the original *matrix_ptr which has size nxbins/2 x nybins/2
// contains no more information than that within *matrix_ptr.  So its
// nxbins x nybins discrete fourier transform also contains redundant
// information outside its first nxbins/2 x nybins/2 quadrant.  In
// particular, let Ytilde(pu,pv) denote the discrete FFT of y(px,py)
// where y(px,py) is an *even* extension of size 2Nx2N of
// some arbitrary image x(px,py)

// Then all 2Nx2N entries in Ytilde(pu,pv) can be derived from first
// NxN entries of Ytilde(pu,pv) as follows:

// Ytilde(N,v) = Ytilde(u,N) = Ytilde(N,N) = 0.
// Ytilde(u, 2N - v) = exp(-v Pi I / N) * Ytilde(u,v)
// Ytilde(2N - u, v) = exp(-u Pi I / N) * Ytilde(u,v)
// Ytilde(2N - u, 2N - v) = Ytilde(u,v).Conjugate()

void fourier_2D::inverse_discrete_cosine_transform(
   const flann::Matrix<float> *dct_matrix_ptr,flann::Matrix<float> *matrix_ptr)
{
//   cout << "inside fourier_2D::inverse_discrete_cosine_transform()" << endl;
//   cout << "nxbins = " << nxbins << " nybins = " << nybins << endl;

   double prefactor = nxbins * nybins/4.0;
   double sqrt_two = sqrt(2.0);

   for (int pu = 0; pu <= nxbins/2; pu++)
   {
      double alpha_u = 1;
      if (pu == 0) alpha_u = sqrt_two;
      if (pu == nxbins/2) alpha_u = 0;
      
      for (int pv = 0; pv <= nybins/2; pv++)
      {
         double alpha_v = 1;
         if (pv == 0) alpha_v = sqrt_two;
         if (pv == nybins/2) alpha_v = 0;

         complex Ytilde_uv = prefactor * alpha_u * alpha_v 
            * (*expon_matrix_ptr)[pu][pv].Conjugate() 
            * (*dct_matrix_ptr)[pu][pv];
         set_tilde(pu,pv,Ytilde_uv);
         
         if (pu > 0 && pu < nxbins/2)
         {
            complex Zu = (*expon_matrix_ptr)[2*pu][0];
            set_tilde(nxbins - pu, pv, Zu * Ytilde_uv );
         }
         
         if (pv > 0 && pv < nybins/2)
         {
            complex Zv = (*expon_matrix_ptr)[0][2*pv];
            set_tilde(pu, nybins - pv, Zv * Ytilde_uv );
         }

         if (pu > 0 && pu < nxbins/2 && pv > 0 && pv < nybins/2)
         {
            set_tilde(nxbins - pu, nybins - pv, Ytilde_uv.Conjugate() );
         }
       
      } // loop over pv
   } // loop over pu

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         in_fftwnd[i+nxbins*j].re=(*tilde_matrix_ptr)[i][j].get_real();
         in_fftwnd[i+nxbins*j].im=(*tilde_matrix_ptr)[i][j].get_imag();
      }
   }

// Take inverse Fourier transform using fftwnd:

   fftwnd_one(thebackward_2d,in_fftwnd,NULL);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         (*value_matrix_ptr)[i][j]=complex(in_fftwnd[i+nxbins*j].re,
                                           in_fftwnd[i+nxbins*j].im);
      }
   }

   null_extremely_tiny_matrix_elements(value_matrix_ptr);

   prefactor = 1.0/( (nxbins) * (nybins));
   for (int px = 0; px < nxbins/2; px++)
   {
      for (int py = 0; py < nybins/2; py++)
      {
         complex Z_xy = prefactor * get_value(px,py);
         (*matrix_ptr)[px][py] = Z_xy.get_real();
         
      } // loop over py
   } // loop over px
}

// ---------------------------------------------------------------------
// Method low_pass_DCT_filter() zeros out all entries in
// *dct_matrix_ptr where pu**2+pv**2 > max_spatial_frequency**2.

void fourier_2D::low_pass_DCT_filter(
   double max_spatial_frequency, flann::Matrix<float> *dct_matrix_ptr)
{
//   cout << "inside fourier_2D::low_pass_DCT_filter()" << endl;
//   cout << "rows = " << dct_matrix_ptr->rows
//        << " cols = " << dct_matrix_ptr->cols
//        << endl;

   for (unsigned int pu = 0; pu < dct_matrix_ptr->rows; pu++)
   {
      for (unsigned int pv = 0; pv < dct_matrix_ptr->cols; pv++)
      {
         if (pu*pu+pv*pv > sqr(max_spatial_frequency)) 
         {
            (*dct_matrix_ptr)[pu][pv] = 0;
         }
      } // loop over pv index
   } // loop over pu index

}

