// ==========================================================================
// Header file for quantum_wavefunction class
// ==========================================================================
// Last modified on 1/12/04
// ==========================================================================

#ifndef QUANTUM_WAVEFUNCTION_H
#define QUANTUM_WAVEFUNCTION_H

#include <string>
#include "datastructures/linkedlist.h"
#include "image/myimage.h"
#include "quantum/potentialfuncs.h"
#include "quantum/quantum_types.h"

class quantum_wavefunction: public myimage
{
  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const quantum_wavefunction& q);

  protected:

   static const int n_potential_params;
   static const int ndigits_max;
   static const double TINY;

   long seed;
   quantum::Domain_name domain_name;
   quantum::Complex_plot_type complex_plot_type;
   potentialfunc::Potential_type potential_type;
   std::string state_type;

// Allow independent variables x and y to be shifted by some constants
// for numerical simulation purposes.  Such constant shifts should
// have no impact upon the dynamics of the quantum system.

   double xshift,yshift;	

   double t,tmin,tmax;

// We introduce the following two time parameters for time-dependent
// potentials.  For 0 < t < potential_t1, we let the potential be
// essentially independent of time.  For potential_t1 < t <
// potential_t2, we let one or more of the potential parameters within
// potential_param[] assume different values.  Finally for
// potential_t2 < t, we generally let the potential parameters return
// to the original values:

   double potential_t1,potential_t2;

  public:

//   static const int nxbins_max;
//   static const int nybins_max;
//   static const int nxbins_max=495; // 495 = 3^2 * 5 * 11
   static const int nybins_max=495;
   static const int nxbins_max=975; // 975 = 3 * 5^2 * 13
//   static const int nybins_max=245; // 245 = 5 * 7^2
//   static const int nybins_max=3; 
//   static const int nxbins_max=2025; // 2025 = 3^4 * 5^2
//   static const int nxbins_max=4095; // 4095 = 3^2 * 5 * 7 * 13

   bool time_dependent_potential;
   bool save_eigenfunction;	// By default, this flag equals true
   time_t start_processing_time;  // Value measured in seconds since some Unix
			          // reference point.

   int ndims,Ntsteps;
   int nxbins,nybins;
   double deltat,T;
   double xmean_init,xsigma_init,kxmean_init,kxsigma_init;
   double norm,energy;
   double display_frac,V_displayfactor;

// Store potential params into the following member variables for
// metafile output purposes:

   double *potential_param;

   fftw_plan theforward_1d,thebackward_1d;
   fftwnd_plan theforward_2d,thebackward_2d;
   linkedlist energylist,Efinallist;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

   quantum_wavefunction(void);
   quantum_wavefunction(const quantum_wavefunction& q);
   ~quantum_wavefunction();
   quantum_wavefunction& operator= (const quantum_wavefunction& q);

// Set & get member functions:

   void set_t(double T);
   void set_tmin(double T);
   void set_tmax(double T);
   void set_potential_t1(double T);
   void set_potential_t2(double T);

   quantum::Domain_name get_domain_name() const;
   quantum::Complex_plot_type get_complex_plot_type() const;
   potentialfunc::Potential_type quantum_wavefunction::get_potential_type() 
      const;
   std::string get_state_type() const;
   double get_t() const;
   double get_tmin() const;
   double get_tmax() const;
   double get_potential_t1() const;
   double get_potential_t2() const;

// Initialization member functions:

   void select_plot_output(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   void select_potential(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   void specify_potential_timedependence(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   virtual void set_potential_spatial_period()=0;
   void init_fftw();

   double initialize_simulation(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   virtual void initialize_spatial_and_momentum_parameters()=0;
   double initialize_simulation_timescales();
   void set_potential_time_dependence(
      bool input_param_file,std::string inputline[],int& currlinenumber);

// Initial state preparation member functions:

   int select_n_energystates(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   void prepare_initial_state(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   int select_initial_state(
      bool input_param_file,std::string inputline[],int& currlinenumber);
   virtual void project_low_energy_states(int n_energystates)=0;
   virtual void initialize_wavefunction()=0;

// Output member functions:

   void summarize_results(int n_timesteps);
   void plot_energies_vs_time();
};

// ==========================================================================
// Inlined methods
// ==========================================================================

// Set and get member functions

inline void quantum_wavefunction::set_t(double T) 
{
   t=T;
}

inline void quantum_wavefunction::set_tmin(double T) 
{
   tmin=T;
}

inline void quantum_wavefunction::set_tmax(double T) 
{
   tmax=T;
}

inline void quantum_wavefunction::set_potential_t1(double T) 
{
   potential_t1=T;
}

inline void quantum_wavefunction::set_potential_t2(double T) 
{
   potential_t2=T;
}

inline quantum::Domain_name quantum_wavefunction::get_domain_name() const
{
   return domain_name;
}

inline quantum::Complex_plot_type quantum_wavefunction::get_complex_plot_type() const
{
   return complex_plot_type;
}

inline potentialfunc::Potential_type quantum_wavefunction::
get_potential_type() const
{
   return potential_type;
}

inline std::string quantum_wavefunction::get_state_type() const
{
   return state_type;
}

inline double quantum_wavefunction::get_t() const
{
   return t;
}

inline double quantum_wavefunction::get_tmin() const
{
   return tmin;
}

inline double quantum_wavefunction::get_tmax() const
{
   return tmax;
}

inline double quantum_wavefunction::get_potential_t1() const
{
   return potential_t1;
}

inline double quantum_wavefunction::get_potential_t2() const
{
   return potential_t2;
}

#endif  // quantum_wavefunction.h
