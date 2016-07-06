// ==========================================================================
// RegionPolyLine class member function definitions
// ==========================================================================
// Last updated on 12/13/08; 5/3/09; 1/1/11
// ==========================================================================

#include <iostream>
#include <string>
#include "geometry/polyline.h"
#include "osg/osgRegions/RegionPolyLine.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RegionPolyLine::initialize_member_objects()
{
   Graphical_name="RegionPolyLine";
}

void RegionPolyLine::allocate_member_objects()
{
}		       

RegionPolyLine::RegionPolyLine():
   PolyLine()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLine::RegionPolyLine(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr,
   osgText::Font* f_ptr,int n_text_messages,int id):
   PolyLine(p_ndims,PI_ptr,GO_ptr,f_ptr,n_text_messages,id)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLine::RegionPolyLine(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr,
   const threevector& reference_origin,const vector<threevector>& V,
   osgText::Font* f_ptr,int n_text_messages,int id):
   PolyLine(p_ndims,PI_ptr,GO_ptr,reference_origin,V,f_ptr,
            n_text_messages,id)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLine::~RegionPolyLine()
{
//   cout << "inside RegionPolyLine destructor" << endl;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

