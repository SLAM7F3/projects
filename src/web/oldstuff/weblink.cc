// ==========================================================================
// Weblink class member function definitions which allow one to
// create HTML links using C++.
// ==========================================================================
// Last modified on 6/21/00
// ==========================================================================

#include "weblink.h"

using std::string;
using std::ostream;
using std::endl;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

weblink::weblink(void)
{
   style="";
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

weblink::weblink(const weblink& l)
{
   docopy(l);
}

weblink::~weblink()
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

void weblink::docopy(const weblink& l)
{
   style=l.style;
   url=l.url;
   descriptor=l.descriptor;
}	

// Overload = operator:

weblink weblink::operator= (const weblink l)
{
   docopy(l);
   return *this;
}




