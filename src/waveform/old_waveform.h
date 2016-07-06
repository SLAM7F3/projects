// As of July 03, this ancient class is deprecated and no longer
// supported.

// ==========================================================================
// Header file for waveform class utilized by all main programs which
// perform Doppler processing:
// ==========================================================================
// Last modified on 4/21/03
// ==========================================================================

#ifndef WAVEFORM_H
#define WAVEFORM_H


#include <fftw.h>
#include "math/complex.h"

class waveform
{
  private:

  public:

// Number of bins in time and frequency domain arrays.  N must be a
// power of 2 for FFT purposes

   static const int N;

   static const int N_EXTRAINFO_LINES;
   static const int NSUBTICS;
   static const double TIMEFRAC;
   static const double FREQFRAC;

   bool dbflag,fewzerosflag,plot_real_part_flag,plot_imag_part_flag;
   bool plot_real_and_imag_flag,plot_phase_flag,plot_mag_and_phase_flag;

   bool negativetime;
   std::ofstream timestream,freqstream;
   std::string timefilenamestr,freqfilenamestr;
   std::string *extrainfo;
   std::string *extraline;
   std::string title,timeaxislabel,timeyaxislabel,
      freqaxislabel,freqyaxislabel;
   int color,nplots;

// Parameter npoints controls the number of points which are printed
// out to the metafile via member function writedataarray.  Setting it
// to some value less than its default NBINS value prevents many zero
// elements from needlessly being printed out to the metafile.

   int npoints;

   double currtime;
   double tmax,tmin,freqmax,freqmin,E,Etilde;

// Important note regarding timenorm and freqnorm:

// For presentation purposes only, we sometimes choose to *display*
// waveform results with renormalized y axis values.  However, the
// waveform y axis data itself is *not* renormalized by either
// timenorm or freqnorm.  The only subroutines in which timenorm and
// freqnorm should appear are those where data are written out to
// files, and not in those where data are calculated.
   
   double timenorm,freqnorm;

   double valmagmax,valmagmin,valmagnexttomin;
   double valrealmax,valrealmin,valimagmax,valimagmin;
   double tildemagmax,tildemagmin,tildemagnexttomin;
   double tilderealmax,tilderealmin,tildeimagmax,tildeimagmin;

// Allow for manual overrides of automatic min/max y axis value settings:

   double timemaxvalue,timeminvalue,freqmaxvalue,freqminvalue;

// Allow for manual overrides of automatic tick/subtick settings on
// independent and dependent axes in both time and frequency domains:

   double timetic,timesubtic,timeytic,timeysubtic;
   double freqtic,freqsubtic,freqytic,freqysubtic;

// To conserve stack space, we dynamically rather than statically
// allocate memory for arrays.  Following Iva's suggestion, we declare
// the pointers for these arrays within the class header files, but we
// only perform the "new" allocation calls within the constructors.
// We must also be sure to include a destructor function when we
// dynamically allocate memory within classes.  As Ed Broach has
// pointed out on two occasions, we must also be careful with equating
// one object with another object whenever the objects contain arrays
// which have been dynamically allocated.  We must overwrite the
// default C++ =operator for the objects, and include a new
// constructor function for the object which takes the object itself
// as an argument.

   double *T,*Freq;
   complex *val,*tilde;

// Introduce arrays in_fftw and out_fftw of type fftw_complex in order
// to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftw;
   fftw_complex *out_fftw;

// ---------------------------------------------------------------------
// Constructor and destructor functions:
// ---------------------------------------------------------------------

   waveform(void);
   waveform(const waveform& w);
   waveform(double delta);
   waveform(double delta,complex Z[]);
   ~waveform();
   
// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   void init_fftw(fftw_plan& theforward,fftw_plan& thebackward);
   void findextrema();
   void docopy(const waveform& w);
   waveform& operator= (const waveform& w);
   void fouriertransform(double delta, fftw_plan theforward);
   void inversefouriertransform(double delta, fftw_plan thebackward);
   void energy(double delta,double deltafreq);
   void rescale_val(complex a);
   void rescale_tilde(complex a);
   void shift_freq(double deltafreq);
   void add_const(double alpha);
   void inspect_contents();

   void timefileheader();
   void freqfileheader();
   void openfiles(double delta,double deltafreq,std::string outfilenamestem,
                  int filenumber);
   void closefiles();
   void writedata(double delta);
   waveform dBconvert(double delta,fftw_plan theforward);
   waveform Loadwaveform(double delta,std::string currline[],int nlines,
                         int& curvenumber);
};

#endif // datastructures/waveform.h


