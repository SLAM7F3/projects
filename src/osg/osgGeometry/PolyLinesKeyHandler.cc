// ==========================================================================
// PolyLinesKeyHandler class member function definitions.  
// ==========================================================================
// Last modified on 1/15/11; 5/22/11; 12/8/11; 7/6/16
// ==========================================================================

#include <string>
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinesKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyLinesKeyHandler::allocate_member_objects()
{
}

void PolyLinesKeyHandler::initialize_member_objects()
{
   PolyLinesGroup_ptr=NULL;
}

PolyLinesKeyHandler::PolyLinesKeyHandler(
   PolyLinesGroup* PLG_ptr,ModeController* MC_ptr):
   GeometricalsKeyHandler(PLG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PolyLinesGroup_ptr=PLG_ptr;
}

PolyLinesKeyHandler::~PolyLinesKeyHandler()
{
}

// ------------------------------------------------------
bool PolyLinesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_LINE ||
          get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_POLYLINE)
      {
//         cout << "ea.getKey() = " << ea.getKey() << endl;
         
//         if (ea.getKey()=='c')
//         {
//            PolyLinesGroup_ptr->clip_PolyLines();
//            return true;
//         }
         if (ea.getKey()=='d')
         {
            PolyLinesGroup_ptr->destroy_all_PolyLines();
            return true;
         }
         else if (ea.getKey()=='e')
         {
            bool flag=PolyLinesGroup_ptr->export_info_to_file();
            return flag;
         }
         else if (ea.getKey()=='i')
         {
            if (!Allow_Insertion_flag) return false;
            PolyLinesGroup_ptr->reconstruct_polylines_from_file_info();
            return true;
         }
//         else if (ea.getKey()=='i')
//         {
//            PolyLinesGroup_ptr->find_intersection_points();
//            return true;
//         }
         else if (ea.getKey()=='m')
         {
            PolyLinesGroup_ptr->merge_PolyLines(
               PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(0));

            return true;
         }
         else if (ea.getKey()=='p')
         {
            PolyLinesGroup_ptr->convert_polylines_to_polygons();
            return true;
         }
         else if (ea.getKey()=='r')
         {
            PolyLinesGroup_ptr->reconstruct_polylines_from_file_info();
            get_ModeController_ptr()->setState(
               ModeController::VIEW_DATA);
            return true;
         }
         else if (ea.getKey()=='t')
         {
            PolyLinesGroup_ptr->toggle_OSGgroup_nodemask();
            return true;
         }

         else if (ea.getKey()=='x')
         {
            PolyLinesGroup_ptr->read_OSG_file();            
            return true;
         }
         else if (ea.getKey()=='y')
         {
            PolyLinesGroup_ptr->write_OSG_file();            
            return true;
         }

// Press "Delete" key to completely destroy a PolyLine:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            int selected_Graphical_ID = PolyLinesGroup_ptr->
               get_selected_Graphical_ID();
            if(selected_Graphical_ID < 0) return true;
            PolyLinesGroup_ptr->destroy_PolyLine();

// Delete any Polygon which is associated with the just-deleted
// PolyLine:

            if(PolyLinesGroup_ptr->get_PolygonsGroup_ptr() != NULL)
            {
               osgGeometry::Polygon* selected_Polygon_ptr = 
                  PolyLinesGroup_ptr->get_PolygonsGroup_ptr()->
                  get_ID_labeled_Polygon_ptr(selected_Graphical_ID);
               selected_Polygon_ptr->set_PolyLine_ptr(NULL);
               PolyLinesGroup_ptr->get_PolygonsGroup_ptr()->destroy_Polygon(
                  selected_Graphical_ID);
            }
            return true;
         }
         
// Press Up [Down] arrow key to move among bounding boxes within
// current image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            PolyLinesGroup_ptr->increment_currimage_PolyLine();
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)            
         {
            PolyLinesGroup_ptr->decrement_currimage_PolyLine();
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
         {
            PolyLinesGroup_ptr->increment_frame();
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)            
         {
            PolyLinesGroup_ptr->decrement_frame();
            return true;
         }

//  Numberpad key values empirically found on 7/6/16 (for TM):

//  KEY_KP_0 = 65438
//  KEY_KP_1 = 65436
//  KEY_KP_2 = 65433 
//  KEY_KP_3 = 65435
//  KEY_KP_4 = 65430
//  KEY_KP_5 = 65437
//  KEY_KP_6 = 65432
//  KEY_KP_7 = 65429
//  KEY_KP_8 = 65431
//  KEY_KP_9 = 65434
         
         else if (ea.getKey()==65438)
         {
            PolyLinesGroup_ptr->set_all_PolyLine_attributes(0);
            return true;
         }
         else if (ea.getKey()==65436)
         {
            PolyLinesGroup_ptr->set_all_PolyLine_attributes(1);
            return true;
         }
         else if (ea.getKey()==65433)
         {
            PolyLinesGroup_ptr->set_all_PolyLine_attributes(2);
            return true;
         }

         else if (ea.getKey() == '0' || ea.getKey() == '`')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(0);
            return true;
         }
         else if (ea.getKey() == '1')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(1);
            return true;
         }
         else if (ea.getKey() == '2')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(2);
            return true;
         }
         else if (ea.getKey() == '3')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(3);
            return true;
         }
         else if (ea.getKey() == '4')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(4);
            return true;
         }
         else if (ea.getKey() == '5')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(5);
            return true;
         }
         else if (ea.getKey() == '6')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(6);
            return true;
         }
         else if (ea.getKey() == '7')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(7);
            return true;
         }
         else if (ea.getKey() == '8')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(8);
            return true;
         }
         else if (ea.getKey() == '9')
         {
            PolyLinesGroup_ptr->set_PolyLine_attribute(9);
            return true;
         }

      } // mode = MANIPULATE_LINE[POLYLINE] conditional

   } // key down conditional
   
   return false;
}


