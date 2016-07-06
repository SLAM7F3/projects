// ==========================================================================
// Header file for ltstring structure
// ==========================================================================
// Last modified on 6/26/07
// ==========================================================================

#ifndef LTSTRING_H
#define LTSTRING_H

// Structure ltstring returns true if string S1 is less than string S2.

struct ltstring
{
      bool operator()(const std::string& S1,const std::string& S2) const
      {
         return (S1 < S2);
      }
};
      
# endif // ltstring.h
