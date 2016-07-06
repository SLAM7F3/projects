// ==========================================================================
// STATEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 3/22/03
// ==========================================================================

#include "statefuncs.h"

using std::string;
using std::ostream;
using std::endl;

namespace statefunc
{
   string get_state_str(const State_type state_type)
      {
         string statestr;
         
         if (state_type==(0))
         {
            statestr="(0)";
         }
         else if (state_type==(1))
         {
            statestr="(1)";
         }
         return statestr;
      }
} // statefunc namespace

