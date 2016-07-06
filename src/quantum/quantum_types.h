// =========================================================================
// Header file for quantum enumerated types
// =========================================================================
// Last modified on 3/22/03
// =========================================================================

#ifndef QUANTUM_TYPES_H
#define QUANTUM_TYPES_H


#include <iomanip>
#include <iostream>
#include <string>

namespace quantum
{
   enum Domain_name
   {
      position_space,momentum_space,position_and_momentum_space
   };

   enum Complex_plot_type
   {
      posn_momentum,mag_phase,real_imag,sqd_amp,energy_prob,energy_real,
      energy_spectrum
   };

   std::string get_complex_plot_str(
      const Complex_plot_type complex_plot_type);
}

#endif // quantum_types.h



