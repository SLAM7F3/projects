// ==========================================================================
// Header file for ReferenceFrameHUD class which displays an
// ReferenceFrame message
// ==========================================================================
// Last modified on 10/15/11
// ==========================================================================

#ifndef ReferenceFrameHUD_H
#define ReferenceFrameHUD_H

#include "osg/GenericHUD.h"

class ReferenceFrameHUD : public GenericHUD
{
  public:

   enum frameType
   {
      FREE_FRAME=0, AIRCRAFT_FRAME=1, NORTH_UP_FRAME=2
   };
   

   ReferenceFrameHUD();
   
   void set_frame_type(frameType curr_frame_type);
   void showHUD();

// Set & get member functions:

  protected:

  private:

   frameType reference_frame_type;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ReferenceFrameHUD::set_frame_type(frameType curr_frame_type)
{
   reference_frame_type=curr_frame_type;
}


#endif 
