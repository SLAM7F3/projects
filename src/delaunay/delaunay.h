// As of November 2005, this namespace is deprecated and should no
// longer be used!  The following "Computational Geometry in C" code
// is definitely buggy and yields wrong results!

// ==========================================================================
// 3D convex hull algorithm converted from "Computational Geometry in C"
// ==========================================================================
// Last updated on 11/15/05; 8/6/06
// ==========================================================================

#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <vector>
#include "math/threevector.h"

namespace delaunay
{

// Define structures for vertices, edges and faces:

   typedef struct tVertexStructure tsVertex;
   typedef struct tEdgeStructure tsEdge;
   typedef struct tFaceStructure tsFace;

// VoronoiStructure stores information for Voronoi polygon vertices.
// Each Voronoi vertex is presumed to lie at the intersection of
// precisely 3 Voronoi polygon regions.  Integer labels for these 3
// Voronoi regions are saved within integer array label[3].  The
// 2-space position of the Voronoi vertex is saved within the posn
// field of VoronoiStructure:

   struct VoronoiStructure {
         int      label[3];	
         threevector posn;
   };

   struct tVertexStructure {
         double   v[3];
         int	  vnum;
         tsEdge*  duplicate;  // pointer to incident cone edge (or NULL) 
         bool     onhull;	   // True iff point on hull. 
         bool	  mark;  // True iff point already processed.
         tsVertex*  next;
         tsVertex*  prev;
   };

   struct tEdgeStructure {
         tsFace*    adjface[2];
         tsVertex*  endpts[2];
         tsFace*    newface;    	// pointer to incident cone face. 
         bool       delete_edge;	// T iff edge should be deleted. 
         tsEdge*    next;
         tsEdge*    prev;
   };

   struct tFaceStructure {
         tsEdge*    edge[3];
         tsVertex*  vertex[3];
         bool	    visible;	// T iff face visible from new point.
	 bool       lower;      // T iff on the lower hull 
         tsFace*    next;
         tsFace*    prev;
   };

// Namespace variable declarations:
   
   extern bool debug,check;
   
// Function declarations:

   void null_circular_lists();
   tsVertex* MakeNullVertex(void);
   void    read_delaunay_vertices(void);
   void    Print( void );
   void    SubVec( int a[3], int b[3], int c[3]);
   void    DoubleTriangle( void );
   void    ConstructHull( void );
   bool	   AddOne( tsVertex* p );
   int     VolumeSign(tsFace* f, tsVertex* p);
   int 	   Volumei( tsFace* f, tsVertex* p );
   tsFace* MakeConeFace( tsEdge* e, tsVertex* p );
   void    MakeCcw( tsFace* f, tsEdge* e, tsVertex* p );
   tsEdge* MakeNullEdge( void );
   tsFace* MakeNullFace( void );
   tsFace* MakeFace( tsVertex* v0, tsVertex* v1, tsVertex* v2, tsFace* f );
   void    CleanUp( tsVertex* *pvnext );
   void    CleanEdges( void );
   void    CleanFaces( void );
   void    CleanVertices( tsVertex* *pvnext );
   void	   delete_circular_lists(void);
   void	   delete_all_edges(void);
   void	   delete_all_faces(void);
   void	   delete_all_vertices(void);

   bool	   Collinear( tsVertex* a, tsVertex* b, tsVertex* c );
   void    CheckEuler(int V, int E, int F );
   void	   PrintPoint( tsVertex* p );
   void    Checks( void );
   void	   Consistency( void );
   void	   Convexity( void );
   void	   PrintOut( tsVertex* v );
   void	   PrintVertices( void );
   void	   PrintEdges( void );
   void	   PrintFaces( void );
   void	   CheckEndpts ( void );
   void	   EdgeOrderOnFaces ( void );

   double  Normz(tsFace* f);
   void    LowerFaces(void);
   int	   number_of_delaunay_triangles(void);
   std::vector<int> delaunay_triangle_vertices();
   int*    delaunay_triangle_vertices(int number_of_delaunay_triangles);
   int     compute_delaunay_triangulation();
   void    read_delaunay_vertices(const std::vector<threevector>& site);

} // delaunay namespace

#endif  // delaunay.h
