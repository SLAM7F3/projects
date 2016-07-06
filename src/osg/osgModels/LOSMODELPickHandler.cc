// ==========================================================================
// LOSMODELPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 9/19/09
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osgModels/LOSMODELPickHandler.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LOSMODELPickHandler::allocate_member_objects()
{
}		       

void LOSMODELPickHandler::initialize_member_objects()
{
}		       

LOSMODELPickHandler::LOSMODELPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,LOSMODELSGROUP* LMG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   MODELPickHandler(PI_ptr,CM_ptr,LMG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   LOSMODELSGROUP_ptr=LMG_ptr;
}

LOSMODELPickHandler::~LOSMODELPickHandler() 
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool LOSMODELPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//    cout << "inside SPPH::pick()" << endl;

   return MODELPickHandler::pick(ea);
}

// --------------------------------------------------------------------------
bool LOSMODELPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   return MODELPickHandler::drag(ea);
}

// --------------------------------------------------------------------------
bool LOSMODELPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside LOSMODELPickHandler::doubleclick()" << endl;
   return MODELPickHandler::doubleclick(ea);
}

// --------------------------------------------------------------------------
bool LOSMODELPickHandler::release()
{
   return MODELPickHandler::release();
}

// ==========================================================================
// Model generation, manipulation and annihilation methods
// ==========================================================================
