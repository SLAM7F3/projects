// ==========================================================================
// Header file for ltonevector structure
// ==========================================================================
// Last modified on 7/6/08
// ==========================================================================

#ifndef LTONEVECTOR_H
#define LTONEVECTOR_H

struct ltonevector
{
      bool operator()(double V1,double V2) const
      {
         const double TINY=1E-9;      
         return (V1 < V2-TINY);
      }
};
      
# endif // ltonevector.h
