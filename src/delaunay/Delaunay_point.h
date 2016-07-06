// ==========================================================================
// Header file for Delaunay_point class
// ==========================================================================
// Last modified on 12/14/05; 7/12/06
// ==========================================================================

#ifndef DELAUNAY_POINT_H
#define DELAUNAY_POINT_H

#include "network/Network.h"

class threevector;

class Delaunay_point
{
   public :

      Delaunay_point(double xx, double yy);
      Delaunay_point(int ID, double xx, double yy);

      int get_ID() const;
      double X() {return x;}
      double Y() {return y;}
      void lineto(Delaunay_point*,Network<threevector*>* vertex_network_ptr);

      friend  Delaunay_point  operator+(Delaunay_point, Delaunay_point);
      friend  Delaunay_point  operator-(Delaunay_point, Delaunay_point);
      friend  double  operator*(Delaunay_point, Delaunay_point);
      friend  double  operator^(Delaunay_point, Delaunay_point);
      
   private :

      int ID;
      double x;
      double y;

      void allocate_member_objects();
      void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int Delaunay_point::get_ID() const
{
   return ID;
}

inline Delaunay_point operator+(Delaunay_point a, Delaunay_point b) 
{
   return Delaunay_point(a.x+b.x, a.y+b.y);
}

inline Delaunay_point operator-(Delaunay_point a, Delaunay_point b) 
{ 
   return Delaunay_point(a.x-b.x, a.y-b.y);
}

inline double operator*(Delaunay_point a, Delaunay_point b) 
{ 
   return a.x*b.x + a.y*b.y;
}

inline double operator^(Delaunay_point a, Delaunay_point b) 
{ 
   return a.x*b.y - a.y*b.x;
}

#endif  // DELAUNAY_POINT
