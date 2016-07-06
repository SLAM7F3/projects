// ==========================================================================
// Polygon triangulation algorithms converted from "Computational
// Geometry in C"

// As of 2012, this entire namespace is DEPRECATED !!!

// ==========================================================================
// Last updated on 6/24/04; 8/5/06
// ==========================================================================

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <vector>
#include "math/threevector.h"
class polygon;

namespace triangulate_func
{

   typedef  double tPointi[2];   		// Type double point 
   typedef  struct tVertexStructure tsVertex;   // Used only in NEW().

   struct tVertexStructure {
         int		vnum;		// Index 
         tPointi	v;		// Coordinates 
         bool 		ear;		// true iff an ear 
         tsVertex* 	next;
         tsVertex* 	prev;
   };

// Function declarations:

   double AreaPoly2( void );
   Linkedlist<polygon*>* Triangulate( void );
   void	EarInit( void );
   bool	Diagonal( tsVertex* a, tsVertex* b );
   bool	Diagonalie( tsVertex* a, tsVertex* b );
   bool	InCone( tsVertex* a, tsVertex* b );

   double Area2( tPointi a, tPointi b, tPointi c );
   int AreaSign( tPointi a, tPointi b, tPointi c );
   bool	Xor( bool x, bool y );
   bool	Left( tPointi a, tPointi b, tPointi c );
   bool	LeftOn( tPointi a, tPointi b, tPointi c );
   bool	Collinear( tPointi a, tPointi b, tPointi c );
   bool	Between( tPointi a, tPointi b, tPointi c );
   bool	Intersect( tPointi a, tPointi b, tPointi c, tPointi d );
   bool	IntersectProp( tPointi a, tPointi b, tPointi c, tPointi d );

   void delete_circular_linked_list();
   tsVertex* generate_and_insert_vertex();
   void read_polygon_vertices(const std::vector<threevector>& site);
   void	PrintDiagonal( tsVertex* a, tsVertex* b );
   void	print_triangle_vertex_labels(tsVertex* a,tsVertex* b,tsVertex* c);
   void show_vertex(tsVertex const *v_ptr);
   polygon* generate_triangle(tsVertex* a,tsVertex* b,tsVertex* c);
   void delete_triangle_list(Linkedlist<polygon*>* triangle_list_ptr);
   void	PrintPoly( void );

} // triangulate_func namespace

#endif  // triangulate_funcs.h
