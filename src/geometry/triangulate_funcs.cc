// ==========================================================================
// Polygon triangulation 
// ==========================================================================
// Last updated on 8/20/04; 4/13/06; 8/5/06
// ==========================================================================

// This code is described in "Computational Geometry in C" (Second
// Edition), Chapter 1.  It is not written to be comprehensible
// without the explanation in that book.

// Input: 2n integer coordinates for vertices of a simple polygon,
//       in counterclockwise order.  NB: the code will not work
//       for points in clockwise order!
// Output: the diagonals of a triangulation, in PostScript.

// Compile: gcc -o tri tri.c (or simply: make)

// Written by Joseph O'Rourke, with contributions by Min Xu.
// Last modified: October 1997
// Questions to orourke@cs.smith.edu.
// --------------------------------------------------------------------
// This code is Copyright 1998 by Joseph O'Rourke.  It may be freely 
// redistributed in its entirety provided that this copyright notice is 
// not removed.
// --------------------------------------------------------------------

#include <iostream>
#include <math.h>
#include <stdio.h>
#include "datastructures/Linkedlist.h"
#include "geometry/polygon.h"
#include "geometry/triangulate_funcs.h"

using std::cout;
using std::endl;
using std::vector;

namespace triangulate_func
{
   
// Namespace constants

   const int X=0;
   const int Y=1;

// Global variable definitions 
   tsVertex*	vertices  = NULL;	// "Head" of circular list
   tsVertex*     next_to_final_vertex=NULL;       
   tsVertex*     final_vertex=NULL;       // "Tail" of circular list
   unsigned int	nvertices = 0;		  // Total number of polygon vertices. 

// ---------------------------------------------------------------------
//  Returns twice the signed area of the triangle determined by a,b,c.
//  The area is positive if a,b,c are oriented ccw, negative if cw,
//  and zero if the points are collinear.
// ---------------------------------------------------------------------

   double Area2( tPointi a, tPointi b, tPointi c )
   {
      return
         (b[X] - a[X]) * (c[Y] - a[Y]) -
         (c[X] - a[X]) * (b[Y] - a[Y]);
   }

// ---------------------------------------------------------------------
// Exclusive or: true iff exactly one argument is true.
// ---------------------------------------------------------------------

   bool	Xor( bool x, bool y )
   {
      /* The arguments are negated to ensure that they are 0/1 values. */
      /* (Idea due to Michael Baldwin.) */
      return   !x ^ !y;
   }

// ---------------------------------------------------------------------
// Returns true iff ab properly intersects cd: they share a point
// interior to both segments.  The properness of the intersection is
// ensured by using strict leftness.
// --------------------------------------------------------------------

   bool	IntersectProp( tPointi a, tPointi b, tPointi c, tPointi d )
   {
      /* Eliminate improper cases. */
      if (
         Collinear(a,b,c) ||
         Collinear(a,b,d) ||
         Collinear(c,d,a) ||
         Collinear(c,d,b)
         )
         return false;

      return
         Xor( Left(a,b,c), Left(a,b,d) )
         && Xor( Left(c,d,a), Left(c,d,b) );
   }

// ---------------------------------------------------------------------
//  Returns true iff c is strictly to the left of the directed
//  line through a to b.
//  --------------------------------------------------------------------

   bool	Left( tPointi a, tPointi b, tPointi c )
   { 
      return  AreaSign( a, b, c ) > 0;
   }

   bool	LeftOn( tPointi a, tPointi b, tPointi c )
   {
      return  AreaSign( a, b, c ) >= 0;
   }

   bool	Collinear( tPointi a, tPointi b, tPointi c )
   {
      return  AreaSign( a, b, c ) == 0;
   }

// ---------------------------------------------------------------------
//  Returns true iff point c lies on the closed segement ab.
//  First checks that c is collinear with a and b.
//  --------------------------------------------------------------------

   bool	Between( tPointi a, tPointi b, tPointi c )
   {
      if ( ! Collinear( a, b, c ) )
         return  false;

      /* If ab not vertical, check betweenness on x; else on y. */
      if ( a[X] != b[X] ) 
         return ((a[X] <= c[X]) && (c[X] <= b[X])) ||
            ((a[X] >= c[X]) && (c[X] >= b[X]));
      else
         return ((a[Y] <= c[Y]) && (c[Y] <= b[Y])) ||
            ((a[Y] >= c[Y]) && (c[Y] >= b[Y]));
   }

// ---------------------------------------------------------------------
// Returns true iff segments ab and cd intersect, properly or
// improperly.
// ---------------------------------------------------------------------
  
   bool	Intersect( tPointi a, tPointi b, tPointi c, tPointi d )
   {
      if      ( IntersectProp( a, b, c, d ) )
         return  true;
      else if (   Between( a, b, c )
                  || Between( a, b, d )
                  || Between( c, d, a )
                  || Between( c, d, b )
         )
         return  true;
      else    return  false;
   }

// ---------------------------------------------------------------------
//  Returns true iff (a,b) is a proper internal *or* external
//  diagonal of P, *ignoring edges incident to a and b*.
//  --------------------------------------------------------------------

   bool Diagonalie( tsVertex* a, tsVertex* b )
   {
      tsVertex *c,*c1;

      /* For each edge (c,c1) of P */
      c = vertices;
      do {
         c1 = c->next;
         /* Skip edges incident to a or b */
         if (    ( c != a ) && ( c1 != a )
                 && ( c != b ) && ( c1 != b )
                 && Intersect( a->v, b->v, c->v, c1->v )
            )
            return false;
         c = c->next;
      } while ( c != vertices );
      return true;
   }

// ---------------------------------------------------------------------
//  This function initializes the data structures
//  --------------------------------------------------------------------

   void   EarInit( void )
   {
      tsVertex *v0, *v1, *v2;   /* three consecutive vertices */

      /* Initialize v1->ear for all vertices. */
      v1 = vertices;
      do {
         v2 = v1->next;
         v0 = v1->prev;
         v1->ear = Diagonal( v0, v2 );
         v1 = v1->next;
      } while ( v1 != vertices );
   }

// --------------------------------------------------------------------
// Method Triangulate returns a linked list of n-2 dynamically
// generated triangles which cover the polygon read in via method
// read_polygon_vertices().
// --------------------------------------------------------------------

   Linkedlist<polygon*>* Triangulate()
   {
      Linkedlist<polygon*>* triangle_list_ptr=new Linkedlist<polygon*>;

      tsVertex *v0, *v1, *v2, *v3, *v4;	// 5 consecutive vertices 
      tsVertex *v1_last,*v2_last,*v3_last;
      int   n = nvertices;		// number of vertices; shrinks to 3.

      EarInit();

      if (n==3)
      {
         v2 = vertices;
         v3 = v2->next; 
         v1 = v2->prev; 
         polygon* triangle_ptr=generate_triangle(v1,v2,v3);
         triangle_list_ptr->append_node(triangle_ptr);
      }
      else
      {
         /* Each step of outer loop removes one ear. */
         while ( n > 3 ) 
         {     
            /* Inner loop searches for an ear. */
            v2 = vertices;
               
            bool earfound = false; // for debugging & error detection only
            do {
               if (v2->ear) 
               {
                  earfound = true;
                  /* Ear found. Fill variables. */
                  v3 = v2->next; v4 = v3->next;
                  v1 = v2->prev; v0 = v1->prev;
                  v1_last=v1;
                  v2_last=v2;
                  v3_last=v3;

                  /* (v1,v3) is a diagonal */
//                     print_triangle_vertex_labels(v2,v1,v3);
                  polygon* triangle_ptr=generate_triangle(v2,v1,v3);
                  triangle_list_ptr->append_node(triangle_ptr);

                  /* Update earity of diagonal endpoints */
                  v1->ear = Diagonal( v0, v3 );
                  v3->ear = Diagonal( v1, v4 );
            
                  /* Cut off the ear v2 */
                  v1->next = v3;
                  v3->prev = v1;
                  vertices = v3;	/* In case the head was v2. */
                  n--;
                  break;   /* out of inner loop; resume outer loop */
               } /* end if ear found */
               v2 = v2->next;
            } while ( v2 != vertices );

            if ( !earfound ) 
            {
               cout << "Error in triangulate_func::Triangulate()" << endl;
               cout << "No ear found." << endl;
//                  PrintPoly();
               cout << "triangle_list_ptr->size() = "
                    << triangle_list_ptr->size() << endl;
               cout << "Returning partial triangle list..." << endl;
//                  outputfunc::enter_continue_char();
               return triangle_list_ptr;
//                  exit(1);
            }
         } // while n > 3 loop

// Generate final triangle:

//            cout << "Generating final triangle:" << endl;
//            cout << "v1_last = " << endl;
//            show_vertex(v1_last);
//            cout << "v3_last = " << endl;
//            show_vertex(v3_last);
//            cout << "next_to_final_vertex = " << endl;
//            show_vertex(next_to_final_vertex);

         polygon* triangle_ptr=generate_triangle(
            next_to_final_vertex,v1_last,v3_last);
         triangle_list_ptr->append_node(triangle_ptr);
      } // n==3 conditional

/*
  cout << "inside triangulate_func::Triangulate()" << endl;
  cout << "triangle_list_ptr->n_nodes = " << triangle_list_ptr->
  size() << endl;
  for (Mynode<polygon*>* currnode_ptr=triangle_list_ptr->
  get_start_ptr();
  currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
  {
  polygon* curr_tri_ptr=currnode_ptr->get_data();
  cout << "curr_tri_ptr = " << curr_tri_ptr << endl;
  cout << "*curr_tri_ptr = " << *curr_tri_ptr << endl;
  }
*/
      return triangle_list_ptr;
   }

// ---------------------------------------------------------------------
// Method delete_triangle_list explicitly deletes the triangles within
// each node of the linked list *triangle_list_ptr which were
// dynamically generated by method generate_triangle().  It
// subsequently deletes *triangle_list_ptr.

   void delete_triangle_list(Linkedlist<polygon*>* triangle_list_ptr)
   {
      for (Mynode<polygon*>* currnode_ptr=triangle_list_ptr->
              get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         delete currnode_ptr->get_data();
      }
      delete triangle_list_ptr;
      delete_circular_linked_list();
   }
 
// ---------------------------------------------------------------------
//  Returns true iff the diagonal (a,b) is strictly internal to the 
//  polygon in the neighborhood of the a endpoint.  
//  --------------------------------------------------------------------

   bool   InCone( tsVertex* a, tsVertex* b )
   {
      tsVertex *a0,*a1;	// a0,a,a1 are consecutive vertices

      a1 = a->next;
      a0 = a->prev;

      /* If a is a convex vertex ... */
      if( LeftOn( a->v, a1->v, a0->v ) )
         return    Left( a->v, b->v, a0->v )
            && Left( b->v, a->v, a1->v );

      /* Else a is reflex: */
      return !(    LeftOn( a->v, b->v, a1->v )
                   && LeftOn( b->v, a->v, a0->v ) );
   }

// ---------------------------------------------------------------------
// Returns true iff (a,b) is a proper internal diagonal.
// --------------------------------------------------------------------

   bool	Diagonal( tsVertex* a, tsVertex* b )
   {
      return InCone( a, b ) && InCone( b, a ) && Diagonalie( a, b );
   }

// ---------------------------------------------------------------------
   void delete_circular_linked_list()
   {
      if (vertices != NULL)
      {
         tsVertex* v_ptr=vertices->next;
         while (v_ptr != vertices)
         {
            tsVertex* next_v_ptr=v_ptr->next;
            delete v_ptr;
            v_ptr=next_v_ptr;
         }
         delete vertices;

         vertices=NULL;
      }
   }

// ---------------------------------------------------------------------
   tsVertex* generate_and_insert_vertex()
   {
      tsVertex* v_ptr=new tsVertex;

// Add v_ptr to circularly linked list behind "vertices" position:

      if (vertices != NULL)
      {
         v_ptr->next=vertices;
         v_ptr->prev=vertices->prev;
         vertices->prev=v_ptr;
         v_ptr->prev->next=v_ptr;
      }
      else
      {
         vertices=v_ptr;
         vertices->next=vertices->prev=v_ptr;
      }
         
      return v_ptr;
   }

// ---------------------------------------------------------------------
// Method read_polygon_vertices takes in an array of vertices and
// links them into a circular list.
// --------------------------------------------------------------------

   void read_polygon_vertices(const vector<threevector>& site)
   {
      delete_circular_linked_list();

      nvertices = site.size();
      for (unsigned int i=0; i<nvertices; i++)
      {
         double x=site[i].get(0);
         double y=site[i].get(1);

         tsVertex* v=generate_and_insert_vertex();
         v->v[X] = x;
         v->v[Y] = y;
         v->vnum = i;

// Save copy of final vertex pointer into final_vertex global variable:

         if (i==nvertices-2)
         {
            next_to_final_vertex=v;
         }
         else if (i==nvertices-1)
         {
            final_vertex=v;
         }
      }

      if (nvertices < 3) 
      {
         cout << "Error in triangulate_funcs::read_polygon_vertices()"
              << endl;
         cout << "nvertices = " << nvertices << endl;
      }
   }


// ---------------------------------------------------------------------
//  For debugging; not currently invoked.
// --------------------------------------------------------------------

   void   PrintPoly( void )
   {
      tsVertex*  v_ptr;
      cout << "Polygon circular list:" << endl;
      v_ptr = vertices;
      do 
      {                                 
         cout << "Vnum = " << v_ptr->vnum
              << " X = " << v_ptr->v[X] << " Y = " << v_ptr->v[Y]
              << " ear = " << v_ptr->ear << endl;
         v_ptr = v_ptr->next;
      } 
      while ( v_ptr != vertices );
   }

   void	PrintDiagonal( tsVertex* a, tsVertex* b )
   {
      printf("%%Diagonal: (%d,%d)\n", a->vnum, b->vnum );
      printf("%d\t%d\tmoveto\n", int(a->v[X]), int(a->v[Y]) );
      printf("%d\t%d\tlineto\n", int(b->v[X]), int(b->v[Y]) );
   }

   void	print_triangle_vertex_labels(tsVertex* a,tsVertex* b,tsVertex* c)
   {
      cout << "Triangle vertex labels: " << endl;
      cout << " v2  = " << a->vnum
           << " v1 = " << b->vnum
           << " v3 = " << c->vnum << endl;
   }

   void show_vertex(tsVertex const *v_ptr)
   {
      cout << "Vertex: X = " << v_ptr->v[X] 
           << " Y = " << v_ptr->v[Y] << endl;
   }

// Method generate_triangle returns a dynamically generated polygon
// containing (x,y) vertices for the input tsVertices.

   polygon* generate_triangle(tsVertex* a,tsVertex* b,tsVertex* c)
   {
      vector<threevector> vertex(3);
      vertex[0]=threevector(a->v[X],a->v[Y]);
      vertex[1]=threevector(b->v[X],b->v[Y]);
      vertex[2]=threevector(c->v[X],c->v[Y]);
//         cout << "V0 = " << vertex[0] << endl;
//         cout << "V1 = " << vertex[1] << endl;
//         cout << "V2 = " << vertex[2] << endl;
      polygon* triangle_ptr=new polygon(vertex);
      return triangle_ptr;
   }

   double AreaPoly2( void )
   {
      int     sum = 0;
      tsVertex *p, *a;

      p = vertices;   /* Fixed. */
      a = p->next;    /* Moving. */
      do {
         sum += Area2( p->v, a->v, a->next->v );
         a = a->next;
      } while ( a->next != vertices );
      return sum;

   }

   int AreaSign( tPointi a, tPointi b, tPointi c )
   {
      double area2 = ( b[0] - a[0] ) * (double)( c[1] - a[1] ) -
         ( c[0] - a[0] ) * (double)( b[1] - a[1] );

      /* The area should be an integer. */
//         if      ( area2 >  0.5 ) return  1;
//         else if ( area2 < -0.5 ) return -1;
//         else                     return  0;

      if      ( area2 >  0.00000005 )  return  1;
      else if ( area2 < -0.00000005 )  return -1;
      else                    return  0;

   }

} // triangulation_func namespace
 
