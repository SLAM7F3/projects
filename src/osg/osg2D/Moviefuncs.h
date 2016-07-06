// ==========================================================================
// Header file for MOVIEFUNCS namespace
// ==========================================================================
// Last modified on 11/24/06
// ==========================================================================

#ifndef MOVIEFUNCS_H
#define MOVIEFUNCS_H

// class osg::Projection;
class AnimationController;

namespace Moviefunc
{
   osg::Projection* create_Imagenumber_HUD(
      AnimationController* AnimationController_ptr,
      bool display_movie_state,bool display_movie_number);

} // Moviefunc namespace

#endif // Moviefuncs.h



