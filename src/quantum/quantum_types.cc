// ==========================================================================
// QUANTUMTYPES stand-alone methods
// ==========================================================================
// Last modified on 3/22/03
// ==========================================================================

#include "quantum/quantum_types.h"

using std::string;
using std::ostream;
using std::endl;

namespace quantum
{
   string get_complex_plot_str(const Complex_plot_type complex_plot_type)
      {
         string plotstr;
         
         if (complex_plot_type==posn_momentum)
         {
            plotstr="posn_momentum";
         }
         else if (complex_plot_type==mag_phase)
         {
            plotstr="mag_phase";
         }
         else if (complex_plot_type==real_imag)
         {
            plotstr="real_imag";
         }
         else if (complex_plot_type==sqd_amp)
         {
            plotstr="sqd_amp";
         }
         else if (complex_plot_type==energy_prob)
         {
            plotstr="energy_prob";
         }
         else if (complex_plot_type==energy_real)
         {
            plotstr="energy_real";
         }
         else if (complex_plot_type==energy_spectrum)
         {
            plotstr="energy_spectrum";
         }

         return plotstr;
      }
} // potentialfunc namespace

