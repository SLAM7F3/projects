// ==========================================================================
// RectanglesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 11/2/06; 12/25/06; 1/21/07; 7/9/11
// ==========================================================================

#include "osg/osgGeometry/RectanglesGroup.h"
#include "osg/osgGeometry/RectanglesKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RectanglesKeyHandler::allocate_member_objects()
{
}

void RectanglesKeyHandler::initialize_member_objects()
{
   RectanglesGroup_ptr=NULL;
}

RectanglesKeyHandler::RectanglesKeyHandler(
   RectanglesGroup* RG_ptr,ModeController* MC_ptr):
   GeometricalsKeyHandler(RG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   RectanglesGroup_ptr=RG_ptr;
}

RectanglesKeyHandler::~RectanglesKeyHandler()
{
}

// ------------------------------------------------------
bool RectanglesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_RECTANGLE)
      {

// Press "s" to save Rectangle information to ascii text file:

         if (ea.getKey()=='s')
         {
            RectanglesGroup_ptr->save_info_to_file();
            return true;
         }

// Press "r" to restore Rectangle information from ascii text file:
      
//         else if (ea.getKey()=='r')
         else if (ea.getKey()=='t')
         {
            RectanglesGroup_ptr->read_info_from_file();
            return true;
         }

// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            RectanglesGroup_ptr->erase_Rectangle();
            return true;
         }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
         {
            RectanglesGroup_ptr->unerase_Rectangle();
            return true;
         }

// Press "Delete" key to completely destroy a Rectangle:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            RectanglesGroup_ptr->destroy_Graphical();
            return true;
         }

// Press "right arrow"/"left arrow" to increase/decrease Rectangle length:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
         {
            Rectangle* Rectangle_ptr=dynamic_cast<Rectangle*>(
               RectanglesGroup_ptr->rescale(0,1.05));
            Rectangle_ptr->reset_bbox(
               RectanglesGroup_ptr->get_curr_t(),
               RectanglesGroup_ptr->get_passnumber());
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
         {
            Rectangle* Rectangle_ptr=dynamic_cast<Rectangle*>(
               RectanglesGroup_ptr->rescale(0,0.95));
            Rectangle_ptr->reset_bbox(
               RectanglesGroup_ptr->get_curr_t(),
               RectanglesGroup_ptr->get_passnumber());
            return true;
         }

// Press "up arrow"/"down arrow" to increase/decrease Rectangle width:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            Rectangle* Rectangle_ptr=dynamic_cast<Rectangle*>(
               RectanglesGroup_ptr->rescale(1,1.05));
            Rectangle_ptr->reset_bbox(
               RectanglesGroup_ptr->get_curr_t(),
               RectanglesGroup_ptr->get_passnumber());
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            Rectangle* Rectangle_ptr=dynamic_cast<Rectangle*>(
               RectanglesGroup_ptr->rescale(1,0.95));
            Rectangle_ptr->reset_bbox(
               RectanglesGroup_ptr->get_curr_t(),
               RectanglesGroup_ptr->get_passnumber());
            return true;
         }

// Press "C" copy position, attitude and scale information from one
// particular image time to the current image time:

         else if (ea.getKey()=='c')
         {
            unsigned int orig_imagenumber;
            cout << "Enter number of image from which Rectangle postion"
                 << endl;
            cout << "orientation and scale information will be copied:"
                 << endl;
            cin >> orig_imagenumber;

            unsigned int start_copy_imagenumber,stop_copy_imagenumber;
            cout << "Enter starting image number for Rectangle cloning:" 
                 << endl;
            cin >> start_copy_imagenumber;
            cout << "Enter ending image number for Rectangle cloning:" 
                 << endl;
            cin >> stop_copy_imagenumber;

            RectanglesGroup_ptr->copy(
               orig_imagenumber,start_copy_imagenumber,stop_copy_imagenumber);
            return true;
         }
      } // mode = MANIPULATE_RECTANGLE conditional
   } // key down conditional
   
   return false;
}


