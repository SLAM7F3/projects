// ==========================================================================
// MovieKeyHandler header file 
// ==========================================================================
// Last modified on 2/8/08; 4/1/09; 10/9/09
// ==========================================================================

#ifndef MOVIEKEYHANDLER_H
#define MOVIEKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class AnimationController;
class ModeController;
class Movie;
class MoviesGroup;

class MovieKeyHandler : public GraphicalsKeyHandler
{
  public:

   MovieKeyHandler(ModeController* p_mode);
   MovieKeyHandler(ModeController* p_mode,MoviesGroup* MG_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );
  
  protected:

   Movie* Movie_ptr;
   MoviesGroup* MoviesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
