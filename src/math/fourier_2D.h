// ==========================================================================
// Header file for two-dimensional FFT methods
// ==========================================================================
// Last updated on 7/22/03; 1/1/15; 1/4/15
// ==========================================================================

#ifndef FOURIER_2D_H
#define FOURIER_2D_H

#include <fftw.h>
#include <flann/flann.hpp>
#include <new>

class complex;

class fourier_2D
{

  public:

// Initialization, constructor and destructor member functions:

   fourier_2D(int nx_bins, int ny_bins);
   fourier_2D(int nx_bins, int ny_bins, 
              double x_lo,double x_hi, double y_lo, double y_hi);
   fourier_2D(const fourier_2D& f);
   void docopy(const fourier_2D& f);
   ~fourier_2D();
   fourier_2D& operator= (const fourier_2D& f);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const fourier_2D& f);
 
// Set and get methods:

   void set_value(int px, int py, complex curr_value);
   complex get_value(int px, int py);
   void set_tilde(int px, int py, complex curr_tilde);
   complex get_tilde(int px, int py);


   void init_fftw();
   void fouriertransform();
   void inverse_fouriertransform();

   double integrated_value_energy();
   double integrated_tilde_energy();

   void discrete_cosine_transform(
      const flann::Matrix<float>* matrix_ptr, 
      flann::Matrix<float>* dct_matrix_ptr);
   void inverse_discrete_cosine_transform(
      const flann::Matrix<float>* dct_matrix_ptr, 
      flann::Matrix<float>* matrix_ptr);
   void low_pass_DCT_filter(
      double max_spatial_frequency, flann::Matrix<float> *dct_matrix_ptr);

  private: 
   
   int nxbins, nybins;
   double xhi,xlo,delta_x;
   double yhi,ylo,delta_y;
   double kx_hi,kx_lo,delta_kx;
   double ky_hi,ky_lo,delta_ky;
   fftwnd_plan theforward_2d,thebackward_2d;

   flann::Matrix<complex> *value_matrix_ptr;
   flann::Matrix<complex> *tilde_matrix_ptr;
   flann::Matrix<complex> *tmp_value_matrix_ptr;
   flann::Matrix<complex> *tmp_value2_matrix_ptr;
   flann::Matrix<complex> *tmp_tilde_matrix_ptr;
   flann::Matrix<complex> *tmp_tilde2_matrix_ptr;

   flann::Matrix<complex> *expon_matrix_ptr;

   fftw_complex *in_fftwnd;
   fftw_complex *out_fftwnd;
   
   void allocate_member_objects();
   void initialize_member_objects();

   void null_extremely_tiny_matrix_elements(
      flann::Matrix<complex> *matrix_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void fourier_2D::set_value(int px, int py, complex curr_value)
{
   (*value_matrix_ptr)[px][py] = curr_value;
}

inline complex fourier_2D::get_value(int px, int py)
{
   return (*value_matrix_ptr)[px][py];
}

inline void fourier_2D::set_tilde(int px, int py, complex curr_tilde)
{
   (*tilde_matrix_ptr)[px][py] = curr_tilde;
}

inline complex fourier_2D::get_tilde(int px, int py)
{
   return (*tilde_matrix_ptr)[px][py];
}

#endif  // fourier_2D.h



