// ==========================================================================
// Header file for gttwovector structure
// ==========================================================================
// Last modified on 3/19/12
// ==========================================================================

#ifndef GTTWOVECTOR_H
#define GTTWOVECTOR_H

#include "math/twovector.h"

// Structure gttwovector returns true if twovector is V1 "greater
// than" twovector V2.  The following algorithm is easily understood
// if we regard V1 and V2 as 2-digit numbers...

struct gttwovector
{
      bool operator()(const twovector& V1,const twovector& V2) const
      {
         const double TINY=1E-9;

         if (V1.get(0) > V2.get(0)-TINY)
         {
            return true;
         }
         else if (V1.get(0) < V2.get(0)+TINY)
         {
            return false;
         }
         else
         {
            if (V1.get(1) > V2.get(1)-TINY)
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
      
# endif // gttwovector.h
