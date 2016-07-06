// ==========================================================================
// ConesKeyHandler header file 
// ==========================================================================
// Last modified on 9/20/06; 1/3/07
// ==========================================================================

#ifndef CONESKEYHANDLER_H
#define CONESKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ConesGroup;
class ModeController;

class ConesKeyHandler : public GraphicalsKeyHandler
{
  public:

   ConesKeyHandler(ConesGroup* CG_ptr,ModeController* MC_ptr);

   ConesGroup* const get_ConesGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ConesKeyHandler();

  private:

   ConesGroup* ConesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
