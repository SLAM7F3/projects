// ==========================================================================
// Header file for ltquadruple structure
// ==========================================================================
// Last modified on 8/3/12
// ==========================================================================

#ifndef LTQUADRUPLE_H
#define LTQUADRUPLE_H

#include "datastructures/Quadruple.h"
typedef Quadruple<int,int,int,int> quadruple;

// Structure ltquadruple returns true if quadruple is Q1 "less
// than" quadruple Q2.  The following algorithm is easily understood
// if we regard Q1 and Q2 as 4-digit numbers...

struct ltquadruple
{
      bool operator()(const quadruple& Q1,const quadruple& Q2) const
      {
         if (Q1.first < Q2.first)
         {
            return true;
         }
         else if (Q1.first > Q2.first)
         {
            return false;
         }
         else
         {
            if (Q1.second < Q2.second)
            {
               return true;
            }
            else if (Q1.second > Q2.second)
            {
               return false;
            }
            else
            {
               if (Q1.third < Q2.third)
               {
                  return true;
               }
               else if (Q1.third > Q2.third)
               {
                  return false;
               }
               else
               {
                  if (Q1.fourth < Q2.fourth)
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
      
# endif // ltquadruple.h
