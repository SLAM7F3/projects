// ==========================================================================
// Header file for fourier class.
// ==========================================================================
// Last updated on 7/22/03
// ==========================================================================

#ifndef FOURIER_H
#define FOURIER_H

#include <fftw.h>
#include <new>

class complex;

class fourier
{
  private: 

   int nxbins;
   double xhi,xlo,deltax;
   double kx_hi,kx_lo,delta_kx;
   complex *value,*tilde;
   fftw_plan theforward_1d,thebackward_1d;

  public:

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

   void allocate_member_objects();
   void initialize_member_objects();
   fourier(int nbins);
   fourier(int nbins,double x_hi,double x_lo);
   fourier(const fourier& f);
   void docopy(const fourier& f);
   ~fourier();
   fourier& operator= (const fourier& f);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const fourier& f);
 
// Set and get methods:

   void init_fftw();
   void fouriertransform(const complex value_in[],complex tilde_out[]);
   void inversefouriertransform(
      const complex tilde_in[],complex value_out[]);
};

#endif  // fourier.h



