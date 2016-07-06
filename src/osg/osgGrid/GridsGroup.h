// ==========================================================================
// Header file for GRIDSGROUP class
// ==========================================================================
// Last modified on 11/21/06; 7/7/07
// ==========================================================================

#ifndef GRIDSGROUP_H
#define GRIDSGROUP_H

#include <iostream>
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgGrid/Grid.h"

class GridsGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   GridsGroup(const int p_ndims,Pass* PI_ptr);
   virtual ~GridsGroup();

// Set & get methods:

  protected:

   void initialize_new_Grid(Grid* Grid_ptr,int OSGsubPAT_number=0);

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const GridsGroup& G);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // GridsGroup.h



