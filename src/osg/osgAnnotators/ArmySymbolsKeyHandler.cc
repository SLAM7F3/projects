// ==========================================================================
// ArmySymbolsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 9/12/06; 10/30/06; 1/21/07
// ==========================================================================

#include "osg/osgAnnotators/ArmySymbolsGroup.h"
#include "osg/osgAnnotators/ArmySymbolsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ArmySymbolsKeyHandler::allocate_member_objects()
{
}

void ArmySymbolsKeyHandler::initialize_member_objects()
{
}

ArmySymbolsKeyHandler::ArmySymbolsKeyHandler(
   ArmySymbolsGroup* ASG_ptr,ModeController* MC_ptr):
   BoxesKeyHandler(ASG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ArmySymbolsGroup_ptr=ASG_ptr;
}

ArmySymbolsKeyHandler::~ArmySymbolsKeyHandler()
{
}

// ---------------------------------------------------------------------
ArmySymbolsGroup* const ArmySymbolsKeyHandler::get_ArmySymbolsGroup_ptr()
{
   return ArmySymbolsGroup_ptr;
}

// ------------------------------------------------------
bool ArmySymbolsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_BOX)
      {

// Press "Delete" key to completely destroy an ArmySymbol:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
//            get_ArmySymbolsGroup_ptr()->destroy_Graphical();
            get_ArmySymbolsGroup_ptr()->destroy_ArmySymbol();
            return true;
         }

// Press "s" to save Box information to ascii text file:

//         else if (ea.getKey()=='s')
//         {
//            get_ArmySymbolsGroup_ptr()->save_info_to_file();
//            return true;
//         }

// Press "r" to restore Box information from ascii text file:
      
//         else if (ea.getKey()=='r')
//         {
//            get_ArmySymbolsController_ptr()->read_ArmySymbols();
//            return true;
//         }

// Press "Up arrow" or "Down arrow" to move a selected Box up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_ArmySymbolsGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_ArmySymbolsGroup_ptr()->move_z(-1);
            return true;
         }
         
      } // mode = MANIPULATE_BOX conditional
   } // key down conditional
   
   return false;
}


