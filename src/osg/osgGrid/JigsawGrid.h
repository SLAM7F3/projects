// ========================================================================
// Header file for Brad Boven's JIGSAWGRID class
// ========================================================================
// Last updated on 10/12/05; 6/13/06
// ========================================================================

#ifndef JIGSAW_GRID_H
#define JIGSAW_GRID_H

#include "osg/osgGrid/Grid2d.h"

class JigsawGrid : public Grid2d
{
  public:

   JigsawGrid::JigsawGrid();

   virtual ~JigsawGrid();
   virtual void update_grid();
};

#endif
