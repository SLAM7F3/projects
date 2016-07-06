// ==========================================================================
// Regular_Polygon class member function definitions
// ==========================================================================
// Last modified on 8/6/06; 1/29/12; 2/29/12
// ==========================================================================

#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/regular_polygon.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void regular_polygon::allocate_member_objects()
{
}		       

void regular_polygon::initialize_member_objects()
{
   center=Zero_vector;
}

regular_polygon::regular_polygon(void)
{
   initialize_member_objects();
}

regular_polygon::regular_polygon(int number_of_sides,double r)
{
   initialize_member_objects();
   radius=r;
   nsides=number_of_sides;
   assign_vertex_array_pointers(number_of_sides);
   
   if (nsides < 3)
   {
      cout << "Error inside regular_polygon!" << endl;
      cout << "nsides = " << nsides << " < 3 " << endl;
   }

   double dtheta=2*PI/nsides;
   for (unsigned int i=0; i<nvertices; i++)
   {
      double theta=i*dtheta;
      set_vertex(i,center+radius*threevector(cos(theta),sin(theta),0));
   }
   compute_normal();
}

// This overloaded constructor actually generates an ellipse with
// semi-major axis a and semi-minor axis b respectively oriented along
// the x and y axes.

regular_polygon::regular_polygon(
   int number_of_sides,double a,double b)
{
   initialize_member_objects();
   nsides=number_of_sides;
   assign_vertex_array_pointers(number_of_sides);
   
   if (nsides < 3)
   {
      cout << "Error inside regular_polygon!" << endl;
      cout << "nsides = " << nsides << " < 3 " << endl;
   }

   double e=sqrt(1-sqr(b/a));
   double dtheta=2*PI/nsides;
   for (unsigned int i=0; i<nvertices; i++)
   {
      double theta=i*dtheta;
      double r=a*sqrt((1-sqr(e))/(1-sqr(e*cos(theta))));
      set_vertex(i,center+r*threevector(cos(theta),sin(theta),0));
   }
   compute_normal();
}

// Copy constructor:

regular_polygon::regular_polygon(const regular_polygon& p):
   polygon(p)
{
   docopy(p);
}

regular_polygon::~regular_polygon()
{
}

// ---------------------------------------------------------------------
void regular_polygon::docopy(const regular_polygon& p)
{
   nsides=p.nsides;
   radius=p.radius;
   center=p.center;
}	

// Overload = operator:

regular_polygon& regular_polygon::operator= (const regular_polygon& p)
{
   if (this==&p) return *this;
   polygon::operator=(p);
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
void regular_polygon::translate(const threevector& rvec)
{
   polygon::translate(rvec);
   center=center+rvec;
}

void regular_polygon::absolute_position(const threevector& rvec)
{
   polygon::absolute_position(rvec);
   center=rvec;
}

// ---------------------------------------------------------------------
void regular_polygon::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

// ---------------------------------------------------------------------
void regular_polygon::rotate(const threevector& rotation_origin,
                             const rotation& R)
{
   polygon::rotate(rotation_origin,R);
   center=rotation_origin+R*(center-rotation_origin);
}

// ---------------------------------------------------------------------
void regular_polygon::rotate(const threevector& rotation_origin,
                             double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}




