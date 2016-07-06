// ========================================================================
// Header file for EARTHGRID class
// ========================================================================
// Last updated on 9/11/06; 11/22/06; 11/25/06
// ========================================================================

#ifndef EARTH_GRID_H
#define EARTH_GRID_H

#include "color/colorfuncs.h"
#include "osg/osgGrid/Grid.h"

class EarthGrid : public Grid
{
  public:
        
   EarthGrid(colorfunc::Color c);
   virtual ~EarthGrid();
   virtual void update_grid();

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

#endif
