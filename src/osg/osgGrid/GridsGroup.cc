// ==========================================================================
// GRIDSGROUP class member function definitions
// ==========================================================================
// Last modified on 1/26/07; 2/22/07; 7/7/07; 3/10/09
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include "osg/osgGrid/GridsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GridsGroup::allocate_member_objects()
{
}		       

void GridsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="GridsGroup";
}		       

GridsGroup::GridsGroup(const int p_ndims,Pass* PI_ptr):
   GraphicalsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

GridsGroup::~GridsGroup()
{
}

void GridsGroup::initialize_new_Grid(Grid* Grid_ptr,int OSGsubPAT_number)
{
//   cout << "inside GridsGroup::initialize_new_Grid()" << endl;
   GraphicalsGroup::insert_Graphical_into_list(Grid_ptr);
   initialize_Graphical(Grid_ptr);

   osg::Geode* geode_ptr=Grid_ptr->generate_drawable_geode();
   Grid_ptr->get_PAT_ptr()->addChild(geode_ptr);

   get_OSGsubPAT_ptr(0)->addChild(
      Grid_ptr->get_drawable_group_ptr());

   insert_graphical_PAT_into_OSGsubPAT(Grid_ptr,OSGsubPAT_number);
}

