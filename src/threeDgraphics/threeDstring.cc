// ==========================================================================
// 3DSTRING class member function definitions
// ==========================================================================
// Last modified on 8/4/06; 8/6/06; 1/29/12; 4/5/14
// ==========================================================================

#include <iostream>
#include "threeDgraphics/characterfuncs.h"
#include "math/constant_vectors.h"
#include "templates/mytemplates.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void threeDstring::allocate_member_objects()
{
}		       

void threeDstring::initialize_member_objects()
{
   nchars=0;
   ascii_string="";
   origin=Zero_vector;
   ascii_char=characterfunc::generate_ascii_characters();
}		       

threeDstring::threeDstring()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

threeDstring::threeDstring(string inputstring)
{	
   initialize_member_objects();
   allocate_member_objects();
   parse_string(inputstring);
}		       

// Copy constructor:

threeDstring::threeDstring(const threeDstring& s)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

threeDstring::~threeDstring()
{
   delete [] ascii_char;
   ascii_char=NULL;
}

// ---------------------------------------------------------------------
void threeDstring::docopy(const threeDstring& s)
{
   nchars=s.nchars;
   ascii_string=s.ascii_string;
   origin=s.origin;
   extent=s.extent;
   charstring=s.charstring;
   *ascii_char=*(s.ascii_char);
}

// ---------------------------------------------------------------------
// Overload = operator:

threeDstring& threeDstring::operator= (const threeDstring& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const threeDstring& s)
{
   outstream << endl;
   outstream << "nchars = " << s.nchars << endl;
   outstream << "ascii_string = " << s.ascii_string << endl;
   outstream << "threeDstring origin = " << s.origin << endl;
   outstream << "threeDstring extent = " << s.extent << endl;
   templatefunc::printVector(s.charstring);
   for (unsigned int i=0; i<s.nchars; i++)
   {
      cout << "i = " << i << " ascii value = " 
           << s.get_char(i).get_ascii_value() << endl;
      cout << " char origin = " << s.get_char(i).
         get_origin() << endl;
      cout << "char extent = " << s.get_char(i).get_extent() << endl;
   }
   return outstream;
}

// =====================================================================
// Moving around member functions
// =====================================================================

void threeDstring::translate(const threevector& rvec)
{
   origin.translate(rvec);
   extent.translate(rvec);
   for (unsigned int i=0; i<charstring.size(); i++)
   {
      charstring[i].translate(rvec);
   }
}

// Member function center_upon_location translates the current
// threeDstring so that its midpoint lies at input position rvec:

void threeDstring::center_upon_location(const threevector& rvec)
{
   translate(rvec-extent.get_midpoint());
}

void threeDstring::scale(const threevector& scale_origin,double scale_factor)
{
   threevector scale_vector(scale_factor,scale_factor,scale_factor);
   origin.scale(scale_origin,scale_vector);
   extent.scale(scale_origin,scale_vector);

   for (unsigned int i=0; i<nchars; i++)
   {
      charstring[i].scale(scale_origin,scale_factor);
   }
}

// ---------------------------------------------------------------------
void threeDstring::rotate(const threevector& rotation_origin,
                          double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void threeDstring::rotate(
   const threevector& rotation_origin,const rotation& R)
{
   origin.rotate(rotation_origin,R);
   extent.rotate(rotation_origin,R);
   for (unsigned int i=0; i<nchars; i++)
   {
      charstring[i].rotate(rotation_origin,R);
   }
}

// ---------------------------------------------------------------------
// Member function parse_string takes in a C++ string object.  It
// returns a dynamically generated STL vector containing a 3D
// character corresponding to each individual character within the
// input string.

void threeDstring::parse_string(string inputstring)
{
   vector<int> ascii_vector=
      stringfunc::decompose_string_to_ascii_rep(inputstring);
   
   for (unsigned int i=0; i<ascii_vector.size(); i++)
   {
      push_back_char(ascii_char[ascii_vector[i]]);
      charstring[i].set_ascii_value(ascii_vector[i]);
      ascii_string += stringfunc::ascii_integer_to_char(ascii_vector[i]);
   } // loop over index i labeling individual chars within inputstring

   origin=charstring[0].get_origin();
   extent=linesegment(origin.get_pnt(),
                      charstring[nchars-1].get_extent().get_v2());
}
