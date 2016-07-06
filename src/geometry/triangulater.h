// ==========================================================================
// Header file for triangulater class
// ==========================================================================
// Last modified on 1/26/12
// ==========================================================================

// COTD Entry submitted by John W. Ratcliff [jratcliff@verant.com]

// ** THIS IS A CODE SNIPPET WHICH WILL EFFICIEINTLY TRIANGULATE ANY
// ** POLYGON/CONTOUR (without holes) AS A STATIC CLASS.  THIS SNIPPET
// ** IS COMPRISED OF 3 FILES, TRIANGULATE.H, THE HEADER FILE FOR THE
// ** TRIANGULATE BASE CLASS, TRIANGULATE.CPP, THE IMPLEMENTATION OF
// ** THE TRIANGULATE BASE CLASS, AND TEST.CPP, A SMALL TEST PROGRAM
// ** DEMONSTRATING THE USAGE OF THE TRIANGULATOR.  THE TRIANGULATE
// ** BASE CLASS ALSO PROVIDES TWO USEFUL HELPER METHODS, ONE WHICH
// ** COMPUTES THE AREA OF A POLYGON, AND ANOTHER WHICH DOES AN EFFICENT
// ** POINT IN A TRIANGLE TEST.
// ** SUBMITTED BY JOHN W. RATCLIFF (jratcliff@verant.com) July 22, 2000

#ifndef TRIANGULATER_CLASS_H
#define TRIANGULATER_CLASS_H

/*****************************************************************/
/** Static class to triangulate any contour/polygon efficiently **/
/** You should replace Vector2d with whatever your own Vector   **/
/** class might be.  Does not support polygons with holes.      **/
/** Uses STL vectors to represent a dynamic array of vertices.  **/
/** This code snippet was submitted to FlipCode.com by          **/
/** John W. Ratcliff (jratcliff@verant.com) on July 22, 2000    **/
/** I did not write the original code/algorithm for this        **/
/** this triangulator, in fact, I can't even remember where I   **/
/** found it in the first place.  However, I did rework it into **/
/** the following black-box static class so you can make easy   **/
/** use of it in your own code.  Simply replace Vector2d with   **/
/** whatever your own Vector implementation might be.           **/
/*****************************************************************/

#include <vector>  // Include STL vector class.
#include "math/twovector.h"

class Triangulater
{

  public:

// Typedef an STL vector of vertices which are used to represent
// a polygon/contour and a series of triangles.

   typedef std::vector< twovector > TwovectorVector;

   // triangulate a contour/polygon, places results in STL vector
   // as series of triangles.

   static bool Process(const TwovectorVector &contour,
   TwovectorVector &result);

   // compute area of a contour/polygon
   static double Area(const TwovectorVector &contour);

   // decide if point Px/Py is inside triangle defined by
   // (Ax,Ay) (Bx,By) (Cx,Cy)
   static bool InsideTriangle(double Ax, double Ay,
   double Bx, double By,
   double Cx, double Cy,
   double Px, double Py);


  private:
   static bool Snip(
      const TwovectorVector &contour,int u,int v,int w,int n,int *V);

};


#endif	// TRIANGULATER_CLASS
