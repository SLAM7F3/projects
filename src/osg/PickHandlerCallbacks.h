// ==========================================================================
// Header file for pure virtual PICKHANDLERCALLBACKS class
// ==========================================================================
// Last modified on 4/30/08
// ==========================================================================

#ifndef PICKHANDLERCALLBACKS_H
#define PICKHANDLERCALLBACKS_H

#include <string>

class PickHandlerCallbacks 
{
  public: 

   PickHandlerCallbacks();
   virtual ~PickHandlerCallbacks();

   virtual void display_selected_vehicle_webpage(std::string vehicle_label)=0;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // PickhandlerCallbacks.h



