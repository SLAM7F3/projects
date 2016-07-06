// ==========================================================================
// AnnotatorPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 11/1/06; 12/29/06; 1/21/07
// ==========================================================================

#include <iostream>
#include "osg/osgAnnotators/AnnotatorPickHandler.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgGraphicals/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AnnotatorPickHandler::allocate_member_objects()
{
}		       

void AnnotatorPickHandler::initialize_member_objects()
{
}		       

AnnotatorPickHandler::AnnotatorPickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr,Transformer* trans_ptr,threevector* GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
}

AnnotatorPickHandler::~AnnotatorPickHandler() 
{
}
