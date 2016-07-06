// ==========================================================================
// ROI_PolyhedraKeyHandler header file 
// ==========================================================================
// Last modified on 12/21/09
// ==========================================================================

#ifndef ROI_POLYHEDRAKEYHANDLER_H
#define ROI_POLYHEDRAKEYHANDLER_H

#include "osg/osgGeometry/PolyhedraKeyHandler.h"

class ModeController;
class ROI_PolyhedraGroup;

class ROI_PolyhedraKeyHandler : public PolyhedraKeyHandler
{

  public:

   ROI_PolyhedraKeyHandler(ROI_PolyhedraGroup* RPHG_ptr,
                           ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ROI_PolyhedraKeyHandler();

  private:

   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif 
