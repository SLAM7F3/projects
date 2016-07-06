// ==========================================================================
// DecorationsKeyHandler header file 
// ==========================================================================
// Last modified on 12/21/07; 5/27/09
// ==========================================================================

#ifndef DECORATIONSKEYHANDLER_H
#define DECORATIONSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class Decorations;
class ModeController;

class DecorationsKeyHandler : public GraphicalsKeyHandler
{
  public:

   DecorationsKeyHandler(Decorations* D_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~DecorationsKeyHandler();

  private:

   Decorations* Decorations_ptr;
   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
