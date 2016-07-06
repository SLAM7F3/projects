// ==========================================================================
// Header file for TOCHUD class which displays a TOC message
// ==========================================================================
// Last modified on 9/12/10
// ==========================================================================

#ifndef TOCHUD_H
#define TOCHUD_H

#include "osg/GenericHUD.h"

class TOCHUD : public GenericHUD
{
  public:

   TOCHUD();
   void set_string0(std::string curr_line);
   void set_string1(std::string curr_line);
   
   void showHUD();

// Set & get member functions:

  protected:

  private:

   std::string HUD_string0,HUD_string1;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void TOCHUD::set_string0(std::string curr_line)
{
   HUD_string0=curr_line;
}

inline void TOCHUD::set_string1(std::string curr_line)
{
   HUD_string1=curr_line;
}


#endif 
