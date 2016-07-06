// ==========================================================================
// Header file for Delaunay_tree class
// ==========================================================================
// Last modified on 12/15/05; 7/12/06
// ==========================================================================

#ifndef DELAUNAY_TREE_H
#define DELAUNAY_TREE_H

/***************************************************************************+
+                                                                           +
+  Delaunay-tree                                                            +
+                                                                           +
+  Copyright (c) 1993  by  INRIA Prisme Project                             +
+  2004 route des Lucioles BP 93 06902 Sophia Antipolis Cedex               +
+  All rights reserved.                                                     +
+                                                                           +
****************************************************************************/

/**************************************************************************+

+ GEOMETRIC OBJECT :                                                       +
+ The Delaunay tree is a randomized geometric data structure computing the  +
+ Delaunay triangulation                                                   +
+ This structure holds insertion and queries. Deletions are not supported  +
+ in this version                                                          +
+                                                                          +

+ INTERNAL IMPLEMENTATION :                                                +
+ See :                                                                    +
+ J.-D. Boissonnat and M. Teillaud. On the randomized construction of the  +
+ Delaunay tree. Theoret. Comput. Sci. 112:339--354, 1993.                 +
+ O. Devillers, S. Meiser and M. Teillaud. Fully dynamic Delaunay          +
+ triangulation in logarithmic expected time per operation. Comput. Geom.  +
+ Theory Appl. 2(2):55-80, 1992.                                           +
+ O. Devillers. Robust and efficient implementation of the Delaunay tree.  +
+ INRIA Research Report 1619, 1992.                                        +
+**************************************************************************/

#include <vector>
#include "geometry/linesegment.h"
#include "network/Network.h"
#include "datastructures/Triple.h"
#include "math/threevector.h"

class Delaunay_point;
class DT_node;

class Delaunay_tree
{

   typedef unsigned char Index;	// used for flag and Index in array

   public :

      Delaunay_tree(std::vector<threevector>* UV_ptr);
      Delaunay_tree(const std::vector<int>& ID,
                    std::vector<threevector>* UV_ptr);
      ~Delaunay_tree();                      

      void build_vertex_network();
      Delaunay_tree& operator+=(Delaunay_point*);      // insertion
      std::vector<Triple<int,int,int> > compute_triangles();

   private :

      int n_vertices;
      int nb; // number of current operation
      std::vector<int> vertex_ID;
      DT_node* root; // root of delaunay_tree

      std::vector<threevector>* vertices_ptr;
      Network<threevector*>* vertex_network_ptr;
      
      void allocate_member_objects();
      void initialize_member_objects();

      double triangle_area(
         const threevector& v1,const threevector& v2,const threevector& v3);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Member function triangle_area computes the area of the triangle
// defined by input vertices v1, v2 and v3 projected into the XY plane:

inline double Delaunay_tree::triangle_area(
   const threevector& v1,const threevector& v2,const threevector& v3)
{
   return 0.5*(
      (v2-v1).get(0)*(v3-v1).get(1)-(v2-v1).get(1)*(v3-v1).get(0));
}

#endif  // DELAUNAY_TREE
