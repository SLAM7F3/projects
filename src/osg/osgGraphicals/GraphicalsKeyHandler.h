// ==========================================================================
// GraphicalsKeyHandler header file 
// ==========================================================================
// Last modified on 11/6/05; 12/7/05; 1/3/07
// ==========================================================================

#ifndef GRAPHICALSKEYHANDLER_H
#define GRAPHICALSKEYHANDLER_H

#include <osgGA/GUIEventHandler>

class GraphicalsGroup;
class ModeController;

class GraphicalsKeyHandler : public osgGA::GUIEventHandler
{
  public:

   GraphicalsKeyHandler(ModeController* MC_ptr);
   GraphicalsKeyHandler(GraphicalsGroup* GG_ptr,ModeController* MC_ptr);

   GraphicalsGroup* const get_GraphicalsGroup_ptr();
   ModeController* const get_ModeController_ptr();

  protected:

   virtual ~GraphicalsKeyHandler();

  private:

   ModeController* ModeController_ptr;
   GraphicalsGroup* GraphicalsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
