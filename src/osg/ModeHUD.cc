// ==========================================================================
// ModeHUD class member function definitions
// ==========================================================================
// Last modified on 10/11/05; 8/4/06; 10/5/07; 10/14/07
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/ModeController.h"
#include "osg/ModeHUD.h"

using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ModeHUD::allocate_member_objects()
{
}

void ModeHUD::initialize_member_objects()
{
   ModeController_ptr=NULL;
}

ModeHUD::ModeHUD(ModeController* MC_ptr,bool hide_Mode_HUD_flag):
   GenericHUD( -640, 640, 0, 1024 )
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   this->hide_Mode_HUD_flag=hide_Mode_HUD_flag;

   setModeCharacterSize();
   setAlignment(osgText::Text::CENTER_BOTTOM );
   setPosition(getPosition() + osg::Vec3(0, 15, 0));
};

ModeHUD::~ModeHUD()
{
}

// --------------------------------------------------------------------------
void ModeHUD::showMode()
{
   ModeController::eState curr_state=ModeController_ptr->getState();
   if (curr_state != ModeController::GENERATE_AVI_MOVIE &&
       !hide_Mode_HUD_flag)
   {
      setText(ModeController_ptr->get_state_name());
   }
   else
   {
      setText("");
   }
//   setCharacterSize( float( 20.0*(1+0.5*m_windowScaleFrac) ) );
}


