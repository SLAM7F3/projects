// ==========================================================================
// ModelsKeyHandler header file 
// ==========================================================================
// Last modified on 10/31/06; 1/3/07
// ==========================================================================

#ifndef MODELSKEYHANDLER_H
#define MODELSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModelsGroup;
class ModeController;

class ModelsKeyHandler : public GraphicalsKeyHandler
{
  public:

   ModelsKeyHandler(ModelsGroup* SPG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ModelsKeyHandler();

  private:

   ModelsGroup* ModelsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
