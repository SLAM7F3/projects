// ==========================================================================
// CylindersKeyHandler class member function definitions
// ==========================================================================
// Last modified on 1/24/07; 8/23/07; 2/26/08; 5/29/08; 5/30/08; 6/15/08
// ==========================================================================

#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgGeometry/CylindersKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CylindersKeyHandler::allocate_member_objects()
{
}

void CylindersKeyHandler::initialize_member_objects()
{
   CylinderPickHandler_ptr=NULL;
}

CylindersKeyHandler::CylindersKeyHandler(
   CylindersGroup* CG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(CG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   CylindersGroup_ptr=CG_ptr;
}

CylindersKeyHandler::~CylindersKeyHandler()
{
}

// ---------------------------------------------------------------------
CylindersGroup* const CylindersKeyHandler::get_CylindersGroup_ptr()
{
   return CylindersGroup_ptr;
}

// ------------------------------------------------------
bool CylindersKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

/*
      if (get_ModeController_ptr()->getState()==
          ModeController::INSERT_CYLINDER ||
          get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_CYLINDER)
      {
      }
*/
    
// ......................................................................

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_CYLINDER)
      {

// Press "s" to save Cylinder information to ascii text file:

         if (ea.getKey()=='s')
         {
            int track_label;
            cout << "Enter Bluegrass ground truth track label:" << endl;
            cin >> track_label;

            int selected_Cylinder_ID=CylindersGroup_ptr->
               find_Cylinder_ID_given_track_label(track_label);
            CylindersGroup_ptr->set_selected_Graphical_ID(
               selected_Cylinder_ID);

            CylinderPickHandler_ptr->select_Cylinder(selected_Cylinder_ID);
//            get_CylindersGroup_ptr()->save_info_to_file();
            return true;
         }

// Press "Delete" key to completely destroy a Cylinder:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_GraphicalsGroup_ptr()->destroy_Graphical();
            return true;
         }

// Press "Up arrow" or "Down arrow" to move a selected Cylinder up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_CylindersGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_CylindersGroup_ptr()->move_z(-1);
            return true;
         }

// Press "r" to restore Cylinder information from ascii text file:
      
//         else if (ea.getKey()=='r')
//         {
//            get_CylindersController_ptr()->read_Cylinders();
//            return true;
//         }

// Press "c" to change cylinder's color:

         else if (ea.getKey()=='c')
         {
            get_CylindersGroup_ptr()->change_color();
            return true;
         }

// Press ">" ["<"] key to increase [decrease] a cylinder's size:

         else if (ea.getKey()=='>')
         {
//            get_CylindersGroup_ptr()->change_size(1 , 1 , 2.0 , 1.1);
            get_CylindersGroup_ptr()->change_size(1 , 1 , 2.0 , 1.5);
//            get_CylindersGroup_ptr()->change_size(1 , 1 , 4.0 , 1.1);
            return true;
         }
         else if (ea.getKey()=='<')
         {
//            get_CylindersGroup_ptr()->change_size(1 , 1, 0.5 , 1.0/1.1);
            get_CylindersGroup_ptr()->change_size(1 , 1, 0.5 , 1.0/1.5);
//            get_CylindersGroup_ptr()->change_size(1 , 1, 0.25 , 1.0/1.1);
            return true;
         }

      } // getState conditional
   } // key down conditional
   
   return false;
}


