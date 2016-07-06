// =========================================================================
// Vertex class member function definitions
// =========================================================================
// Last modified on 5/12/08; 1/29/12; 6/29/12
// =========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "math/rotation.h"
#include "geometry/vertex.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void vertex::allocate_member_objects()
{
}

void vertex::initialize_member_objects()
{
   ID=-1;
   V=Zero_vector;
}		 

// ---------------------------------------------------------------------
vertex::vertex()
{
   allocate_member_objects();
   initialize_member_objects();
}

vertex::vertex(const threevector& V,int id)
{
   allocate_member_objects();
   initialize_member_objects();

   ID=id;
   this->V=V;
}

// ---------------------------------------------------------------------
// Copy constructor:

vertex::vertex(const vertex& v)
{
   docopy(v);
}

vertex::~vertex()
{
}

// ---------------------------------------------------------------------
void vertex::docopy(const vertex& v)
{
   ID=v.ID;
   V=v.V;
}

// Overload = operator:

vertex& vertex::operator= (const vertex& v)
{
   if (this==&v) return *this;
   docopy(v);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const vertex& v)
{
//    outstream << endl;
   outstream << "Vertex ID = " << v.ID 
             << " x = " << v.get_posn().get(0) 
             << " y = " << v.get_posn().get(1)
             << " z = " << v.get_posn().get(2) << endl;
   return(outstream);
}

// =========================================================================
// Moving around vertex member functions
// =========================================================================

void vertex::translate(const threevector& rvec)
{
//   cout << "inside vertex::Translate(), V = " << V
//        << " rvec = " << rvec << endl;
//   cout << "this = " << this << endl;
   V += rvec;
}

void vertex::absolute_position(const threevector& rvec)
{
   V=rvec;
}

// ---------------------------------------------------------------------
void vertex::scale(
   const threevector& scale_origin,double s)
{
   threevector scalefactor(s,s,s);
   scale(scale_origin,scalefactor);
}

void vertex::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   threevector dv(V-scale_origin);
   dv=threevector(dv.get(0)*scalefactor.get(0),
                  dv.get(1)*scalefactor.get(1),
                  dv.get(2)*scalefactor.get(2));
   V=scale_origin+dv;
}

// ---------------------------------------------------------------------
void vertex::rotate(const rotation& R)
{
   threevector rotation_origin(Zero_vector);
   rotate(rotation_origin,R);
}

void vertex::rotate(const threevector& rotation_origin,const rotation& R)
{
   threevector dv(V-rotation_origin);
   dv=R*dv;
   V=rotation_origin+dv;
}

void vertex::rotate(const threevector& rotation_origin,
                    double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}
