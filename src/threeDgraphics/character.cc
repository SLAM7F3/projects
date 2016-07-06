// ==========================================================================
// CHARACTER base class member function definitions
// ==========================================================================
// Last modified on 7/29/06; 8/4/06; 1/29/12; 4/5/14
// ==========================================================================

#include <iostream>
#include "threeDgraphics/character.h"
#include "math/constant_vectors.h"
#include "templates/mytemplates.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void character::allocate_member_objects()
{
}		       

void character::initialize_member_objects()
{
   ascii_value=-1;
   origin=Zero_vector;
   extent=linesegment(origin.get_pnt(),threevector(4,0));
}		       

character::character()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

character::character(const character& c)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(c);
}

character::~character()
{
}

// ---------------------------------------------------------------------
void character::docopy(const character& c)
{
   ascii_value=c.ascii_value;
   origin=c.origin;
   extent=c.extent;
   segment=c.segment;
}

// ---------------------------------------------------------------------
// Overload = operator:

character& character::operator= (const character& c)
{
   if (this==&c) return *this;
   docopy(c);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const character& c)
{
   outstream << endl;
   outstream << "character ascii value = " << c.ascii_value << endl;
   outstream << "character origin = " << c.origin << endl;
   outstream << "character extent = " << c.extent << endl;
//   templatefunc::printVector(c.segment);
   return outstream;
}

// =====================================================================
// Moving around member functions
// =====================================================================

void character::translate(const threevector& rvec)
{
   origin.translate(rvec);
   extent.translate(rvec);
   for (unsigned int i=0; i<segment.size(); i++)
   {
      segment[i].translate(rvec);
   }
}

void character::scale(const threevector& scale_origin,double scale_factor)
{
   threevector scale_vector(scale_factor,scale_factor,scale_factor);
   origin.scale(scale_origin,scale_vector);
   extent.scale(scale_origin,scale_vector);
   for (unsigned int i=0; i<segment.size(); i++)
   {
      segment[i].scale(scale_origin,scale_vector);
   }
}

// ---------------------------------------------------------------------
void character::rotate(const threevector& rotation_origin,
                       double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void character::rotate(const threevector& rotation_origin,const rotation& R)
{
   origin.rotate(rotation_origin,R);
   extent.rotate(rotation_origin,R);
   
   for (unsigned int i=0; i<segment.size(); i++)
   {
      segment[i].rotate(rotation_origin,R);
   }
}
