// ==========================================================================
// Header file for ltmatrix structure
// ==========================================================================
// Last modified on 8/24/15; 8/25/15
// ==========================================================================

#ifndef LTMATRIX_H
#define LTMATRIX_H

#include <iostream>
#include "math/genmatrix.h"

// Structure ltmatrix returns true if matrix is M1 "less
// than" matrix M2.  The following algorithm is easily understood
// if we regard M1 and M2 as mdim*ndim digit numbers...

struct ltmatrix
{
      bool operator()(const genmatrix& M1,const genmatrix& M2) const
      {
         const double TINY=1E-7;
         unsigned int mdim = M1.get_mdim();
         unsigned int ndim = M1.get_ndim();

         if((M2.get_mdim() != mdim) || (M2.get_ndim() != ndim))
         {
            std::cout << "Error in operator() in ltmatrix.h! " << std::endl;
            std::cout << "M1.mdim = " << mdim << " M2.mdim = " << M2.get_mdim()
                      << std::endl;
            std::cout << "M1.ndim = " << ndim << " M2.ndim = " << M2.get_ndim()
                      << std::endl;
            exit(-1);
         }
         
	 for(unsigned int r = 0; r < mdim; r++)
         {
            for(unsigned int c = 0; c < ndim; c++)
            {
               if(M1.get(r,c) < M2.get(r,c) - TINY)
               {
                  return true;
               }
               else if (M1.get(r,c) > M2.get(r,c) + TINY)
               {
                  return false;
               }
            } // loop over index c labeling matrix columns
	 } // loop over index r labeling matrix rows

	 if(M1.get(mdim - 1, ndim - 1) < M2.get(mdim - 1, ndim - 1))
	 {
            return true;
	 }
	 else
         {
            return false;
	 }
      }
};
      
# endif // ltmatrix.h
