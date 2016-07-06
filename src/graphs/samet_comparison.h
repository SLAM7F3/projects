// ==========================================================================
// Header file for samet_comparison structure
// ==========================================================================
// Last modified on 3/19/12; 4/5/14
// ==========================================================================

#ifndef SAMET_COMPARISON_H
#define SAMET_COMPARISON_H

#include <iostream>
#include "math/threevector.h"

// Structure samet_comparison returns true if distance contained
// within zeroth component of V1 is greater than distance of V2.  If
// distances are equal, then samet_comparison returns true if node
// type contained within first component of V1 is greater than node
// type of V2.

// This structure was created for use in Priority Queues used by
// H. Samet's Incremental Nearest Neighbor Algorithm.

struct samet_comparison
{
   bool operator()(const threevector& V1,const threevector& V2) const
   {
//         std::cout << "V1 = " << V1 << " V2 = " << V2 << std::endl;
//         std::cout << "V1.get(0) = " << V1.get(0)
//                   << " V2.get(0) = " << V2.get(0) << std::endl;

      if (nearly_equal(V1.get(0),V2.get(0)))
      {
//            std::cout << "V1 and V2 have same first digit" << std::endl;
//            std::cout << "V1.get(1) = " << V1.get(1)
//                      << " V2.get(1) = " << V2.get(1) << std::endl;
            
         if (V1.get(1) > V2.get(1))
         {
//               std::cout << "returning TRUE" << std::endl;
            return true;

         }
         else 
         {
//               std::cout << "returning FALSE" << std::endl;
            return false;
         }
      }
      else if (V1.get(0) > V2.get(0))
      {
//            std::cout << "returning TRUE" << std::endl;
         return true;
      }
      else // if (V1.get(0) < V2.get(0))
      {
//            std::cout << "returning FALSE" << std::endl;
         return false;
      }

   }
};
      
# endif // samet_comparison.h
