// ==========================================================================
// FishnetsKeyHandler header file 
// ==========================================================================
// Last modified on 11/17/11
// ==========================================================================

#ifndef FISHNETS_KEYHANDLER_H
#define FISHNETS_KEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModeController;
class FishnetsGroup;

class FishnetsKeyHandler : public GraphicalsKeyHandler
{
  public:

   FishnetsKeyHandler(FishnetsGroup* FG_ptr,ModeController* MC_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

// Set & get member functions:
   
  protected:

   virtual ~FishnetsKeyHandler();

  private:

   FishnetsGroup* FishnetsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif // FISHNETSKEYHANDLER.h
