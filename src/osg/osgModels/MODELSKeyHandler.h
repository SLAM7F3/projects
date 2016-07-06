// ==========================================================================
// MODELSKeyHandler header file 
// ==========================================================================
// Last modified on 10/31/06; 1/3/07; 7/20/07
// ==========================================================================

#ifndef NEW_MODELSKEYHANDLER_H
#define NEW_MODELSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class MODELSGROUP;
class ModeController;

class MODELSKeyHandler : public GraphicalsKeyHandler
{
  public:

   MODELSKeyHandler(MODELSGROUP* MG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ga);

  protected:

   virtual ~MODELSKeyHandler();

  private:

   MODELSGROUP* MODELSGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
