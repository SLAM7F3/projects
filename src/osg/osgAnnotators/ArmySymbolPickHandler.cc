// ==========================================================================
// ArmySymbolPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 6/15/08; 9/2/08; 8/13/09
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <osg/Object>
#include "osg/osgAnnotators/ArmySymbol.h"
#include "osg/osgAnnotators/ArmySymbolsGroup.h"
#include "osg/osgAnnotators/ArmySymbolPickHandler.h"
#include "osg/CustomManipulator.h"
#include "general/inputfuncs.h"
#include "osg/ModeController.h"
#include "osg/ModeController.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ArmySymbolPickHandler::allocate_member_objects()
{
}		       

void ArmySymbolPickHandler::initialize_member_objects()
{
}		       

ArmySymbolPickHandler::ArmySymbolPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ArmySymbolsGroup* ASG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   BoxPickHandler(PI_ptr,CM_ptr,ASG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ArmySymbolsGroup_ptr=ASG_ptr;
}

ArmySymbolPickHandler::~ArmySymbolPickHandler() 
{
}

// ---------------------------------------------------------------------
ArmySymbolsGroup* ArmySymbolPickHandler::get_ArmySymbolsGroup_ptr() 
{
   return ArmySymbolsGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool ArmySymbolPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ASPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_BOX, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Box whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_BOX)
         {
            return instantiate_ArmySymbol(ea.getX(),ea.getY());
         }
         else
         {
            return select_ArmySymbol();
          }
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   } // INSERT_BOX or MANIPULATE_BOX mode conditional
}

// --------------------------------------------------------------------------
bool ArmySymbolPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ArmySymbolPickHandler::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         ArmySymbol* curr_ArmySymbol_ptr=get_ArmySymbolsGroup_ptr()->
            get_ID_labeled_ArmySymbol_ptr(
               get_selected_Graphical_ID());
         get_ArmySymbolsGroup_ptr()->update_mybox(curr_ArmySymbol_ptr);
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool ArmySymbolPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ArmySymbolPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {
         return select_ArmySymbol();
      }
   }

   return false;
}

// ==========================================================================
// ArmySymbol generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_ArmySymbol creates a new ArmySymbol, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_ArmySymbol_number equal to that ID and adds it to
// the OSG ArmySymbol group.

bool ArmySymbolPickHandler::instantiate_ArmySymbol(double X,double Y)
{   
//   cout << "inside ASPH::instantiate_ArmySymbol" << endl;

   bool armysymbol_instantiated_flag=true;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      ArmySymbol* curr_ArmySymbol_ptr=ArmySymbolsGroup_ptr->
         generate_new_ArmySymbol();
      instantiate_Graphical(curr_ArmySymbol_ptr);

      int symbol_type=1;
      string label="Enter Army Symbol identified integer:";
      symbol_type=inputfunc::enter_nonnegative_integer(label);
//      cout << "symbol_type = " << symbol_type << endl;
      curr_ArmySymbol_ptr->set_symbol_type(symbol_type);

      ArmySymbolsGroup_ptr->generate_armysymbol_group(curr_ArmySymbol_ptr);

// Move instantiated box in the positive z direction by half its
// height so that its bottom just touches the world-grid:

      double t=get_ArmySymbolsGroup_ptr()->get_curr_t();
      int passnumber=get_ArmySymbolsGroup_ptr()->get_passnumber();
      threevector ArmySymbol_posn;
      if (curr_ArmySymbol_ptr->get_UVW_coords(t,passnumber,ArmySymbol_posn))
      {
         ArmySymbol_posn.put(2,ArmySymbol_posn.get(2)+
                             0.5*get_ArmySymbolsGroup_ptr()->get_height());
         curr_ArmySymbol_ptr->set_UVW_coords(t,passnumber,ArmySymbol_posn);
      }

      int face_number=4;	// Initially select top face
      threevector origin(0,0,0);
      curr_ArmySymbol_ptr->reset_selected_face_drawable(face_number,origin);

      get_ArmySymbolsGroup_ptr()->update_mybox(curr_ArmySymbol_ptr);
   }
   
   return armysymbol_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_ArmySymbol assigns selected_ArmySymbol_number equal
// to the ID of an existing ArmySymbol which lies sufficiently close
// to a point picked by the user with his mouse.  If no ArmySymbol is
// nearby the selected point, selected_ArmySymbol_number is set equal
// to -1, and all ArmySymbols are effectively de-selected.

bool ArmySymbolPickHandler::select_ArmySymbol()
{   
//   cout << "inside ArmySymbolPickHandler::select_ArmySymbol()" << endl;
   int ArmySymbol_ID=select_Graphical();
   ArmySymbolsGroup_ptr->reset_colors();
//   cout << "ArmySymbol_ID = " << ArmySymbol_ID << endl;
   return (ArmySymbol_ID > -1);
}
