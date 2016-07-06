// ==========================================================================
// Header file for LATLONGGRIDSGROUP class
// ==========================================================================
// Last modified on 3/9/09; 5/22/09; 1/31/10
// ==========================================================================

#ifndef LATLONGGRIDSGROUP_H
#define LATLONGGRIDSGROUP_H

#include <iostream>
#include <string>
#include "osg/Custom3DManipulator.h"
#include "osg/osgGrid/GridsGroup.h"
#include "osg/osgGrid/LatLongGrid.h"

// class osg::BoundingBox;
class WindowManager;

class LatLongGridsGroup : public GridsGroup
{

  public:

// Initialization, constructor and destructor functions:

   LatLongGridsGroup(const int p_ndims,Pass* PI_ptr,
                     osgGA::Custom3DManipulator* CM_3D_ptr);
   virtual ~LatLongGridsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const LatLongGridsGroup& L);

// Set & get methods:

   LatLongGrid* get_Grid_ptr(int n) const;
   LatLongGrid* get_ID_labeled_Grid_ptr(int ID) const;
   void set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_3D_ptr);
   void set_dynamic_Grid_ID(int id);
   
// Grid creation methods:

   LatLongGrid* generate_new_Grid();
   LatLongGrid* generate_new_Grid(
      int UTM_zonenumber,bool northern_hemisphere_flag,
      const osg::BoundingBox& bbox);
   LatLongGrid* generate_new_Grid(
      int UTM_zonenumber,bool northern_hemisphere_flag,
      double min_east,double max_east,
      double min_north,double max_north,double min_Z);
   LatLongGrid* generate_new_Grid(
      double min_long,double max_long,double min_lat,double max_lat,
      double min_Z);

   void initialize_grid(
      int UTM_zonenumber,bool northern_hemisphere_flag,
      const osg::BoundingBox& bbox,LatLongGrid* latlonggrid_ptr);
   void initialize_grid(
      double min_long,double max_long,double min_lat,double max_lat,
      double min_Z,LatLongGrid* latlonggrid_ptr);

   void update_display();

  protected:

  private:

   int dynamic_Grid_ID;
   osgGA::Custom3DManipulator* CM_3D_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const LatLongGridsGroup& G);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline LatLongGrid* LatLongGridsGroup::get_Grid_ptr(int n) const
{
   return dynamic_cast<LatLongGrid*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline LatLongGrid* LatLongGridsGroup::get_ID_labeled_Grid_ptr(int ID) const
{
   return dynamic_cast<LatLongGrid*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline void LatLongGridsGroup::set_CM_3D_ptr(
   osgGA::Custom3DManipulator* CM_3D_ptr)
{
   this->CM_3D_ptr=CM_3D_ptr;
}

// --------------------------------------------------------------------------
inline void LatLongGridsGroup::set_dynamic_Grid_ID(int id)
{
   dynamic_Grid_ID=id;
}


#endif // LatLongGridsGroup.h



