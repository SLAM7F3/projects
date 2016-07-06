// ==========================================================================
// Waveform class member function definitions
// ==========================================================================
// Last modified on 6/16/03
// ==========================================================================

#include "genfuncs.h"
#include "math/mathfuncs.h"
#include "colorfuncs.h"
#include "datastructures/waveform.h"

using std::ios;
using std::string;
using std::ostream;
using std::cout;
using std::endl;

const int waveform::N=2048;
const int waveform::N_EXTRAINFO_LINES=50;
const int waveform::NSUBTICS=5;

// We adjust the time (frequency) axis limits so that only TIMEFRAC
// (FREQFRAC) of the total time (frequency) range is displayed in meta
// plots:

const double waveform::TIMEFRAC=0.4;
const double waveform::FREQFRAC=0.1;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

// Null constructor function useful for declaring waveform objects in
// exactly the same manner as integers, via "waveform w;".

// Note: We want frequency domain arrays to be symmetric about nu=0.
// We consequently set their dimensions equal to the ODD value N+1
// (rather than the even value N) so that their indices effectively
// run from -N/2, ..., -1, 0, 1, ..., +N/2:

waveform::waveform(void) 
{

// Set dbflag to true if frequency domain output is to be reported in
// dB's.  Variable fewzerosflag can be used to override dbflag for
// those plots where most frequency domain data values equal zero.
// When set to true, variable plot_real_part_flag causes the real part
// rather than magnitudes of frequency domain data to be sent to
// metafile plots.

   dbflag=false;
   fewzerosflag=true;
   plot_real_part_flag=true;
   plot_imag_part_flag=false;
   plot_real_and_imag_flag=false;
   plot_phase_flag=false;
   plot_mag_and_phase_flag=false;

   extrainfo=new string[N_EXTRAINFO_LINES];
   extraline=new string[N_EXTRAINFO_LINES];
   T=new double[N];
   Freq=new double[N+1];
   val=new complex[N];
   tilde=new complex[N+1];
   in_fftw=new fftw_complex[N];
   out_fftw=new fftw_complex[N];
}
   
// This next constructor is apparently called whenever a function is passed
// a waveform as an argument.

waveform::waveform(const waveform& w)
{

// Set dbflag to true if frequency domain output is to be reported in
// dB's.  Variable fewzerosflag can be used to override dbflag for
// those plots where most frequency domain data values equal zero.
// When set to true, variable plot_real_part_flag causes the real part
// rather than magnitudes of frequency domain data to be sent to
// metafile plots.

   dbflag=false;
   fewzerosflag=true;
   plot_real_part_flag=true;
   plot_imag_part_flag=false;
   plot_real_and_imag_flag=false;
   plot_phase_flag=false;
   plot_mag_and_phase_flag=false;

   extrainfo=new string[N_EXTRAINFO_LINES];
   extraline=new string[N_EXTRAINFO_LINES];
   T=new double[N];
   Freq=new double[N+1];
   val=new complex[N];
   tilde=new complex[N+1];
   in_fftw=new fftw_complex[N];
   out_fftw=new fftw_complex[N];
   docopy(w);
}

waveform::waveform(double delta)
{
   int i;

// Set dbflag to true if frequency domain output is to be reported in
// dB's.  Variable fewzerosflag can be used to override dbflag for
// those plots where most frequency domain data values equal zero.
// When set to true, variable plot_real_part_flag causes the real part
// rather than magnitudes of frequency domain data to be sent to
// metafile plots.

   dbflag=false;
   fewzerosflag=true;
   plot_real_part_flag=true;
   plot_imag_part_flag=false;
   plot_real_and_imag_flag=false;
   plot_phase_flag=false;
   plot_mag_and_phase_flag=false;

   extrainfo=new string[N_EXTRAINFO_LINES];
   extraline=new string[N_EXTRAINFO_LINES];
   T=new double[N];
   Freq=new double[N+1];
   val=new complex[N];
   tilde=new complex[N+1];
   in_fftw=new fftw_complex[N];
   out_fftw=new fftw_complex[N];

   for (i=0; i<N; i++)
   {
      T[i]=double(i*delta);
      val[i]=0;
      Freq[i]=(-N/2+i)/double(N*delta);
      tilde[i]=0;
   }
   Freq[N]=0.5/delta;
   tilde[N]=0;
   
   negativetime=false;
   currtime=0;
   tmin=TIMEFRAC*T[0];
   tmax=TIMEFRAC*T[N-1];
   freqmin=FREQFRAC*Freq[0];
   freqmax=FREQFRAC*Freq[N];
   timemaxvalue=timeminvalue=freqmaxvalue=freqminvalue=0;
   timetic=timesubtic=timeytic=timeysubtic=0;
   freqtic=freqsubtic=freqytic=freqysubtic=0;
   timenorm=freqnorm=1;
   color=-1; // black
   nplots=1;
   title="";
   timeaxislabel="";
   timeyaxislabel="";
   freqaxislabel="";
   freqyaxislabel="";
   for (i=0; i<N_EXTRAINFO_LINES; i++)
   {
      extrainfo[i]="";
      extraline[i]="";
   }
}
 
// We use the following constructor to load *BOTH* the time and
// frequency domain arrays with the entries contained within the
// complex input array Z.  A priori, we don't know whether the complex
// data corresponds to either the time or frequency domains.  But
// since in practice we always take either the fourier or inverse
// fourier transforms, the array with garbage data will be
// overwritten.  Note: Frequency domain array has N+1 bins, whereas
// time array has only N.  To avoid memory overflow problems, we
// should always pass to this constructor a complex array Z which has
// N and *NOT* N+1 bins!

waveform::waveform(double delta,complex Z[])
{
   int i;
   double valmag,tildemag;

// Set dbflag to true if frequency domain output is to be reported in
// dB's.  Variable fewzerosflag can be used to override dbflag for
// those plots where most frequency domain data values equal zero.
// When set to true, variable plot_real_part_flag causes the real part
// rather than magnitudes of frequency domain data to be sent to
// metafile plots.

   bool dbflag=false;
   bool fewzerosflag=true;
   bool plot_real_part_flag=true;
   bool plot_imag_part_flag=false;
   bool plot_real_and_imag_flag=false;
   bool plot_phase_flag=false;
   bool plot_mag_and_phase_flag=false;
         
   extrainfo=new string[N_EXTRAINFO_LINES];
   extraline=new string[N_EXTRAINFO_LINES];
   T=new double[N];
   Freq=new double[N+1];
   val=new complex[N];
   tilde=new complex[N+1];
   in_fftw=new fftw_complex[N];
   out_fftw=new fftw_complex[N];

   negativetime=false;
   currtime=0;
   tmin=TIMEFRAC*T[0];
   tmax=TIMEFRAC*T[N-1];
   freqmin=FREQFRAC*Freq[0];
   freqmax=FREQFRAC*Freq[N];
   timemaxvalue=timeminvalue=freqmaxvalue=freqminvalue=0;
   timetic=timesubtic=timeytic=timeysubtic=0;
   freqtic=freqsubtic=freqytic=freqysubtic=0;
   timenorm=freqnorm=1;
   color=-1; //black
   nplots=1;
   title=timeaxislabel=timeyaxislabel=freqaxislabel=freqyaxislabel="";
   for (i=0; i<N_EXTRAINFO_LINES; i++)
   {
      extrainfo[i]="";
      extraline[i]="";
   }

   for (i=0; i<N; i++)
   {
      T[i]=double(i*delta);
      Freq[i]=(-N/2+i)/double(N*delta);
      val[i]=Z[i];
      tilde[i]=Z[i];
   }
   Freq[N]=0.5/delta;
 
// Recall tilde array has N+1 elements whereas val array has only N
// elements.  Since we do not a priori know whether the waveform
// object is being initialized with either time or frequency domain
// data, we'll simply set the last frequency bin's value each to the
// first frequency bin's value:

   tilde[N]=tilde[0];  

// findextrema();
}

// ---------------------------------------------------------------------
// We must build a destructor function whenever memory is dynamically
// allocated for arrays within objects:

waveform::~waveform()
{
   delete [] extrainfo;
   delete [] extraline;
   delete [] T;
   delete [] Freq;
   delete [] val;
   delete [] tilde;
   delete [] in_fftw;
   delete [] out_fftw;
   extrainfo=NULL;
   extraline=NULL;
   T=NULL;
   Freq=NULL;
   val=NULL;
   tilde=NULL;
   in_fftw=NULL;
   out_fftw=NULL;
}

// ---------------------------------------------------------------------
// Initialize FFTW algorithm by either calculating FFT weights from
// scratch or else reading in previously computed values from files
// "fftw.forward" and "fftw.backward" within cplusplusrootdir/classes:

// Note: The senses of "theforward" and "thebackward" are adjusted
// here so as to be compatible with Numerical Recipes' Fourier
// transform conventions.

void waveform::init_fftw(fftw_plan& theforward,fftw_plan& thebackward)
{
   bool save_forward_wisdom=false;
   bool save_backward_wisdom=false;
   string prefix,forwardfilenamestr,backwardfilenamestr;
   FILE *forward_wisdomfile,*backward_wisdomfile;

   prefix=get_cplusplusrootdir()+"classes/";
   forwardfilenamestr=prefix+"fftw.forward";
   backwardfilenamestr=prefix+"fftw.backward";
   
// Ifstream constructor must take a C-style char* string argument
// rather than a C++ string class object:

   forward_wisdomfile=fopen(forwardfilenamestr.c_str(),"r");
   backward_wisdomfile=fopen(backwardfilenamestr.c_str(),"r");

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
   theforward=fftw_create_plan(N,FFTW_BACKWARD,FFTW_MEASURE | 
                               FFTW_USE_WISDOM);
   thebackward=fftw_create_plan(N,FFTW_FORWARD,FFTW_MEASURE | 
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
// Locate maximum and minimum values within the time and frequency
// domain signals for plotting purposes.  We should not bother to call
// this routine until we are ready to make plots.  In particular,
// calling findextrema for every waveform used at intermediate stages
// of calculation is a waste of time.

void waveform::findextrema()
{
   int i;
   double valmag,tildemag;
     
   valmagmax=0;
   valrealmax=valimagmax=NEGATIVEINFINITY;
   valmagmin=valrealmin=valimagmin=POSITIVEINFINITY;
//   valmagnexttomin=POSITIVEINFINITY;

   tildemagmax=0;
   tilderealmax=tildeimagmax=NEGATIVEINFINITY;
   tildemagmin=tilderealmin=tildeimagmin=POSITIVEINFINITY;
   tildemagnexttomin=POSITIVEINFINITY;

   for (i=0; i<N; i++)
   {
      valmag=val[i].getmod();

      if (valmagmax < valmag) valmagmax=valmag;
      if (valrealmax < val[i].x) valrealmax=val[i].x;
      if (valimagmax < val[i].y) valimagmax=val[i].y;

      if (valmagmin > valmag)
      {
//         valmagnexttomin=valmagmin;
         valmagmin=valmag;
      }
      if (valrealmin > val[i].x && val[i].x > NEGATIVEINFINITY) 
         valrealmin=val[i].x;
      if (valimagmin > val[i].y && val[i].y > NEGATIVEINFINITY) 
         valimagmin=val[i].y;

// Since the extrema frequency domain values are used only for
// plotting purposes, we choose to exclude the first 2 and last 2 bins
// from the minimum and maximum values computation since these bins'
// values are sometimes dramatically different than those within the
// rest of the array.

      if (i >1 && i < N-2)
      {
         tildemag=tilde[i].getmod();
         if (tildemagmax < tildemag) tildemagmax=tildemag;
         if (tilderealmax < tilde[i].x) tilderealmax=tilde[i].x;
         if (tildeimagmax < tilde[i].y) tildeimagmax=tilde[i].y;
         
         if (tildemagmin > tildemag) 
         {
            tildemagnexttomin=tildemagmin;
            tildemagmin=tildemag;
         }
         if (tilderealmin > tilde[i].x) tilderealmin=tilde[i].x;
         if (tildeimagmin > tilde[i].y) tildeimagmin=tilde[i].y;
      }
   }
}

// ---------------------------------------------------------------------
// As Ed Broach has pointed out, the default C++ =operator for objects
// may simply equate pointers to arrays within objects when one object
// is equated with another.  Individual elements within the arrays
// apparently are not equated to one another by the default C++
// =operator.  This can lead to segmentation errors if the arrays are
// dynamically rather than statically allocated, for the pointer to
// the original array may be destroyed before the elements within the second
// array are copied over.  So we need to write an explicit copy function 
// which transfers all of the subfields within an object to another object
// whenever the object in question has dynamically allocated arrays rather 
// than relying upon C++'s default =operator:

void waveform::docopy(const waveform& w)
{
   int i,j;

   negativetime=w.negativetime;
   timefilenamestr=w.timefilenamestr;
   freqfilenamestr=w.freqfilenamestr;
   
   for (i=0; i<N_EXTRAINFO_LINES; i++)
   {
      extrainfo[i]=w.extrainfo[i];
      extraline[i]=w.extraline[i];
   }

   title=w.title;
   timeaxislabel=w.timeaxislabel;
   timeyaxislabel=w.timeyaxislabel;
   freqaxislabel=w.freqaxislabel;
   freqyaxislabel=w.freqyaxislabel;
   
   color=w.color;
   nplots=w.nplots;
   currtime=w.currtime;
   timenorm=w.timenorm;
   freqnorm=w.freqnorm;

   tmax=w.tmax;
   tmin=w.tmin;
   freqmax=w.freqmax;
   freqmin=w.freqmin;
   E=w.E;
   Etilde=w.Etilde;

   valmagmax=w.valmagmax;
   valmagmin=w.valmagmin;
//   valmagnexttomin=w.valmagnexttomin;
   valrealmax=w.valrealmax;
   valrealmin=w.valrealmin;
   valimagmax=w.valimagmax;
   valimagmin=w.valimagmin;

   tildemagmax=w.tildemagmax;
   tildemagmin=w.tildemagmin;
   tildemagnexttomin=w.tildemagnexttomin;
   tilderealmax=w.tilderealmax;
   tilderealmin=w.tilderealmin;
   tildeimagmax=w.tildeimagmax;
   tildeimagmin=w.tildeimagmin;

   timemaxvalue=w.timemaxvalue;
   timeminvalue=w.timeminvalue;
   freqmaxvalue=w.freqmaxvalue;
   freqminvalue=w.freqminvalue;
   
   timetic=w.timetic;
   timesubtic=w.timesubtic;
   timeytic=w.timeytic;
   timeysubtic=w.timeysubtic;
   freqtic=w.freqtic;
   freqsubtic=w.freqsubtic;
   freqytic=w.freqytic;
   freqysubtic=w.freqysubtic;

   for (i=0; i<N; i++)
   {
      T[i]=w.T[i];
      val[i]=w.val[i];
      Freq[i]=w.Freq[i];
      tilde[i]=w.tilde[i];
      in_fftw[i]=w.in_fftw[i];
      out_fftw[i]=w.out_fftw[i];
   }
   Freq[N]=w.Freq[N];
   tilde[N]=w.tilde[N];
}

// Overload = operator:

waveform& waveform::operator= (const waveform& w)
{
   docopy(w);
   return *this;
}

// ---------------------------------------------------------------------
// Function fouriertransform returns the *true* Fourier transform
// H(f_n) of the time domain signal contained within the complex val
// array.  Recall that the *discrete* fourier transform H_n is related
// to the continuous true Fourier transform H(f_n) by H(f_n) = delta *
// H_n where delta denotes the sampling time interval.  (See eqn
// (12.1.8) in Numerical Recipes.)  We *do* include a factor of delta
// into the tilde array values which are computed by this routine.
// See eqn (12.1.8) in Numerical Recipes.

void waveform::fouriertransform(double delta, fftw_plan theforward)
{
   int i,iskip;

// Copy time domain values into in_fftw array:

   for (i=0; i<N; i++)
   {
      in_fftw[i].re=val[i].x;
      in_fftw[i].im=val[i].y;
   }

// Take Fourier transform using fftw:

   fftw_one(theforward, in_fftw, out_fftw);

// Reorder output from fftw and return results within array tilde.
// Multiply *discrete* fourier transform results contained within
// array tildecopy by DELTA to turn them into *true* continuous
// fourier transform values.  See eqn (12.1.8) in Numerical Recipes.

   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      tilde[i]=delta*complex(out_fftw[iskip].re,out_fftw[iskip].im);
      tilde[iskip]=delta*complex(out_fftw[i].re,out_fftw[i].im);
   }
   tilde[N]=tilde[0]; 
 
// After performing the fourier transform, find extrema values
// and compute energies in both the time and frequency domain signals

//   findextrema();
//   energy();

}

// ---------------------------------------------------------------------
void waveform::inversefouriertransform(double delta, fftw_plan thebackward)
{
   int i,iskip;

// Copy frequency domain values into tildearray.  Recall values
// contained within tilde array are *true* continuous fourier
// transform values.  We must first convert these into *discrete*
// fourier transform values H_k = H(f_k)/delta before we can use the
// Numerical Recipes FFT routines.  (See eqn 12.1.8).  Recall also
// that the frequency domain array has N+1 complex elements, whereas
// the time domain array has only N complex elements.  We simply throw
// away the N+1st element within the frequency domain array before
// taking the inverse fourier transform.
         
// Copy frequency domain values into in_fftw array and scale noting a
// reordering analagous to the forward transform

   for (i=0; i<N/2; i++)
   {
      iskip=i+N/2;
      in_fftw[i].re=tilde[iskip].x/delta;
      in_fftw[i].im=tilde[iskip].y/delta;
      in_fftw[iskip].re=tilde[i].x/delta;
      in_fftw[iskip].im=tilde[i].y/delta;
   }
 
// Take inverse Fourier transform using fftw:

   fftw_one(thebackward, in_fftw, out_fftw);

   for(i=0; i<N; i++)
   {

// According to eqn (12.1.9) in Numerical Recipes, we must divide the
// inverse Fourier transform by N in order to recover the correct
// time domain result

      val[i]=complex(out_fftw[i].re/N,out_fftw[i].im/N);
   }
   
// After performing the inverse fourier transform, find extrema values
// and compute energies in both the time and frequency domain signals

//   findextrema();
//   energy();
}

// ---------------------------------------------------------------------
// Routine energy computes the *true* total energies put out over the
// TOTALTIME interval in the time and frequency domain signals.  See
// eqn (12.1.10) in Numerical Recipes.  The DELTA and DELTAFREQ
// prefactors effectively render the energy results independent of the
// number of bins N.

// Note: In the time domain, E_true = sum_i |h_i|**2 DELTA.  In the
// frequency domain, E_true = sum_i |Htildetrue_i|**2 DELTAFREQ.

void waveform::energy(double delta,double deltafreq)
{
   int i;
   
   E=Etilde=0;
   for (i=0; i<N; i++)
   {
      E += (val[i].x*val[i].x+val[i].y*val[i].y)*delta;
      Etilde += (tilde[i].x*tilde[i].x+tilde[i].y*tilde[i].y)*deltafreq;
   }
}
   
// ---------------------------------------------------------------------
// We include the rescale_val and its analogous rescale_tilde routines below
// into the waveform class definition mainly so that we can rescale
// time domain Dolph-Chebyshev window coefficients by an arbitrary complex
// constant.

void waveform::rescale_val(complex a)
{
   int i;
   double valmag;
   
   for (i=0; i<N; i++)
   {
      val[i] *= a;
      valmag=val[i].getmod();
      
      if (valmagmax < valmag) valmagmax=valmag;
      if (valrealmax < val[i].x) valrealmax=val[i].x;
      if (valimagmax < val[i].y) valimagmax=val[i].y;

      if (valmagmin > valmag)
      {
//         valmagnexttomin=valmagmin;
         valmagmin=valmag;
      }
      if (valrealmin > val[i].x && val[i].x > NEGATIVEINFINITY) 
         valrealmin=val[i].x;
      if (valimagmin > val[i].y && val[i].y > NEGATIVEINFINITY) 
         valimagmin=val[i].y;
   }
}

void waveform::rescale_tilde(complex a)
{
   int i;
   double amag;

   for (i=0; i<N; i++)
   {
      tilde[i] *= a;
   }
   amag=a.getmod();
   
   tildemagmax *= amag;
   tildemagmin *= amag;
}
   
// ---------------------------------------------------------------------
void waveform::shift_freq(double deltafreq)
{
   int i;
         
   for (i=0; i<N+1; i++)
   {
      Freq[i] += deltafreq;
   }
}
 
// ---------------------------------------------------------------------
void waveform::add_const(double alpha)
{
   int i;
         
   for (i=0; i<N+1; i++)
   {
      tilde[i] += alpha;
   }
}

// ---------------------------------------------------------------------
void waveform::inspect_contents()
{
   int i,j;

   cout << "timefilenamestr = " << timefilenamestr << endl;
   cout << "freqfilenamestr = " << freqfilenamestr << endl;
   for (i=0; i<N_EXTRAINFO_LINES; i++)
   {
      cout << extrainfo[i] << endl;
   }

   cout << "valmagmax = " << valmagmax << endl;
   cout << "valmagmin = " << valmagmin << endl;
//   cout << "valmagnexttomin = " << valmagnexttomin << endl;
   cout << "valrealmax = " << valrealmax << endl;
   cout << "valrealmin = " << valrealmin << endl;
   cout << "valimagmax = " << valimagmax << endl;
   cout << "valimagmin = " << valimagmin << endl;

   cout << "tildemagmax = " << tildemagmax << endl;
   cout << "tildemagmin = " << tildemagmin << endl;
   cout << "tildemagnexttomin = " << tildemagnexttomin << endl;
   cout << "tilderealmax = " << tilderealmax << endl;
   cout << "tilderealmin = " << tilderealmin << endl;
   cout << "tildeimagmax = " << tildeimagmax << endl;
   cout << "tildeimagmin = " << tildeimagmin << endl;

   cout << "timenorm = " << timenorm << endl;
   cout << "freqnorm = " << freqnorm << endl;
   cout << "tmax = " << tmax << endl;
   cout << "tmin = " << tmin << endl;
   cout << "freqmax = " << freqmax << endl;
   cout << "freqmin = " << freqmin << endl;
   cout << "E = " << E << endl;
   cout << "Etilde = " << Etilde << endl;
   
   for (j=0; j<10; j++)
   {
      cout << "j = " << j << " T[j] = " << T[j] << " val[j] = " 
           << val[j] << endl;
   }
    
   for (j=N-10; j<N; j++)
   {
      cout << "j = " << j << " T[j] = " << T[j] << " val[j] = " 
           << val[j] << endl;
   }

   for (j=0; j<10; j++)
   {
      cout << "j = " << j << " Freq[j] = " << Freq[j] << " tilde[j] = " 
           << tilde[j] << endl;
   }

   for (j=N-10; j<N; j++)
   {
      cout << "j = " << j << " Freq[j] = " << Freq[j] << " tilde[j] = " 
           << tilde[j] << endl;
   }
}

// ---------------------------------------------------------------------
// Member function timefileheader writes out preliminary header
// information at the top of waveform file to set up for meta
// plotting.

void waveform::timefileheader()
{
   int i,posn;
   double minval,maxval,valdiff,timebinsize=T[1]-T[0];
   double minval_real,minval_imag,maxval_real,maxval_imag;
   string date=getcurrdate();

/*
   cout << "dbflag = " << dbflag << endl;
   cout << "fewzerosflag = " << fewzerosflag << endl;
   cout << "plot_real_part_flag = " << plot_real_part_flag << endl;
   cout << "plot_imag_part_flag = " << plot_imag_part_flag << endl;
   cout << "plot_real_and_imag_flag = " << plot_real_and_imag_flag << endl;
   cout << "plot_phase_flag = " << plot_phase_flag << endl;
   cout << "plot_mag_and_phase_flag = " << plot_mag_and_phase_flag << endl;
*/
 
// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis calls are mandatory when setting
// up a plot.
   
   if (extrainfo[0] != "")
   {
      timestream << "title ''" << endl;
      timestream << "x axis min 0 max 0.0001" << endl;
      timestream << "y axis min 0 max 0.0001" << endl;
      
      i=0;
      while (i < N_EXTRAINFO_LINES)
      {
         if (extrainfo[i] != "" )
         {
            timestream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      timestream << "storyloc 0 6.5" << endl;
      timestream << "" << endl;
   }

   if (title=="")
   {
      timestream << "title 'Waveform in Time Domain'" << endl;
   }
   else
   {
      timestream << "title '"+title+"'" << endl;
   }

// Allow for user to display times from negative to positive values.
// Recall that in discrete time, a negative time value tneg is wrapped
// around to TOTALTIME-t.

   if (negativetime)
   {
      tmax=TIMEFRAC*T[N/2];
      tmin=-TIMEFRAC*T[N/2];
   }
   timestream << "x axis min "+number_to_string(tmin)+" max "
      +number_to_string(tmax) << endl;
   
   if (timeaxislabel=="")
   {
      timestream << "label 'Time (msec)'" << endl;
   }
   else
   {
      timestream << "label '"+timeaxislabel+"'" << endl;
   }

// Allow for manual overriding of automatic time axis tic settings

   if (timetic==0)
   {
      if (negativetime)
      {
         timestream << "tics "+number_to_string(trunclog(tmax-tmin)/2)+" "
            +number_to_string(trunclog(tmax-tmin)/(2*NSUBTICS)) << endl;
      }
      else
      {
         timestream << "tics "+number_to_string(trunclog(tmax-tmin))+" "
            +number_to_string(trunclog(tmax-tmin)/NSUBTICS) << endl;
      }
   }
   else
   {
      timestream << "tics "+scinumber_to_string(timetic)
         +" "+scinumber_to_string(timesubtic) << endl;
   }

// ----------------------------------------------------------------------
// Establish y axis min and max values
// ----------------------------------------------------------------------

   if (dbflag && fewzerosflag)
   {

// In order to avoid taking logs of zero, replace a zero value for tildemagmin
// with the smallest nonzero value

      if (valmagmin==0) valmagmin=valmagnexttomin;

      if (timenorm*valmagmin > 0)
      {
         minval=mathfunc::dB(timenorm*valmagmin);
      }
      else
      {
         cout << "timenorm = " << timenorm << endl;
         cout << "valmagmin = " << valmagmin << endl;
         cout << "Can't take dB of product !" << endl;
      }
      if (timenorm*valmagmax > 0)
      {
         maxval=mathfunc::dB(timenorm*valmagmax);
      }
      else
      {
         cout << "timenorm = " << timenorm << endl;
         cout << "valmagmax = " << valmagmax << endl;
         cout << "Can't take dB of product !" << endl;
      }  
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);

// If maxval is negative and within 20 dB of 0 dB, set maxval equal to 0 dB:

      if ((maxval < 0) && (abs(maxval) < 20)) maxval=0;
      if (timeyaxislabel=="") timeyaxislabel="|Y(T) * norm| (dB)";
   }
   else if (plot_real_part_flag)
   {
      minval=timenorm*valrealmin;
      maxval=timenorm*valrealmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (timeyaxislabel=="") timeyaxislabel="Re(Y) * norm";
   }
   else if (plot_imag_part_flag)
   {
      minval=timenorm*valimagmin;
      maxval=timenorm*valimagmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (timeyaxislabel=="") timeyaxislabel="Im(Y) * norm";
   }
   else if (plot_real_and_imag_flag)
   {
      minval_real=timenorm*valrealmin;
      maxval_real=timenorm*valrealmax;
      minval_imag=timenorm*valimagmin;
      maxval_imag=timenorm*valimagmax;

      minval=min(minval_real,minval_imag);
      maxval=max(maxval_real,maxval_imag);
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (timeyaxislabel=="") timeyaxislabel
                                   ="Re(Y) [red] and Im(Y) [blue] * norm";
   }
   else if (plot_phase_flag)
   {
      maxval=180;
      minval=-180;
      if (timeyaxislabel=="") timeyaxislabel="Phase (degrees)";
   }
   else if (plot_mag_and_phase_flag)
   {
      maxval=timenorm*valmagmax;
      maxval=maxval+0.2*abs(maxval);
      minval=-maxval;
      timeyaxislabel="|Y * norm| (red) and rescaled phase (blue)";
   }
   else   // plot magnitude on linear scale
   {
      minval=timenorm*valmagmin;
      maxval=timenorm*valmagmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (timeyaxislabel=="") timeyaxislabel="|Y * norm|";
   }
   
// Allow for overriding of automatic y axis min/max value setting:
      
   if (timemaxvalue != 0) maxval=timemaxvalue;
   if (timeminvalue != 0) minval=timeminvalue;

   timestream << "yaxis min "+scinumber_to_string(minval)
      +" max "+scinumber_to_string(maxval) << endl;
   timestream << "label '"+timeyaxislabel+"'" << endl;

// Allow for manual overriding of automatic dependent variable axis tic 
// settings

   if (timeytic != 0)
   {
      timestream << "tics "+scinumber_to_string(timeytic)
         +" "+scinumber_to_string(timeysubtic) << endl;
   }
   else
   {

// If maxval > 0 and minval < 0, use larger absolute value of maxval
// and minval to set tics rather than their difference:

      if (maxval*minval > 0) 
      {
         valdiff=(maxval-minval);
      }
      else
      {
         valdiff=max(abs(maxval),abs(minval));
      }
//      cout << "maxval = " << maxval << " minval = " << minval << endl;
//      cout << "timenorm = " << timenorm << endl;
//      cout << "valdiff = " << valdiff << " trunclog(valdiff) = " << 
//         trunclog(valdiff) << endl;
      timestream << "tics "+scinumber_to_string(trunclog(valdiff))
         +" "+scinumber_to_string(trunclog(valdiff)/NSUBTICS) << endl;
   }

   timestream << "story 'Filename = "+getbasename(timefilenamestr)+"'" 
	<< endl;
   timestream << "story '"+date+"'" << endl;

   if (currtime != 0) 
	timestream << "story 'time = "+number_to_string(currtime)+"'" << endl;

   timestream << "story 'N = "+number_to_string(N)+"'" << endl;
//   timestream << "story '"+number_to_string(int(TIMEFRAC*N))+" bins displayed'"
//	     << endl;
   timestream << "story 'Bin size = "+scinumber_to_string(timebinsize)
	     +" msec'"<< endl;

   posn=timefilenamestr.find("convolve");
   timestream << "story 'Energy = "+scinumber_to_string(E)+"'" << endl;
   timestream << "story 'norm = "+number_to_string(timenorm)+"'" << endl;
   timestream << "storyloc 6 6.5 " << endl;
   timestream << endl;
}

// ---------------------------------------------------------------------
// Routine freqfileheader writes out preliminary header information at top
// of frequency domain file to set up for meta plotting.

void waveform::freqfileheader()
{
   int i,zerocount,posn;
   double minval,maxval,valdiff,freqbinsize=Freq[1]-Freq[0];
   double minval_real,minval_imag,maxval_real,maxval_imag;
   string date=getcurrdate();

// First scan through tilde array.  If more than 50% of its elements
// are 0, set fewzerosflag to false which overrides dbflag.  This
// option is useful for spectra which are essentially delta functions
// in the frequency domain:

   zerocount=0;

   for (i=0; i<N+1; i++)
   {
      if (tilde[i].getmod()==0) zerocount++;
   }
   if (zerocount/double(N) > 0.5) fewzerosflag=false;
   fewzerosflag=true;

// Two sets of "story" commands cannot be requested within one plot.  So to get
// more than one column of story output to appear, we need to create dummy
// plots with no content and then call the story command.  Title, x axis
// and y axis calls are mandatory when setting up a plot.  
   
   if (extrainfo[0] != "")
   {
      freqstream << "title ''" << endl;
      freqstream << "x axis min 0 max 0.0001" << endl;
      freqstream << "y axis min 0 max 0.0001" << endl;
      
      i=0;
      while (i < N_EXTRAINFO_LINES)
      { 
         if (extrainfo[i] != "")
         {
            freqstream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      freqstream << "storyloc 0 6.5" << endl;
      freqstream << "" << endl;
   }
   
   if (title=="")
   {
      freqstream << "title 'Waveform in Frequency Domain'" << endl;
   }
   else
   {
      freqstream << "title '"+title+"'" << endl;
   }
   freqstream << "x axis min "+number_to_string(freqmin)+" max "
      +number_to_string(freqmax) << endl;

   if (freqaxislabel=="")
   {
      freqstream << "label 'Frequency (kHz)'" << endl;
   }
   else
   {
      freqstream << "label '"+freqaxislabel+"'" << endl;
   }

// Allow for manual overriding of automatic frequency axis tic settings

   if (freqtic!=0)
   {
      freqstream << "tics "+scinumber_to_string(freqtic)
         +" "+scinumber_to_string(freqsubtic) << endl;
   }
   else
   {
      freqstream << "tics "+number_to_string(trunclog(freqmax))+" "
         +number_to_string(trunclog(freqmax)/NSUBTICS) << endl;
   }
   
// ----------------------------------------------------------------------
// Establish y axis min and max values
// ----------------------------------------------------------------------

   if (dbflag && fewzerosflag)
   {

// In order to avoid taking logs of zero, replace a zero value for tildemagmin
// with the smallest nonzero value

      if (tildemagmin==0) tildemagmin=tildemagnexttomin;

      if (freqnorm*tildemagmin > 0)
      {
         minval=mathfunc::dB(freqnorm*tildemagmin);
      }
      else
      {
         cout << "freqnorm = " << freqnorm << endl;
         cout << "tildemagmin = " << tildemagmin << endl;
         cout << "Can't take dB of product !" << endl;
      }
      if (freqnorm*tildemagmax > 0)
      {
         maxval=mathfunc::dB(freqnorm*tildemagmax);
      }
      else
      {
         cout << "freqnorm = " << freqnorm << endl;
         cout << "tildemagmax = " << tildemagmax << endl;
         cout << "Can't take dB of product !" << endl;
      }  
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);

// If maxval is negative and within 20 dB of 0 dB, set maxval equal to 0 dB:

      if ((maxval < 0) && (abs(maxval) < 20)) maxval=0;
      if (freqyaxislabel=="") freqyaxislabel="|Ytilde(Freq) * norm| (dB)";
   }
   else if (plot_real_part_flag)
   {
      minval=freqnorm*tilderealmin;
      maxval=freqnorm*tilderealmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (freqyaxislabel=="") freqyaxislabel="Re(Ytilde) * norm";
   }
   else if (plot_imag_part_flag)
   {
      minval=freqnorm*tildeimagmin;
      maxval=freqnorm*tildeimagmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (freqyaxislabel=="") freqyaxislabel="Im(Ytilde) * norm";
   }
   else if (plot_real_and_imag_flag)
   {
      minval_real=freqnorm*tilderealmin;
      maxval_real=freqnorm*tilderealmax;
      minval_imag=freqnorm*tildeimagmin;
      maxval_imag=freqnorm*tildeimagmax;

      minval=min(minval_real,minval_imag);
      maxval=max(maxval_real,maxval_imag);
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (freqyaxislabel=="") freqyaxislabel
          ="Re(Ytilde) [red] and Im(Ytilde) [blue] * norm";
   }
   else if (plot_phase_flag)
   {
      maxval=180;
      minval=-180;
      if (freqyaxislabel=="") freqyaxislabel="Phase (degrees)";
   }
   else if (plot_mag_and_phase_flag)
   {
      maxval=freqnorm*tildemagmax;
      maxval=maxval+0.2*abs(maxval);
      minval=-maxval;
      freqyaxislabel="|Y * norm| (red) and rescaled phase (blue)";
   }
   else   // plot magnitude on linear scale
   {
      minval=freqnorm*tildemagmin;
      maxval=freqnorm*tildemagmax;
      minval=minval-0.2*abs(minval);
      maxval=maxval+0.2*abs(maxval);
      if (freqyaxislabel=="") freqyaxislabel="|Ytilde * norm|";
   }

// Allow for overriding of automatic y axis min/max value setting:
      
      if (freqmaxvalue != 0) maxval=freqmaxvalue;
      if (freqminvalue != 0) minval=freqminvalue;

      freqstream << "yaxis min "+scinumber_to_string(minval)
         +" max "+scinumber_to_string(maxval) << endl;
      freqstream << "label '"+freqyaxislabel+"'" << endl;

// ----------------------------------------------------------------------
// Establish y axis tic mark settings
// ----------------------------------------------------------------------

// Allow for manual overriding of dependent variable axis automatic
// tic settings:

   if (freqytic != 0)
   {
      freqstream << "tics "+scinumber_to_string(freqytic)
         +" "+scinumber_to_string(freqysubtic) << endl;
   }
   else
   {

// If maxval > 0 and minval < 0, use larger absolute value of maxval
// and minval to set tics rather than their difference:

      if (maxval*minval > 0) 
      {
         valdiff=(maxval-minval);
      }
      else
      {
         valdiff=max(abs(maxval),abs(minval));
      }

//      cout << "In freqheader section:" << endl;
//      cout << "maxval = " << maxval << " minval = " << minval << endl;
//      cout << "freqnorm = " << timenorm << endl;
//      cout << "valdiff = " << valdiff << " trunclog(valdiff) = " << 
//         trunclog(valdiff) << endl;

      freqstream << "tics "+scinumber_to_string(trunclog(valdiff))
         +" "+scinumber_to_string(trunclog(valdiff)/NSUBTICS) << endl;
   }

// ----------------------------------------------------------------------
// Story lines
// ----------------------------------------------------------------------

   freqstream << "story 'Filename = "+getbasename(freqfilenamestr)+"'" 
	<< endl;
   freqstream << "story '"+date+"'" << endl;

   if (currtime != 0) 
	freqstream << "story 'time = "+number_to_string(currtime)+"'" << endl;

   freqstream << "story 'N = "+number_to_string(N)+"'" << endl;
//   freqstream << "story '"+number_to_string(int(FREQFRAC*N))+" bins displayed'"
//	     << endl;
   freqstream << "story 'Bin size = "+scinumber_to_string(freqbinsize)
	     +" kHz'"<< endl;

   posn=freqfilenamestr.find("convolve");

   freqstream << "story 'Energy = "+scinumber_to_string(Etilde)+"'" << endl;
   freqstream << "story 'norm = "+number_to_string(freqnorm)+"'" << endl;
   freqstream << "storyloc 6 6.5" << endl;
   freqstream << endl;
}

// ---------------------------------------------------------------------
// Member function openfiles opens up time and frequency domain files
// and initializes both with calls to the appropriate header
// subroutines:

void waveform::openfiles(double delta,double deltafreq,string outfilenamestem,
                         int filenumber)
{
   string timedir=setdirname("timefiles/");
   string freqdir=setdirname("freqfiles/");
   
// First call findextrema and energy to set extremal and energy
// values within time and frequency domain plots:

   findextrema();
   energy(delta,deltafreq);

// Next set up filenames for time and frequency domain plots

   string filenumberstring="";
   if (filenumber >= 0) filenumberstring=number_to_string(filenumber);

   timefilenamestr=timedir+outfilenamestem+filenumberstring+".time"; 
   freqfilenamestr=freqdir+outfilenamestem+filenumberstring+".freq"; 
   cout << "timefilename = " << timefilenamestr << endl;
   cout << "freqfilename = " << freqfilenamestr << endl;

   if (openfile(timefilenamestr,timestream))
   {
      timefileheader();
   }
   if (openfile(freqfilenamestr,freqstream))
   {
      freqfileheader();
   }
}

void waveform::closefiles()
{
   closefile(timefilenamestr,timestream);
   closefile(freqfilenamestr,freqstream);
}

// ---------------------------------------------------------------------
// Member function writewaveforms writes out time and frequency domain
// waveforms to separate metafiles:

void waveform::writedata(double delta)
{
   const int NPRECISION=5;
   
   int i;
   double currdBval;
   double Tnew[N];
   complex valnew[N];
   string unixcommandstr;
   
   timestream.setf(ios::fixed);
   timestream.setf(ios::showpoint);  
   timestream.precision(NPRECISION);
   timestream << "curve" << endl;

   freqstream.setf(ios::fixed);
   freqstream.setf(ios::showpoint);
   freqstream.precision(NPRECISION);
   freqstream << "curve" << endl;

   if (color != -1)
   {
      timestream << "color "+colorfunc::getcolor(color);
      freqstream << "color "+colorfunc::getcolor(color);
      timestream << endl;
      freqstream << endl;
   }

// Allow user to enter extra lines into timefile and freqfile by hand.
// Same lines will be added to both.

   i=0;
   while(i < N_EXTRAINFO_LINES)
   {
      if (extraline[i] != "")
      {
         timestream << extraline[i] << endl;
         freqstream << extraline[i] << endl;
      }
      i++;
   }
   timestream << endl;
   freqstream << endl;

// If negativetime==true, rearrange the first and second N/2 data
// points so that the second set of N/2 data points which correspond
// to negative times is printed out to file first and the first N/2
// data points which correspond to positive times are printed second:

   if (negativetime)
   {
      for (i=N/2; i<N; i++)		// rearrange second N/2 data points
      {
         Tnew[i-N/2]=T[i]-N*delta;	// set times to negative values
         valnew[i-N/2]=val[i];
      }
      for (i=0; i<N/2; i++)		// rearrange first N/2 data points
      {
         Tnew[i+N/2]=T[i];
         valnew[i+N/2]=val[i];
      }
      for (i=0; i<N; i++)
      {
         T[i]=Tnew[i];
         val[i]=valnew[i];
      }
   }

   if (plot_real_and_imag_flag)
   {
      timestream << "color red" << endl;
      timestream << endl;
      for (i=0; i<N; i++)
      {
         timestream << T[i] << "\t\t" << timenorm*val[i].x << endl;
      }
      timestream << endl;
      timestream << "curve" << endl;
      timestream << "color blue" << endl;
      timestream << endl;
      for (i=0; i<N; i++)
      {
         timestream << T[i] << "\t\t" << timenorm*val[i].y << endl;
      }
   }
   else if (plot_mag_and_phase_flag)
   {
      timestream << "color red" << endl;
      timestream << endl;
      for (i=0; i<N; i++)
      {
         timestream << T[i] << "\t\t" << 
            timenorm*val[i].getmod() << endl;
      }
      timestream << endl;
      timestream << "curve" << endl;
      timestream << "color blue" << endl;
      timestream << endl;

// Plot rescaled phase information on same graph as linear magnitude curve.
// Rescale phase so that theta=PI=180 degrees maps onto valmagmax:

      for (i=0; i<N; i++)
      {
         timestream << T[i] << "\t\t" 
                    << val[i].getarg()*valmagmax/PI
                    << endl;
      }
      timestream << endl;
   }
   else
   {
      for (i=0; i<N; i++)
      {
         if (plot_real_part_flag)
         {
            timestream << T[i] << "\t\t" << timenorm*val[i].x << endl;
         }
         else if (plot_imag_part_flag)
         {
            timestream << T[i] << "\t\t" << timenorm*val[i].y << endl;
         }
         else if (plot_phase_flag)
         {
            timestream << T[i] << "\t\t" 
                       << val[i].getarg()*180/PI << endl;
         }
         else if (dbflag && fewzerosflag)
         {
            if (timenorm*val[i].getmod() > 0)
            {
               currdBval=mathfunc::dB(timenorm*val[i].getmod());
            }
            else
            {
               cout << "timenorm = " << timenorm << endl;
               cout << "|val[i]| = " << val[i].getmod() << endl;
               cout << "i = " << i << endl;
               cout << "Can't take dB of product !" << endl;
            }

            if (currdBval > NEGATIVEINFINITY)
            {
               timestream << T[i] << "\t\t" << currdBval << endl;
            }
         }
         else  // plot magnitude on linear scale
         {
            timestream << T[i] << "\t\t" << 
               timenorm*val[i].getmod() << endl;
         }
      }
   }
   timestream << endl;
//   if (nplots==1) timestream.close();

   if (plot_real_and_imag_flag)
   {
      freqstream << "color red" << endl;
      freqstream << endl;
      for (i=0; i<N+1; i++)
      {
         freqstream << Freq[i] << "\t\t" << freqnorm*tilde[i].x 
                    << endl;
      }
      freqstream << endl;
      freqstream << "curve" << endl;
      freqstream << "color blue" << endl;
      freqstream << endl;
      for (i=0; i<N+1; i++)
      {
         freqstream << Freq[i] << "\t\t" << freqnorm*tilde[i].y 
                    << endl;
      }
   }
   else if (plot_mag_and_phase_flag)
   {
      freqstream << "color red" << endl;
      freqstream << endl;
      for (i=0; i<N; i++)
      {
         freqstream << Freq[i] << "\t\t" << 
            freqnorm*tilde[i].getmod() << endl;
      }
      freqstream << endl;
      freqstream << "curve" << endl;
      freqstream << "color blue" << endl;
      freqstream << endl;

// Plot rescaled phase information on same graph as linear magnitude curve.
// Rescale phase so that theta=PI=180 degrees maps onto valmagmax:

      for (i=0; i<N; i++)
      {
         freqstream << Freq[i] << "\t\t" << tilde[i].getarg()
            *tildemagmax/PI << endl;
      }
      freqstream << endl;
   }
   else
   {
      for (i=0; i<N+1; i++)
      {
         if (plot_real_part_flag)
         {
            freqstream << Freq[i] << "\t\t" << freqnorm*tilde[i].x 
                       << endl;
         }
         else if (plot_imag_part_flag)
         {
            freqstream << Freq[i] << "\t\t" << freqnorm*tilde[i].y 
                       << endl;
         }
         else if (plot_phase_flag)
         {
            freqstream << Freq[i] << "\t\t" 
                       << tilde[i].getarg()*180/PI
                       << endl;
         }
         else if (dbflag && fewzerosflag)
         {
            if (freqnorm*tilde[i].getmod() > 0)
            {
               currdBval=mathfunc::dB(freqnorm*tilde[i].getmod());
            }
            else
            {
               cout << "freqnorm = " << freqnorm << endl;
               cout << "|tilde[i]| = " << tilde[i].getmod()
                    << endl;
               cout << "i = " << i << endl;
               cout << "Can't take dB of product !" << endl;
            }

            if (currdBval > NEGATIVEINFINITY)
            {
               freqstream << Freq[i] << "\t\t" << currdBval << endl;
            }
         }
         else
         {
            freqstream << Freq[i] << "\t\t" 
                       << freqnorm*tilde[i].getmod() 
                       << endl;
         }
      }
   }
   freqstream << endl;
//   if (nplots==1) freqstream.close();   

// Call adobe -ps  to convert meta files into postscript

   unixcommandstr="adobe -ps -o "+timefilenamestr;
   unix_command(unixcommandstr);
   unixcommandstr="adobe -ps -o "+freqfilenamestr;
   unix_command(unixcommandstr);
}

// ---------------------------------------------------------------------
// Member function dBconvert takes in a waveform w and returns a new
// waveform whose time domain values equal the real parts of those in
// w in dB

waveform waveform::dBconvert(double delta,fftw_plan theforward)
{
   complex dBmag[N];

   for (int i=0; i<N; i++)
   {
      dBmag[i]=complex(mathfunc::dB(abs(val[i].x)),0);
   }
   
   waveform wdB(delta,dBmag);
   wdB.fouriertransform(delta,theforward);
   return wdB;
}

// ---------------------------------------------------------------------
// Member function Loadwaveform is a quick-and-dirty variant on
// Loaddataarray.  This routine converts the entire contents of a meta
// file which has been read into the string array currline via our
// ReadInfile routine into the frequency domain part of a waveform.
// (Someday, we should generalize this routine so that the data can be
// read into the time domain part as well.)  The number of lines
// within the string array currline must be passed as the input
// parameter nlines to this routine. The number of curves contained
// within the metafile (which should always be one) is returned as an
// output parameter.

waveform waveform::Loadwaveform(double delta,string currline[],int nlines,
                                int& curvenumber)
{
   int linenumber;
   double x,y;
   complex value[N+1];

   curvenumber=linenumber=0;
   while (linenumber < nlines)
   {
      while(!is_number(currline[linenumber]) && linenumber < nlines)
      {
         linenumber++;
      }

      if (linenumber >= nlines)
      {
         break;      
      }
      else
      {
         cout << "First (x,y) pair encountered at linenumber "
              << linenumber << endl;
      }
      
      int i=0;
      while(is_number(currline[linenumber]))
      {
         string_to_two_numbers(currline[linenumber],x,y);
         value[i]=y;
         i++;
         linenumber++;
      }
      curvenumber++;
      outputfunc::newline();
   }
   cout << "curvenumber = " << curvenumber << endl;
   waveform w(delta,value);
   return w;
}



