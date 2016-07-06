// ==========================================================================
// PointsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 9/23/06; 9/24/06; 6/21/07
// ==========================================================================

#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PointsKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PointsKeyHandler::allocate_member_objects()
      {
      }

   void PointsKeyHandler::initialize_member_objects()
      {
         PointsGroup_ptr=NULL;
      }

   PointsKeyHandler::PointsKeyHandler(
      const int p_ndims,PointsGroup* PG_ptr,ModeController* MC_ptr):
      GraphicalsKeyHandler(PG_ptr,MC_ptr)
      {
         allocate_member_objects();
         initialize_member_objects();
         ndims=p_ndims;
         PointsGroup_ptr=PG_ptr;
      }

   PointsKeyHandler::~PointsKeyHandler()
      {
      }

// ------------------------------------------------------
   bool PointsKeyHandler::handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
      {
         if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
         {

            if (get_ModeController_ptr()->getState()==
                ModeController::MANIPULATE_POINT)
            {

// Press "s" to save feature information to ascii text file:

               if (ea.getKey()=='s')
               {
                  get_PointsGroup_ptr()->save_point_info_to_file();
                  return true;
               }
      
// Press "r" to restore point information from ascii text file:
      
               else if (ea.getKey()=='r')
               {
                  get_PointsGroup_ptr()->read_point_info_from_file();
                  return true;
               }

// Press "e" to edit a point:

               else if (ea.getKey()=='e')
               {
                  get_PointsGroup_ptr()->edit_label();
                  return true;
               }

// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
               {
                  get_PointsGroup_ptr()->erase_point();
                  return true;
               }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
               {
                  get_PointsGroup_ptr()->unerase_point();
                  return true;
               }

// Press "Delete" key to completely destroy a point:

               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
               {
                  get_PointsGroup_ptr()->destroy_Point();
                  return true;
               }

// Press ">" ["<"] key to increase [decrease] a point's size:

               else if (ea.getKey()=='>')
               {
                  get_PointsGroup_ptr()->change_size(2.0);
                  return true;
               }
               else if (ea.getKey()=='<')
               {
                  get_PointsGroup_ptr()->change_size(0.5);
                  return true;
               }

// Press "Up arrow" or "Down arrow" to move a selected point up or
// down in the world-z direction:

               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
               {
                  get_PointsGroup_ptr()->move_z(1);
                  return true;
               }
               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
               {
                  get_PointsGroup_ptr()->move_z(-1);
                  return true;
               }
            } // mode = MANIPULATE_POINT conditional         
      
         } // key down conditional
   
         return false;
      }

} // osgGeometry namespace

