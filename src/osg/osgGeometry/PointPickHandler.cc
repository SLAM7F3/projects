// ==========================================================================
// PointPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 9/2/08; 12/17/09; 1/1/11
// ==========================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <set>
#include <vector>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgGeometry/Point.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PointPickHandler.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PointPickHandler::allocate_member_objects()
      {
      }		       

   void PointPickHandler::initialize_member_objects()
      {
      }		       

   PointPickHandler::PointPickHandler(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      PointsGroup* PG_ptr,ModeController* MC_ptr,
      WindowManager* WCC_ptr,threevector* GO_ptr):
      GeometricalPickHandler(p_ndims,PI_ptr,CM_ptr,PG_ptr,MC_ptr,WCC_ptr,
                             GO_ptr)
      {
         allocate_member_objects();
         initialize_member_objects();
         PointsGroup_ptr=PG_ptr;
      }

   PointPickHandler::~PointPickHandler() 
      {
      }

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

   bool PointPickHandler::pick(const osgGA::GUIEventAdapter& ea)
   {
//      cout <<  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//      cout << "inside PointPickHandler::pick()" << endl;
      ModeController::eState curr_state=get_ModeController_ptr()->
         getState();
      if (curr_state==ModeController::INSERT_POINT ||
          curr_state==ModeController::MANIPULATE_POINT )
      {
         if (GraphicalPickHandler::pick(ea))
         {
            if (curr_state==ModeController::INSERT_POINT)
            {
               return instantiate_point();
            }
            else if (curr_state==ModeController::MANIPULATE_POINT)
            {
               return select_point();
            }
         }
      } // curr_state==INSERT_POINT || MANIPULATE_POINT conditional
   
      return false;
   }

// --------------------------------------------------------------------------
bool PointPickHandler::drag(const osgGA::GUIEventAdapter& ea)
      {
//         cout <<  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//         cout << "inside PointPickHandler::drag()" << endl;
         ModeController::eState curr_state=get_ModeController_ptr()->
            getState();
         if (curr_state==ModeController::INSERT_POINT ||
             curr_state==ModeController::MANIPULATE_POINT )
         {
            return GraphicalPickHandler::drag(ea);
         }
         else
         {
            return false;
         }
      }

// --------------------------------------------------------------------------
   bool PointPickHandler::release()
      {
//         cout <<  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//         cout << "inside PointPickHandler::release()" << endl;

         ModeController::eState curr_state=get_ModeController_ptr()->
            getState();

         if (curr_state==ModeController::INSERT_POINT ||
             curr_state==ModeController::MANIPULATE_POINT )
         {
            get_PointsGroup_ptr()->reset_colors();
//   cout << "selected point number = "
//        << get_selected_Graphical_ID() << endl;
            return true;
         }
         else
         {
            return false;
         }
      }

// ==========================================================================
// Point generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_point creates a new point, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_point_number equal to that ID and adds it to
// the OSG point group.

   bool PointPickHandler::instantiate_point()
      {   
//         cout << "inside PointPickHandler::instantiate_point()" << endl;
         Point* curr_Point_ptr=get_PointsGroup_ptr()->generate_new_Point();
         instantiate_Graphical(curr_Point_ptr);

         return true;
      }

// --------------------------------------------------------------------------
// Method select_point assigns selected_point_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no point is nearby the
// selected point, selected_point_number is set equal to -1, and all
// points are effectively de-selected.

   bool PointPickHandler::select_point()
      {   
//         cout << "inside PointPickHandler::select_point()" << endl;
         int selected_point_ID=select_Graphical();
         get_PointsGroup_ptr()->reset_colors();
         return (selected_point_ID > -1);
      }

} // osgGeometry namespace
