// ==========================================================================
// SphereSegmentsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 6/25/06; 5/10/10
// ==========================================================================

#include "osg/ModeController.h"
#include "osg/osgAnnotators/SphereSegmentsGroup.h"
#include "osg/osgAnnotators/SphereSegmentsKeyHandler.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SphereSegmentsKeyHandler::allocate_member_objects()
{
}

void SphereSegmentsKeyHandler::initialize_member_objects()
{
   SphereSegmentsGroup_ptr=NULL;
}

SphereSegmentsKeyHandler::SphereSegmentsKeyHandler(
   SphereSegmentsGroup* SSG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(SSG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   SphereSegmentsGroup_ptr=SSG_ptr;
}

SphereSegmentsKeyHandler::~SphereSegmentsKeyHandler()
{
}

// ------------------------------------------------------
bool SphereSegmentsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_HEMISPHERE)
      {

/*
// Press "s" to save SphereSegment information to ascii text file:

         if (ea.getKey()=='s')
         {
            SphereSegmentsGroup_ptr->save_info_to_file();
            return true;
         }

// Press "r" to restore SphereSegment information from ascii text file:
      
         else if (ea.getKey()=='r')
         {
            SphereSegmentsGroup_ptr->read_info_from_file();
            return true;
         }

// Press "e" to edit a SphereSegment:

         else if (ea.getKey()=='e')
         {
            SphereSegmentsGroup_ptr->edit_SphereSegment_label();
            return true;
         }
*/

/*
// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            SphereSegmentsGroup_ptr->erase_SphereSegment();
            return true;
         }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
         {
            SphereSegmentsGroup_ptr->unerase_SphereSegment();
            return true;
         }
*/

// Press "Delete" key to completely destroy a SphereSegment:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            SphereSegmentsGroup_ptr->destroy_Graphical();
            return true;
         }

/*
// Press ">" ["<"] key to increase [decrease] a SphereSegment's size:

         else if (ea.getKey()=='>')
         {
            SphereSegmentsGroup_ptr->change_size(2.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            SphereSegmentsGroup_ptr->change_size(0.5);
            return true;
         }
*/

// Press "Up arrow" or "Down arrow" to move a selected SphereSegment up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            SphereSegmentsGroup_ptr->move_z(2.0);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            SphereSegmentsGroup_ptr->move_z(-2.0);
            return true;
         }
         
      } // mode = MANIPULATE_HEMISPHERE conditional
   } // key down conditional
   
   return false;
}


