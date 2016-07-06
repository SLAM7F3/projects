// ==========================================================================
// Header file for DT_node class
// ==========================================================================
// Last modified on 12/14/05; 7/12/06
// ==========================================================================

#ifndef DT_NODE_H
#define DT_NODE_H

#include <iostream>
#include "delaunay/DT_flag.h"
#include "network/Network.h"

class Delaunay_point;
class DT_list;
class threevector;

class DT_node
{

   typedef unsigned char Index;	// used for flag and Index in array

   public :

      DT_node();		// initialize the root
      DT_node(DT_node*, Index);	// initialize nowhere
      DT_node(DT_node*, Delaunay_point*, Index);
   // father, creator, direction of stepfather

      Index conflict(Delaunay_point *); 
   // true if the Delaunay_point is inside the (closed) circumdisk

      DT_node* find_conflict(Delaunay_point* p);
   // return an alive node in conflict
      
      void output(Network<threevector*>* vertex_network_ptr);

      // the first vertex is the creator, that is finite
      // except for the root and its neighbors

      Index cw_neighbor_Index(Delaunay_point *p)
         { return ((p==vertices[0]) ? 2 : ((p==vertices[1]) ? 0 : 1)); }
      Index neighbor_Index(DT_node *n)
         { return ( (neighbors[0]==n)?0:((neighbors[1]==n)?1:2) );}

      DT_flag		flag;
      unsigned int	nb;
      Delaunay_point*   vertices[3];
      DT_node*          neighbors[3];
      DT_list*		sons;

   private :

};

#endif  // DT_NODE
