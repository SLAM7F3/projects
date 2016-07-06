// ==========================================================================
// TrianglesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 12/18/05; 7/13/06
// ==========================================================================

#include "osg/ModeController.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgGeometry/TrianglesKeyHandler.h"

using std::cout;
using std::cin;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TrianglesKeyHandler::allocate_member_objects()
{
}

void TrianglesKeyHandler::initialize_member_objects()
{
   TrianglesGroup_ptr=NULL;
}

TrianglesKeyHandler::TrianglesKeyHandler(
   TrianglesGroup* TG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(TG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   TrianglesGroup_ptr=TG_ptr;
}

TrianglesKeyHandler::~TrianglesKeyHandler()
{
}

// ---------------------------------------------------------------------
TrianglesGroup* const TrianglesKeyHandler::get_TrianglesGroup_ptr()
{
   return TrianglesGroup_ptr;
}

// ------------------------------------------------------
bool TrianglesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_TRIANGLE)
      {

// Press "s" to save Triangle information to ascii text file:

         if (ea.getKey()=='s')
         {
            get_TrianglesGroup_ptr()->save_info_to_file();
            return true;
         }

// Press "r" to restore Triangle information from ascii text file:
      
         else if (ea.getKey()=='r')
         {
            get_TrianglesGroup_ptr()->reconstruct_triangles_from_file_info();
            return true;
         }

/*
// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            get_TrianglesGroup_ptr()->erase_Triangle();
            return true;
         }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
         {
            get_TrianglesGroup_ptr()->unerase_Triangle();
            return true;
         }
*/

// Press "Delete" key to completely destroy a Triangle:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_TrianglesGroup_ptr()->destroy_Graphical();
            return true;
         }

// Press "g" key to generate grid sampled approximation to triangle
// network:

         else if (ea.getKey()=='g')
         {
            get_TrianglesGroup_ptr()->sample_zcoords_on_XYgrid();
            return true;
         }

      } // mode = MANIPULATE_TRIANGLE conditional
   } // key down conditional
   
   return false;
}


