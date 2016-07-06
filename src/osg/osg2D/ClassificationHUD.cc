// ==========================================================================
// ClassificationHUD class member function definitions
// ==========================================================================
// Last modified on 9/12/08; 10/4/08; 10/6/08
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/AbstractOSGCallback.h"
#include "color/colorfuncs.h"
#include "osg/osg2D/ClassificationHUD.h"
#include "general/stringfuncs.h"

#include "math/threevector.h"

using std::cout;
using std::endl;
using std::string;

void ClassificationHUD::allocate_member_objects()
{
}

void ClassificationHUD::initialize_member_objects()
{
   if (classification==PassesGroup::unclassified)
   {
      return;
   }
   else if (classification==PassesGroup::secret)
   {
      setPosition(osg::Vec3(1080,980,0));
      setPosition(osg::Vec3(m_margin,15,0),1);
   }
   else if (classification==PassesGroup::FOUO)
   {
      setPosition(osg::Vec3(760,980,0));
   }
   
   setAlignment(osgText::Text::LEFT_BASE_LINE);
   set_text_color(colorfunc::red);

//   osgText::Text::BackdropType bdt=osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
   osgText::Text::BackdropType bdt=osgText::Text::OUTLINE;
   set_text_backdrop_type(bdt);
}

ClassificationHUD::ClassificationHUD(PassesGroup::ClassificationType classification):
   GenericHUD( 0, 1280, 0, 1024 )
{
   this->classification=classification;

   allocate_member_objects();
   initialize_member_objects();
   
   getProjection()->setUpdateCallback( 
      new AbstractOSGCallback<ClassificationHUD>(
         this, &ClassificationHUD::showHUD) );
}

// --------------------------------------------------------------------------
void ClassificationHUD::showHUD()
{
//   cout << "inside ClassificationHUD::showHUD()" << endl;

   if (classification==PassesGroup::secret)
   {
      string HUD_string="SECRET";
      setText(HUD_string,0);
      setText(HUD_string,1);
   }
   else if (classification==PassesGroup::FOUO)
   {
      string HUD_string="FOUO-PRIVACY SENSITIVE";
      setText(HUD_string);
   }
}

