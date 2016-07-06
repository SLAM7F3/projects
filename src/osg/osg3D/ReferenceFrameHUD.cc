// ==========================================================================
// ReferenceFrameHUD class member function definitions
// ==========================================================================
// Last modified on 10/15/11
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/AbstractOSGCallback.h"
#include "color/colorfuncs.h"
#include "osg/osg3D/ReferenceFrameHUD.h"
#include "general/stringfuncs.h"

#include "math/threevector.h"

using std::cout;
using std::endl;
using std::string;

void ReferenceFrameHUD::allocate_member_objects()
{
}

void ReferenceFrameHUD::initialize_member_objects()
{
   reference_frame_type=FREE_FRAME;
 
   setAlignment(osgText::Text::LEFT_BASE_LINE);
//   set_text_color(colorfunc::orange);
   set_text_color(colorfunc::pink);

//   osgText::Text::BackdropType bdt=osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
   osgText::Text::BackdropType bdt=osgText::Text::OUTLINE;
   set_text_backdrop_type(bdt);
}

ReferenceFrameHUD::ReferenceFrameHUD():
   GenericHUD( 0, 1280, 0, 1024 )
{
   allocate_member_objects();
   initialize_member_objects();
   
   getProjection()->setUpdateCallback( 
      new AbstractOSGCallback<ReferenceFrameHUD>(
         this, &ReferenceFrameHUD::showHUD) );
}

// --------------------------------------------------------------------------
void ReferenceFrameHUD::showHUD()
{
//   cout << "inside ReferenceFrameHUD::showHUD()" << endl;

   if (reference_frame_type==FREE_FRAME)
   {
      setPosition(osg::Vec3(810,980,0));
      string HUD_string="FREE MANIPULATE FRAME";
      setText(HUD_string);
   }
   if (reference_frame_type==AIRCRAFT_FRAME)
   {
      setPosition(osg::Vec3(940,980,0));
      string HUD_string="AIRCRAFT FRAME";
      setText(HUD_string);
   }
   else if (reference_frame_type==NORTH_UP_FRAME)
   {
      setPosition(osg::Vec3(940,980,0));
      string HUD_string="NORTH-UP FRAME";
      setText(HUD_string);
   }
}

