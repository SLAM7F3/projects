// ==========================================================================
// Header file for QUANTUMARRAY class 
// ==========================================================================
// Last modified on 1/12/04
// ==========================================================================

#ifndef QUANTUMARRAY_H
#define QUANTUMARRAY_H

#include "datastructures/dataarray.h"
#include "quantum/quantum_types.h"
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class quantumarray: public dataarray
{
  private:

   void docopy(const quantumarray& d);

  public:
   
// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

   quantumarray(void);
   quantumarray(const quantumarray& d);
   quantumarray(double minx,double delta,twoDarray& Tdata);
   ~quantumarray();
   quantumarray& operator= (const quantumarray& d);

// ---------------------------------------------------------------------
// Quantumarray manipulation member functions:
// ---------------------------------------------------------------------

   void write_singlet_data(int n_energystates,double xshift,
                           quantum::Complex_plot_type complex_plot_type);
   void write_singlet_data(
      int n_energystates,double xshift,
      quantum::Complex_plot_type complex_plot_type,
      std::string potential_type);
   void write_doublet_data(
      int doublet_pair_member,double xshift,
      quantum::Domain_name domain_name,
      quantum::Complex_plot_type complex_plot_type);
   void write_doublet_data(
      int doublet_pair_member,int n_energystates,double xshift);
};

#endif // quantumarray.h




