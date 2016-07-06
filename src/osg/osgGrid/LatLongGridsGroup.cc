// ==========================================================================
// LATLONGGRIDSGROUP class member function definitions
// ==========================================================================
// Last modified on 5/22/09; 6/2/09; 1/31/10
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/BoundingBox>
#include <osg/Geode>
#include "osg/osgGrid/LatLongGridsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LatLongGridsGroup::allocate_member_objects()
{
}		       

void LatLongGridsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="LatLongGridsGroup";
   get_OSGgroup_ptr()->setName("LatLongGrids OSGgroup");

   dynamic_Grid_ID=0;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<LatLongGridsGroup>(
         this, &LatLongGridsGroup::update_display));
}		       

LatLongGridsGroup::LatLongGridsGroup(
   const int p_ndims,Pass* PI_ptr,osgGA::Custom3DManipulator* CM_3D_ptr):
   GridsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   this->CM_3D_ptr=CM_3D_ptr;
}		       

LatLongGridsGroup::~LatLongGridsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LatLongGridsGroup& G)
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

LatLongGrid* LatLongGridsGroup::generate_new_Grid()
{
   return new LatLongGrid(
      get_pass_ptr(),get_ndims(),get_next_unused_ID(),CM_3D_ptr);
}

// ---------------------------------------------------------------------
LatLongGrid* LatLongGridsGroup::generate_new_Grid(
   int UTM_zonenumber,bool northern_hemisphere_flag,
   const osg::BoundingBox& bbox)
{
   return generate_new_Grid(
      UTM_zonenumber,northern_hemisphere_flag,
      bbox.xMin(),bbox.xMax(),bbox.yMin(),bbox.yMax(),bbox.zMin());
}

// ---------------------------------------------------------------------
LatLongGrid* LatLongGridsGroup::generate_new_Grid(
   int UTM_zonenumber,bool northern_hemisphere_flag,
   double min_east,double max_east,
   double min_north,double max_north,double min_Z)
{
//   cout << "inside LatLongGridsGroup::generate_new_Grid() #3" << endl;
//   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//   cout << "min_east = " << min_east << " max_east = " << max_east << endl;
//   cout << "min_north = " << min_north << " max_north = " << max_north
//        << endl;

   LatLongGrid* LatLongGrid_ptr=new LatLongGrid(
      get_pass_ptr(),get_ndims(),get_next_unused_ID(),CM_3D_ptr);
   GridsGroup::initialize_new_Grid(LatLongGrid_ptr);

   LatLongGrid_ptr->initialize(
      UTM_zonenumber,northern_hemisphere_flag, 
      min_east,max_east,min_north,max_north,min_Z);

   return LatLongGrid_ptr;
}

// ---------------------------------------------------------------------
LatLongGrid* LatLongGridsGroup::generate_new_Grid(
   double min_long,double max_long,double min_lat,double max_lat,double min_Z)
{
   cout << "inside LatLongGridsGroup::generate_new_Grid() #4" << endl;

   LatLongGrid* LatLongGrid_ptr=new LatLongGrid(
      get_pass_ptr(),get_ndims(),get_next_unused_ID(),CM_3D_ptr);
   GridsGroup::initialize_new_Grid(LatLongGrid_ptr);

   LatLongGrid_ptr->initialize(min_long,max_long,min_lat,max_lat,min_Z);
   return LatLongGrid_ptr;
}

// ---------------------------------------------------------------------
// Member function initialize_grid is intended to be called by main
// programs where the LatLongGrid is first instantiated and a point
// cloud is subsquently instantiated.  Later the point cloud's
// bounding box information is used to set the LatLongGrid's size and
// middle location.

void LatLongGridsGroup::initialize_grid(
   int UTM_zonenumber,bool northern_hemisphere_flag,
   const osg::BoundingBox& bbox,LatLongGrid* LatLongGrid_ptr)
{
   GridsGroup::initialize_new_Grid(LatLongGrid_ptr);
   LatLongGrid_ptr->initialize(
      UTM_zonenumber,northern_hemisphere_flag,
      bbox.xMin(),bbox.xMax(),bbox.yMin(),bbox.yMax(),bbox.zMin());
}

void LatLongGridsGroup::initialize_grid(
   double min_long,double max_long,double min_lat,double max_lat,double min_Z,
   LatLongGrid* LatLongGrid_ptr)
{
   GridsGroup::initialize_new_Grid(LatLongGrid_ptr);
   LatLongGrid_ptr->initialize(
      min_long,max_long,min_lat,max_lat,min_Z);
}

// ---------------------------------------------------------------------
void LatLongGridsGroup::update_display()
{
//   cout << "inside LatLongGridsGroup::update_display()" << endl;

   LatLongGrid* LatLongGrid_ptr=get_Grid_ptr(dynamic_Grid_ID);
//   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "grid_origin = " << *grid_origin_ptr << endl;

   if (LatLongGrid_ptr->get_dynamic_grid_flag())
   {
//      cout << "camera Zhat = " << CM_3D_ptr->get_camera_Zhat() << endl;
      double curr_theta=acos(CM_3D_ptr->get_camera_Zhat().get(2));
//      cout << "curr_theta = " << curr_theta*180/PI << endl;
      const double max_tilt_angle_from_nadir=5*PI/180;
      if (curr_theta < max_tilt_angle_from_nadir)
      {
         LatLongGrid_ptr->turn_on_dynamic_LongLatLines();
         LatLongGrid_ptr->redraw_long_lat_lines();
      }
      else
      {
         LatLongGrid_ptr->turn_off_dynamic_LongLatLines();
      }
   }

   GraphicalsGroup::update_display();
}
