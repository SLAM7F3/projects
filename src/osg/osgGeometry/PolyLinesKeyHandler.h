// ==========================================================================
// PolyLinesKeyHandler header file 
// ==========================================================================
// Last modified on 1/17/07; 5/22/11
// ==========================================================================

#ifndef POLYLINESKEYHANDLER_H
#define POLYLINESKEYHANDLER_H

#include "osg/osgGeometry/GeometricalsKeyHandler.h"

class ModeController;
class PolyLinesGroup;

class PolyLinesKeyHandler : public GeometricalsKeyHandler
{

  public:

   PolyLinesKeyHandler(PolyLinesGroup* PLG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~PolyLinesKeyHandler();

  private:

   PolyLinesGroup* PolyLinesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
