// ==========================================================================
// Moviefuncs namespace method definitions
// ==========================================================================
// Last modified on 11/24/06; 10/14/07
// ==========================================================================

#include <iostream>
#include <osg/Projection>
#include "osg/AbstractOSGCallback.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/osg2D/Moviefuncs.h"

using std::cout;
using std::endl;

namespace Moviefunc
{

// Method create_Imagenumber_HUD creates a Heads-Up-Display (HUD) for
// image number text display and set up its display callback:

   osg::Projection* create_Imagenumber_HUD(
      AnimationController* AnimationController_ptr,
      bool display_movie_state,bool display_movie_number)
      {
         ImageNumberHUD* movieHUD_ptr=new ImageNumberHUD( 
            AnimationController_ptr,display_movie_number,display_movie_state);
         osg::Projection* movie_projection_ptr=movieHUD_ptr->getProjection();
         movie_projection_ptr->setUpdateCallback( 
            new AbstractOSGCallback<ImageNumberHUD>(
               movieHUD_ptr, &ImageNumberHUD::showFrame) );
         return movie_projection_ptr;
      }
   
} // Moviefunc namespace

