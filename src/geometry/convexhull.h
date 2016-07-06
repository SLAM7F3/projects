// This ancient class is DEPRECATED.  Use QHULL instead!

// ==========================================================================
// Header file for Graham's 2D convex hull algorithm functions
// ==========================================================================
// Last updated on 6/24/04; 8/3/06; 8/5/06
// ==========================================================================

#ifndef CONVEXHULL_H
#define CONVEXHULL_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>
#include "datastructures/datapoint.h"
#include "math/threevector.h"

class polygon;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;

namespace convexhull
{
   const int X=0;
   const int Y=1;
   const int DIM=2;			// Spatial dimension
   typedef double tPointd[DIM];   	// Type double point 

// Point(s) structure

   typedef struct tPointStructure tsPoint;
   struct tPointStructure 
   {
         int vnum;
         tPointd v;
         bool delete_flag;
   };

// Stack structure

   typedef struct tStackCell tsStack;
   struct tStackCell 
   {
         tsPoint* p_ptr;
         tsStack* next_ptr;
   };

// Namespace variables

   const int PMAX=100000;			// Maximum number of points
   typedef tsPoint tPointArray[PMAX];
   static tPointArray P;

   void Initialize_Points(const std::vector<threevector>& currpoint);
   void FindLowest(void);
   void	Swap(int i,int j);
   void Sort_Points();
   int Compare( const void *tp1, const void *tp2 );

   tsStack* Pop(tsStack* s_ptr);
   tsStack* Push(tsPoint* p_ptr,tsStack* top_ptr);
   void PrintStack(tsStack* t_ptr);
   int Stacklength(tsStack* t_ptr);
   void delete_stack(tsStack* s_ptr);

   void Squash(void);
   tsStack* Graham(void);
   void	Copy(int i,int j);

   double Area2(tPointd a,tPointd b,tPointd c);
   int AreaSign(tPointd a,tPointd b,tPointd c);
   bool Left(tPointd a,tPointd b,tPointd c);

   void PrintPoints(void);
   void PrintPostscript(tsStack* t_ptr);
   std::ostream& operator<< (std::ostream& outstream,const tsPoint& p);

   polygon* convex_hull_poly(std::vector<threevector>& pixel_vertex);
   polygon* convex_hull_poly(
      std::vector<threevector>& pixel_vertex,std::vector<int>& vertex_order);

   polygon* convex_hull_poly(const linkedlist* pixel_list_ptr);
   polygon* convex_hull_poly(const Linkedlist<std::pair<int,int> >* 
                             pixel_list_ptr);

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Method Left returns true iff c is strictly to the left of the
// directed line through a to b.

   inline bool Left(tPointd a, tPointd b, tPointd c)
      { 
         return Area2( a, b, c ) > 0;
      }

} // convexhull namespace

#endif  // convexhull.h
