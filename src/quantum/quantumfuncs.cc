// ==========================================================================
// QUANTUMFUNCS stand-alone methods
// ==========================================================================
// Last modified on 6/15/03
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "datastructures/linkedlist.h"
#include "quantum/quantumfuncs.h"

using std::string;
using std::ostream;
using std::endl;

namespace quantumfunc
{

/*
// Method save_energy_value stores the input energy into the input
// energy linkedlist Elist:

   void save_energy_value(double t,double E,linkedlist& Elist)
      {
         Elist.append_node(t,E);

         const int n_indep_vars=1;
         int posn;
         double var[n_indep_vars];
   
         double orderfunc=t;
         if (!Elist.ordered_node_existence(orderfunc,posn))
         {
            var[0]=orderfunc;
            Elist.insert_node(posn,orderfunc,n_indep_vars,var,E);
         }
      }
*/

} // quantumfunc namespace

