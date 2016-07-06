// ==========================================================================
// Header file for urban object network construction and manipulation
// methods
// ==========================================================================
// Last modified on 4/17/05; 6/14/06; 8/3/06
// ==========================================================================

#ifndef NETWORKFUNCS_H
#define NETWORKFUNCS_H

#include <string>
#include <vector>
#include "datastructures/datapoint.h"
#include "network/Network.h"
#include "math/threevector.h"

template <class T> class Hashtable;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
template <class T> class Network;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace Networkfunc
{
   template <class T> Network<T*>* generate_network(
      twoDarray const *ztwoDarray_ptr,
      Hashtable<linkedlist*>* connected_pixellist_hashtable_ptr);

   template <class T> void delete_dynamically_allocated_objects_in_network(
      const Network<T*>* network_ptr);

   template <class T> std::vector<threevector> generate_site_posn_array(
      Network<T*>* network_ptr);
   template <class T> void output_site_posns(
      std::string filenamestr,Network<T*>* network_ptr);
   template <class T> void initialize_site_neighbors(
      int number_of_delaunay_triangles,int* delaunay_triangle_vertex,
      Network<T*>* network_ptr);
}

#include "Networkfuncs.cc"

#endif // Networkfuncs.h



