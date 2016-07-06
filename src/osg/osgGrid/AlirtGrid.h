// ========================================================================
// Header file for ALIRTGRID class
// ========================================================================
// Last updated on 12/30/07; 6/9/10; 3/15/13
// ========================================================================

#ifndef ALIRT_GRID_H
#define ALIRT_GRID_H

#include "color/colorfuncs.h"
#include "osg/osgGrid/Grid.h"

// class osg::BoundingBox;

class AlirtGrid : public Grid
{
  public:
        
   AlirtGrid(bool wopillc_flag=false,int ndims=3,int ID=-1);
   AlirtGrid(colorfunc::Color c,
             bool wopillc_flag=false,int ndims=3,int ID=-1);
   virtual ~AlirtGrid();
   virtual void update_grid();

   void initialize_ALIRT_grid(
      const osg::BoundingBox& bbox,
      Grid::Distance_Scale distance_scale=meter,double delta_s=-1,
      double magnification_factor=1);
   void initialize_ALIRT_grid(
      double min_X,double max_X,double min_Y,double max_Y,double min_Z,
      Grid::Distance_Scale distance_scale=meter,double delta_s=-1,
      double magnification_factor=1);
   void initialize_ALIRT_grid(
      double min_X,double max_X,double min_Y,double max_Y,double min_Z,
      double max_Z,Grid::Distance_Scale distance_scale=meter,
      double delta_s=-1,double magnification_factor=1);

   void initialize_satellite_grid(bool SPASE_flag);

  private:

   bool world_origin_precisely_in_lower_left_corner;

   void allocate_member_objects();
   void initialize_member_objects();
};

#endif
