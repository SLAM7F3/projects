// ==========================================================================
// Header file for ltthreevector structure
// ==========================================================================
// Last modified on 1/6/07; 5/12/08; 7/6/08; 9/25/09
// ==========================================================================

#ifndef LTTHREEVECTOR_H
#define LTTHREEVECTOR_H

#include "math/threevector.h"

// Structure ltthreevector returns true if threevector is V1 "less
// than" threevector V2.  The following algorithm is easily understood
// if we regard V1 and V2 as 3-digit numbers...

struct ltthreevector
{
      bool operator()(const threevector& V1,const threevector& V2) const
      {
         const double TINY=1E-5;
//         const double TINY=1E-9;

         if (V1.get(0) < V2.get(0)-TINY)
         {
            return true;
         }
         else if (V1.get(0) > V2.get(0)+TINY)
         {
            return false;
         }
         else
         {
            if (V1.get(1) < V2.get(1)-TINY)
            {
               return true;
            }
            else if (V1.get(1) > V2.get(1)+TINY)
            {
               return false;
            }
            else
            {
               if (V1.get(2) < V2.get(2)-TINY)
               {
                  return true;
               }
               else
               {
                  return false;
               }
            }
         }
      }
};
      
# endif // ltthreevector.h
