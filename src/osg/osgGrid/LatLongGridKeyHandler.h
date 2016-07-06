// ==========================================================================
// LatLongGridKeyHandler header file 
// ==========================================================================
// Last modified on 3/12/09
// ==========================================================================

#ifndef LATLONGGRIDKEYHANDLER_H
#define LATLONGGRIDKEYHANDLER_H

#include "osg/osgGrid/GridKeyHandler.h"

class LatLongGrid;

class LatLongGridKeyHandler : public GridKeyHandler
{
  public:

   LatLongGridKeyHandler(ModeController* MC_ptr,LatLongGrid* G_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~LatLongGridKeyHandler();

  private:

   LatLongGrid* LatLongGrid_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
