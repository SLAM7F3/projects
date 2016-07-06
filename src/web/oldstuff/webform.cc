// ==========================================================================
// Webform class member function definitions which allow one to
// create HTML links using C++.
// ==========================================================================
// Last modified on 6/26/00
// ==========================================================================

#include "webform.h"

using std::string;
using std::ostream;
using std::endl;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

webform::webform(void)
{
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

webform::webform(const webform& f)
{
   docopy(f);
}

webform::~webform()
{
}

// ---------------------------------------------------------------------
// As Ed Broach has pointed out, the default C++ =operator for objects
// may simply equate pointers to arrays within objects when one object
// is equated with another.  Individual elements within the arrays
// apparently are not equated to one another by the default C++
// =operator.  This can lead to segmentation errors if the arrays are
// dynamically rather than statically allocated, for the pointer to
// the original array may be destroyed before the elements within the second
// array are copied over.  So we need to write an explicit copy function 
// which transfers all of the subfields within an object to another object
// whenever the object in question has dynamically allocated arrays rather 
// than relying upon C++'s default =operator:

void webform::docopy(const webform& f)
{
   method=f.method;
   action=f.action;
   formtable=f.formtable;
}	

// Overload = operator:

webform webform::operator= (const webform f)
{
   docopy(f);
   return *this;
}




