// ==========================================================================
// Quantum_wavefunction class member function definitions
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include "math/basic_math.h"
#include "math/complex.h"
#include "math/constants.h"
#include "datastructures/containerfuncs.h"
#include "plot/metafile.h"
#include "datastructures/mynode.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/quantumarray.h"
#include "quantum_wavefunction.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;

// Maximum number of pixels in x direction 

// const int quantum_wavefunction::nxbins_max=495;  // 495 = 3^2 * 5 * 11
// const int quantum_wavefunction::nxbins_max=975;    // 975 = 3 * 5^2 * 13
// const int nxbins_max=2025; // 2025 = 3^4 * 5^2
// const int nxbins_max=4095; // 4095 = 3^2 * 5 * 7 * 13

// Maximum number of pixels in y direction 

// const int quantum_wavefunction::nybins_max=495;	

const int quantum_wavefunction::n_potential_params=5;

// Max number of image label digits
const int quantum_wavefunction::ndigits_max=3;	

const double quantum_wavefunction::TINY=1E-12;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void quantum_wavefunction::allocate_member_objects()
{
   new_clear_array(potential_param,n_potential_params);
   energylist.allocate_member_objects();
   Efinallist.allocate_member_objects();
}		      

void quantum_wavefunction::initialize_member_objects()
{
   seed=-1000;
   xshift=yshift=0;
   potential_t1=0;
   potential_t2=100;
}

quantum_wavefunction::quantum_wavefunction(void)
{
   allocate_member_objects();
   initialize_member_objects();
   time_dependent_potential=false;
   save_eigenfunction=true;
}

// Copy constructor

quantum_wavefunction::quantum_wavefunction(const quantum_wavefunction& q):
   myimage(q)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(q);
}

quantum_wavefunction::~quantum_wavefunction()
{
   delete [] potential_param;
}

// ---------------------------------------------------------------------
void quantum_wavefunction::docopy(const quantum_wavefunction& q)
{
   seed=q.seed;
   domain_name=q.domain_name;
   complex_plot_type=q.complex_plot_type;
   potential_type=q.potential_type;
   state_type=q.state_type;
   xshift=q.xshift;
   yshift=q.yshift;
   potential_t1=q.potential_t1;
   potential_t2=q.potential_t2;

   time_dependent_potential=q.time_dependent_potential;
   save_eigenfunction=q.save_eigenfunction;
   start_processing_time=q.start_processing_time;
   ndims=q.ndims;
   Ntsteps=q.Ntsteps;
   tmin=q.tmin;
   tmax=q.tmax;
   deltat=q.deltat;
   t=q.t;
   T=q.T;
   xmean_init=q.xmean_init;
   xsigma_init=q.xsigma_init;
   kxmean_init=q.kxmean_init;
   kxsigma_init=q.kxsigma_init;
   norm=q.norm;
   energy=q.energy;
   display_frac=q.display_frac;
   V_displayfactor=q.V_displayfactor;

   for (int n=0; n<n_potential_params; n++)
   {
      potential_param[n]=q.potential_param[n];
   }
   
   theforward_1d=q.theforward_1d;
   thebackward_1d=q.thebackward_1d;
   theforward_2d=q.theforward_2d;
   thebackward_2d=q.thebackward_2d;
}	

// Overload = operator:

quantum_wavefunction& quantum_wavefunction::operator= 
(const quantum_wavefunction& q)
{
   if (this==&q) return *this;
   myimage::operator=(q);
   docopy(q);
   return *this;
}

// =====================================================================
// Initialization member functions:
// =====================================================================

// Member function select_plot_output queries the user to specify
// whether the Position or momentum space wavefunctions should be
// displayed.  It also asks whether to plot the magnitude/phase or
// real/imaginary parts of the complex wavefunction:

void quantum_wavefunction::select_plot_output(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   int plottype_int;
   string outputline[10];
   
   bool valid_response=false;
   do
   {
      outputline[0]="Enter wavefunction to be displayed:";
      outputline[1]="1 = Position space";
      outputline[2]="2 = Momentum space";
      outputline[3]="3 = Position and momentum space magnitudes";
      plottype_int=stringfunc::mygetinteger(
         4,outputline,input_param_file,inputline,currlinenumber);
      if (plottype_int==1)
      {
         domain_name=quantum::position_space;
         pagetitle="Position Space Wavefunction";
         valid_response=true;
      }
      else if (plottype_int==2)
      {
         domain_name=quantum::momentum_space;
         pagetitle="Momentum Space Wavefunction";
         valid_response=true;
      }
      else if (plottype_int==3)
      {
         domain_name=quantum::position_and_momentum_space;
         pagetitle="Position and Momentum Space Wavefunction Magnitudes";
         complex_plot_type=quantum::posn_momentum;
         valid_response=true;
      }
   }
   while (!valid_response);

   if (domain_name != quantum::position_and_momentum_space)
   {
      valid_response=false;
      do
      {
         outputline[0]="Enter output to be displayed:";
         outputline[1]="1 = Magnitude and phase";
         outputline[2]="2 = Real and imaginary";
         outputline[3]="3 = Probability density";
         outputline[4]="4 = Potential + state energy vs probability density";
         outputline[5]="5 = Potential + state energy vs real part";
         outputline[6]="6 = Energy spectrum";
         plottype_int=stringfunc::mygetinteger(
            7,outputline,input_param_file,inputline,currlinenumber);
         if (plottype_int==1)
         {
            complex_plot_type=quantum::mag_phase;
            valid_response=true;
         }
         else if (plottype_int==2)
         {
            complex_plot_type=quantum::real_imag;
            valid_response=true;
         }
         else if (plottype_int==3)
         {
            complex_plot_type=quantum::sqd_amp;
            valid_response=true;
         }
         else if (plottype_int==4)
         {
            complex_plot_type=quantum::energy_prob;
            valid_response=true;
         }
         else if (plottype_int==5)
         {
            complex_plot_type=quantum::energy_real;
            valid_response=true;
         }
         else if (plottype_int==6)
         {
            complex_plot_type=quantum::energy_spectrum;
            valid_response=true;
         }
      }
      while (!valid_response);
   } // domain_name != position_and_momentum_space conditional
}

// ---------------------------------------------------------------------
// Member function select_potential queries the user to select a
// potential from a menu list.  It also queries the user as to whether
// the potential is time dependent.

void quantum_wavefunction::select_potential(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   string outputline[20];
   
   int i=0;
   outputline[i++]="Select potential:";
   outputline[i++]="1 = Box (1D): deprecated";
   outputline[i++]="2 = Harmonic oscillator (1D & 2D)";
   outputline[i++]="3 = Lambda phi**4 (1D)";
   outputline[i++]="4 = Double well trap (1D & 2D)";
   outputline[i++]="5 = Mathieu (1D & 2D)";
   outputline[i++]="6 = Free particle (1D)";
   outputline[i++]="7 = Ramp (1D)";
   outputline[i++]="8 = Inverted parabola (1D)";
   outputline[i++]="9 = Aperiodic cosine (1D)";
   outputline[i++]="10 = Smooth step (1D)";
   outputline[i++]="11 = SQUID (1D)";
   outputline[i++]="12 = QFP (2D)";
   outputline[i++]="13 = Smooth box (1D)";

   int potential_number=stringfunc::mygetinteger(
      i,outputline,input_param_file,inputline,currlinenumber);

// We apply a scaling factor in order to keep values for particular
// potentials which are written to metafile output within reasonable
// ranges:

   if (potential_number==1)
   {
      potential_type=potentialfunc::box;
      display_frac=1.0/4.0;
      V_displayfactor=1;
   }
   else if (potential_number==2)
   {
      potential_type=potentialfunc::harmonic_osc;
      display_frac=0.25;
//      display_frac=0.33;
//      display_frac=1.0;
      V_displayfactor=1;
//      V_displayfactor=1E-6;
   }
   else if (potential_number==3)
   {
      potential_type=potentialfunc::lambda_phi_4;
//      display_frac=1.0;
//      display_frac=1.0/2.0;
      display_frac=1.0/4.0;
      V_displayfactor=1;
   }
   else if (potential_number==4)
   {
      potential_type=potentialfunc::doublewell;
      display_frac=1.0;
      V_displayfactor=1;
   }
   else if (potential_number==5)
   {
      potential_type=potentialfunc::mathieu;
      display_frac=1.0;
      V_displayfactor=1;
   }
   else if (potential_number==6)
   {
      potential_type=potentialfunc::freeparticle;
      display_frac=1.0;
      V_displayfactor=1;
   }
   else if (potential_number==7)
   {
      potential_type=potentialfunc::ramp;
//      display_frac=1.0;
      display_frac=0.1;
      V_displayfactor=1;
   }
   else if (potential_number==8)
   {
      potential_type=potentialfunc::inverted_parabola;
      display_frac=0.2;
//      display_frac=1.0;
      V_displayfactor=1E-4;
//      V_displayfactor=1;
   }
   else if (potential_number==9)
   {
      potential_type=potentialfunc::aperiodic_cosine;
      display_frac=1.0;
//      display_frac=0.1;
      V_displayfactor=1E-3;
   }
   else if (potential_number==10)
   {
      potential_type=potentialfunc::smooth_step;
      display_frac=1;
      V_displayfactor=1;
   }
   else if (potential_number==11)
   {
      potential_type=potentialfunc::squid;
//      display_frac=0.625;
      display_frac=0.75;
//      display_frac=1;
      V_displayfactor=1;
   }
   else if (potential_number==12)
   {
      potential_type=potentialfunc::QFP;
//      display_frac=0.05;
//      display_frac=0.25;
      display_frac=1.0;
      V_displayfactor=0.3;
//      V_displayfactor=1E-6;
   }
   else if (potential_number==13)
   {
      potential_type=potentialfunc::smooth_box;
      display_frac=1;
      V_displayfactor=1;
   }
}

// ---------------------------------------------------------------------
// Member function specify_potential_timedependence queries the user
// as to whether the potential is time dependent or not:

void quantum_wavefunction::specify_potential_timedependence(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   string outputline[1];
   outputline[0]="Time dependent potential? (y/n)";
   char time_dependent_char=stringfunc::mygetchar(
      1,outputline,input_param_file,inputline,currlinenumber);
   if (time_dependent_char=='y')
   {
      time_dependent_potential=true;
   }
   else
   {
      time_dependent_potential=false;
   }
}

// ---------------------------------------------------------------------
// Initialize FFTW algorithm by either calculating FFT weights from
// scratch or else reading in previously computed values from files
// "fftw.forward" and "fftw.backward" within cplusplusrootdir/classes:

// Note: The senses of "theforward" and "thebackward" are adjusted
// here so as to be compatible with Numerical Recipes' Fourier
// transform conventions.

void quantum_wavefunction::init_fftw()
{
   bool save_forward_wisdom=false;
   bool save_backward_wisdom=false;

   string prefix=sysfunc::get_projectsrootdir()+"/src/mains/quantum";
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

   if (ndims==1)
   {
      theforward_1d=fftw_create_plan(nxbins_max,FFTW_BACKWARD,FFTW_MEASURE | 
                                     FFTW_USE_WISDOM);
      thebackward_1d=fftw_create_plan(nxbins_max,FFTW_FORWARD,FFTW_MEASURE | 
                                      FFTW_USE_WISDOM);
   }
   else if (ndims==2)
   {
      theforward_2d=fftw2d_create_plan(
         nxbins_max,nybins_max,FFTW_FORWARD,
         FFTW_MEASURE | FFTW_IN_PLACE);
      thebackward_2d=fftw2d_create_plan(
         nxbins_max,nybins_max,
         FFTW_BACKWARD,FFTW_MEASURE | FFTW_IN_PLACE);
   }
   
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
// Member function initialize_simulation queries the user to input a
// few basic input/output as well as potential parameters:

double quantum_wavefunction::initialize_simulation(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   imagedir=outputfunc::select_output_directory(
      false,false,input_param_file,inputline,currlinenumber,"images");
   select_plot_output(input_param_file,inputline,currlinenumber);

// Select potential and determine characteristic time and length
// scales which depend upon the system's dimensionless coupling
// constant:

   init_fftw();
   select_potential(input_param_file,inputline,currlinenumber);

// Note:  The following call needs to be generalized to ndims=2 !

   if (ndims==1)
   {
      xshift=potentialfunc::set_xshift(potential_type);
   }
   set_potential_spatial_period();

   initialize_spatial_and_momentum_parameters();
   double dt_plot=initialize_simulation_timescales();
   return dt_plot;
}

// ---------------------------------------------------------------------
// Member function initialize_simulation_timescales relates the
// evolution time step size to a fixed, small fraction of the system's
// resonant frequency.  Recall that a system's characteristic time
// scale goes like 1/sqrt(alpha).  We set one deltat step in time to
// correpond approximately to 1/8th of a degree in phase.  This method
// also returns the temporal step between image output.

double quantum_wavefunction::initialize_simulation_timescales()
{
//   const int Nperiods=2;
   const int Nperiods=6;
//   const int Nperiods=12;
//   const int Nperiods=30;
   const double phasestep_per_plot=5 ; // Phase evolution (in degs) between
   
// Recall that the system's dimensionless coupling constant alpha ( =
// ratio of potential to kinetic energies) is saved within the
// potential_param[0] member variable:

   double V,dV,d2V;
   potentialfunc::potential(
      false,potential_type,seed,potential_t1,potential_t2,0,potential_param,
      0,V,dV,d2V);
   double alpha=potential_param[0];

   int ntsteps_per_plot=basic_math::round(phasestep_per_plot/0.125);
   int ntsteps_per_period=basic_math::round(360.0/0.125);
   deltat=0.001/sqrt(alpha);
   tmin=0;
   tmax=Nperiods*ntsteps_per_period*deltat;
   double dt_plot=ntsteps_per_plot*deltat;

   cout << "Evolution time step size = " << deltat << endl;
   cout << "tmin = " << tmin << endl;
   cout << "tmax = " << tmax << endl;
   cout << "dt_plot = " << dt_plot << endl;

   return dt_plot;
}

// ---------------------------------------------------------------------
// Member function set_potential_time_dependence first queries the
// user as to whether the potential should be time dependent or not.
// If so, this method sets the potential temporal parameters
// potential_t1 and potential_t2 during which potential parameters are
// altered away from their starting and ending values.

void quantum_wavefunction::set_potential_time_dependence(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   specify_potential_timedependence(
      input_param_file,inputline,currlinenumber);
   if (time_dependent_potential) 
   {
      if (potential_type==potentialfunc::harmonic_osc)
      {
         potential_t1=4;
         potential_t2=potential_t1+5;
      }
      else if (potential_type==potentialfunc::squid)
      {
         potential_t1=5;
//         potential_t2=potential_t1+12;
//         potential_t2=potential_t1+12.8; // Yields decent results
         potential_t2=potential_t1+13.8; // Yields decent results
//         potential_t2=potential_t1+11.8; // Yields decent results
      }

// Reset value for tmax so that it goes for some reasonable amount of
// time past the second potential transition time:

      tmax=potential_t2+15;

      potentialfunc::plot_potential_param_time_dependence(
         potential_type,tmin,tmax,potential_t1,potential_t2,
         imagedir,potential_param);
   }
}

// =====================================================================
// Initial state preparation member functions:
// =====================================================================

// Member function select_n_energystates queries the user to enter the
// number of lowest energy eigenstates to be calculated.  This
// subroutine is called by program SPECTRUM.

int quantum_wavefunction::select_n_energystates(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   string outputline[10];
   outputline[0]="Select number of energy eigenstates:";
   int n_energystates=stringfunc::mygetinteger(
      1,outputline,input_param_file,inputline,currlinenumber);
   return n_energystates;
}

// ---------------------------------------------------------------------
// Member function prepare_initial_stae computes low-lying energy
// eigenstates and initializes the wavefunction:

void quantum_wavefunction::prepare_initial_state(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   int n_energystates=select_initial_state(
      input_param_file,inputline,currlinenumber);
   project_low_energy_states(n_energystates);
   initialize_wavefunction();
}

// ---------------------------------------------------------------------
int quantum_wavefunction::select_initial_state(
   bool input_param_file,string inputline[],int& currlinenumber)
{
   outputfunc::newline();
   string outputline[10];
   outputline[0]="Select initial state:";

   int i=1;
   int state_number,n_energystates=0;
   if (ndims==1)
   {
      outputline[i++]="1 = (0) state";
      outputline[i++]="2 = (1) state";
      outputline[i++]="3 = (0)+(1) superposition";
      outputline[i++]="4 = (2) state";
      outputline[i++]="5 = (3) state";
      outputline[i++]="6 = Gaussian wave packet";
      state_number=stringfunc::mygetinteger(
         i,outputline,input_param_file,inputline,currlinenumber);

      if (state_number==1)
      {
         state_type="(0)";
         n_energystates=1;
      }
      else if (state_number==2)
      {
         state_type="(1)";
         n_energystates=2;
      }
      else if (state_number==3)
      {
         state_type="(0)+(1)";
         n_energystates=2;
      }
      else if (state_number==4)
      {
         state_type="(2)";
         n_energystates=3;
      }
      else if (state_number==5)
      {
         state_type="(3)";
         n_energystates=4;
      }
      else if (state_number==6)
      {
         state_type="gaussian wavepacket";
         n_energystates=0;
      }
   }
   else if (ndims==2)
   {
      outputline[i++]="1 = (0,0) state";
      outputline[i++]="2 = (1,0) state";
      outputline[i++]="3 = (0,1) state";
      outputline[i++]="4 = (0,0)+(1,0) superposition";
      outputline[i++]="5 = (0,0)+(0,1) superposition";
      outputline[i++]="6 = (0,0)+(1,0)+(0,1) superposition";
      outputline[i++]="7 = (2,0) state";
      outputline[i++]="8 = (1,1) state";
      outputline[i++]="9 = (0,2) state";
      state_number=stringfunc::mygetinteger(
         i,outputline,input_param_file,inputline,currlinenumber);
   
      if (state_number==1)
      {
         state_type="(0,0)";
         n_energystates=1;
      }
      else if (state_number==2)
      {
         state_type="(1,0)";
         n_energystates=2;
      }
      else if (state_number==3)
      {
         state_type="(0,1)";
         n_energystates=2;
      }
      else if (state_number==4)
      {
         state_type="(0,0)+(1,0)";
         n_energystates=2;
      }
      else if (state_number==5)
      {
         state_type="(0,0)+(0,1)";
         n_energystates=2;
      }
      else if (state_number==6)
      {
         state_type="(0,0)+(1,0)+(0,1)";
         n_energystates=2;
      }
      else if (state_number==7)
      {
         state_type="(2,0)";
         n_energystates=3;
      }
      else if (state_number==8)
      {
         state_type="(1,1)";
         n_energystates=3;
      }
      else if (state_number==9)
      {
         state_type="(0,2)";
         n_energystates=3;
      }
   }

   cout << "state_type = " << state_type << endl;
   cout << "n_energystates = " << n_energystates << endl;
   return n_energystates;
}

// =====================================================================
// Output member functions:
// =====================================================================

// Member function summarize_results writes out an ascii text file
// containing parameter and processing time information:

void quantum_wavefunction::summarize_results(int n_timesteps)
{
   string data_filename=imagedir+"results.summary";
   ofstream datastream,logstream;

   filefunc::openfile(data_filename,datastream);
   datastream << "Wavefunction evolution results generated on "
              << timefunc::getcurrdate() << endl;
   datastream << "Output directory containing all results = " 
              << imagedir << endl << endl;
   datastream << endl;
   
   datastream << "Coupling constant alpha = " << potential_param[0] << endl;
   if (ndims==1)
   {
      datastream << "Initial x mean = " << xmean_init << endl;
      datastream << "Initial x sigma = " << xsigma_init << endl;
      datastream << "Initial kx mean = " << kxmean_init << endl;
      datastream << "Initial kx sigma = " << kxsigma_init << endl;
   }

// Write out time required to complete wavefunction evolution to
// screen and results summary file:

   outputfunc::newline();
   cout << "**************************************************************** "
        << endl;
   cout << "Time required to complete wavefunction evolution = "
      +stringfunc::number_to_string(
         outputfunc::processing_time(start_processing_time),1)+" mins = " 
      +stringfunc::number_to_string(
         outputfunc::processing_time(start_processing_time)/60.0,2)+" hours" 
        << endl;
   cout << "**************************************************************** "
        << endl;
   outputfunc::newline();

   datastream << endl;
   datastream << "Time required to complete wavefunction evolution = "
      +stringfunc::number_to_string(
         outputfunc::processing_time(start_processing_time),1)+" mins = " 
      +stringfunc::number_to_string(outputfunc::processing_time(
         start_processing_time)/60.0,2)+" hours" << endl;
   datastream << "Number of time step iterations = " << n_timesteps
              << endl;
   datastream << "Total evolution time = " << tmax << endl;
   datastream << endl;
   filefunc::closefile(data_filename,datastream);
}

// ---------------------------------------------------------------------
// Member function plot_energies_vs_time writes to meta file output
// energy vs time

void quantum_wavefunction::plot_energies_vs_time()
{
   containerfunc::find_max_min_func_values(&energylist);

   energylist.get_metafile_ptr()->set_title("System Energy");
   energylist.get_metafile_ptr()->set_labels("Time","Energy");
   energylist.get_metafile_ptr()->set_filename(imagedir+"energy");
   energylist.get_metafile_ptr()->set_xbounds(tmin,tmax);
   energylist.get_metafile_ptr()->set_ybounds(
      -0.001,1.2*energylist.get_fmax());
   energylist.get_metafile_ptr()->set_ytic(
      trunclog(1.2*energylist.get_fmax()));
   energylist.get_metafile_ptr()->set_ysubtic(
      0.5*energylist.get_metafile_ptr()->get_ytic());
   plotfunc::writelist(energylist);

   if (time_dependent_potential)
   {
      Efinallist.get_metafile_ptr()->set_filename(
         energylist.get_metafile_ptr()->get_filename());
//      cout << "Efinallist.size() = " 
//           << Efinallist.size() << endl;
      for (int i=0; i<Efinallist.size(); i++)
      {
         Efinallist.get_node(i)->get_data().set_color(colorfunc::red);
      }
      plotfunc::append_plot(Efinallist);
   }
}



