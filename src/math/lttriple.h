// ==========================================================================
// Header file for lttriple structure
// ==========================================================================
// Last modified on 9/1/12
// ==========================================================================

#ifndef LTTRIPLE_H
#define LTTRIPLE_H

#include "datastructures/Triple.h"
typedef Triple<int,int,int> triple;

// Structure lttriple returns true if triple is T1 "less
// than" triple T2.  The following algorithm is easily understood
// if we regard T1 and T2 as 3-digit numbers...

struct lttriple
{
      bool operator()(const triple& T1,const triple& T2) const
      {
         if (T1.first < T2.first)
         {
            return true;
         }
         else if (T1.first > T2.first)
         {
            return false;
         }
         else
         {
            if (T1.second < T2.second)
            {
               return true;
            }
            else if (T1.second > T2.second)
            {
               return false;
            }
            else
            {
               if (T1.third < T2.third)
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
      
# endif // lttriple.h
