// ==========================================================================
// Header file for ALIRTGRIDSGROUP class
// ==========================================================================
// Last modified on 7/7/07; 8/7/07; 6/9/10; 3/15/13
// ==========================================================================

#ifndef ALIRTGRIDSGROUP_H
#define ALIRTGRIDSGROUP_H

#include <iostream>
#include <string>
#include "color/colorfuncs.h"
#include "osg/osgGrid/GridsGroup.h"
#include "osg/osgGrid/AlirtGrid.h"

// class osg::BoundingBox;

class AlirtGridsGroup : public GridsGroup
{

  public:

// Initialization, constructor and destructor functions:

   AlirtGridsGroup(const int p_ndims,Pass* PI_ptr);
   virtual ~AlirtGridsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const AlirtGridsGroup& L);

// Set & get methods:

   AlirtGrid* get_Grid_ptr(int n) const;
   AlirtGrid* get_ID_labeled_Grid_ptr(int ID) const;

// Grid creation methods:

   AlirtGrid* generate_new_Grid(bool wopillc=false);
   AlirtGrid* generate_new_Grid(colorfunc::Color c,bool wopillc=false);
   AlirtGrid* generate_new_Grid(
      double min_east,double max_east,
      double min_north,double max_north,double min_Z,
      bool world_origin_precisely_in_lower_left_corner=false);
   void initialize_grid(
      AlirtGrid* alirtgrid_ptr,const osg::BoundingBox& bbox,
      Grid::Distance_Scale distance_scale=Grid::meter,
      double delta_s=-1,double magnification_factor=1);
   void initialize_grid(
      AlirtGrid* alirtgrid_ptr,
      double min_X,double max_X,double min_Y,double max_Y,double min_Z,
      Grid::Distance_Scale distance_scale=Grid::meter);

   void update_display();

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const AlirtGridsGroup& G);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline AlirtGrid* AlirtGridsGroup::get_Grid_ptr(int n) const
{
   return dynamic_cast<AlirtGrid*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline AlirtGrid* AlirtGridsGroup::get_ID_labeled_Grid_ptr(int ID) const
{
   return dynamic_cast<AlirtGrid*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // AlirtGridsGroup.h



