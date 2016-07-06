// ==========================================================================
// BoxesKeyHandler header file 
// ==========================================================================
// Last modified on 11/6/05; 12/6/05; 1/3/07
// ==========================================================================

#ifndef BOXESKEYHANDLER_H
#define BOXESKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class BoxesGroup;
class ModeController;

class BoxesKeyHandler : public GraphicalsKeyHandler
{
  public:

   BoxesKeyHandler(BoxesGroup* BG_ptr,ModeController* MC_ptr);

   BoxesGroup* const get_BoxesGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~BoxesKeyHandler();

  private:

   BoxesGroup* BoxesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
