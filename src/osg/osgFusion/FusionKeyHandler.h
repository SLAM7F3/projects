// ==========================================================================
// FusionKeyHandler header file 
// ==========================================================================
// Last modified on 2/4/07; 2/5/07
// ==========================================================================

#ifndef FUSIONKEYHANDLER_H
#define FUSIONKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include <string>
#include "osg/osgFusion/FusionGroup.h"
#include "osg/ModeController.h"

class AnimationController;
class Movie;
class MoviesGroup;

class FusionKeyHandler : public osgGA::GUIEventHandler
{
  public:

   FusionKeyHandler(ModeController* MC_ptr,FusionGroup* FG_ptr);
   FusionKeyHandler(
    ModeController* MC_ptr,FusionGroup* FG_ptr,
    Movie* p_display,AnimationController* p_controller,MoviesGroup* MG_ptr);

   ModeController* const get_ModeController_ptr();
   FusionGroup* const get_FusionGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~FusionKeyHandler();

  private:

   FusionGroup* FusionGroup_ptr;
   ModeController* ModeController_ptr;

   AnimationController* m_controller; 
   Movie* m_display;   
   MoviesGroup* MoviesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   MoviesGroup* get_MoviesGroup_ptr();
   Movie* get_Movie_ptr();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ModeController* const FusionKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

// ---------------------------------------------------------------------
inline FusionGroup* const FusionKeyHandler::get_FusionGroup_ptr()
{
   return FusionGroup_ptr;
}

#endif 
