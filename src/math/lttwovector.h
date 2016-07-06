// ==========================================================================
// Header file for lttwovector structure
// ==========================================================================
// Last modified on 9/4/07; 2/1/08; 7/6/08
// ==========================================================================

#ifndef LTTWOVECTOR_H
#define LTTWOVECTOR_H

#include "math/twovector.h"

// Structure lttwovector returns true if twovector is V1 "less
// than" twovector V2.  The following algorithm is easily understood
// if we regard V1 and V2 as 2-digit numbers...

struct lttwovector
{
      bool operator()(const twovector& V1,const twovector& V2) const
      {
         const double TINY=1E-2;
//         const double TINY=1E-6;
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
            else 
            {
               return false;
            }
         }
      }
};
      
# endif // lttwovector.h
