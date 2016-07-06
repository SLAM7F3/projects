// ==========================================================================
// Header file for quantum_2Dwavefunction class
// ==========================================================================
// Last modified on 1/12/04
// ==========================================================================

#ifndef QUANTUM_2DWAVEFUNCTION_H
#define QUANTUM_2DWAVEFUNCTION_H

#include "quantum/quantumimage.h"

class quantum_2Dwavefunction: public quantumimage
{
  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const quantum_2Dwavefunction& q);

  public:

   static const int Nmax_energystates=3;

   bool efunc_calculated[Nmax_energystates][Nmax_energystates];
   double x,y;

// Allow independent variables x and y to be shifted by some constants
// for numerical simulation purposes.  Such constant shifts should
// have no impact upon the dynamics of the quantum system.

//   double xshift,yshift;	

   double (*curr_arg)[nybins_max];
   double (*prev_arg)[nybins_max];
   complex (*value)[nybins_max];
   complex (*tilde)[nybins_max];
   complex (*energy_eigenstate)[Nmax_energystates][nxbins_max][nybins_max];

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions
// ---------------------------------------------------------------------

   quantum_2Dwavefunction();
   quantum_2Dwavefunction(const quantum_2Dwavefunction& q);
   ~quantum_2Dwavefunction();
   quantum_2Dwavefunction& operator= (const quantum_2Dwavefunction& q);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

// Meta file member functions:

   void writeimagedata();
   void writeimagedata(int doublet_pair_member);
   void writeimagedata(
      int doublet_pair_member,const complex carray[nxbins_max][nybins_max]);
   void writeimage(
      std::string base_imagefilename,std::string currtitle,
      const complex carray[nxbins_max][nybins_max]);
   void plot_potential(int imagenumber);
   void plot_potential(std::string plotfilename);
   void write_potential_data();

// Wavefunction manipulation member functions:

   void clear_wavefunction();
   void copy_wavefunction(const complex value_orig[nxbins_max][nybins_max],
                          complex value_copy[nxbins_max][nybins_max]);
   virtual void initialize_spatial_and_momentum_parameters();
   void trial_wavefunction(
      int m,int n,complex curr_value[nxbins_max][nybins_max]);
   virtual void initialize_wavefunction();
   void evolve_phase();
   double potential(double x,double y);
   void potential(double x,double y,double& V,
                  double& dVdx,double& dVdy,double& laplacianV);
   void potential(
      double x,double y,double& V,double& dVdx,double& dVdy,
      double& laplacianV,double& doublelaplacianV,double& Kconst,double& K12);
   virtual void set_potential_spatial_period();
   void renormalize_wavefunction(complex curr_value[nxbins_max][nybins_max]);
   double double_simpsonsum(double f[nxbins_max][nybins_max]);
   complex double_simpsonsum(complex f[nxbins_max][nybins_max]);

// Wavefunction evolution member functions:

   void FFT_step_wavefunction(
      bool Wick_rotate,complex curr_value[nxbins_max][nybins_max]);
   virtual void project_low_energy_states(int n_energystates);
   complex energystate_overlap(
      int m,int n,complex curr_value[nxbins_max][nybins_max]);
   void remove_overlap(int mmax,int nmax,
                       complex curr_value[nxbins_max][nybins_max]);
   void project_energy_eigenstate(
      int m,int n,complex prev_value[nxbins_max][nybins_max]);

// Wavefunction properties member functions:

   double compute_normalization(
      const complex curr_value[nxbins_max][nybins_max]);
   double compute_energy(const complex curr_value[nxbins_max][nybins_max]);

// Wavefunction dumping and restoration member functions:

   void dump_eigenfunction(int m,int n);
   bool restore_eigenfunction(int m,int n);
   void dump_wavefunction(const complex curr_value[nxbins_max][nybins_max]);
   bool restore_wavefunction(
      bool input_param_file,std::string inputline[],int currlinenumber,
      complex curr_value[nxbins_max][nybins_max]);
};

#endif  // quantum_2Dwavefunction.h


