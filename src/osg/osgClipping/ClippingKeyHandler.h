// ==========================================================================
// ClippingKeyHandler header file 
// ==========================================================================
// Last modified on 2/28/12
// ==========================================================================

#ifndef CLIPPINGKEYHANDLER_H
#define CLIPPINGKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class Clipping;
class ModeController;

class ClippingKeyHandler : public GraphicalsKeyHandler
{
  public:

   ClippingKeyHandler(Clipping* C_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ClippingKeyHandler();

  private:

   Clipping* Clipping_ptr;
   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
