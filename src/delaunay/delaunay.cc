// As of November 2005, this namespace is deprecated and should no
// longer be used!  The following "Computational Geometry in C" code
// is definitely buggy and yields wrong results!

// ==========================================================================
// 3D convex hull algorithm
// ==========================================================================
// Last updated on 11/15/05; 4/13/06; 6/14/06; 8/6/06
// ==========================================================================

// This code is described in "Computational Geometry in C" (Second
// Edition), Chapter 4.  It is not written to be comprehensible
// without the explanation in that book.

// Input: 3n integer coordinates for the points.
// Output: the 3D convex hull, in postscript with embedded comments
//         showing the vertices and faces.

// Compile: gcc -o chull chull.c (or simply: make)

// Written by Joseph O'Rourke, with contributions by Kristy Anderson,
// John Kutcher, Catherine Schevon, Susan Weller.  Last modified: May
// 2000 

// Questions to orourke@cs.smith.edu.

// This code is Copyright 2000 by Joseph O'Rourke.  It may be freely
// redistributed in its entirety provided that this copyright notice
// is not removed.

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "math/mathfuncs.h"
#include "delaunay.h"

using std::cout;
using std::endl;
using std::vector;

namespace delaunay
{

// Namespace constants:

   const bool ONHULL=true;
   const bool REMOVED=true;
   const bool VISIBLE=true;
   const bool PROCESSED=true;
   const int X=0;
   const int Y=1;
   const int Z=2;

// Namespace global variables:

   tsVertex* vertices=NULL;
   tsEdge* edges=NULL;
   tsFace* faces=NULL;
   bool debug=false;
   bool check=false;

// Macros used to access data structures and perform quick tests:

#define FREE(p)		if (p) { delete ((char *) p); p = NULL; }

#define ADD( head, p )  if ( head )  { \
				p->next = head; \
				p->prev = head->prev; \
				head->prev = p; \
				p->prev->next = p; \
			} \
			else { \
				head = p; \
				head->next = head->prev = p; \
			}

#define DELETE( head, p ) if ( head )  { \
				if ( head == head->next ) \
					head = NULL;  \
				else if ( p == head ) \
					head = head->next; \
				p->next->prev = p->prev;  \
				p->prev->next = p->next;  \
				FREE( p ); \
			} 

// ---------------------------------------------------------------------
// Method reset_circular_lists sets the pointers for the vertex, edge
// and face circular lists to NULL:

   void null_circular_lists()
      {
         vertices=NULL;
         edges=NULL;
         faces=NULL;
      }
   
// ---------------------------------------------------------------------
// MakeNullVertex: Makes a vertex, nulls out fields.

   tsVertex* MakeNullVertex( void )
      {
         tsVertex* v_ptr=new tsVertex;

         v_ptr->duplicate = NULL;
         v_ptr->onhull = !ONHULL;
         v_ptr->mark = !PROCESSED;
         ADD( vertices, v_ptr );

         return v_ptr;
      }

// ---------------------------------------------------------------------
// ReadVertices: Reads in the vertices, and links them into a circular
// list with MakeNullVertex.  There is no need for the # of vertices to be
// the first line: the function looks for EOF instead.  Sets the global
// variable vertices via the ADD macro.

   void read_delaunay_vertices(const vector<threevector>& site)
      {
         const int SAFE=1000000;	// Range of safe coord values

         delete_circular_lists();
         for (unsigned int i=0; i<site.size(); i++)
         {
            double x=site[i].get(0);
            double y=site[i].get(1);
            double z=x*x+y*y;

            tsVertex* v = MakeNullVertex();
            v->v[X] = x;
            v->v[Y] = y;
            v->v[Z] = z;
            v->vnum = i;
            if (( fabs(x) > SAFE) || (fabs(y) > SAFE) || (fabs(z) > SAFE)) 
            {
               cout << "inside delaunay::read_delaunay_vertices()" << endl;
               cout << "Coordinate of input point might be too large" << endl;
            }
         }
      }

// ---------------------------------------------------------------------
// Print: Prints out the vertices and the faces.  Uses the vnum
// indices corresponding to the order in which the vertices were
// input.  Output is in PostScript format.

   void	Print( void )
      {
         // Pointers to vertices, edges, faces. 
         tsVertex*  v;
         tsEdge*    e;
         tsFace*    f;
         int xmin, ymin, xmax, ymax;
         int a[3], b[3];  // used to compute normal vector 
         // Counters for Euler's formula. 
         int 	V = 0, E = 0 , F = 0;
         // Note: lowercase==pointer, uppercase==counter. 

         //-- find X min & max --
         v = vertices;
         xmin = xmax = v->v[X];
         do {
            if( v->v[X] > xmax ) xmax = v->v[X];
            else
               if( v->v[X] < xmin ) xmin = v->v[X];
            v = v->next;
         } while ( v != vertices );
	
         //-- find Y min & max --
         v = vertices;
         ymin = ymax = v->v[Y];
         do {
            if( v->v[Y] > ymax ) ymax = v->v[Y];
            else
               if( v->v[Y] < ymin ) ymin = v->v[Y];
            v = v->next;
         } while ( v != vertices );
	
         // PostScript header 
         printf("%%!PS\n");
         printf("%%%%BoundingBox: %d %d %d %d\n", 
                xmin, ymin, xmax, ymax);
         printf(".00 .00 setlinewidth\n");
         printf("%d %d translate\n", -xmin+100, -ymin+100 );
         // The +72 shifts the figure one inch from the lower left corner 

   // Vertices. 
         v = vertices;
         do {                                 
            if( v->mark ) V++;           
            v = v->next;
         } while ( v != vertices );
         printf("\n%%%% Vertices:\tV = %d\n", V);
         printf("%%%% index:\tx\ty\tz\n");
         do {                                 
            printf( "%%%% %5d:\t%d\t%d\t%d\n", 
                    v->vnum, v->v[X], v->v[Y], v->v[Z] );
            printf("newpath\n");
            printf("%d\t%d 2 0 360 arc\n", v->v[X], v->v[Y]);
            printf("closepath stroke\n\n");
            v = v->next;
         } while ( v != vertices );
	
         // Faces. 
         // visible faces are printed as PS output 
         f = faces;
         do {
            ++F;                              
            f  = f ->next;
         } while ( f  != faces );
         printf("\n%%%% Faces:\tF = %d\n", F );
         printf("%%%% Lower faces only: \n");
         do {           
            // Print face only if it is lower 
            if ( f-> lower )
            {
               printf("%%%% vnums:  %d  %d  %d\n", 
                      f->vertex[0]->vnum, 
                      f->vertex[1]->vnum, 
                      f->vertex[2]->vnum);
               printf("newpath\n");
               printf("%d\t%d\tmoveto\n", 
                      f->vertex[0]->v[X], f->vertex[0]->v[Y] );
               printf("%d\t%d\tlineto\n", 
                      f->vertex[1]->v[X], f->vertex[1]->v[Y] );
               printf("%d\t%d\tlineto\n", 
                      f->vertex[2]->v[X], f->vertex[2]->v[Y] );
               printf("closepath stroke\n\n");
            }
            f = f->next;
         } while ( f != faces );

         // prints a list of all faces 
         printf("%%%% List of all faces: \n");
         printf("%%%%\tv0\tv1\tv2\t(vertex indices)\n");
         do {
            printf("%%%%\t%d\t%d\t%d\n",
                   f->vertex[0]->vnum,
                   f->vertex[1]->vnum,
                   f->vertex[2]->vnum );
            f = f->next;
         } while ( f != faces );
	
         // Edges. 	
         e = edges;
         do {
            E++;
            e = e->next;
         } while ( e != edges );
         printf("\n%%%% Edges:\tE = %d\n", E );
         // Edges not printed out (but easily added). 

         printf("\nshowpage\n\n");
         printf("%%EOF\n");

         check = true;
         CheckEuler( V, E, F );

      }

// ---------------------------------------------------------------------
// SubVec:  Computes a - b and puts it into c.

   void SubVec( int a[3], int b[3], int c[3])
      {
         for(int i=0; i < 2; i++ ) c[i] = a[i] - b[i];
      }

// ---------------------------------------------------------------------
// DoubleTriangle builds the initial double triangle.  It first finds
// 3 noncollinear points and makes two faces out of them, in opposite
// order.  It then finds a fourth point that is not coplanar with that
// face.  The vertices are stored in the face structure in
// counterclockwise order so that the volume between the face and the
// point is negative. Lastly, the 3 newfaces to the fourth point are
// constructed and the data structures are cleaned up.

   void    DoubleTriangle( void )
      {
         // Find 3 noncollinear points. 
         tsVertex* v0 = vertices;
         while ( Collinear( v0, v0->next, v0->next->next ) )
            if ( ( v0 = v0->next ) == vertices )
               printf("DoubleTriangle:  All points are Collinear!\n"), exit(0);
         tsVertex* v1 = v0->next;
         tsVertex* v2 = v1->next;
	
         // Mark the vertices as processed. 
         v0->mark = PROCESSED;
         v1->mark = PROCESSED;
         v2->mark = PROCESSED;
   
         // Create the two "twin" faces. 

         tsFace* f1=NULL;
         tsFace* f0 = MakeFace( v0, v1, v2, f1 );
         f1 = MakeFace( v2, v1, v0, f0 );

         // Link adjacent face fields. 
         f0->edge[0]->adjface[1] = f1;
         f0->edge[1]->adjface[1] = f1;
         f0->edge[2]->adjface[1] = f1;
         f1->edge[0]->adjface[1] = f0;
         f1->edge[1]->adjface[1] = f0;
         f1->edge[2]->adjface[1] = f0;
	
         // Find a fourth, noncoplanar point to form tetrahedron. 
         tsVertex* v3 = v2->next;
         int vol = VolumeSign( f0, v3 );
         while ( !vol )   {
            if ( ( v3 = v3->next ) == v0 ) 
               printf("DoubleTriangle:  All points are coplanar!\n"), exit(0);
            vol = VolumeSign( f0, v3 );
         }
	
         // Insure that v3 will be the first added. 
         vertices = v3;
         if ( debug ) {
            fprintf(stderr, 
                    "DoubleTriangle: finished. Head repositioned at v3.\n");
            PrintOut( vertices );
         }
      }
  
// ---------------------------------------------------------------------
// ConstructHull adds the vertices to the hull one at a time.  The hull
// vertices are those in the list marked as onhull.

   void	ConstructHull( void )
      {
         tsVertex* v = vertices;
         do {
            tsVertex* vnext = v->next;
            if ( !v->mark ) {
               v->mark = PROCESSED;
               AddOne( v );
				  // T if addition changes hull; not used.
               CleanUp( &vnext ); // Pass down vnext in case it gets deleted. 

               if ( check ) {
                  fprintf(stderr,"ConstructHull: After Add of %d & Cleanup:\n", 
                          v->vnum);
                  Checks();
               }
               if ( debug )
                  PrintOut( v );
            }
            v = vnext;
         } while ( v != vertices );
      }

// ---------------------------------------------------------------------
// AddOne is passed a vertex.  It first determines all faces visible from 
// that point.  If none are visible then the point is marked as not 
// onhull.  Next is a loop over edges.  If both faces adjacent to an edge
// are visible, then the edge is marked for deletion.  If just one of the
// adjacent faces is visible then a new face is constructed.

   bool 	AddOne( tsVertex* p )
      {
         bool vis = false;

         if ( debug ) {
            fprintf(stderr, "AddOne: starting to add v%d.\n", p->vnum);
            PrintOut( vertices );
         }

         // Mark faces visible from p. 
         tsFace* f = faces;
         do {
            int vol = VolumeSign( f, p );
            if (debug) fprintf(
               stderr, "faddr: %6x   paddr: %6x   Vol = %d\n", f,p,vol);
            if (vol < 0 ) {
               f->visible = VISIBLE;  
               vis = true;                      
            }
            f = f->next;
         } while ( f != faces );

         // If no faces are visible from p, then p is inside the hull. 
         if ( !vis ) {
            p->onhull = !ONHULL;  
            return false; 
         }

// Mark edges in interior of visible region for deletion.  Erect a
// newface based on each border edge.

         tsEdge* e=edges;
         do {
            tsEdge* temp = e->next;
            if ( e->adjface[0] != NULL && e->adjface[0]->visible && 
                 e->adjface[1] != NULL && e->adjface[1]->visible )
               // e interior: mark for deletion. 
               e->delete_edge = REMOVED;
            else if ( (e->adjface[0] != NULL && e->adjface[0]->visible) || 
                      (e->adjface[1] != NULL && e->adjface[1]->visible)  ) 
               // e border: make a new face. 
               e->newface = MakeConeFace( e, p );
            e = temp;
         } while ( e != edges );
         return true;
      }

// ---------------------------------------------------------------------
// VolumeSign returns the sign of the volume of the tetrahedron
// determined by f and p.  VolumeSign is +1 iff p is on the negative
// side of f, where the positive side is determined by the rh-rule.
// So the volume is positive if the ccw normal to f points outside the
// tetrahedron.  The final fewer-multiplications form is due to Bob
// Williamson.

   int  VolumeSign( tsFace* f, tsVertex* p )
      {
         double ax = f->vertex[0]->v[X] - p->v[X];
         double ay = f->vertex[0]->v[Y] - p->v[Y];
         double az = f->vertex[0]->v[Z] - p->v[Z];
         double bx = f->vertex[1]->v[X] - p->v[X];
         double by = f->vertex[1]->v[Y] - p->v[Y];
         double bz = f->vertex[1]->v[Z] - p->v[Z];
         double cx = f->vertex[2]->v[X] - p->v[X];
         double cy = f->vertex[2]->v[Y] - p->v[Y];
         double cz = f->vertex[2]->v[Z] - p->v[Z];

         double vol =   ax * (by*cz - bz*cy)
            + ay * (bz*cx - bx*cz)
            + az * (bx*cy - by*cx);

         if ( debug ) {
            // Compute the volume using integers for comparison. 
            int voli = Volumei( f, p );
            fprintf(stderr,
                    "Face=%6x; Vertex=%d: vol(int) = %d, vol(double) = %lf\n",
                    f,p->vnum,voli,vol);
         }

         // The volume should be an integer. 
//         if      ( vol >  0.5 )  return  1;
//         else if ( vol < -0.5 )  return -1;
//         else                    return  0;
         if      ( vol >  0.00000005 )  return  1;
         else if ( vol < -0.00000005 )  return -1;
         else                    return  0;
      }

// ---------------------------------------------------------------------
// Same computation, but computes using ints, and returns the actual
// volume.

   int  Volumei( tsFace* f, tsVertex* p )
      {
         int ax = f->vertex[0]->v[X] - p->v[X];
         int ay = f->vertex[0]->v[Y] - p->v[Y];
         int az = f->vertex[0]->v[Z] - p->v[Z];
         int bx = f->vertex[1]->v[X] - p->v[X];
         int by = f->vertex[1]->v[Y] - p->v[Y];
         int bz = f->vertex[1]->v[Z] - p->v[Z];
         int cx = f->vertex[2]->v[X] - p->v[X];
         int cy = f->vertex[2]->v[Y] - p->v[Y];
         int cz = f->vertex[2]->v[Z] - p->v[Z];
         int vol =  (ax * (by*cz - bz*cy)
                     + ay * (bz*cx - bx*cz)
                     + az * (bx*cy - by*cx));
         return vol;
      }


// -------------------------------------------------------------------
   void	PrintPoint( tsVertex* p )
      {
         int	i;

         for ( i = 0; i < 3; i++ )
            printf("\t%d", p->v[i]);
         putchar('\n');
      }

// ---------------------------------------------------------------------
// MakeConeFace makes a new face and two new edges between the edge
// and the point that are passed to it. It returns a pointer to the
// new face.

   tsFace*	MakeConeFace( tsEdge* e, tsVertex* p )
      {
         tsEdge*  new_edge[2];
         tsFace*  new_face;

         // Make two new edges (if don't already exist). 
         for (int i=0; i < 2; ++i ) 
            // If the edge exists, copy it into new_edge. 
            if ( !( new_edge[i] = e->endpts[i]->duplicate) ) {
               // Otherwise (duplicate is NULL), MakeNullEdge. 
               new_edge[i] = MakeNullEdge();
               new_edge[i]->endpts[0] = e->endpts[i];
               new_edge[i]->endpts[1] = p;
               e->endpts[i]->duplicate = new_edge[i];
            }

         // Make the new face. 
         new_face = MakeNullFace();   
         new_face->edge[0] = e;
         new_face->edge[1] = new_edge[0];
         new_face->edge[2] = new_edge[1];
         MakeCcw( new_face, e, p ); 
        
         // Set the adjacent face pointers. 
         for (int i=0; i < 2; ++i )
            for (int j=0; j < 2; ++j )  
               // Only one NULL link should be set to new_face. 
               if ( !new_edge[i]->adjface[j] ) {
                  new_edge[i]->adjface[j] = new_face;
                  break;
               }
        
         return new_face;
      }

// ---------------------------------------------------------------------
// MakeCcw puts the vertices in the face structure in counterclock
// wise order.  We want to store the vertices in the same order as in
// the visible face.  The third vertex is always p.

// Although no specific ordering of the edges of a face are used by
// the code, the following condition is maintained for each face f:
// one of the two endpoints of f->edge[i] matches f->vertex[i].  But
// note that this does not imply that f->edge[i] is between
// f->vertex[i] and f->vertex[(i+1)%3].  (Thanks to Bob Williamson.)

   void	MakeCcw( tsFace* f, tsEdge* e, tsVertex* p )
      {
         tsFace*  fv;   // The visible face adjacent to e 
         int    i;    // Index of e->endpoint[0] in fv. 
         tsEdge*  s;	// Temporary, for swapping 
      
         if  ( e->adjface[0]->visible )      
            fv = e->adjface[0];
         else fv = e->adjface[1];
       
// Set vertex[0] & [1] of f to have the same orientation as do the
// corresponding vertices of fv.

         for (i=0; fv->vertex[i] != e->endpts[0]; ++i )
            ;
         // Orient f the same as fv. 
         if ( fv->vertex[ (i+1) % 3 ] != e->endpts[1] ) {
            f->vertex[0] = e->endpts[1];  
            f->vertex[1] = e->endpts[0];    
         }
         else {                               
            f->vertex[0] = e->endpts[0];   
            f->vertex[1] = e->endpts[1];      
            s=f->edge[1];
            f->edge[1]=f->edge[2];
            f->edge[2]=s;
//            SWAP( s, f->edge[1], f->edge[2] );
         }

// This swap is tricky. e is edge[0]. edge[1] is based on endpt[0],
// edge[2] on endpt[1].  So if e is oriented "forwards," we need to
// move edge[1] to follow [0], because it precedes.
   
         f->vertex[2] = p;
      }
 
// ---------------------------------------------------------------------
// MakeNullEdge creates a new cell and initializes all pointers to
// NULL and sets all flags to off.  It returns a pointer to the empty
// cell.

   tsEdge* MakeNullEdge( void )
      {
         tsEdge* e_ptr=new tsEdge;
         e_ptr->adjface[0] = e_ptr->adjface[1] = e_ptr->newface = NULL;
         e_ptr->endpts[0] = e_ptr->endpts[1] = NULL;
         e_ptr->delete_edge = !REMOVED;
         ADD( edges, e_ptr );
         return e_ptr;
      }

// --------------------------------------------------------------------
// MakeNullFace creates a new face structure and initializes all of
// its flags to NULL and sets all the flags to off.  It returns a
// pointer to the empty cell.

   tsFace* MakeNullFace( void )
      {
         tsFace* f_ptr=new tsFace;
         for (int i=0; i < 3; ++i ) 
         {
            f_ptr->edge[i] = NULL;
            f_ptr->vertex[i] = NULL;
         }
         f_ptr->visible = !VISIBLE;
         ADD( faces, f_ptr);
         return f_ptr;
      }

// ---------------------------------------------------------------------
// MakeFace creates a new face structure from three vertices (in ccw
// order).  It returns a pointer to the face.

   tsFace*   MakeFace( tsVertex* v0, tsVertex* v1, tsVertex* v2, tsFace* fold )
      {
         tsFace* f;
         tsEdge* e0;
         tsEdge* e1; 
         tsEdge* e2;

         // Create edges of the initial triangle. 
         if( !fold ) {
            e0 = MakeNullEdge();
            e1 = MakeNullEdge();
            e2 = MakeNullEdge();
         }
         else { // Copy from fold, in reverse order. 
            e0 = fold->edge[2];
            e1 = fold->edge[1];
            e2 = fold->edge[0];
         }
         e0->endpts[0] = v0;              e0->endpts[1] = v1;
         e1->endpts[0] = v1;              e1->endpts[1] = v2;
         e2->endpts[0] = v2;              e2->endpts[1] = v0;
	
         // Create face for triangle. 
         f = MakeNullFace();
         f->edge[0]   = e0;  f->edge[1]   = e1; f->edge[2]   = e2;
         f->vertex[0] = v0;  f->vertex[1] = v1; f->vertex[2] = v2;
	
         // Link edges to face. 
         e0->adjface[0] = e1->adjface[0] = e2->adjface[0] = f;
	
         return f;
      }

// ---------------------------------------------------------------------
// CleanUp goes through each data structure list and clears all flags
// and NULLs out some pointers.  The order of processing (edges,
// faces, vertices) is important.

   void	CleanUp(tsVertex* *pvnext )
      {
         CleanEdges();
         CleanFaces();
         CleanVertices( pvnext );
      }

// ---------------------------------------------------------------------
// CleanEdges runs through the edge list and cleans up the structure.
// If there is a newface then it will put that face in place of the
// visible face and NULL out newface. It also deletes so marked edges.

   void	CleanEdges( void )
      {
         tsEdge*  e;	// Primary index into edge list. 
         tsEdge*  t;	// Temporary edge pointer. 
		
         // Integrate the newface's into the data structure. 
         // Check every edge. 
         e = edges;
         do {
            if ( e->newface ) { 
               if ( e->adjface[0]->visible )
                  e->adjface[0] = e->newface; 
               else	e->adjface[1] = e->newface;
               e->newface = NULL;
            }
            e = e->next;
         } while ( e != edges );

         // Delete any edges marked for deletion. 
         while ( edges && edges->delete_edge ) { 
            e = edges;
            DELETE( edges, e );
         }
         e = edges->next;
         do {
            if ( e->delete_edge ) {
               t = e;
               e = e->next;
               DELETE( edges, t );
            }
            else e = e->next;
         } while ( e != edges );
      }

// ---------------------------------------------------------------------
// CleanFaces runs through the face list and deletes any face marked
// visible.

   void	CleanFaces( void )
      {
         tsFace*  f;	// Primary pointer into face list. 
         tsFace*  t;	// Temporary pointer, for deleting. 
	
         while ( faces && faces->visible ) { 
            f = faces;
            DELETE( faces, f );
         }
         f = faces->next;
         do {
            if ( f->visible ) {
               t = f;
               f = f->next;
               DELETE( faces, t );
            }
            else f = f->next;
         } while ( f != faces );
      }

// ---------------------------------------------------------------------
// CleanVertices runs through the vertex list and deletes the vertices
// that are marked as processed but are not incident to any undeleted
// edges.  The pointer to vnext, pvnext, is used to alter vnext in
// ConstructHull() if we are about to delete vnext.

   void	CleanVertices( tsVertex* *pvnext )
      {

// Mark all vertices incident to some undeleted edge as on the hull.

         tsEdge* e = edges;
         do {
            e->endpts[0]->onhull = e->endpts[1]->onhull = ONHULL;
            e = e->next;
         } while (e != edges);
	
// Delete all vertices that have been processed but are not on the
// hull.

         tsVertex*  v;
         tsVertex* t;
         while ( vertices && vertices->mark && !vertices->onhull ) { 

// If about to delete vnext, advance it first. 

            if ( v == *pvnext )
               *pvnext = v->next;
            v = vertices;
            DELETE( vertices, v );
         }
         v = vertices->next;
         do {
            if ( v->mark && !v->onhull ) {    
               t = v; 
               v = v->next;
               DELETE( vertices, t )
                  }
            else v = v->next;
         } while ( v != vertices );
	
         // Reset flags. 
         v = vertices;
         do {
            v->duplicate = NULL; 
            v->onhull = !ONHULL; 
            v = v->next;
         } while ( v != vertices );
      }

// ---------------------------------------------------------------------
// Method delete_circular_lists calls methods delete_all_edges,
// delete_all_faces and delete_all_vertices which delete all nodes
// within the circularly linked lists.

   void delete_circular_lists()
      {
         delete_all_edges();
         delete_all_faces();
         delete_all_vertices();
         null_circular_lists();
      }

   void	delete_all_edges(void)
      {
         tsEdge* e_ptr=edges;
         if (e_ptr != NULL)
         {
            do
            { 
               tsEdge* next_edge_ptr=e_ptr->next;
               DELETE(edges,e_ptr);
               e_ptr=next_edge_ptr;
            }
            while (e_ptr != edges);
            FREE(e_ptr);
         }
      }

   void	delete_all_faces(void)
      {
         tsFace* f_ptr=faces;
         if (f_ptr != NULL)
         {
            do
            { 
               tsFace* next_face_ptr=f_ptr->next;
               DELETE(faces,f_ptr);
               f_ptr=next_face_ptr;
            }
            while (f_ptr != faces);
            FREE(f_ptr);
         }
      }

   void	delete_all_vertices(void)
      {
         tsVertex* v_ptr=vertices;
         if (v_ptr != NULL)
         {
            do
            { 
               tsVertex* next_vertex_ptr=v_ptr->next;
               DELETE(vertices,v_ptr);
               v_ptr=next_vertex_ptr;
            }
            while (v_ptr != vertices);
            FREE(v_ptr);
         }
      }

// ---------------------------------------------------------------------
// Collinear checks to see if the three points given are collinear, by
// checking to see if each element of the cross product is zero.

   bool	Collinear( tsVertex* a, tsVertex* b, tsVertex* c )
      {
         return 
            ( c->v[Z] - a->v[Z] ) * ( b->v[Y] - a->v[Y] ) -
            ( b->v[Z] - a->v[Z] ) * ( c->v[Y] - a->v[Y] ) == 0
            && ( b->v[Z] - a->v[Z] ) * ( c->v[X] - a->v[X] ) -
            ( b->v[X] - a->v[X] ) * ( c->v[Z] - a->v[Z] ) == 0
            && ( b->v[X] - a->v[X] ) * ( c->v[Y] - a->v[Y] ) -
            ( b->v[Y] - a->v[Y] ) * ( c->v[X] - a->v[X] ) == 0  ;
      }

// ---------------------------------------------------------------------
// Consistency runs through the edge list and checks that all adjacent
// faces have their endpoints in opposite order.  This verifies that
// the vertices are in counterclockwise order.

   void	Consistency( void )
      {
         register tsEdge*  e=edges;
         register int    i, j;

         do {
            // find index of endpoint[0] in adjacent face[0] 
            for ( i = 0; e->adjface[0]->vertex[i] != e->endpts[0]; ++i )
               ;
   
            // find index of endpoint[0] in adjacent face[1] 
            for ( j = 0; e->adjface[1]->vertex[j] != e->endpts[0]; ++j )
               ;

            // check if the endpoints occur in opposite order 
            if ( !( e->adjface[0]->vertex[ (i+1) % 3 ] ==
                    e->adjface[1]->vertex[ (j+2) % 3 ] ||
                    e->adjface[0]->vertex[ (i+2) % 3 ] ==
                    e->adjface[1]->vertex[ (j+1) % 3 ] )  )
               break;
            e = e->next;

         } while ( e != edges );

         if ( e != edges )
            fprintf( stderr, "Checks: edges are NOT consistent.\n");
         else
            fprintf( stderr, "Checks: edges consistent.\n");

      }

// ---------------------------------------------------------------------
// Convexity checks that the volume between every face and every point
// is negative.  This shows that each point is inside every face and
// therefore the hull is convex.

   void	Convexity( void )
      {
         register tsFace*    f;
         register tsVertex*  v;
         int               vol;

         f = faces;
   
         do {
            v = vertices;
            do {
               if ( v->mark ) {
                  vol = VolumeSign( f, v );
                  if ( vol < 0 )
                     break;
               }
               v = v->next;
            } while ( v != vertices );

            f = f->next;

         } while ( f != faces );

         if ( f != faces )
            fprintf( stderr, "Checks: NOT convex.\n");
         else if ( check ) 
            fprintf( stderr, "Checks: convex.\n");
      }

// ---------------------------------------------------------------------
// CheckEuler checks Euler's relation, as well as its implications
// when all faces are known to be triangles.  Only prints positive
// information when debug is true, but always prints negative
// information.

   void	CheckEuler( int V, int E, int F )
      {
         if ( check )
            fprintf( stderr, "Checks: V, E, F = %d %d %d:\t", V, E, F);

         if ( (V - E + F) != 2 )
            fprintf( stderr, "Checks: V-E+F != 2\n");
         else if ( check )
            fprintf( stderr, "V-E+F = 2\t");


         if ( F != (2 * V - 4) )
            fprintf( stderr, "Checks: F=%d != 2V-4=%d; V=%d\n",
                     F, 2*V-4, V);
         else if ( check ) 
            fprintf( stderr, "F = 2V-4\t");
   
         if ( (2 * E) != (3 * F) )
            fprintf( stderr, "Checks: 2E=%d != 3F=%d; E=%d, F=%d\n",
                     2*E, 3*F, E, F );
         else if ( check ) 
            fprintf( stderr, "2E = 3F\n");
      }

// -------------------------------------------------------------------
   void	Checks( void )
      {
         tsVertex*  v;
         tsEdge*    e;
         tsFace*    f;
         int 	   V = 0, E = 0 , F = 0;

         Consistency();
         Convexity();
         if ( (v = vertices) )
            do {
               if (v->mark) V++;
               v = v->next;
            } while ( v != vertices );
         if ( (e = edges ) )
            do {
               E++;
               e = e->next;
            } while ( e != edges );
         if ( (f = faces) )
            do {
               F++;
               f  = f ->next;
            } while ( f  != faces );
         CheckEuler( V, E, F );
         CheckEndpts();
      }


// ==================================================================
//   These functions are used whenever the debug flag is set.
//   They print out the entire contents of each data structure.  
//   Printing is to standard error.  To grab the output in a file in the csh, 
//   use this:
//   chull < i.file >&! o.file
//   =====================================================================

// -------------------------------------------------------------------
   void	PrintOut( tsVertex* v )
      {
         fprintf( stderr, "\nHead vertex %d = %6x :\n", v->vnum, v );
         PrintVertices();
         PrintEdges();
         PrintFaces();
      }

// -------------------------------------------------------------------
   void	PrintVertices( void )
      {
         tsVertex*  temp;

         temp = vertices;
         fprintf (stderr, "Vertex List\n");
         if (vertices) do {
            fprintf(stderr,"  addr %6x\t", vertices );
            fprintf(stderr,"  vnum %4d", vertices->vnum );
            fprintf(stderr,"   (%6d,%6d,%6d)",vertices->v[X],
                    vertices->v[Y], vertices->v[Z] );
            fprintf(stderr,"   active:%3d", vertices->onhull );
            fprintf(stderr,"   dup:%5x", vertices->duplicate );
            fprintf(stderr,"   mark:%2d\n", vertices->mark );
            vertices = vertices->next;
         } while ( vertices != temp );

      }

// -------------------------------------------------------------------
   void	PrintEdges( void )
      {
         tsEdge*  temp;
         int 	  i;
	
         temp = edges;
         fprintf (stderr, "Edge List\n");
         if (edges) do {
            fprintf( stderr, "  addr: %6x\t", edges );
            fprintf( stderr, "adj: ");
            for (i=0; i<2; ++i) 
               fprintf( stderr, "%6x", edges->adjface[i] );
            fprintf( stderr, "  endpts:");
            for (i=0; i<2; ++i) 
               fprintf( stderr, "%4d", edges->endpts[i]->vnum);
            fprintf( stderr, "  del:%3d\n", edges->delete_edge );
            edges = edges->next; 
         } while (edges != temp );

      }

// -------------------------------------------------------------------
   void	PrintFaces( void )
      {
         int 	  i;
         tsFace*  temp=faces;

         fprintf (stderr, "Face List\n");
         if (faces) do {
            fprintf(stderr, "  addr: %10x  ", faces );
            fprintf(stderr, "  edges:");
            for( i=0; i<3; ++i )
               fprintf(stderr, "%10x ", faces->edge[i] );
            fprintf(stderr, "  vert:");
            for ( i=0; i<3; ++i)
               fprintf(stderr, "%4d", faces->vertex[i]->vnum );
            fprintf(stderr, "  vis: %d\n", faces->visible );
            faces= faces->next;
         } while ( faces != temp );

      }

// -------------------------------------------------------------------
// Checks that, for each face, for each i={0,1,2}, the [i]th vertex of
// that face is either the [0]th or [1]st endpoint of the [ith] edge
// of the face.
// -------------------------------------------------------------------
   void	CheckEndpts ( void )
      {
         int 	   i;
         tsEdge*   e;
         tsVertex* v;
         bool error = false;

         tsFace* fstart = faces;
         if (faces) do {
            for( i=0; i<3; ++i ) {
               v = faces->vertex[i];
               e = faces->edge[i];
               if ( v != e->endpts[0] && v != e->endpts[1] ) {
                  error = true;
                  fprintf(stderr,"CheckEndpts: Error!\n");
                  fprintf(stderr,"  addr: %8x;", faces );
                  fprintf(stderr,"  edges:");
                  fprintf(stderr,"(%3d,%3d)", 
                          e->endpts[0]->vnum,
                          e->endpts[1]->vnum);
                  fprintf(stderr,"\n");
               }
            }
            faces= faces->next;
         } while ( faces != fstart );

         if ( error )
            fprintf(stderr,"Checks: ERROR found and reported above.\n");
         else
            fprintf(stderr,
                    "Checks: All endpts of all edges of all faces check.\n");

      }

// ------------------------------------------------------------------
// EdgeOrderOnFaces: puts e0 between v0 and v1, e1 between v1 and v2,
// e2 between v2 and v0 on each face.  This should be unnecessary,
// alas.  Not used in code, but useful for other purposes.
// ------------------------------------------------------------------
   void    EdgeOrderOnFaces ( void ) {
      tsFace* f = faces;
      tsEdge* new_edge;
      int i,j;

      do {
         for (i = 0; i < 3; i++) {
            if (!(((f->edge[i]->endpts[0] == f->vertex[i]) &&
                   (f->edge[i]->endpts[1] == f->vertex[(i+1)%3])) ||
                  ((f->edge[i]->endpts[1] == f->vertex[i]) &&
                   (f->edge[i]->endpts[0] == f->vertex[(i+1)%3])))) {
               // Change the order of the edges on the face: 
               for (j = 0; j < 3; j ++) {
                  // find the edge that should be there 
                  if (((f->edge[j]->endpts[0] == f->vertex[i]) &&
                       (f->edge[j]->endpts[1] == f->vertex[(i+1)%3])) ||
                      ((f->edge[j]->endpts[1] == f->vertex[i]) &&
                       (f->edge[j]->endpts[0] == f->vertex[(i+1)%3]))) {
                     // Swap it with the one erroneously put into its place: 
                     if ( debug )
                        fprintf(stderr,
                                "Making a swap in EdgeOrderOnFaces: F(%d,%d,%d): e[%d] and e[%d]\n",
                                f->vertex[0]->vnum,
                                f->vertex[1]->vnum,
                                f->vertex[2]->vnum,
                                i, j);
                     new_edge = f->edge[i];
                     f->edge[i] = f->edge[j];
                     f->edge[j] = new_edge;
                  }
               }
            }
         }
         f = f->next;
      } while (f != faces);
   
   }

// ------------------------------------------------------------------
   void LowerFaces(void)
      {
         tsFace* f = faces;
         int Flower = 0;   // Total number of lower faces. 

         do 
         {
//             int z = Normz( f );
//              if ( z < 0 ) {
            if ( Normz( f ) < 0 ) 
            {
               Flower++;
               f->lower = true;
//                printf("z=%10d; lower face indices: %d, %d, %d\n", z, 
//                printf("lower face indices: %d, %d, %d\n",
//                  f->vertex[0]->vnum,
//                  f->vertex[1]->vnum,
//                 f->vertex[2]->vnum );
            }
            else f->lower = false;
            f = f->next;
         } while ( f != faces );
//         printf("A total of %d lower faces identified.\n", Flower);
      }

// ---------------------------------------------------------------------
// Computes the z-coordinate of the vector normal to face f.
// ---------------------------------------------------------------------

   double Normz(tsFace* f )
      {
         tsVertex* a = f->vertex[0];
         tsVertex* b = f->vertex[1];
         tsVertex* c = f->vertex[2];

//  double ba0 = ( b->v[X] - a->v[X] );
//  double ca1 = ( c->v[Y] - a->v[Y] );
//  double ba1 = ( b->v[Y] - a->v[Y] );
//  double ca0 = ( c->v[X] - a->v[X] );

//  double z = ba0 * ca1 - ba1 * ca0; 
//  printf("Normz = %lf=%g\n", z,z);
//  if      ( z > 0.0 )  return  1;
//  else if ( z < 0.0 )  return -1;
//  else                 return  0;

         return 
            ( b->v[X] - a->v[X] ) * ( c->v[Y] - a->v[Y] ) -
            ( b->v[Y] - a->v[Y] ) * ( c->v[X] - a->v[X] );
      }

// ---------------------------------------------------------------------
// Method number_of_delaunay_triangles returns the total number of
// triangles needed to Delaunay tesselate the input surface:

   int number_of_delaunay_triangles()
      {
         tsFace* f = faces;
         int triangle_number=0;
         do {           
            if ( f-> lower )
            {
               triangle_number++;
            }
            f = f->next;
         } while ( f != faces );
         return triangle_number;
      }

// ---------------------------------------------------------------------
// Method compute_delaunay_triangulation represents the main
// subroutine which users from outside this namespace need to call in
// order to obtain Delaunay triangulation information.

   int compute_delaunay_triangulation()
      {
         DoubleTriangle();
         ConstructHull();
         LowerFaces();
         return number_of_delaunay_triangles();
      }

// ---------------------------------------------------------------------
// Method delaunay_triangle_vertices returns a dynamically generated
// integer array which contains vertex information for each of the
// Delaunay triangles needed to tesselate the input surface.  The
// labels of the input points which make up the first Delaunay
// triangle are contained within the first 3 integers in the array.
// The labels of the input points making up the 2nd triangle are
// contained in the next 3 integers in the array, etc...

   int* delaunay_triangle_vertices(int number_of_triangles)
      {
         int* triangle_vertex=new int[3*number_of_triangles];

         tsFace* f = faces;
         int triangle_number=0;
         do {           
            if ( f-> lower )
            {
               triangle_vertex[3*triangle_number+0]=f->vertex[0]->vnum;
               triangle_vertex[3*triangle_number+1]=f->vertex[1]->vnum;
               triangle_vertex[3*triangle_number+2]=f->vertex[2]->vnum;
               triangle_number++;
            }
            f = f->next;
         } while ( f != faces );

//         cout << "At end of delaunay::delaunay_triangle_vertices()" << endl;
//         cout << "number of delaunay triangles = "
//              << number_of_delaunay_triangles() << endl;

         return triangle_vertex;
      }

   vector<int> delaunay_triangle_vertices()
      {
         vector<int> triangle_vertex;
         triangle_vertex.reserve(3*number_of_delaunay_triangles());

         tsFace* f = faces;
         do {           
            if ( f-> lower )
            {
               triangle_vertex.push_back(f->vertex[0]->vnum);
               triangle_vertex.push_back(f->vertex[1]->vnum);
               triangle_vertex.push_back(f->vertex[2]->vnum);
            }
            f = f->next;
         } while ( f != faces );
         cout << "At end of delaunay::delaunay_triangle_vertices()" << endl;
         cout << "number of delaunay triangles = "
              << number_of_delaunay_triangles() << endl;
         return triangle_vertex;
      }
   
   
} // delaunay namespace

