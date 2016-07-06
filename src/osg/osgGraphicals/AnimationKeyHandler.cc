// ==========================================================================
// AnimationKeyHandler class member function definitions
// ==========================================================================
// Last modified on 12/18/06; 2/9/07; 5/2/08; 7/6/16
// ==========================================================================

#include <iostream>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "general/inputfuncs.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AnimationKeyHandler::allocate_member_objects()
{
}		       

void AnimationKeyHandler::initialize_member_objects()
{
}		       

AnimationKeyHandler::AnimationKeyHandler( 
   ModeController* MC_ptr,AnimationController* AC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   ModeController_ptr=MC_ptr;
   AnimationController_ptr=AC_ptr;
}

// ------------------------------------------------------

bool AnimationKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ModeController_ptr->getState() == ModeController::RUN_MOVIE ||
       ModeController_ptr->getState() == ModeController::MANIPULATE_MOVIE ||
       ModeController_ptr->getState() == 
       ModeController::GENERATE_AVI_MOVIE || 
       ModeController_ptr->getState() == ModeController::INSERT_FEATURE ||
       ModeController_ptr->getState() == ModeController::TRACK_FEATURE)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {

// Press right arrow to advance to next frame:

            case osgGA::GUIEventAdapter::KEY_Right :

               AnimationController_ptr->setState( 
                  AnimationController::INCREMENT_FRAME );
               return true;
               break;
               
// Press left arrow to move back to previous image:

            case osgGA::GUIEventAdapter::KEY_Left :

               AnimationController_ptr->setState( 
                  AnimationController::DECREMENT_FRAME );
               return true;
               break;
	    
// Press 'p' to toggle between play and pause modes:

            case 'p' :
               if (AnimationController_ptr->getState()==
                   AnimationController::PLAY)
               {
                  AnimationController_ptr->setState( 
                     AnimationController::PAUSE );
               }
               else
               {
                  AnimationController_ptr->setState( 
                     AnimationController::PLAY );
               }
               return true;
               break;

            case 'r' :
               AnimationController_ptr->setState(
                  AnimationController::REVERSE);
               return true;
               break;
	       
            case 'b' :	// Restart animation at beginning frame

               AnimationController_ptr->set_curr_framenumber(
                  AnimationController_ptr->get_first_framenumber());
               
               return true;
               break;
               
            case 'g' :
               AnimationController_ptr->setState(AnimationController::PAUSE);

               AnimationController_ptr->set_curr_framenumber(
                  inputfunc::enter_nonnegative_integer(
                     "Enter image number to goto:"));
               return true;
               break;

            case 'j' :
               AnimationController_ptr->setState(
                  AnimationController::JUMP_FORWARD);
               return true;
               break;

            case 'k' :
               AnimationController_ptr->setState(
                  AnimationController::JUMP_BACKWARD);
               return true;
               break;

            case '=':
               AnimationController_ptr->increment_frame_skip();
               cout << "Skip between animation frames = " 
                    << AnimationController_ptr->get_frame_skip()
                    << endl;
               return true;
               break;

            case '-':
               AnimationController_ptr->decrement_frame_skip();
               cout << "Skip between animation frames = " 
                    << AnimationController_ptr->get_frame_skip()
                    << endl;
               return true;
               break;

            case '+':
               AnimationController_ptr->increment_delay();
               cout << "Animation delay value = " 
                    << AnimationController_ptr->getDelay()
                    << endl;
               return true;
               break;

            case '_':
               AnimationController_ptr->decrement_delay();
               cout << "Animation delay value = " 
                    << AnimationController_ptr->getDelay()
                    << endl;
               return true;
               break;

         } // switch ( ea.getKey() )
      } // ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN conditional
   } // Mode = RUN_MOVIE, MANIPULATE_MOVIE, GENERATE_AVI_MOVIE or
     //  TRACK_FEATURE conditional

/*
   if (ModeController_ptr->getState() == ModeController::VIEW_DATA ||
       ModeController_ptr->getState() == ModeController::RUN_MOVIE ||
       ModeController_ptr->getState() == 
       ModeController::GENERATE_AVI_MOVIE)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {
            case 'r' :
               AnimationController_ptr->setState(
                  AnimationController::REVERSE);
               return true;
               break;
         }
      }
   }
*/

/*
   if ((ModeController_ptr->getState() == ModeController::RUN_MOVIE ||
        ModeController_ptr->getState() == 
        ModeController::GENERATE_AVI_MOVIE))
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {

            case 'c' :	// Display current cumulative image counter value
			//  Then reset it to zero
               AnimationController_ptr->set_cumulative_framecounter(0);
               return true;
               break;
         } // switch ( ea.getKey() )
      } // ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN conditional
   } // Mode = RUN_MOVIE or GENERATE_AVI_MOVIE conditional
*/

   return false;
}


