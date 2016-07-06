// ==========================================================================
// Header file for ltduple structure
// ==========================================================================
// Last modified on 8/17/12
// ==========================================================================

#ifndef LTDUPLE_H
#define LTDUPLE_H

typedef std::pair<int,int> DUPLE;

// Structure ltduple returns true if DUPLE is D1 "less than" DUPLE D2.
// The following algorithm is easily understood if we regard D1 and D2
// as 2-digit numbers...

struct ltduple
{
      bool operator()(const DUPLE& D1,const DUPLE& D2) const
      {
         if (D1.first < D2.first)
         {
            return true;
         }
         else if (D1.first > D2.first)
         {
            return false;
         }
         else
         {
            if (D1.second < D2.second)
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

         
# endif // ltduple.h
