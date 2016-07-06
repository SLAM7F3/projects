// ==========================================================================
// ImageNumberHUD class member function definitions
// ==========================================================================
// Last modified on 7/20/10; 7/23/10; 12/31/11; 4/2/14
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/AbstractOSGCallback.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;

void ImageNumberHUD::allocate_member_objects()
{
}

void ImageNumberHUD::initialize_member_objects()
{
   setDefaultPosition();
   setAlignment(osgText::Text::LEFT_BASE_LINE);

//   osgText::Text::BackdropType bdt=osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
   osgText::Text::BackdropType bdt=osgText::Text::OUTLINE;
   set_text_backdrop_type(bdt);
   display_UTC_flag=true;
}

ImageNumberHUD::ImageNumberHUD(
   AnimationController* AC_ptr,
   bool display_movie_number,bool display_movie_state,
   bool display_movie_world_time,bool display_movie_elapsed_time):
   GenericHUD( 0, 1280, 0, 1024 )
{
   allocate_member_objects();
   initialize_member_objects();
   
   AnimationController_ptr=AC_ptr;
   m_display_movie_number=display_movie_number;
   m_display_movie_state=display_movie_state;
   m_display_movie_world_time=display_movie_world_time;
   m_display_movie_elapsed_time=display_movie_elapsed_time;
   if (m_display_movie_world_time) m_display_movie_elapsed_time=false;

   getProjection()->setUpdateCallback( 
      new AbstractOSGCallback<ImageNumberHUD>(
         this, &ImageNumberHUD::showFrame) );
}

// --------------------------------------------------------------------------
void ImageNumberHUD::showFrame()
{
//   cout << "inside ImageNumberHUD::showFrame()" << endl;
   string HUD_string="";

   if (m_display_movie_state)
   {
      switch (AnimationController_ptr->getState())
      {
         case 0: 
            HUD_string="PAUSE: ";
            break;
         case 1:
            HUD_string="PLAY: ";
            break;
         case 2:
            HUD_string="REVERSE: ";
            break;
         case 3:
            HUD_string="INCREMENT: ";
            break;
         case 4:
            HUD_string="DECREMENT: ";
            break;
         default:
            break;
      }
   }

   if (m_display_movie_number)
   {
      HUD_string += "Frame "+stringfunc::number_to_string(
         AnimationController_ptr->get_true_framenumber());
   }
   setText(HUD_string);

   if (m_display_movie_world_time)
   {
      if (AnimationController_ptr->
          get_increment_time_rather_than_frame_number_flag())
      {
         AnimationController_ptr->get_clock_ptr()->
            current_local_time_and_UTC();
      }
      else
      {

// Recall trueframe# = currframe# + frame_counter_offset
// AC works internally with currframe# and NOT with trueframe#.
// World times are correlated with currframe# and NOT with trueframe#:

         AnimationController_ptr->get_time_corresponding_to_frame(
            AnimationController_ptr->get_curr_framenumber());
      }
      setText(AnimationController_ptr->get_world_time_string(
         display_UTC_flag),1);
   }
   else if (m_display_movie_elapsed_time)
   {
      double start_time = 
         AnimationController_ptr->get_time_corresponding_to_frame(
            AnimationController_ptr->get_first_framenumber());
      double curr_time = 
         AnimationController_ptr->get_time_corresponding_to_curr_frame();
      double elapsed_time = curr_time - start_time;
      setText("Elapsed secs = "+stringfunc::number_to_string(
                 elapsed_time,2),1);
   }
   else
   {
      setText("",1);
   }

   if (labels.size() > 0)
   {
      unsigned int frame=AnimationController_ptr->get_curr_framenumber();

      string curr_label="";
      if (frame >= 0 && frame <labels.size())
      {
         curr_label=labels[frame];
      }
      setText(curr_label,2);
   }
      
}

