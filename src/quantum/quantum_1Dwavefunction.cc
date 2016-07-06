// ==========================================================================
// Quantum_1Dwavefunction class member function definitions
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include "math/complex.h"
#include "datastructures/containerfuncs.h"
#include "datastructures/datapoint.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/genfuncs_complex.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/quantum_1Dwavefunction.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"

using std::string;
using std::ostream;
using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;

const int quantum_1Dwavefunction::Nmax_energystates=50;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void quantum_1Dwavefunction::allocate_member_objects()
{
   new_clear_array(prev_arg,nxbins_max);
   new_clear_array(curr_arg,nxbins_max);
   value=new_clear_carray(nxbins_max);
   tilde=new_clear_carray(nxbins_max);
   energy_eigenstate=new complex[Nmax_energystates][nxbins_max];
   xmeanlist.allocate_member_objects();
   xsigmalist.allocate_member_objects();
   kmeanlist.allocate_member_objects();
   ksigmalist.allocate_member_objects();
}

void quantum_1Dwavefunction::initialize_member_objects()
{
   ndims=1;
}

quantum_1Dwavefunction::quantum_1Dwavefunction(void)
{
   allocate_member_objects();
   initialize_member_objects();
   clear_wavefunction();
}

// Copy constructor

quantum_1Dwavefunction::quantum_1Dwavefunction(
   const quantum_1Dwavefunction& q):
   quantum_wavefunction(q)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(q);
}

quantum_1Dwavefunction::~quantum_1Dwavefunction()
{
   delete [] prev_arg;
   delete [] curr_arg;
   delete [] value;
   delete [] tilde;
   delete [] energy_eigenstate;
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::docopy(const quantum_1Dwavefunction& q)
{
   kx_hi=q.kx_hi;
   kx_lo=q.kx_lo;
   delta_kx=q.delta_kx;
   xperiod=q.xperiod;
   for (int i=0; i<nxbins_max; i++)
   {
      curr_arg[i]=q.curr_arg[i];
      prev_arg[i]=q.prev_arg[i];
      value[i]=q.value[i];
      tilde[i]=q.tilde[i];
      for (int j=0; j<Nmax_energystates; j++)
      {
         energy_eigenstate[j][i]=q.energy_eigenstate[j][i];
      }
   }
}	

// Overload = operator:

quantum_1Dwavefunction& quantum_1Dwavefunction::operator= 
(const quantum_1Dwavefunction& q)
{
   if (this==&q) return *this;
   quantum_wavefunction::operator=(q);
   docopy(q);
   return *this;
}

// =====================================================================
// FFT member functions:
// =====================================================================

// Function fouriertransform returns the *true* momentum space Fourier
// transform H(kx) of the spatial domain signal contained within the
// complex val array.  The number of bins within the incoming data
// array must be ODD!  Recall that the *discrete* fourier transform
// H(n) is related to the continuous true Fourier transform H(kx) by
// H(kx) = dx * H(n) where dx denotes the bin size.  We *do* include a
// factor of dx into the momentum space tilde_out array values which
// are computed by this routine.

void quantum_1Dwavefunction::fouriertransform(
   const complex value_in[nxbins_max],complex tilde_out[nxbins_max])
{
   const double EXTREMELY_TINY=1.0E-10;
//   const double EXTREMELY_TINY=1.0E-15;

   int i,iskip;
   complex (*tmp_value)=new_clear_carray(nxbins_max);
   complex (*tmp_tilde)=new_clear_carray(nxbins_max);

// Introduce arrays in_fftw and out_fftw of type fftw_complex in order
// to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftw=new fftw_complex[nxbins_max];
   fftw_complex *out_fftw=new fftw_complex[nxbins_max];

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
      in_fftw[i].re=tmp_value[i].x;
      in_fftw[i].im=tmp_value[i].y;
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
      if (fabs(tilde_out[i].x) < EXTREMELY_TINY) tilde_out[i].x=0;
      if (fabs(tilde_out[i].y) < EXTREMELY_TINY) tilde_out[i].y=0;
   }

   delete [] tmp_value;
   delete [] tmp_tilde;
   delete [] in_fftw;
   delete [] out_fftw;
}

// ---------------------------------------------------------------------
// Function inversefouriertransform returns the *true* spatial domain
// inverse Fourier transform h(x) of the 1D momentum space signal
// contained within the complex array tilde.  The number of rows and
// columns within the incoming data array must be ODD!  Recall that
// the *discrete* inverse fourier transform H(n) is related to the
// continuous true inverse Fourier transform h(x) by h(x) =
// H(n)/nxbins.  We *do* include a factor of 1/nxbins into the spatial
// domain value_out array computed by this subroutine.

void quantum_1Dwavefunction::inversefouriertransform(
   const complex tilde_in[nxbins_max],complex value_out[nxbins_max])
{
   const double EXTREMELY_TINY=1.0E-10;
//   const double EXTREMELY_TINY=1.0E-15;

   int i,iskip;
   complex (*tmp_tilde)=new_clear_carray(nxbins_max);
   complex (*tmp_value)=new_clear_carray(nxbins_max);

// Introduce arrays in_fftw and out_fftw of type fftw_complex in order
// to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftw=new fftw_complex[nxbins_max];
   fftw_complex *out_fftw=new fftw_complex[nxbins_max];

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
      in_fftw[i].re=tmp_tilde[i].x;
      in_fftw[i].im=tmp_tilde[i].y;
   }

// Take inverse Fourier transform using fftw:

   fftw_one(thebackward_1d,in_fftw,out_fftw);

   for (i=0; i<nxbins; i++)
   {
      tmp_value[i]=complex(out_fftw[i].re,out_fftw[i].im);
      if (fabs(tmp_value[i].x) < EXTREMELY_TINY) tmp_value[i].x=0;
      if (fabs(tmp_value[i].y) < EXTREMELY_TINY) tmp_value[i].y=0;
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
      if (fabs(value_out[i].x) < EXTREMELY_TINY) value_out[i].x=0;
      if (fabs(value_out[i].y) < EXTREMELY_TINY) value_out[i].y=0;
   }

   delete [] tmp_value;
   delete [] tmp_tilde;
   delete [] in_fftw;
   delete [] out_fftw;
}

// =====================================================================
// Metafile member functions:
// =====================================================================

quantumarray quantum_1Dwavefunction::plot_potential(int imagenumber)
{
   string plotfilename="potential"+stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   return plot_potential(plotfilename);
}

// ---------------------------------------------------------------------
quantumarray quantum_1Dwavefunction::plot_potential(string plotfilename)
{
   outputfunc::write_banner("Plotting potential:");

   const double TINY_NEGATIVE=-0.00001;
   const double TINY_OFFSET=0.002;  // Displace potential zero values by
				    // tiny vertical distance so that they
				    // can be seen in magnitude plots

// First compute potential values and store them within twoDarray
// potential_twoDarray:

   twoDarray potential_twoDarray(1,nxbins);
   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;
      double V,dV,d2V;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);

      if (V < POSITIVEINFINITY)
      {
         potential_twoDarray.put(0,i,V+TINY_OFFSET);
      }
      else
      {
         potential_twoDarray.put(0,i,POSITIVEINFINITY/2.0);
         // Divide by 2 so that potential value will show up on plot
      }
   }
   quantumarray Vpot(xlo,deltax,potential_twoDarray);

   Vpot.datafilenamestr=imagedir+plotfilename;
   filefunc::openfile(Vpot.datafilenamestr+".meta",Vpot.datastream);

// We assume that period for periodic potentials is the interval [-PI,PI]:

   if (xperiod<POSITIVEINFINITY)
   {
      Vpot.xmin=-PI;
      Vpot.xmax=PI;
      Vpot.xtic=PI/2;
      Vpot.xsubtic=PI/4;
      Vpot.xticlabel=
         "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
      Vpot.xlabel="Phase ^g\152^u";
   }
   else
   {
      Vpot.xmin=display_frac*(xlo)-0.5*deltax;
      Vpot.xmax=display_frac*(xhi)+0.5*deltax;

      if (potential_type==potentialfunc::squid)
      {
         Vpot.xlabel="^g\152^u-^g\152^u^-0^n";
         Vpot.xmin=-3*PI/2;
         Vpot.xmax=3*PI/2;
         Vpot.xtic=PI/2;
         Vpot.xsubtic=PI/2;
         Vpot.xticlabel=
     "[-3^g\160^u/2 -^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u 3^g\160^u/2]";
         Vpot.ylabel="V/E^-C^n";
         Vpot.ytic=10;		// notional SQUID
         Vpot.ysubtic=5;	// notional SQUID
//         Vpot.ytic=2000;	// realistic 1D SQUID
//         Vpot.ysubtic=1000;	// realistic 1D SQUID
         Vpot.title="SQUID Potential";
         Vpot.subtitle="^g\152^u^-0^n = "+
            stringfunc::number_to_string(potential_param[2]*180/PI)+"\241";
      }
      else
      {
         Vpot.xlabel="x";
         Vpot.ylabel="V";
         Vpot.title="Potential";
      }
   }

//   const double Vplotmax=12;	// appropriate for smooth box
   const double Vplotmax=30;	// appropriate for notional squid
//   const double Vplotmax=14000;  // appropriate for realistic 1D squid
   double maxval,minval;
   Vpot.find_max_min_vals(maxval,minval);

   if (potential_type==potentialfunc::squid)
   {
      Vpot.yplotmaxval=Vplotmax;
      Vpot.yplotminval=TINY_NEGATIVE;
   }
   else
   {
      Vpot.yplotmaxval=maxval;
      Vpot.yplotminval=minval;
   }
   
   Vpot.singletfile_header();
   Vpot.writedataarray(xshift);
   filefunc::closefile(Vpot.datafilenamestr+".meta",Vpot.datastream);

   filefunc::meta_to_jpeg(Vpot.datafilenamestr);
   filefunc::gzip_file(Vpot.datafilenamestr+".meta");
   outputfunc::newline();

   return Vpot;
}

// ---------------------------------------------------------------------
quantumarray quantum_1Dwavefunction::plot_wavefunction(
   twoDarray& psi_twoDarray,int imagenumber)
{
   const double TINY_NEGATIVE=-0.00001;
   const double ymax=1;
   const double ymin=-ymax;
//   const double Vplotmax=8;  // appropriate for harmonic oscillator
   const double Vplotmax=30; // appropriate for notional squid
//   const double Vplotmax=12; // appropriate for smooth box

   quantumarray wavefunction(xlo,deltax,psi_twoDarray);

   wavefunction.datafilenamestr=imagedir+"image"
       +stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   if (!viewgraph_mode) wavefunction.pagetitle=pagetitle;
   filefunc::openfile(wavefunction.datafilenamestr+".meta",wavefunction.datastream);

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (xperiod<POSITIVEINFINITY)
      {
         wavefunction.xmin=-PI;
         wavefunction.xmax=PI;
         wavefunction.xtic=PI/2;
         wavefunction.xsubtic=PI/4;
         wavefunction.xticlabel=
            "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
         wavefunction.xlabel="^g\152^u";
      }
      else
      {
         if (potential_type==potentialfunc::squid)
         {
            wavefunction.xlabel="^g\152^u-^g\152^u^-0^n";
//            wavefunction.xlabel="^g\152^u";
            wavefunction.xmin=-3*PI/2;
            wavefunction.xmax=3*PI/2;
            wavefunction.xtic=PI/2;
            wavefunction.xsubtic=PI/2;
            wavefunction.xticlabel=
     "[-3^g\160^u/2 -^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u 3^g\160^u/2]";
         }
         else
         {
            wavefunction.xlabel="X";
            wavefunction.xmin=display_frac*(xlo)-0.5*deltax;
            wavefunction.xmax=display_frac*(xhi)+0.5*deltax;
         }
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      wavefunction.xmin=display_frac*kx_lo-0.5*delta_kx;
      wavefunction.xmax=display_frac*kx_hi+0.5*delta_kx;
      wavefunction.xlabel="Kx";
   }

   for (int doublet_pair_member=0; doublet_pair_member<2; 
        doublet_pair_member++)
   {
      if (doublet_pair_member==0)
      {
         if (complex_plot_type==quantum::mag_phase)
         {
            wavefunction.title="Magnitude";
            wavefunction.ylabel="|^g\171^u|";
            wavefunction.yplotminval=TINY_NEGATIVE;
            wavefunction.yplotmaxval=ymax;
           
         }
         else if (complex_plot_type==quantum::real_imag)
         {
            wavefunction.title="Real Part";
            wavefunction.ylabel="Re(^g\171^u)";
            wavefunction.yplotmaxval=ymax;
            wavefunction.yplotminval=ymin;
         }
         else if (complex_plot_type==quantum::energy_prob ||
                  complex_plot_type==quantum::energy_real)
         {
            wavefunction.title="Potential Energy";
            wavefunction.ylabelsize=1.3;
            if (potential_type==potentialfunc::squid)
            {
               wavefunction.xtic=PI/2;
               wavefunction.xsubtic=PI/2;
               wavefunction.xticlabel=
    "[-3^g\160^u/2 -^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u 3^g\160^u/2]";
               wavefunction.ylabel="V(^g\152^u)";
               wavefunction.ytic=10;
               wavefunction.ysubtic=5;
            }
            else
            {
               wavefunction.xtic=2;
               wavefunction.xsubtic=1;
               wavefunction.ylabel="V(x)";
               wavefunction.ytic=2;
               wavefunction.ysubtic=1;
            }
            wavefunction.yplotminval=TINY_NEGATIVE;
            wavefunction.yplotmaxval=Vplotmax;
         }
      }
      else if (doublet_pair_member==1)
      {
         if (complex_plot_type==quantum::mag_phase)
         {
            wavefunction.title="Phase";
            wavefunction.ylabel="arg(^g\171^u)";
            wavefunction.yplotminval=-PI;
            wavefunction.yplotmaxval=PI;
            wavefunction.ytic=PI/2;
            wavefunction.ysubtic=PI/4;
            wavefunction.yticlabel=
               "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
         }
         else if (complex_plot_type==quantum::real_imag)
         {
            wavefunction.title="Imaginary Part";
            wavefunction.ylabel="Im(^g\171^u)";
            wavefunction.yplotmaxval=ymax;
            wavefunction.yplotminval=ymin;
         }
         else if (complex_plot_type==quantum::energy_prob)
         {
            wavefunction.title="Probability Density";
            if (potential_type==potentialfunc::squid)
            {
               wavefunction.xtic=PI/2;
               wavefunction.xsubtic=PI/2;
               wavefunction.xticlabel=
    "[-3^g\160^u/2 -^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u 3^g\160^u/2]";
            }
            else
            {
               wavefunction.xtic=2;
               wavefunction.xsubtic=1;
            }
            wavefunction.ylabel="|^g\171^u|^+2^n";
            wavefunction.ylabelsize=1.3;
            wavefunction.yplotminval=TINY_NEGATIVE;
            wavefunction.yplotmaxval=ymax;
            wavefunction.ytic=0.2;
            wavefunction.ysubtic=0.2;
         }
         else if (complex_plot_type==quantum::energy_real)
         {
            wavefunction.title="Real Part";
            wavefunction.ylabel="Re(^g\171^u)";
            wavefunction.yplotmaxval=ymax;
            wavefunction.yplotminval=ymin;
            wavefunction.ytic=0.2;
            wavefunction.ysubtic=0.2;
         }
      }
      wavefunction.thickness=2;
      wavefunction.doubletfile_header(doublet_pair_member);
      wavefunction.write_doublet_data(doublet_pair_member,xshift,domain_name,
                                      complex_plot_type);
   }	// doublet_pair_member loop

   filefunc::closefile(wavefunction.datafilenamestr+".meta",wavefunction.datastream);

   return wavefunction;
}

// ---------------------------------------------------------------------
// This overloaded version of member function plot_wavefunction takes
// in position and momentum space twoDarrays which are filled with
// wavefunction magnitude information.  This subroutine writes the
// double magnitude information to metafile output.

quantumarray quantum_1Dwavefunction::plot_wavefunction(
   twoDarray& position_twoDarray,twoDarray& momentum_twoDarray,
   int imagenumber)
{
   const double TINY_NEGATIVE=-0.00001;
   const double ymax=1;

   quantumarray posnfunction(xlo,deltax,position_twoDarray);
   quantumarray momentumfunction(kx_lo,delta_kx,momentum_twoDarray);

   posnfunction.datafilenamestr=momentumfunction.datafilenamestr=
      imagedir+"image"+stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   if (!viewgraph_mode) posnfunction.pagetitle=pagetitle;

// We assume that period for periodic potentials is the interval [-PI,PI]:

   if (xperiod<POSITIVEINFINITY)
   {
      posnfunction.xmin=-PI;
      posnfunction.xmax=PI;
      posnfunction.xtic=PI/2;
      posnfunction.xsubtic=PI/4;
      posnfunction.xticlabel=
         "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
      posnfunction.xlabel="^g\152^u";
   }
   else
   {
      posnfunction.xmin=display_frac*(xlo)-0.5*deltax;
      posnfunction.xmax=display_frac*(xhi)+0.5*deltax;
      if (potential_type==potentialfunc::squid)
      {
         posnfunction.xlabel="^g\152^u-^g\152^u^-0^n";
      }
      else
      {
         posnfunction.xlabel="X";
      }
   }

   momentumfunction.xmin=display_frac*kx_lo-0.5*delta_kx;
   momentumfunction.xmax=display_frac*kx_hi+0.5*delta_kx;
   momentumfunction.xlabel="Kx";

   for (int doublet_pair_member=0; doublet_pair_member<2; 
        doublet_pair_member++)
   {
      if (doublet_pair_member==0)
      {
         filefunc::openfile(posnfunction.datafilenamestr+".meta",
                  posnfunction.datastream);

         posnfunction.title="Position space magnitude";
         posnfunction.ylabel="|^g\171^u|";
         posnfunction.yplotminval=TINY_NEGATIVE;
         posnfunction.yplotmaxval=ymax;
         posnfunction.thickness=2;
         posnfunction.doubletfile_header(doublet_pair_member);
         posnfunction.write_doublet_data(
            doublet_pair_member,xshift,quantum::position_space,
            complex_plot_type);
         filefunc::closefile(posnfunction.datafilenamestr+".meta",
                   posnfunction.datastream);
      }
      else if (doublet_pair_member==1)
      {
         filefunc::appendfile(momentumfunction.datafilenamestr+".meta",
                    momentumfunction.datastream);
         momentumfunction.title="Momentum space magnitude";
         momentumfunction.ylabel="|^g\171^u|";
         momentumfunction.yplotminval=TINY_NEGATIVE;
         momentumfunction.yplotmaxval=ymax;
         momentumfunction.thickness=2;
         momentumfunction.doubletfile_header(doublet_pair_member);
         momentumfunction.write_doublet_data(
            doublet_pair_member,xshift,quantum::momentum_space,
            complex_plot_type);
         filefunc::closefile(momentumfunction.datafilenamestr+".meta",
                   momentumfunction.datastream);
      }
   }	// doublet_pair_member loop
   return posnfunction;
}

// ---------------------------------------------------------------------
// Member function plot_probdensity takes in some combination of
// potential energy, wavefunction probability density and energy
// eigenstate information in psi_twoDarray and returns a corresponding
// quantumarray with this information encoded.  It also sets various
// metafile plot parameters.

quantumarray quantum_1Dwavefunction::plot_probdensity(
   twoDarray& psi_twoDarray,int imagenumber)
{
   const double TINY_NEGATIVE=-0.00001;
   const double ymax=1;
   
   quantumarray probdensity(xlo,deltax,psi_twoDarray);

   probdensity.datafilenamestr=imagedir+"image"
      +stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   filefunc::openfile(probdensity.datafilenamestr+".meta",probdensity.datastream);

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (xperiod<POSITIVEINFINITY)
      {
         probdensity.xmin=-PI;
         probdensity.xmax=PI;
         probdensity.xtic=PI/2;
         probdensity.xsubtic=PI/4;
         probdensity.xticlabel=
            "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
         probdensity.xlabel="^g\152^u";
      }
      else
      {
         if (potential_type==potentialfunc::squid)
         {
            probdensity.xlabel="^g\152^u-^g\152^u^-0^n";
//            probdensity.xlabel="^g\152^u";
            probdensity.xmin=-3*PI/2;
            probdensity.xmax=3*PI/2;
            probdensity.xtic=PI/2;
            probdensity.xsubtic=PI/2;
            probdensity.xticlabel=
     "[-3^g\160^u/2 -^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u 3^g\160^u/2]";
         }
         else
         {
            probdensity.xmin=display_frac*(xlo)-0.5*deltax;
            probdensity.xmax=display_frac*(xhi)+0.5*deltax;
            probdensity.xlabel="X";
         }
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      probdensity.xmin=display_frac*kx_lo-0.5*delta_kx;
      probdensity.xmax=display_frac*kx_hi+0.5*delta_kx;
      probdensity.xlabel="Kx";
   }

   if (complex_plot_type==quantum::sqd_amp)
   {
      probdensity.ylabel="Probability Density";
      probdensity.yplotminval=TINY_NEGATIVE;
      probdensity.yplotmaxval=ymax;
      probdensity.ytic=0.2;
      probdensity.ysubtic=probdensity.ytic/2;
   }

   probdensity.npoints=nxbins_max;
   probdensity.thickness=2;
   probdensity.print_storylines=!viewgraph_mode;
   probdensity.singletfile_header();
   probdensity.write_singlet_data(
      0,xshift,complex_plot_type,
      potentialfunc::get_potential_str(potential_type));
   filefunc::closefile(probdensity.datafilenamestr+".meta",probdensity.datastream);

   return probdensity;
}

// ---------------------------------------------------------------------
// Member function plot_spectrum takes in the potential energy
// function and energy eigenstate information which are stored in
// spectrum_twoDarary and returns a corresponding quantumarray with
// this information encoded.  It also sets various metafile plot
// parameters.

quantumarray quantum_1Dwavefunction::plot_spectrum(
   int n_energystates,twoDarray& spectrum_twoDarray)
{
   const double TINY_NEGATIVE=-0.00001;
   
   quantumarray spectrum(xlo,deltax,spectrum_twoDarray);
   spectrum.datafilenamestr=imagedir+"spectrum";
   filefunc::openfile(spectrum.datafilenamestr+".meta",spectrum.datastream);

   for (int n=0; n<n_energystates; n++)
   {
      spectrum.extrainfo[n]="Eigenenergy"+stringfunc::number_to_string(n)+"="
         +stringfunc::number_to_string(spectrum_twoDarray.get(n+1,0));
   }

// We assume that period for periodic potentials is the interval [-PI,PI]:

   if (xperiod<POSITIVEINFINITY)
   {
      spectrum.xmin=-PI;
      spectrum.xmax=PI;
      spectrum.xtic=PI/2;
      spectrum.xsubtic=PI/4;
      spectrum.xticlabel=
         "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
      spectrum.xlabel="^g\152^u";
   }
   else
   {
      spectrum.xmin=display_frac*(xlo)-0.5*deltax;
      spectrum.xmax=display_frac*(xhi)+0.5*deltax;
      if (potential_type==potentialfunc::squid)
      {
         spectrum.xlabel="Phase ^g\152^u";
//         spectrum.xlabel="Phase difference ^g\152^u-^g\152^u^-0^n";
         spectrum.xtic=2;
         spectrum.xsubtic=1;
         spectrum.extrainfo[n_energystates]=
            "Ej = "+stringfunc::number_to_string(potential_param[0]);
         spectrum.extrainfo[n_energystates+1]=
            "BetaL = "+stringfunc::number_to_string(potential_param[1]);
         spectrum.extrainfo[n_energystates+2]
            ="phi0 = "+stringfunc::number_to_string(
               potential_param[2]*180/PI)+"\241";
      }
      else
      {
         spectrum.xlabel="X";
      }
   }

   if (potential_type==potentialfunc::squid)
   {
      spectrum.title="SQUID Energy Spectrum";
      spectrum.ylabel="Energy/E^-C^n";

// Notional squid:

      spectrum.yplotmaxval=28;
      spectrum.ytic=2;
      spectrum.ysubtic=2;

// Realistic 1D squid:

//      spectrum.yplotmaxval=14000;
//      spectrum.ytic=2000;
//      spectrum.ysubtic=1000;
   }
   else
   {
      spectrum.title="Energy Spectrum";
      spectrum.ylabel="Energy";
      spectrum.yplotmaxval=
         1.2*compute_energy(energy_eigenstate[n_energystates-1]);
   }

   spectrum.thickness=2;
   spectrum.yplotminval=TINY_NEGATIVE;
   spectrum.singletfile_header();
   spectrum.write_singlet_data(n_energystates,0,complex_plot_type);
   filefunc::closefile(spectrum.datafilenamestr+".meta",spectrum.datastream);

   return spectrum;
}

// ---------------------------------------------------------------------
// Member function plot_eigenfunctions takes in energy eigenstate
// information which is stored in twoDarray efunc_twoDarary and
// returns a corresponding quantumarray with this information encoded.
// It also sets various metafile plot parameters.

quantumarray quantum_1Dwavefunction::plot_eigenfunctions(
   int n_energystates,twoDarray& efunc_twoDarray)
{
   quantumarray efunc(xlo,deltax,efunc_twoDarray);
   efunc.datafilenamestr=imagedir+"efuncs";
   filefunc::openfile(efunc.datafilenamestr+".meta",efunc.datastream);

// We assume that period for periodic potentials is the interval [-PI,PI]:

   if (xperiod<POSITIVEINFINITY)
   {
      efunc.xmin=-PI;
      efunc.xmax=PI;
      efunc.xtic=PI/2;
      efunc.xsubtic=PI/4;
      efunc.xticlabel=
         "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
      efunc.xlabel="^g\152^u";
   }
   else
   {
      efunc.xmin=display_frac*(xlo)-0.5*deltax;
      efunc.xmax=display_frac*(xhi)+0.5*deltax;
      if (potential_type==potentialfunc::squid)
      {
         efunc.xlabel="Phase ^g\152^u";
//         efunc.xlabel="Phase difference ^g\152^u-^g\152^u^-0^n";
         efunc.xtic=2;
         efunc.xsubtic=1;
         efunc.extrainfo[n_energystates]=
            "Ej = "+stringfunc::number_to_string(potential_param[0]);
         efunc.extrainfo[n_energystates+1]=
            "BetaL = "+stringfunc::number_to_string(potential_param[1]);
         efunc.extrainfo[n_energystates+2]
            ="phi0 = "+stringfunc::number_to_string(
               potential_param[2]*180/PI)+"\241";
      }
      else
      {
         efunc.xlabel="X";
      }
   }

   if (potential_type==potentialfunc::squid)
   {
      efunc.title="Initial SQUID Energy Eigenfunctions";
   }
   else
   {
      efunc.title="Initial Energy Eigenfunctions";
   }

   double maxval,minval;
   efunc.ylabel="Eigenfunction value";
   efunc.thickness=2;
   efunc.find_max_min_vals(maxval,minval);
   efunc.yplotminval=1.2*minval;
   efunc.yplotmaxval=1.2*maxval;
   efunc.singletfile_header();
   efunc.write_singlet_data(n_energystates,0,complex_plot_type);
   filefunc::closefile(efunc.datafilenamestr+".meta",efunc.datastream);

   return efunc;
}

// ---------------------------------------------------------------------
// Member function plot_spectrum_and_efuncs generates metafile output
// in doublet format.  In the right tableau, n_energystates energy
// eigenvalues are graphed as horizontal lines superimposed on the
// potential.  In the left tableau, the energy eigenfunctions
// corresponding to the eigenvalues are displayed.

quantumarray quantum_1Dwavefunction::plot_spectrum_and_efuncs(
   int n_energystates,twoDarray& spectrum_twoDarray,
   twoDarray& efunc_twoDarray,int imagenumber)
{
   const double TINY_NEGATIVE=-0.00001;

// Load potential energy info into 0th row of
// spectrum_efuncs_twoDarray.  Load n_energystates eigenvalues into
// rows 1 through n_energystates of spectrum_efuncs_twoDarray.  Load
// n_energystates eigenfunctions into rows n_energystates+1 through
// 2*n_energystates+1:

   twoDarray spectrum_efuncs_twoDarray(2*n_energystates+1,nxbins);
   for (int n=0; n<n_energystates; n++)
   {
      for (int m=0; m<nxbins; m++)
      {
         spectrum_efuncs_twoDarray.put(
            0,m,spectrum_twoDarray.get(0,m));
         spectrum_efuncs_twoDarray.put(
            n+1,m,spectrum_twoDarray.get(n+1,m));
         spectrum_efuncs_twoDarray.put(
            n_energystates+n+1,m,efunc_twoDarray.get(n,m));
      }
   }

   quantumarray spectrum_efunc(xlo,deltax,spectrum_efuncs_twoDarray);
   spectrum_efunc.datafilenamestr=imagedir+"spectrum_efunc"
      +stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   filefunc::openfile(spectrum_efunc.datafilenamestr+".meta",spectrum_efunc.datastream);

   for (int n=0; n<n_energystates; n++)
   {
      spectrum_efunc.extrainfo[n]="Eigenenergy"
         +stringfunc::number_to_string(n)+"="
         +stringfunc::number_to_string(spectrum_efuncs_twoDarray.get(n+1,0));
   }

// We assume that period for periodic potentials is the interval [-PI,PI]:

   if (xperiod<POSITIVEINFINITY)
   {
      spectrum_efunc.xmin=-PI;
      spectrum_efunc.xmax=PI;
      spectrum_efunc.xtic=PI/2;
      spectrum_efunc.xsubtic=PI/4;
      spectrum_efunc.xticlabel=
         "[-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]";
      spectrum_efunc.xlabel="^g\152^u";
   }
   else
   {
      spectrum_efunc.xmin=display_frac*(xlo)-0.5*deltax;
      spectrum_efunc.xmax=display_frac*(xhi)+0.5*deltax;
      if (potential_type==potentialfunc::squid)
      {
         spectrum_efunc.xlabel="Phase ^g\152^u";
//         spectrum_efunc.xlabel="Phase difference ^g\152^u-^g\152^u^-0^n";
         spectrum_efunc.xtic=2;
         spectrum_efunc.xsubtic=1;
         spectrum_efunc.extrainfo[n_energystates]=
            "Ej = "+stringfunc::number_to_string(potential_param[0]);
         spectrum_efunc.extrainfo[n_energystates+1]=
            "BetaL = "+stringfunc::number_to_string(potential_param[1]);
         spectrum_efunc.extrainfo[n_energystates+2]
            ="phi0 = "+stringfunc::number_to_string(
               potential_param[2]*180/PI)+"\241";
      }
      else
      {
         spectrum_efunc.xlabel="X";
      }
   }

   for (int doublet_pair_member=0; doublet_pair_member<2; 
        doublet_pair_member++)
   {
      if (doublet_pair_member==0)
      {
         if (potential_type==potentialfunc::squid)
         {
            spectrum_efunc.pagetitle=
               "Phi0 = "+stringfunc::number_to_string(
                  potential_param[2]*180/PI)+"\241";
            spectrum_efunc.title="SQUID Energy Spectrum";
            spectrum_efunc.ylabel="Energy/E^-C^n";
//            spectrum_efunc.ylabel="Energy/[(2e)^+2^n/(2C)]";

// Notional squid:

            spectrum_efunc.yplotmaxval=28;
            spectrum_efunc.ytic=2;
            spectrum_efunc.ysubtic=2;

// Realistic 1D squid:
            
//            spectrum_efunc.yplotmaxval=14000;
//            spectrum_efunc.ytic=2000;
//            spectrum_efunc.ysubtic=1000;
         }
         else
         {
            spectrum_efunc.title="Energy Spectrum";
            spectrum_efunc.ylabel="Energy";
            spectrum_efunc.yplotmaxval=
               1.2*compute_energy(energy_eigenstate[n_energystates-1]);
         }
         spectrum_efunc.thickness=2;
         spectrum_efunc.yplotminval=TINY_NEGATIVE;
      }
      else if (doublet_pair_member==1)
      {
         if (potential_type==potentialfunc::squid)
         {
            spectrum_efunc.title="Initial SQUID Energy Eigenfunctions";
         }
         else
         {
            spectrum_efunc.title="Initial Energy Eigenfunctions";
         }

         double maxval,minval;
         spectrum_efunc.ylabel="Eigenfunction value";
         spectrum_efunc.thickness=2;
         spectrum_efunc.find_max_min_vals(
            maxval,minval,n_energystates+1,2*n_energystates+1);
         spectrum_efunc.yplotminval=1.2*minval;
         spectrum_efunc.yplotmaxval=1.2*maxval;
      }
      spectrum_efunc.doubletfile_header(doublet_pair_member);
      spectrum_efunc.write_doublet_data(
         doublet_pair_member,n_energystates,xshift);
   }	// doublet_pair_member loop

   filefunc::closefile(spectrum_efunc.datafilenamestr+".meta",
             spectrum_efunc.datastream);
   return spectrum_efunc;
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::singletfile_footer(
   string image_basename,int imagenumber,double E)
{
   string image_filename=image_basename+".meta";
   ofstream imagestream;
   filefunc::appendfile(image_filename,imagestream);

   imagestream << endl;
   imagestream << "title ''" << endl;
   imagestream << "size 8.8 6.0" << endl;
   imagestream << "physor 1.0 1.2" << endl;
   imagestream << "x axis min 0 max 1" << endl;
   imagestream << "y axis min 0 max 1" << endl;
   imagestream << "textsize 2" << endl;
   imagestream << "textcolor 'purple'" << endl;
   imagestream << "textsize 1.5" << endl;
   imagestream << "text 0.27 -0.1 't = "+
      stringfunc::number_to_string(t,2)+"'" << endl;
   imagestream << "text 0.67 -0.1 'E = "+stringfunc::number_to_string(E,2)
      +"'" << endl;

   filefunc::closefile(image_filename,imagestream);
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::doubletfile_footer(
   string image_basename,int imagenumber,double E,double Efinal)
{
   string image_filename=image_basename+".meta";
   ofstream imagestream;
   filefunc::appendfile(image_filename,imagestream);

   imagestream << endl;
   imagestream << "title ''" << endl;
  
   if (viewgraph_mode)
   {
      imagestream << "size 10.2 5.3" << endl;
      imagestream << "physor 0.3 1.1" << endl;
      imagestream << "x axis min 0 max 1" << endl;
      imagestream << "y axis min 0 max 1" << endl;
      imagestream << "textcolor 'purple'" << endl;
      imagestream << "textsize 2.5" << endl; 
      imagestream << "text 0.465 0.07 't = "+
         stringfunc::number_to_string(t,1)+"'" << endl;
      imagestream << "curve color cyan" << endl;
      imagestream << "0 0 0 1 1 1 1 0 0 0" << endl;
   }
   else if (!viewgraph_mode)
   {
      imagestream << "size 10 5.25" << endl;
      imagestream << "physor 0.4 1.25" << endl;
      imagestream << "x axis min 0 max 1" << endl;
      imagestream << "y axis min 0 max 1" << endl;
      imagestream << "curve color cyan" << endl;
      imagestream << "0 0 0 1 1 1 1 0 0 0" << endl;
      imagestream << "textsize 2" << endl;
      imagestream << "text 0.43 1.15 'Image "+stringfunc::number_to_string(
         imagenumber)+"'" << endl;
      imagestream << "textcolor 'purple'" << endl;
      imagestream << "textsize 1.5" << endl;

// For time dependent potentials, compute energy of current
// wavefunction using final system's potential.  We included these
// lines on 11/16/01 in an attempt to deduce the optimal time at which
// the NOT gate potential should be raised in order to capture and
// retain most of the wavefunction probability density into the second
// well of the 1D SQUID potential.

      if (time_dependent_potential)
      {
         imagestream << "text 0.15 -0.15 't = "+
            stringfunc::number_to_string(t,2)+"'" << endl;
         imagestream << "text 0.35 -0.15 'E = "
            +stringfunc::number_to_string(E,3)+"'" << endl;
         imagestream << "text 0.55 -0.15 'E^-final^n = "
            +stringfunc::number_to_string(Efinal,3)+"'" << endl;
         imagestream << "text 0.75 -0.15 'Normalization = "
            +stringfunc::number_to_string(compute_normalization(value),3)
            +"'" << endl;
      }
      else
      {
         imagestream << "text 0.22 -0.15 't = "+
            stringfunc::number_to_string(t,2)+"'" << endl;
         imagestream << "text 0.45 -0.15 'E = "
            +stringfunc::number_to_string(E,3)+"'" << endl;
         imagestream << "text 0.67 -0.15 'Normalization = "
            +stringfunc::number_to_string(compute_normalization(value),3)
            +"'" << endl;
      } // time_dependent_potential conditional
   } // viewgraph_mode conditional
   
   filefunc::closefile(image_filename,imagestream);
}

// ---------------------------------------------------------------------
// Member function write_packet_posn_and_width writes to meta file
// output gaussian wavepacket mean posn and width vs time

void quantum_1Dwavefunction::write_packet_posn_and_width()
{
   containerfunc::find_max_min_func_values(&xmeanlist);
   xmeanlist.get_metafile_ptr()->set_title("Wave packet average location");
   xmeanlist.get_metafile_ptr()->set_labels(
      "Time","Pack mean position");
   xmeanlist.get_metafile_ptr()->set_filename(imagedir+"mean_location");
   xmeanlist.get_metafile_ptr()->set_xbounds(tmin,tmax);
   xmeanlist.get_metafile_ptr()->set_ybounds(
      xmeanlist.get_fmin()-1,xmeanlist.get_fmax()+1);
   xmeanlist.get_metafile_ptr()->set_ytic(
      trunclog(xmeanlist.get_metafile_ptr()->get_ymax()-
               xmeanlist.get_metafile_ptr()->get_ymin()));
   xmeanlist.get_metafile_ptr()->set_ysubtic(
      0.5*xmeanlist.get_metafile_ptr()->get_ytic());
   plotfunc::writelist(xmeanlist);

   containerfunc::find_max_min_func_values(&xsigmalist);
   xsigmalist.get_metafile_ptr()->set_title("Wave packet spread");
   xsigmalist.get_metafile_ptr()->set_labels(
      "Time","Packet standard deviation");
   xsigmalist.get_metafile_ptr()->set_filename(imagedir+"spread");
   xsigmalist.get_metafile_ptr()->set_xbounds(tmin,tmax);
   xsigmalist.get_metafile_ptr()->set_ybounds(
      xsigmalist.get_fmin()-1,xsigmalist.get_fmax()+1);
   xsigmalist.get_metafile_ptr()->set_ytic(trunclog(
      xsigmalist.get_metafile_ptr()->get_ymax()-
      xsigmalist.get_metafile_ptr()->get_ymin()));
   xsigmalist.get_metafile_ptr()->set_ysubtic(
      0.5*xsigmalist.get_metafile_ptr()->get_ytic());
   plotfunc::writelist(xsigmalist);
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::output_wavefunction_info(
   double E,int& nimage,double& t_lastplot,
   quantumarray& psi_dataarray,twoDarray& psi_twoDarray,
   twoDarray& position_twoDarray,twoDarray& momentum_twoDarray)
{
// Update momentum space wavefunction only if it is to be written to
// metafile output or if momentum space wavefunction's mean and
// standard deviation are to be calculated:

   if (domain_name==quantum::momentum_space ||
       domain_name==quantum::position_and_momentum_space || 
       state_type=="gaussian wavepacket")
   {
      fouriertransform(value,tilde);
   }

// Save current and "final" energies into linked lists:

   energylist.append_node(datapoint(t,E));
         
// For time dependent potentials, compute energy of current
// wavefunction using final system's potential.  We included these
// lines on 11/16/01 in an attempt to deduce the optimal time at which
// the NOT gate potential should be raised in order to capture and
// retain most of the wavefunction probability density into the second
// well of the 1D SQUID potential.

   double Efinal=0;
   if (time_dependent_potential)
   {
      double currt=t;
      t=tmax;
      Efinal=compute_energy(value);
      t=currt;
      Efinallist.append_node(datapoint(t,Efinal));
      cout << "t = " << t << " Energy = " << E 
           << " Efinal = " << Efinal << endl;
   }
   else if (!time_dependent_potential)
   {
      cout << "t = " << t << " Energy = " << E << endl;
   }
   outputfunc::newline();

   if (domain_name==quantum::position_and_momentum_space)
   {
      prepare_plot_data(position_twoDarray,momentum_twoDarray);
   }
   else
   {
      prepare_plot_data(psi_twoDarray,E);
   }
         
   if (complex_plot_type==quantum::sqd_amp)
   {
      psi_dataarray=plot_probdensity(psi_twoDarray,nimage);
      if (!viewgraph_mode)
      {
         singletfile_footer(psi_dataarray.datafilenamestr,nimage,E);
      }
   }
   else if (get_complex_plot_type()==quantum::posn_momentum)
   {
      psi_dataarray=plot_wavefunction(
         position_twoDarray,momentum_twoDarray,nimage);
      doubletfile_footer(
         psi_dataarray.datafilenamestr,nimage,E,Efinal);
   }
   else
   {
      psi_dataarray=plot_wavefunction(psi_twoDarray,nimage);
      doubletfile_footer(
         psi_dataarray.datafilenamestr,nimage,E,Efinal);
   }

   filefunc::meta_to_jpeg(psi_dataarray.datafilenamestr);
   filefunc::gzip_file(psi_dataarray.datafilenamestr+".meta");
   if (viewgraph_mode) imagefunc::crop_jpegimage(
      psi_dataarray.datafilenamestr+".jpg",0,0);

// Calculate wavefunction mean location and width for gaussian
// wavepackets:

   if (state_type=="gaussian wavepacket")
   {
      compute_posn_mean_and_spread(value);
      compute_momentum_mean_and_spread(tilde);
      write_packet_posn_and_width();
   }

// Dump contents of current wavefunction to output file for possible
// future restoration:

   dump_wavefunction(value);

   nimage++;
   t_lastplot=t;

// Generate updated animation script and energy plot after each new
// image is written to metafile output:

   outputfunc::generate_animation_script(
      nimage,"image",imagedir,"show_images");
//         generate_animation_script("image","show_images",nimage);
   plot_energies_vs_time();
}

// =====================================================================
// Wavefunction manipulation member functions:
// =====================================================================

void quantum_1Dwavefunction::clear_wavefunction()
{
   for (int i=0; i<nxbins_max; i++)
   {
      value[i]=tilde[i]=0;
   }
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::copy_wavefunction(
   const complex value_orig[],complex value_copy[])
{
   for (int i=0; i<nxbins_max; i++)
   {
      value_copy[i]=value_orig[i];
   }
}

// ---------------------------------------------------------------------
// Member function initialize_spatial_and_momentum_parameters sets the
// size of the domain over which the wavefunction is to be calculated.
// We exploit the periodicity of FFT's for problems in which the
// potential is periodic.  In particular, we restrict the domain on
// the x axis to just a single period for periodic potentials.  We
// also take great care to ensure that x_{nxbins-1} does NOT
// correspond to x_{0}.  For periodic problems, x_{nxbins-1} should
// instead correspond to "x_{-1}".  The maximum/minimum limits of
// discretized momentum space variables are similarly arranged so that
// kx_{nxbins-1} corresponds to kx_{-1} for problems with period
// nxbins.

void quantum_1Dwavefunction::initialize_spatial_and_momentum_parameters()
{
   nxbins=nxbins_max;  // odd

// First set wavefunction normalization value:

   norm=1;

// Next set spatial and momentum step sizes.  Recall that the system's
// dimensionless coupling constant alpha ( = ratio of potential to
// kinetic energies) is saved within the potential_param[0] member
// variable.  And an aperiodic system's characteristic length scale
// goes like 1/sqrt(sqrt(alpha)).

   double V,dV,d2V;
   potentialfunc::potential(
      false,potential_type,seed,potential_t1,potential_t2,0,potential_param,
      0,V,dV,d2V);
   double alpha=potential_param[0];

   double characteristic_lengthscale;
   if (xperiod >= sqrt(nxbins))		// aperiodic potential
   {
      characteristic_lengthscale=1.0/sqrt(sqrt(alpha));
      xhi=sqrt(double(nxbins))/2.0*characteristic_lengthscale;
      deltax=2*xhi/(nxbins-1);
   }
   else					// periodic potential
   {
      deltax=xperiod/double(nxbins);
      xhi=(nxbins-1)/2.0*deltax;
   }
   xlo=-xhi;

   delta_kx=1.0/(nxbins*deltax);
   kx_hi=(nxbins-1)*delta_kx/2.0;
   kx_lo=-kx_hi;

   cout << "Coupling constant alpha = " << alpha << endl;
   cout << "xhi = " << xhi << " xlo = " << xlo << endl;
   cout << "x bin size = " << deltax << endl;
   cout << "nxbins = " << nxbins << endl;
   cout << "kx bin size = " << delta_kx << endl;
}

// ---------------------------------------------------------------------
// If the potential V(x) is an aperiodic function of x, member
// function trial_wavefunction returns within complex array curr_value
// a trial wavefunction proportional to exp[-(x-x0)^2] or
// (x-x0)*exp[-(x-x0)^2], depending upon whether input boolean flag
// even_wavefunction is true or false.  Here x0=PI for the squid
// potential, while x0=0 for all other aperiodic potentials.  If V(x)
// is periodic on the other hand, we take even trial wavefunctions to
// be proportional to 1/V(x) and odd trial wavefunctions to be
// proportional to x/V(x).  These guesses appear to correctly
// incorporate salient symmetry information as well as placing the
// bulk of the wavefunction in regions where the potential is small...

void quantum_1Dwavefunction::trial_wavefunction(
   bool even_wavefunction,complex curr_value[])
{
   const double minV=1E-5;
   double x,V,dV,d2V;
   double currterm,x0;

   if (xperiod < POSITIVEINFINITY)
   {
      for (int i=0; i<nxbins; i++)
      {
         x=xlo+i*deltax;
         potentialfunc::potential(
            time_dependent_potential,potential_type,seed,
            potential_t1,potential_t2,t,potential_param,
            x,V,dV,d2V);

         if (fabs(V) > minV)
         {
            currterm=1.0/V;
         }
         else
         {
            currterm=1.0/minV;
         }
         
         if (!even_wavefunction)
         {
            currterm *= x;
         }
         curr_value[i]=currterm;
         curr_arg[i]=prev_arg[i]=curr_value[i].getarg();
      } // loop over index i
   }
   else 
   {
      if (potential_type==potentialfunc::squid)
      {
         x0=PI;
      }
      else
      {
         x0=0;
      }
      for (int i=0; i<nxbins; i++)
      {
         x=xlo+i*deltax;
         currterm=exp(-sqr(x-x0));
         if (!even_wavefunction)
         {
            currterm *= (x-x0);
         }
         curr_value[i]=currterm;
         curr_arg[i]=prev_arg[i]=curr_value[i].getarg();
      } // loop over index i
   } // potential_type conditional

   renormalize_wavefunction(curr_value);
}

// ---------------------------------------------------------------------
// Member function gaussian_wavepacket generates an initial normalized
// gaussian wavepacket state with some particular average momentum.

void quantum_1Dwavefunction::gaussian_wavepacket(
   double x0,complex curr_value[])
{
   const double alpha=1;
//   const double alpha=1E3;
//   const double alpha=1E6;
   const double sigma=1/pow(4*alpha,0.25);
   const double prefactor=1/(pow(2*PI,0.25)*sqrt(sigma));
//   const double p0=0;
//   const double p0=1;
//   const double p0=2;
   const double p0=5;
//   const double p0=10;
//   const double p0=10*2*PI;
   
   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;
      complex momentum_factor=complex(cos(p0*x),sin(p0*x));
      curr_value[i]=prefactor*momentum_factor*exp(-sqr(x-x0)/(4*sqr(sigma)));
      curr_arg[i]=prev_arg[i]=curr_value[i].getarg();
   } // loop over index i
   renormalize_wavefunction(curr_value);
}

// ---------------------------------------------------------------------
// Member function initialize_wavefunction fills up the position space
// value array with either the projected ground state, first excited
// state or a superposition of the two depending upon the state_type
// flag set within quantum_wavefunction::select_initial_state:

void quantum_1Dwavefunction::initialize_wavefunction()
{
   complex *packet_value=new_clear_carray(nxbins_max);
   if (state_type=="gaussian wavepacket")
   {
//      gaussian_wavepacket(0,packet_value);
      gaussian_wavepacket(-8,packet_value);
//      gaussian_wavepacket(-18,packet_value);
   }
   
   for (int i=0; i<nxbins; i++)
   {
      if (state_type=="(0)")
      {
         value[i]=energy_eigenstate[0][i];
      }
      else if (state_type=="(1)")
      {
         value[i]=energy_eigenstate[1][i];
      }
      else if (state_type=="(0)+(1)")
      {
         value[i]=1/sqrt(2)*(energy_eigenstate[0][i]+
                             energy_eigenstate[1][i]);
      }
      else if (state_type=="(2)")
      {
         value[i]=energy_eigenstate[2][i];
      }
      else if (state_type=="(3)")
      {
         value[i]=energy_eigenstate[3][i];
      }
      else if (state_type=="gaussian wavepacket")
      {
         value[i]=packet_value[i];
      }
      prev_arg[i]=curr_arg[i]=value[i].getarg();
   }

   delete [] packet_value;
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::evolve_phase()
{
   for (int i=0; i<nxbins; i++)
   {
      if (domain_name==quantum::position_space)
      {
         if (value[i].getmod() < TINY)
         {
            curr_arg[i]=0;
         }
         else
         {
            curr_arg[i]=value[i].getarg(prev_arg[i]);
         }
      }
      else if (domain_name==quantum::momentum_space)
      {
         if (tilde[i].getmod() < TINY)
         {
            curr_arg[i]=0;
         }
         else
         {
            curr_arg[i]=tilde[i].getarg(prev_arg[i]);
         }
      }

// For display purposes only, we restrict the range of curr_arg to lie
// within the interval (-PI,PI):

      curr_arg[i]=basic_math::phase_to_canonical_interval(curr_arg[i],-PI,PI);
      prev_arg[i]=curr_arg[i];
   }
}

// ---------------------------------------------------------------------
void quantum_1Dwavefunction::set_potential_spatial_period()
{
   if (potential_type==potentialfunc::box || 
       potential_type==potentialfunc::harmonic_osc ||
       potential_type==potentialfunc::lambda_phi_4 || 
       potential_type==potentialfunc::squid ||
       potential_type==potentialfunc::freeparticle || 
       potential_type==potentialfunc::ramp ||
       potential_type==potentialfunc::inverted_parabola ||
       potential_type==potentialfunc::aperiodic_cosine ||
       potential_type==potentialfunc::smooth_step || 
       potential_type==potentialfunc::smooth_box)
   {
      xperiod=POSITIVEINFINITY;
   }
   else if (potential_type==potentialfunc::doublewell || 
            potential_type==potentialfunc::mathieu)
   {
      xperiod=2*PI;
   }
}

// ---------------------------------------------------------------------
// Member function null_tiny_value sets to zero any wavefunction value
// whose modulus is less than some tiny value.  

void quantum_1Dwavefunction::null_tiny_value(complex curr_value[])
{
   for (int i=0; i<nxbins; i++)
   {
      if (curr_value[i].getmod() < TINY) curr_value[i]=0;
   }
}

// ---------------------------------------------------------------------
// Member function renormalize_wavefunction renormalizes the
// wavefunction to some user specified value norm:

void quantum_1Dwavefunction::renormalize_wavefunction(complex curr_value[])
{
   double curr_norm=compute_normalization(curr_value);
   for (int i=0; i<nxbins; i++)
   {
      curr_value[i]=sqrt(norm/curr_norm)*curr_value[i];
   }
}

// ---------------------------------------------------------------------
// Member function prepare_plot_data copies either position or
// momentum space information into psi_twoDarray which is to be
// plotted in doublet form:

void quantum_1Dwavefunction::prepare_plot_data(
   twoDarray& psi_twoDarray,double E)
{
   const double TINY_OFFSET=0.002;  // Displace potential zero values by
				    // tiny vertical distance so that they
				    // can be seen in magnitude plots
   double V,dV,d2V;
   for (int i=0; i<nxbins; i++)
   {
      if (domain_name==quantum::position_space)
      {
         double x=xlo+i*deltax;
         potentialfunc::potential(
            time_dependent_potential,potential_type,seed,
            potential_t1,potential_t2,t,potential_param,
            x,V,dV,d2V);

         if (complex_plot_type==quantum::mag_phase)
         {
            psi_twoDarray.put(0,i,value[i].getmod());
            psi_twoDarray.put(1,i,curr_arg[i]);
         }
         else if (complex_plot_type==quantum::real_imag)
         {
            psi_twoDarray.put(0,i,value[i].getreal());
            psi_twoDarray.put(1,i,value[i].getimag());
         }
         else if (complex_plot_type==quantum::sqd_amp)
         {
            psi_twoDarray.put(0,i,sqr(value[i].getmod()));
            psi_twoDarray.put(1,i,sqr(value[i].getmod()));
         }
         else if (complex_plot_type==quantum::energy_prob)
         {
            psi_twoDarray.put(0,i,V);
            psi_twoDarray.put(1,i,sqr(value[i].getmod()));
         }
         else if (complex_plot_type==quantum::energy_real)
         {
            psi_twoDarray.put(0,i,V);
            psi_twoDarray.put(1,i,value[i].getreal());
         }

// Store either system's spatially independent energy value or the
// spatially dependent potential energy function in the r=2 row of
// psi_twoDarray:

         if (complex_plot_type==quantum::energy_prob ||
             complex_plot_type==quantum::energy_real)
         {
            psi_twoDarray.put(2,i,E);
         }
         else
         {
            if (V < POSITIVEINFINITY)
            {
               psi_twoDarray.put(2,i,V+TINY_OFFSET);
            }
            else
            {

// Divide POSITIVEINFINITY by 2 so that potential value shows up on plot:

               psi_twoDarray.put(2,i,POSITIVEINFINITY/2.0); 
            }
         } // complex_plot_type==energy_prob || energy_real conditional
      }
      else if (domain_name==quantum::momentum_space)
      {
         if (complex_plot_type==quantum::mag_phase)
         {
            psi_twoDarray.put(0,i,tilde[i].getmod());
            psi_twoDarray.put(1,i,curr_arg[i]);
         }
         else if (complex_plot_type==quantum::real_imag)
         {
            psi_twoDarray.put(0,i,tilde[i].getreal());
            psi_twoDarray.put(1,i,tilde[i].getimag());
         }
         else if (complex_plot_type==quantum::sqd_amp)
         {
            psi_twoDarray.put(0,i,sqr(tilde[i].getmod()));
            psi_twoDarray.put(1,i,sqr(tilde[i].getmod()));
         }
      }
   } // loop over index i labelling xbin number
}

// ---------------------------------------------------------------------
// This overloaded version of member function prepare_plot_data copies
// position AND momentum space magnitude information into
// position_twoDarray and momentum_twoDarray which are to be plotted
// in doublet form.  Specifically, it loads position space
// wavefunction magnitudes into the 0th row of position_twoDarray,
// momentum space wavefunction magnitudes into the 1st row
// momentum_twoDarray and potential values into the 2nd row of
// position_twoDarray.

void quantum_1Dwavefunction::prepare_plot_data(
   twoDarray& position_twoDarray,twoDarray& momentum_twoDarray)
{
   const double TINY_OFFSET=0.002;  // Displace potential zero values by
				    // tiny vertical distance so that they
				    // can be seen in magnitude plots
   double V,dV,d2V;
   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;
      position_twoDarray.put(0,i,value[i].getmod());

// Store the spatially dependent potential energy function in the r=1
// row of position_twoDarray:

      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);
      if (V < POSITIVEINFINITY)
      {
         position_twoDarray.put(2,i,V+TINY_OFFSET);
      }
      else
      {

// Divide POSITIVEINFINITY by 2 so that potential value shows up on plot:

         position_twoDarray.put(2,i,POSITIVEINFINITY/2.0); 
      }
      momentum_twoDarray.put(1,i,tilde[i].getmod());
   } // loop over index i 
}

// ---------------------------------------------------------------------
// Member function prepare_spectrum_data stores the spatially
// dependent potential function plus up to the n_energystates lowest
// lying energy eigenvalues into spectrum_twoDarray.  It also stores
// the real parts of the corresponding eigenfunctions in
// efunc_twoDarray.

void quantum_1Dwavefunction::prepare_spectrum_data(
   int n_energystates,twoDarray& spectrum_twoDarray,
   twoDarray& efunc_twoDarray)
{
   double V,dV,d2V;
   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);
      spectrum_twoDarray.put(0,i,V);

      for (int m=0; m<n_energystates; m++)
      {
         spectrum_twoDarray.put(m+1,i,compute_energy(energy_eigenstate[m]));
         efunc_twoDarray.put(m,i,energy_eigenstate[m][i].getreal());
      }
   } // loop over index i 
}

// =====================================================================
// Wavefunction evolution member functions
// =====================================================================

// Member function evolve_wavefunction_thru_one_timestep
// Evolve wavefunction through one dynamically controlled time step:

void quantum_1Dwavefunction::evolve_wavefunction_thru_one_timestep(
   int& n_redos,double E,complex value_copy[])
{
   const bool Wick_rotate=false;
   const double deltat_min=0.0001; // Min allowed size for dynamic timestep
   const double deltat_max=0.1; // Max allowed size for dynamic timestep
   const double Efrac_max=1E-8; // Max allowed frac error in energy
				//  between time step i and i+1
   const double Efrac_min=1E-10; // Min allowed frac error in energy 
				//  between timestep i and i+1

   bool evolve_again;
   copy_wavefunction(value,value_copy);
   do // while loop over current timestep 
   {
      FFT_step_wavefunction(Wick_rotate,value);
      double Enew=compute_energy(value);
      double deltaE=Enew-E;

      if (fabs(deltaE)/E > Efrac_max && deltat > deltat_min)
      {
         deltat /= 2;
         evolve_again=true;
         copy_wavefunction(value_copy,value);
         n_redos++;
      }
      else if (fabs(deltaE)/E < Efrac_min && deltat < deltat_max)
      {
         deltat *= 2;
         evolve_again=false;
      }
      else
      {
         evolve_again=false;
      }
   }
   while (evolve_again==true);
}

// ---------------------------------------------------------------------
// Member function FFT_step_wavefunction implements a momentum space
// approach to evolving the wavefunction by either exp (-iHt) or its
// Wick rotated counterpart exp(-Ht):

// |psi(t+dt)> = ... exp(-i dt^3/3 [V,[V,P^2]]) * exp(i dt^3/6 [[V,P^2],P^2)
// 	       * exp(dt^2/2 [V,P^2]) exp(-i dt V) exp(-i dt P^2) |psi(t)>,

//		   	or the Wick rotated version

// |psi(t+dt)> = ... exp(dt^3/3 [V,[V,P^2]]) * exp(dt^3/6 [[V,P^2],P^2)
//	       * exp(-dt^2/2 [V,P^2]) exp(-dt V) exp(-dt P^2) |psi(t)>. 

// O(dt^4) terms represented by the initial ellipses are neglected
// below.  These formulas are only truly accurate to O(dt^2), for we
// have to approximate the potential's second derivative by some
// constant K.  In order to preserve amplitude content, the
// wavefunction is renormalized after it has been evolved by one time
// step.

void quantum_1Dwavefunction::FFT_step_wavefunction(
   bool Wick_rotate,complex curr_value[])
{
   double term0,term1,term2,term3,xstar,xj;
   complex (*value1a)=new_clear_carray(nxbins_max);
   complex value1b,value1c;
   complex (*value1d)=new_clear_carray(nxbins_max);
   complex (*curr_tilde)=new_clear_carray(nxbins_max);
   complex (*tilde1a)=new_clear_carray(nxbins_max);

// First evaluate |psi_1a> =  exp({-2/3 K i dt^3 - i dt} P^2) |psi(t)>

// or (Wick rotated)

//	        |psi_1a> = exp({2/3 K i dt^3 - dt} P^2) |psi(t)>

   fouriertransform(curr_value,curr_tilde);

   double V,dV,d2V,d3V,d4V,Kconst,x=0;
   potentialfunc::potential(
      time_dependent_potential,potential_type,seed,
      potential_t1,potential_t2,t,potential_param,
      x,V,dV,d2V,d3V,d4V,Kconst);

   for (int i=0; i<nxbins; i++)
   {
      double kx=kx_lo+i*delta_kx;

      if (Wick_rotate)
      {
         term0=1-0.666*Kconst*sqr(deltat);
         term1=sqr(2*PI*kx)*deltat*term0;
         tilde1a[i]=exp(-term1)*curr_tilde[i];
      }
      else
      {
         term0=1+0.666*Kconst*sqr(deltat);
         term1=sqr(2*PI*kx)*deltat*term0;
         tilde1a[i]=complex(cos(term1),-sin(term1))*curr_tilde[i];
      }
   }
   inversefouriertransform(tilde1a,value1a);

// Next evaluate |psi_1b> = exp(-i dt V(x)) |psi_1a> 
//               |psi_1c> = exp(1/2 dt^2 d^2V/dx^2) |psi_1b>, or
// 	        |psi_1d> =exp(i dt^3 { 2/3 (dV/dX)^2 + 1/6 d^4V/dX^4}) |psi_1c>

// or (Wick rotated)

// 		 |psi_1b> = exp(-dt V(x)) |psi_1a> 
//               |psi_1c> = exp(-1/2 dt^2 d^2V/dx^2) |psi_1b> (Wick rotated)
//            |psi_1d> =exp(-i dt^3 { 2/3 (dV/dX)^2 + 1/6 d^4V/dX^4}) |psi_1c> 

   for (int i=0; i<nxbins; i++)
   {
      x=xlo+i*deltax;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V,d3V,d4V,Kconst);

// Set position space wavefunction values to zero at points where
// potential is infinite:

      if (V >= POSITIVEINFINITY)
      {
         value1d[i]=0;
      }
      else
      {
         term2=V*deltat;
         term3=(0.666*sqr(dV)+d4V/6.)*pow(deltat,3);
         if (Wick_rotate)
         {
            value1b=exp(-term2)*value1a[i];
            value1c=exp(-0.5*sqr(deltat)*d2V)*value1b;
            value1d[i]=exp(-term3)*value1c;
         }
         else
         {
            value1b=complex(cos(term2),-sin(term2))*value1a[i];
            value1c=exp(0.5*sqr(deltat)*d2V)*value1b;
            value1d[i]=complex(cos(term3),sin(term3))*value1c;
         }
      }	// V >= POSITIVEINFINITY conditional
   } // loop over index i

// Set |psi_new(x)> = exp(i dt^2 dV/dx P) |psi_1d(x)> 
//		    = |psi_1d(x+dt^2 dV/dx)> or

// or (Wick rotated)

//     |psi_new(x)> = exp(-i dt^2 dV/dx P) |psi_1d(x)> 
// 		    = |psi_1d(x-dt^2 dV/dx)> 

   for (int i=0; i<nxbins; i++)
   {
      x=xlo+i*deltax;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);

      if (fabs(V) >= POSITIVEINFINITY)
      {
         curr_value[i]=0;
      }
      else
      {
         if (Wick_rotate)
         {
            xstar=x-sqr(deltat)*dV;
         }
         else
         {
            xstar=x+sqr(deltat)*dV;
         }
         int j=basic_math::round(floor((xstar-xlo)/deltax));
         xj=xlo+j*deltax;

// If V(x) is periodic, we must exercise care when shifting the
// wavefunction to discretized values of x above xhi or below xlo.  If
// the wavefunction is periodic, we cannot generally set its value at
// such hi and lo points to zero without doing violence to
// wavefunction continuity. Instead, we recall that x_{nxbins} maps
// onto x_{0}, x_{nxbins-1} maps onto x_{-1}, etc...

         if (j>=0 && j<nxbins-1)
         {
            curr_value[i]=
               value1d[j]+(xstar-xj)/deltax*(value1d[j+1]-value1d[j]);
         }
         else
         {
            if (xperiod==POSITIVEINFINITY)
            {
               curr_value[i]=0;
            }
            else
            {
               if (j >= nxbins)
               {
                  j -= nxbins;
               }
               else if (j < 0)
               {
                  j += nxbins;
               }
               
               int jplusone=j+1;
               if (jplusone >= nxbins)
               {
                  jplusone -= nxbins;
               }
               else if (jplusone < 0)
               {
                  jplusone += nxbins;
               }

               curr_value[i]=
                  value1d[j]+(xstar-xj)/deltax*(value1d[jplusone]-value1d[j]);
            } // xperiod==POSITIVEINFINITY conditional
         }  // j>=0 && j<nxbins-1 conditional
      }	// V >= POSITIVEINFINITY conditional
   } // loop over index i

   renormalize_wavefunction(curr_value);
   null_tiny_value(curr_value);

   delete [] curr_tilde;
   delete [] tilde1a;
   delete [] value1a;
   delete [] value1d;
}

// ---------------------------------------------------------------------
// Member function project_low_energy_states calculates the lowest
// energy eigenstates for any 1D potential.  Recall that an arbitrary
// trial wavefunction can be decomposed in terms of energy eigenstates
// as

// 		psi_trial = C0 psi_0 + C1 psi_1 + C2 psi_2 + ...

// In order to project out the ground state psi_0, we first evolve the
// trial wavefunction according to the Wick rotated time evolution
// operator exp(-H t).  The wavefunction is renormalized after each
// time step in order to preserve amplitude content.  As the iteration
// proceeds, the excited energy state content of the trial
// wavefunction becomes exponentially suppressed compared to its
// ground state component.  In this way, the trial wavefunction
// relaxes to the ground state.

// Once the ground state is known, we compute C0=<psi_0|psi_trial> and
// then remove the lowest energy state from a new, odd trial
// wavefunction to obtain the reduced wavefunction psi_reduced =
// psi_trial-C0 psi_0.  At this point, we can project out the first
// excited eigenstate by following the procedure described above.  

// Following this strategy of evolving and projecting, we can compute
// any low energy eigenstate so long as build-up of numerical
// inaccuracies does not become overwhelmingly large.

void quantum_1Dwavefunction::project_low_energy_states(
   int n_energystates)
{
   bool time_dependent_potential_orig=time_dependent_potential;
   complex (*curr_value)=new_clear_carray(nxbins_max);

   time_dependent_potential=false;
   
   for (int n=0; n<n_energystates; n++)
   {
      if (!restore_eigenfunction(n))
      {
         trial_wavefunction(is_even(n),curr_value);
         remove_overlap(n,curr_value);
         project_energy_eigenstate(n,curr_value);
      }

      double E_n=compute_energy(energy_eigenstate[n]);
      cout << "n = " << n << " E_n = " << E_n << endl;
      cout << "normalization = " 
           << compute_normalization(energy_eigenstate[n])
           << endl;

      for (int m=0; m<=n; m++)
      {
         cout << "n = " << n << " m = " << m << " <n|m> = " 
              << energystate_overlap(m,energy_eigenstate[n]) << endl;
      }
   }

   time_dependent_potential=time_dependent_potential_orig;
   delete [] curr_value;
}

// ---------------------------------------------------------------------
// Member function energystate_overlap calculates the projection
// coefficient <n|psi> where |n> is the nth energy eigenstate, and
// |psi> is the state specified within input array curr_value:

complex quantum_1Dwavefunction::energystate_overlap(
   int n,complex curr_value[])
{
   complex (*overlap)=new_clear_carray(nxbins_max);
   for (int i=0; i<nxbins; i++)
   {
      overlap[i]=energy_eigenstate[n][i].Conjugate()*curr_value[i];
   }
   complex C_n=simpsonsum_complex(overlap,0,nxbins-1)*deltax;

   delete [] overlap;
   return C_n;
}

// ---------------------------------------------------------------------
// Member function remove_overlap takes in a wavefunction contained
// within input array curr_value and subtracts away the energy
// eigenstates corresponding to quantum numbers n < n_energystate.
// The reduced wavefunction with these lower energy eigenstates
// removed is returned within array curr_value:

void quantum_1Dwavefunction::remove_overlap(
   int n_energystate,complex curr_value[])
{
   complex C_n;
   complex (*overlap)=new_clear_carray(nxbins_max);

   for (int n=0; n<n_energystate; n++)
   {
      C_n=energystate_overlap(n,curr_value);

      for (int i=0; i<nxbins; i++)
      {
         curr_value[i]=curr_value[i]-C_n*energy_eigenstate[n][i];
      }
   }

   delete [] overlap;
}

// ---------------------------------------------------------------------
// Member function project_energy_eigenstate evolves a wavefunction
// contained within input array prev_value by the Wick rotated time
// evolution operator exp(-Ht).  The wavefunction is renormalized
// after each time step in order to preserve amplitude content.  The
// evolution continues until the fractional difference in each bin is
// smaller than some specified tolerance value.  At this point, all
// higher energy eigenstates are exponentially suppressed compared to
// the lowest lying energy state.  The surviving energy eigenstate
// values are copied into the array energy_eigenstate.

void quantum_1Dwavefunction::project_energy_eigenstate(
   int n_energystate,complex prev_value[])
{
   const bool Wick_rotate=true;
//   const double tolerance=1E-2;
   const double tolerance=1E-4;
//   const double nt_min=1000;
   const double nt_min=20000;
   const double EXTREMELY_TINY=1E-13;
   
   double max_frac_diff;
   complex curr_diff;
   complex (*curr_value)=new_clear_carray(nxbins_max);

   for (int i=0; i<nxbins; i++)
   {
      curr_value[i]=prev_value[i];
   }

   int nt=0;
   do
   {
      t=tmin+nt*deltat;

      FFT_step_wavefunction(Wick_rotate,curr_value);
      max_frac_diff=0;
      for (int i=0; i<nxbins; i++)
      {
         curr_diff=curr_value[i]-prev_value[i];
         if (curr_value[i].getmod() > EXTREMELY_TINY && 
             prev_value[i].getmod() > EXTREMELY_TINY)
         {
            max_frac_diff=max(
               max_frac_diff,
               fabs(curr_diff.getmod())/fabs(curr_value[i].getmod()));
         }
         prev_value[i]=curr_value[i];
      }

//      cout << " nt = " << nt << " max_frac_diff = " << max_frac_diff
//           << endl;
      nt++;

// After each time step, subtract away any residual eigenstates
// corresponding to quantum numbers LESS than n_energystate:

      remove_overlap(n_energystate,curr_value);
   }	
   while (max_frac_diff > tolerance || nt < nt_min);
   
   cout << "Number of iterations needed to project out energy eigenstate "
        << n_energystate << " = " << nt << endl;
   cout << "max_frac_diff = " << max_frac_diff << endl;

   for (int i=0; i<nxbins; i++)
   {
      energy_eigenstate[n_energystate][i]=curr_value[i];
   }

   if (save_eigenfunction) dump_eigenfunction(n_energystate);
   delete [] curr_value;
}

// =====================================================================
// Wavefunction properties member functions:
// =====================================================================

double quantum_1Dwavefunction::compute_normalization(
   const complex curr_value[])
{
   double *magsq;
   new_clear_array(magsq,nxbins);

   for (int i=0; i<nxbins; i++)
   {
      magsq[i]=sqr(curr_value[i].getreal())
         +sqr(curr_value[i].getimag());
   }
   double curr_norm=mathfunc::simpsonsum(magsq,0,nxbins-1)*deltax;

   delete [] magsq;
   return curr_norm;
}

// ---------------------------------------------------------------------
// Member function compute_energy calculates the matrix element
// <psi|H|psi>/<psi|psi> where psi denotes any (true or trial;
// normalized or unnormalized) state vector:

double quantum_1Dwavefunction::compute_energy(const complex curr_value[])
{
   double KEdensity[nxbins_max];
   complex curr_term;
   complex curr_tilde[nxbins_max];
   fouriertransform(curr_value,curr_tilde);

   for (int i=0; i<nxbins; i++)
   {
      double kx=kx_lo+i*delta_kx;
      curr_term=curr_tilde[i].Conjugate()*sqr(2*PI*kx)*curr_tilde[i];
      KEdensity[i]=curr_term.getreal();
   }

   double magsq[nxbins_max];
   double PEdensity[nxbins_max];
   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;

// Set position space density to zero at points where potential is
// infinite:

      double V,dV,d2V;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);

      if (fabs(V) >= POSITIVEINFINITY)
      {
         PEdensity[i]=0;
         magsq[i]=0;
      }
      else
      {
         curr_term=curr_value[i].Conjugate()*V*curr_value[i];
         PEdensity[i]=curr_term.getreal();
         magsq[i]=sqr(curr_value[i].getmod());
      }
   }

   double KE=mathfunc::simpsonsum(KEdensity,0,nxbins-1)*delta_kx;
   double PE=mathfunc::simpsonsum(PEdensity,0,nxbins-1)*deltax;
   double curr_norm=mathfunc::simpsonsum(magsq,0,nxbins-1)*deltax;
   double Etot=(KE+PE)/curr_norm;

//   cout << "t = " << t << " alpha = " << potential_param[0] << endl;
//   cout << "KE = " << KE << " PE = " << PE << endl;
//   cout << " currnorm = " << curr_norm << " Etot = " << Etot << endl;
   return Etot;
}

// ---------------------------------------------------------------------
// Note added on 5/22/05: In order to disentangle the dependence of
// our Linkedlist class upon the mathfuncs namespace, we have
// commented out the following two methods which call the
// ordered_node_existence() member function of Linkedlist.  These two
// methods appear to be the only ones in our entire library structure
// which execute these calls.

/*
// ---------------------------------------------------------------------
// Member function compute_posn_mean_and_spread calculates the
// expectation values <x> and sqrt(<x^2)-<x>^2) for any (true or
// trial; normalized or unnormalized) state vector.

void quantum_1Dwavefunction::compute_posn_mean_and_spread(
   const complex curr_value[])
{
   double *magsq,*xdensity,*xsqdensity;
   new_clear_array(magsq,nxbins_max);
   new_clear_array(xdensity,nxbins_max);
   new_clear_array(xsqdensity,nxbins_max);
   complex curr_xterm,curr_xsqterm;

   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;

// Set position space density to zero at points where potential is
// infinite:

      double V,dV,d2V;
      potentialfunc::potential(
         time_dependent_potential,potential_type,seed,
         potential_t1,potential_t2,t,potential_param,
         x,V,dV,d2V);

      if (fabs(V) >= POSITIVEINFINITY)
      {
         xdensity[i]=xsqdensity[i]=magsq[i]=0;
      }
      else
      {
         curr_xterm=curr_value[i].Conjugate()*x*curr_value[i];
         curr_xsqterm=x*curr_xterm;
         xdensity[i]=curr_xterm.getreal();
         xsqdensity[i]=curr_xsqterm.getreal();
         magsq[i]=sqr(curr_value[i].getmod());
      }
   }

   double xnumer=mathfunc::simpsonsum(xdensity,0,nxbins-1)*deltax;
   double xsqnumer=mathfunc::simpsonsum(xsqdensity,0,nxbins-1)*deltax;
   double curr_norm=mathfunc::simpsonsum(magsq,0,nxbins-1)*deltax;
   double xmean=xnumer/curr_norm;
   double xsqmean=xsqnumer/curr_norm;
   double xsigma=sqrt(xsqmean-sqr(xmean));
   delete [] magsq;
   delete [] xdensity;
   delete [] xsqdensity;

   cout << "xmean = " << xmean << " xsigma = " << xsigma << endl;
   if (t==tmin)
   {
      xmean_init=xmean;
      xsigma_init=xsigma;
   }

   double orderfunc=t;
   const int n_indep_vars=1;
   int posn;
   double var[n_indep_vars];
   if (!xmeanlist.ordered_node_existence(orderfunc,posn))
   {
      var[0]=orderfunc;
      xmeanlist.create_and_insert_node(
         posn,orderfunc,datapoint(n_indep_vars,var,xmean));
   }
   if (!xsigmalist.ordered_node_existence(orderfunc,posn))
   {
      var[0]=orderfunc;
      xsigmalist.create_and_insert_node(
         posn,orderfunc,datapoint(n_indep_vars,var,xsigma));
   }
}

// ---------------------------------------------------------------------
// Member function compute_momentum_mean_and_spread calculates the
// expectation values <k> and sqrt(<k^2)-<k>^2) for any (true or
// trial; normalized or unnormalized) state vector.  

// Note: For gaussian wavepackets, xsigma*ksigma = 1/(4*PI).

void quantum_1Dwavefunction::compute_momentum_mean_and_spread(
   const complex curr_tilde[])
{
   double *magsq,*kdensity,*ksqdensity;
   new_clear_array(magsq,nxbins_max);
   new_clear_array(kdensity,nxbins_max);
   new_clear_array(ksqdensity,nxbins_max);
   complex curr_kterm,curr_ksqterm;

   for (int i=0; i<nxbins; i++)
   {
      double kx=kx_lo+i*delta_kx;
      curr_kterm=curr_tilde[i].Conjugate()*kx*curr_tilde[i];
      curr_ksqterm=kx*curr_kterm;
      kdensity[i]=curr_kterm.getreal();
      ksqdensity[i]=curr_ksqterm.getreal();
      magsq[i]=sqr(curr_tilde[i].getmod());
   }

   double knumer=mathfunc::simpsonsum(kdensity,0,nxbins-1)*delta_kx;
   double ksqnumer=mathfunc::simpsonsum(ksqdensity,0,nxbins-1)*delta_kx;
   double curr_norm=mathfunc::simpsonsum(magsq,0,nxbins-1)*delta_kx;
   double kmean=knumer/curr_norm;
   double ksqmean=ksqnumer/curr_norm;
   double ksigma=sqrt(ksqmean-sqr(kmean));
   delete [] magsq;
   delete [] kdensity;
   delete [] ksqdensity;

   cout << "kmean = " << kmean << " ksigma = " << ksigma << endl;
   if (t==tmin)
   {
      kxmean_init=kmean;
      kxsigma_init=ksigma;
   }

   double orderfunc=t;
   const int n_indep_vars=1;
   int posn;
   double var[n_indep_vars];
   if (!kmeanlist.ordered_node_existence(orderfunc,posn))
   {
      var[0]=orderfunc;
      kmeanlist.create_and_insert_node(
         posn,orderfunc,datapoint(n_indep_vars,var,kmean));
   }
   if (!xsigmalist.ordered_node_existence(orderfunc,posn))
   {
      var[0]=orderfunc;
      ksigmalist.create_and_insert_node(
         posn,orderfunc,datapoint(n_indep_vars,var,ksigma));
   }
}
*/

// =====================================================================
// Wavefunction dumping and restoration member functions:
// =====================================================================

// Member function dump_eigenfunction writes out the contents of the
// eigenfunction specified by quantum number n to a gzipped file for
// later retrieval by subroutine restore_eigenfunction:

void quantum_1Dwavefunction::dump_eigenfunction(int n)
{
   string suffix=stringfunc::number_to_string(n);
   string eigendir=sysfunc::get_projectsrootdir()
      +"src/mains/quantum/eigenfunctions/1D/";
   filefunc::dircreate(eigendir);
   string dumpfilename=
      eigendir+potentialfunc::get_potential_str(potential_type)
      +"_"+suffix+".dump";
   ofstream dumpstream;
   filefunc::openfile(dumpfilename,dumpstream);

   for (int i=0; i<nxbins_max; i++)
   {
         dumpstream << energy_eigenstate[n][i].getreal();
         dumpstream << " ";
         dumpstream << energy_eigenstate[n][i].getimag();
         dumpstream << " ";
   }
   filefunc::closefile(dumpfilename,dumpstream);
   filefunc::gzip_file(dumpfilename);
}

// ---------------------------------------------------------------------
// Member function restore_eigenfunction retrieves the eigenfunction
// data stored within gzipped textfiles by subroutine
// dump_eigenfunction.  This subroutine returns true or false,
// depending upon whether or not a dump file exists.

bool quantum_1Dwavefunction::restore_eigenfunction(int n)
{
   ofstream dumpstream;
   string eigendir=sysfunc::get_projectsrootdir()
      +"src/mains/quantum/eigenfunctions/1D/";
   string suffix=stringfunc::number_to_string(n);
   string dumpfilename=
      eigendir+potentialfunc::get_potential_str(potential_type)
      +"_"+suffix+".dump";
   string gzipped_dumpfilename=dumpfilename+".gz";

   double x,y;
   ifstream restorestream;
   if (filefunc::fileexist(gzipped_dumpfilename))
   {
      cout << "Reading from dumpfile eigenstate (" << n << "):" << endl;

      filefunc::gunzip_file(gzipped_dumpfilename);
      filefunc::openfile(dumpfilename,restorestream);
      for (int i=0; i<nxbins_max; i++)
      {
         restorestream >> x;
         restorestream >> y;
         energy_eigenstate[n][i]=complex(x,y);
      }
      filefunc::closefile(dumpfilename,restorestream);
      filefunc::gzip_file(dumpfilename);

      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function dump_wavefunction writes out the contents of the
// wavefunction contained within input array curr_value to an output
// file for later retrieval by subroutine restore_wavefunction:

void quantum_1Dwavefunction::dump_wavefunction(
   const complex curr_value[nxbins_max])
{
   outputfunc::write_banner("Dumping contents of current wavefunction:");
   
   ofstream dumpstream;
   string dumpfilename=imagedir+"wavefunction.dump";
   filefunc::openfile(dumpfilename,dumpstream);

   for (int i=0; i<nxbins_max; i++)
   {
         dumpstream << curr_value[i].getreal();
         dumpstream << " ";
         dumpstream << curr_value[i].getimag();
         dumpstream << " ";
   }
   filefunc::closefile(dumpfilename,dumpstream);
   filefunc::gzip_file(dumpfilename);
}

// ---------------------------------------------------------------------
// Boolean member function restore_wavefunction retrieves the
// wavefunction data stored within gzipped textfiles by subroutine
// dump_wavefunction.  This subroutine returns true or false,
// depending upon whether or not a dump file exists.

bool quantum_1Dwavefunction::restore_wavefunction(complex curr_value[])
{
   ofstream dumpstream;
   string dumpfilename=imagedir+"wavefunction.dump";
   string gzipped_dumpfilename=dumpfilename+".gz";
   filefunc::openfile(dumpfilename,dumpstream);

   double x,y;
   ifstream restorestream;
   if (filefunc::fileexist(gzipped_dumpfilename))
   {
      outputfunc::write_banner("Restoring contents of current wavefunction:");

      filefunc::gunzip_file(gzipped_dumpfilename);
      filefunc::openfile(dumpfilename,restorestream);
      for (int i=0; i<nxbins_max; i++)
      {
         restorestream >> x;
         restorestream >> y;
         curr_value[i]=complex(x,y);
      }
      filefunc::closefile(dumpfilename,restorestream);
      filefunc::gzip_file(dumpfilename);
      return true;
   }
   else
   {
      return false;
   }
}
