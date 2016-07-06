// ==========================================================================
// Annotator class member function definitions
// ==========================================================================
// Last updated on 1/21/07; 5/20/08
// ==========================================================================

#include <iostream>
#include "osg/osgAnnotators/Annotator.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Annotator::allocate_member_objects()
{
   geopoint_ptr=new geopoint();
}		       

void Annotator::initialize_member_objects()
{
}		       

Annotator::Annotator()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

Annotator::Annotator(const int p_ndims,int id)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

Annotator::~Annotator()
{
   delete geopoint_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Annotator& a)
{
//   outstream << static_cast<const Geometrical&>(a) << endl;
   return(outstream);
}
