// ==========================================================================
// AnimationKeyHandler header file 
// ==========================================================================
// Last modified on 11/2/06
// ==========================================================================

#ifndef ANIMATIONKEYHANDLER_H
#define ANIMATIONKEYHANDLER_H

#include <osgGA/GUIEventHandler>

class AnimationController;
class ModeController;

class AnimationKeyHandler : public osgGA::GUIEventHandler
{
  public:

   AnimationKeyHandler(
      ModeController* MC_ptr,AnimationController* AC_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );
  
  protected:

   ModeController* ModeController_ptr;
   AnimationController* AnimationController_ptr; 

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
