// ==========================================================================
// Header file for ltfourvector structure
// ==========================================================================
// Last modified on 3/5/12
// ==========================================================================

#ifndef LTFOURVECTOR_H
#define LTFOURVECTOR_H

#include "math/fourvector.h"

// Structure ltfourvector returns true if fourvector is V1 "less
// than" fourvector V2.  The following algorithm is easily understood
// if we regard V1 and V2 as 4-digit numbers...

struct ltfourvector
{
      bool operator()(const fourvector& V1,const fourvector& V2) const
      {
         const double TINY=1E-9;

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
               else if (V1.get(2) > V2.get(2)+TINY)
               {
                  return false;
               }
               else
               {
                  if (V1.get(3) < V2.get(3)-TINY)
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
      }
};
      
# endif // ltfourvector.h
