// ==========================================================================
// Header file for ANNOTATORPICKHANDLER class
// ==========================================================================
// Last modfied on 11/1/06; 12/29/06; 1/3/07; 1/21/07
// ==========================================================================

#ifndef ANNOTATOR_PICK_HANDLER_H
#define ANNOTATOR_PICK_HANDLER_H

#include "osg/CustomManipulator.h"

class Pass;
class GraphicalsGroup;
class ModeController;
class threevector;
class Transformer;
class WindowManager;

class AnnotatorPickHandler 
{

  public: 

   AnnotatorPickHandler(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
      WindowManager* WCC_ptr,
      Transformer* trans_ptr=NULL,threevector* GO_ptr=NULL);

// Set and get methods:

// Annotator generation, manipulation and annihiilation methods:

  protected:

   virtual ~AnnotatorPickHandler();

// Mouse event handling methods:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif // AnnotatorPickHandler.h



