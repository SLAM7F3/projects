// ==========================================================================
// TOCHUD class member function definitions
// ==========================================================================
// Last modified on 9/12/10
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/AbstractOSGCallback.h"
#include "color/colorfuncs.h"
#include "osg/osg2D/TOCHUD.h"


using std::cout;
using std::endl;
using std::string;

void TOCHUD::allocate_member_objects()
{
}

void TOCHUD::initialize_member_objects()
{
   setPosition(osg::Vec3(1000,980,0),0);
   setPosition(osg::Vec3(1000,940,0),1);
   setAlignment(osgText::Text::LEFT_BASE_LINE);

//   osgText::Text::BackdropType bdt=osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
   osgText::Text::BackdropType bdt=osgText::Text::OUTLINE;
   set_text_backdrop_type(bdt);
}

TOCHUD::TOCHUD():
   GenericHUD( 0, 1280, 0, 1024 )
{
   allocate_member_objects();
   initialize_member_objects();
   
   getProjection()->setUpdateCallback( 
      new AbstractOSGCallback<TOCHUD>(
         this, &TOCHUD::showHUD) );
}

// --------------------------------------------------------------------------
void TOCHUD::showHUD()
{
//   cout << "inside TOCHUD::showHUD()" << endl;

   setText(HUD_string0,0);
   setText(HUD_string1,1);
}

