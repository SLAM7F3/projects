// ==========================================================================
// LOSMODEL class member function definitions
// ==========================================================================
// Last updated on 9/19/09
// ==========================================================================

#include <iostream>
#include "osg/osgModels/LOSMODEL.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LOSMODEL::allocate_member_objects()
{
}		       

void LOSMODEL::initialize_member_objects()
{
   Graphical_name="LOSMODEL";
}		       

LOSMODEL::LOSMODEL(threevector* GO_ptr,string filename,int id):
   MODEL(GO_ptr,filename,id)
{	
//   cout << "inside LOSMODEL constructor #1" << endl;

   allocate_member_objects();
   initialize_member_objects();
}		       

LOSMODEL::~LOSMODEL()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LOSMODEL& m)
{
   outstream << "inside LOSMODEL::operator<<" << endl;
   return outstream;
}

// ==========================================================================
// Model manipulation methods
// ==========================================================================
