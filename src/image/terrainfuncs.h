// =========================================================================
// Header file for stand-alone terrain analysis functions
// =========================================================================
// Last modified on 12/1/10; 12/2/10; 12/5/10; 5/23/11
// =========================================================================

#ifndef TERRAINFUNCS_H
#define TERRAINFUNCS_H

#include <set>
#include <vector>
#include "math/threevector.h"

class graph;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace terrainfunc
{
   int px_py_to_node_ID(int mdim,int px,int py);
   std::pair<int,int> node_ID_to_px_py(int mdim,int node_ID);

   std::vector<threevector> compute_shortest_path(
      node* destination_node_ptr,const twoDarray* ztwoDarray_ptr);
   graph* generate_Dijkstra_DTED_graph(
      const twoDarray* ztwoDarray_ptr,double alpha,int skip);

}

#endif // terrainfuncs.h




