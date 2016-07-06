// ==========================================================================
// Header file for quantum_1Dwavefunction class
// ==========================================================================
// Last modified on 1/12/04
// ==========================================================================

#ifndef QUANTUM_1DWAVEFUNCTION_H
#define QUANTUM_1DWAVEFUNCTION_H

#include "math/complex.h"
#include "datastructures/linkedlist.h"
#include "quantum/quantumarray.h"
#include "quantum/quantum_wavefunction.h"
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class quantum_1Dwavefunction: public quantum_wavefunction
{
  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const quantum_1Dwavefunction& q);

  public:

   static const int Nmax_energystates;

   double xhi,xlo,deltax;
   double kx_hi,kx_lo,delta_kx;
   double xperiod;	// Potential period in x direction
   double *prev_arg,*curr_arg;
   complex *value,*tilde;
   complex (*energy_eigenstate)[nxbins_max];
   linkedlist xmeanlist,xsigmalist,kmeanlist,ksigmalist;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions
// ---------------------------------------------------------------------

   quantum_1Dwavefunction();
   quantum_1Dwavefunction(const quantum_1Dwavefunction& q);
   ~quantum_1Dwavefunction();
   quantum_1Dwavefunction& operator= (const quantum_1Dwavefunction& q);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

// FFT member functions:

   void fouriertransform(const complex value_in[nxbins_max],
                         complex tilde_out[nxbins_max]);
   void inversefouriertransform(const complex tilde_in[nxbins_max],
                                complex value_out[nxbins_max]);
// Meta file member functions:

   quantumarray plot_potential(std::string plotfilename);
   quantumarray plot_potential(int imagenumber);
   quantumarray plot_wavefunction(twoDarray& psi_twoDarray,int imagenumber);
   quantumarray plot_wavefunction(
      twoDarray& position_twoDarray,twoDarray& momentum_twoDarray,
      int imagenumber);
   quantumarray plot_probdensity(twoDarray& psi_twoDarray,int imagenumber);
   quantumarray plot_spectrum(
      int n_energystates,twoDarray& spectrum_twoDarray);
   quantumarray plot_eigenfunctions(
      int n_energystates,twoDarray& efunc_twoDarray);
   quantumarray plot_spectrum_and_efuncs(
      int n_energystates,twoDarray& spectrum_twoDarray,
      twoDarray& efunc_twoDarray,int imagenumber);
   void singletfile_footer(std::string imagefilename,int nimage,double E);
   void doubletfile_footer(
      std::string imagefilename,int nimage,double E,double Efinal);
   void write_packet_posn_and_width();
   void output_wavefunction_info(
      double E,int& nimage,double& t_lastplot,
      quantumarray& pwi_dataarray,twoDarray& psi_twoDarray,
      twoDarray& position_twoDarray,twoDarray& momentum_twoDarray);

// Wavefunction manipulation member functions:

   void clear_wavefunction();
   void copy_wavefunction(const complex value_orig[],complex value_copy[]);
   void initialize_spatial_and_momentum_parameters();
   void trial_wavefunction(bool even_wavefunction,complex curr_value[]);
   void gaussian_wavepacket(double x0,complex curr_value[]);
   virtual void initialize_wavefunction();
   void evolve_phase();
   virtual void set_potential_spatial_period();
   void compute_tridiagonal_matrix_elements(
      complex a[],complex b[],complex c[]);
   void renormalize_wavefunction(complex curr_value[]);
   void null_tiny_value(complex curr_value[]);
   void prepare_plot_data(twoDarray& psi_twoDarray,double E);
   void prepare_plot_data(
      twoDarray& position_twoDarray,twoDarray& momentum_twoDarray);
   void prepare_spectrum_data(
      int n_energystates,twoDarray& spectrum_twoDarray,
      twoDarray& efunc_twoDarray);

// Wavefunction evolution member functions:

   void evolve_wavefunction_thru_one_timestep(
      int& n_redos,double E,complex value_copy[]);
   void FFT_step_wavefunction(bool Wick_rotate,complex curr_value[]);
   virtual void project_low_energy_states(int n_energystates);
   complex energystate_overlap(int n_energystate,complex curr_value[]);
   void remove_overlap(int n_energystate,complex curr_value[]);
   void project_energy_eigenstate(int n_energystate,complex prev_value[]);

// Wavefunction properties member functions:

   double compute_normalization(const complex curr_value[]);
   double compute_energy(const complex curr_value[]);
   void compute_posn_mean_and_spread(const complex curr_value[]);
   void compute_momentum_mean_and_spread(const complex curr_tilde[]);

// Wavefunction dumping and restoration member functions:

   void dump_eigenfunction(int n);
   bool restore_eigenfunction(int n);
   void dump_wavefunction(const complex curr_value[]);
   bool restore_wavefunction(complex curr_value[]);
};

#endif  // quantum_1Dwavefunction.h




