// ==========================================================================
// RegionPolyLinesKeyHandler header file 
// ==========================================================================
// Last modified on 12/13/08
// ==========================================================================

#ifndef REGIONPOLYLINESKEYHANDLER_H
#define REGIONPOLYLINESKEYHANDLER_H

#include "osg/osgGeometry/PolyLinesKeyHandler.h"

class ModeController;
class PolyLinesGroup;

class RegionPolyLinesKeyHandler : public PolyLinesKeyHandler
{

  public:

   RegionPolyLinesKeyHandler(PolyLinesGroup* PLG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& gaa);

  protected:

   virtual ~RegionPolyLinesKeyHandler();

  private:

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
