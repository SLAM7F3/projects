// ==========================================================================
// LineSegmentsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 9/25/06; 11/28/06; 11/22/07
// ==========================================================================

#include <string>
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgGeometry/LineSegmentsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LineSegmentsKeyHandler::allocate_member_objects()
{
}

void LineSegmentsKeyHandler::initialize_member_objects()
{
   LineSegmentsGroup_ptr=NULL;
}

LineSegmentsKeyHandler::LineSegmentsKeyHandler(
   LineSegmentsGroup* LSG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(LSG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   LineSegmentsGroup_ptr=LSG_ptr;
}

LineSegmentsKeyHandler::~LineSegmentsKeyHandler()
{
}

// ---------------------------------------------------------------------
LineSegmentsGroup* const LineSegmentsKeyHandler::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

// ------------------------------------------------------
bool LineSegmentsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_LINE)
      {

// Press "s" to save LineSegment information to ascii text file:

         if (ea.getKey()=='s')
         {
            get_LineSegmentsGroup_ptr()->save_info_to_file();
            return true;
         }

// Press "r" to restore LineSegment information from ascii text file:
      
         else if (ea.getKey()=='r')
         {
            string lines_filename;
            cout << "Enter linesegment text file name:" << endl;
            cin >> lines_filename;
            get_LineSegmentsGroup_ptr()->reconstruct_lines_from_file_info(
               lines_filename);
            return true;
         }

// Press "f" to draw view frustum:

         else if (ea.getKey()=='f')
         {
            get_LineSegmentsGroup_ptr()->draw_FOV_frustum();
            return true;
         }

/*
// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            get_LineSegmentsGroup_ptr()->erase_LineSegment();
            return true;
         }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
         {
            get_LineSegmentsGroup_ptr()->unerase_LineSegment();
            return true;
         }
*/

// Press "Delete" key to completely destroy a LineSegment:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_LineSegmentsGroup_ptr()->destroy_Graphical();
            return true;
         }

//         else if (ea.getKey()=='m')
//         {
//            get_LineSegmentsGroup_ptr()->collect_pixels_under_segments();
//            get_LineSegmentsGroup_ptr()->mark_pixels_under_segments();
//            get_LineSegmentsGroup_ptr()->compute_segment_pixel_gradients();
//            return true;
//         }
      } // mode = MANIPULATE_LINE conditional
   } // key down conditional
   
   return false;
}


