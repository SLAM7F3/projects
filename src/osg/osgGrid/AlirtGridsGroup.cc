// ==========================================================================
// ALIRTGRIDSGROUP class member function definitions
// ==========================================================================
// Last modified on 3/10/09; 6/9/10; 3/15/13
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/BoundingBox>
#include <osg/Geode>
#include "osg/osgGrid/AlirtGridsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AlirtGridsGroup::allocate_member_objects()
{
}		       

void AlirtGridsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="AlirtGridsGroup";
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<AlirtGridsGroup>(
         this, &AlirtGridsGroup::update_display));
}		       

AlirtGridsGroup::AlirtGridsGroup(const int p_ndims,Pass* PI_ptr):
   GridsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

AlirtGridsGroup::~AlirtGridsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const AlirtGridsGroup& G)
{
   for (unsigned int n=0; n<G.get_n_Graphicals(); n++)
   {
      Grid* Grid_ptr=G.get_Grid_ptr(n);
      outstream << "Grid node # " << n << endl;
      outstream << "Grid = " << *Grid_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Grid creation methods
// ==========================================================================

// Member function generate_new_Grid returns a dynamically
// instantiated grid running through the vertices contained within
// input vector V.

AlirtGrid* AlirtGridsGroup::generate_new_Grid(bool wopillc)
{
//   cout << "inside AlirtGridsGroup::generate_new_Grid() #1" << endl;
      AlirtGrid* AlirtGrid_ptr=new AlirtGrid(
      wopillc,get_ndims(),get_next_unused_ID());
   GridsGroup::initialize_new_Grid(AlirtGrid_ptr);
   return AlirtGrid_ptr;
}

AlirtGrid* AlirtGridsGroup::generate_new_Grid(
   colorfunc::Color c,bool wopillc)
{
//   cout << "inside AlirtGridsGroup::generate_new_Grid() #2" << endl;
   AlirtGrid* AlirtGrid_ptr=new AlirtGrid(
      c,wopillc,get_ndims(),get_next_unused_ID());
   GridsGroup::initialize_new_Grid(AlirtGrid_ptr);
   return AlirtGrid_ptr;
}

// ---------------------------------------------------------------------
AlirtGrid* AlirtGridsGroup::generate_new_Grid(
   double min_east,double max_east,
   double min_north,double max_north,double min_Z,
   bool world_origin_precisely_in_lower_left_corner)
{
//   cout << "inside AlirtGridsGroup::generate_new_Grid()" << endl;

   AlirtGrid* AlirtGrid_ptr=new AlirtGrid(
      world_origin_precisely_in_lower_left_corner,
      get_ndims(),get_next_unused_ID());
   GridsGroup::initialize_new_Grid(AlirtGrid_ptr);

   AlirtGrid_ptr->initialize_ALIRT_grid(
      min_east,max_east,min_north,max_north,min_Z);

   return AlirtGrid_ptr;
}

// ---------------------------------------------------------------------
// This next overloaded version of initialize_grid is intended to be
// called by main programs where the AlirtGrid is first instantiated,
// a point cloud is subsquently instantiated.  Later the point cloud's
// bounding box information is used to set the AlirtGrid's size and
// middle location.

void AlirtGridsGroup::initialize_grid(
   AlirtGrid* AlirtGrid_ptr,const osg::BoundingBox& bbox,
   Grid::Distance_Scale distance_scale,double delta_s,
   double magnification_factor)
{
//   cout << "inside AGG::initialize_grid()" << endl;
   
   GridsGroup::initialize_new_Grid(AlirtGrid_ptr);
   AlirtGrid_ptr->initialize_ALIRT_grid(
      bbox,distance_scale,delta_s,magnification_factor);
}

void AlirtGridsGroup::initialize_grid(
   AlirtGrid* AlirtGrid_ptr,
   double min_X,double max_X,double min_Y,double max_Y,double min_Z,
   Grid::Distance_Scale distance_scale)
{
   GridsGroup::initialize_new_Grid(AlirtGrid_ptr);
   AlirtGrid_ptr->initialize_ALIRT_grid(min_X,max_X,min_Y,max_Y,min_Z,
                                        distance_scale);
}

// ---------------------------------------------------------------------
void AlirtGridsGroup::update_display()
{
//   cout << "inside AlirtGridsGroup::update_display()" << endl;
   GraphicalsGroup::update_display();
}
