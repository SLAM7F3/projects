// ==========================================================================
// Header file for modeHUD class 
// ==========================================================================
// Last modified on 10/5/07; 10/14/07; 4/13/10
// ==========================================================================

#ifndef MODEHUD_H
#define MODEHUD_H

#include <string>
#include "osg/GenericHUD.h"

class ModeController;

class ModeHUD : public GenericHUD
{
  public:

   ModeHUD(ModeController* MC_ptr,bool hide_Mode_HUD_flag=false);
   void showMode();

  protected:

   ~ModeHUD();
   ModeController* ModeController_ptr;

  private:

   bool hide_Mode_HUD_flag;
   void allocate_member_objects();
   void initialize_member_objects();
};


#endif 
