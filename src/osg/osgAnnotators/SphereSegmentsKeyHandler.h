// ==========================================================================
// SphereSegmentsKeyHandler header file 
// ==========================================================================
// Last modified on 6/25/06; 1/3/07; 5/10/10
// ==========================================================================

#ifndef SPHERESEGMENTSKEYHANDLER_H
#define SPHERESEGMENTSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class SphereSegmentsGroup;
class ModeController;

class SphereSegmentsKeyHandler : public GraphicalsKeyHandler
{
  public:

   SphereSegmentsKeyHandler(SphereSegmentsGroup* SSG_ptr,
                            ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~SphereSegmentsKeyHandler();

  private:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
