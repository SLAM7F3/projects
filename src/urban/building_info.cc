// ==========================================================================
// BUILDING_INFO class member function definitions
// ==========================================================================
// Last modified on 4/18/05
// ==========================================================================

#include <iostream>
#include "urban/building_info.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void building_info::allocate_member_objects()
{
}		       

void building_info::initialize_member_objects()
{
   building_ID=-1;
   relative_height=unknown;
   spine_dir=undefined;
   tall_tree_posn=small_shrub_posn=none;
}		       

building_info::building_info()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

building_info::building_info(int id)
{	
   initialize_member_objects();
   allocate_member_objects();

   building_ID=id;
}		       

// Copy constructor:

building_info::building_info(const building_info& b)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(b);
}

building_info::~building_info()
{
}

// ---------------------------------------------------------------------
void building_info::docopy(const building_info& b)
{
   building_ID=b.building_ID;
   relative_height=b.relative_height;
   spine_dir=b.spine_dir;
   tall_tree_posn=b.tall_tree_posn;
   small_shrub_posn=b.small_shrub_posn;
}

// ---------------------------------------------------------------------
// Overload = operator:

building_info& building_info::operator= (const building_info& b)
{
   if (this==&b) return *this;
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (std::ostream& outstream,const building_info& b)
{
   outstream << endl;
   outstream << "Building ID = " << b.building_ID << endl;
   if (b.relative_height==building_info::less_than)
   {
      outstream << "Relative height is less than LHS neighbor" << endl;
   }
   else if (b.relative_height==building_info::equal_to)
   {
      outstream << "Relative height is equal to LHS neighbor" << endl;
   }
   else if (b.relative_height==building_info::greater_than)
   {
      outstream << "Relative height is greater than LHS neighbor" << endl;
   }
   else if (b.relative_height==building_info::unknown)
   {
      outstream << "Relative height is unknown" << endl;
   }

   if (b.spine_dir==building_info::parallel)
   {
      outstream << "Spine direction is parallel to street" << endl;
   }
   else if (b.spine_dir==building_info::perpendicular)
   {
      outstream << "Spine direction is perpendicular to street" << endl;
   }
   else if (b.spine_dir==building_info::undefined)
   {
      outstream << "Spine direction is undefined" << endl;
   }

   if (b.tall_tree_posn==building_info::in_front)
   {
      outstream << "Tall tree stands in front of buiding" << endl;
   }
   else if (b.tall_tree_posn==building_info::in_back)
   {
      outstream << "Tall tree stands in back of building" << endl;
   }
   else if (b.tall_tree_posn==building_info::on_right)
   {
      outstream << "Tall tree stands on right of building" << endl;
   }
   else if (b.tall_tree_posn==building_info::on_left)
   {
      outstream << "Tall tree stands on left of building" << endl;
   }
   else if (b.tall_tree_posn==building_info::none)
   {
      outstream << "Tall tree direction is undefined" << endl;
   }

   if (b.small_shrub_posn==building_info::in_front)
   {
      outstream << "Small shrub lies in front of buiding" << endl;
   }
   else if (b.small_shrub_posn==building_info::in_back)
   {
      outstream << "Small shrub lies in back of building" << endl;
   }
   else if (b.small_shrub_posn==building_info::on_right)
   {
      outstream << "Small shrub lies on right of building" << endl;
   }
   else if (b.small_shrub_posn==building_info::on_left)
   {
      outstream << "Small shrub lies on left of building" << endl;
   }
   else if (b.small_shrub_posn==building_info::none)
   {
      outstream << "Small shrub direction is undefined" << endl;
   }

   return outstream;
}
