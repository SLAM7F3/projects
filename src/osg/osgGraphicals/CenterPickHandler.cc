// ==========================================================================
// CenterPickHandler class to interpret mouse picks as 2D and
// 3D worldspace events
// ==========================================================================
// Last modified on 6/27/07; 7/28/07; 8/17/07; 4/6/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "math/threevector.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cout;
using std::cin;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CenterPickHandler::allocate_member_objects()
{
}		       

void CenterPickHandler::initialize_member_objects()
{
}		       

CenterPickHandler::CenterPickHandler(
   const int p_ndims,Pass* PI_ptr,
   osgGA::CustomManipulator* CM_ptr,CentersGroup* CG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GraphicalPickHandler(p_ndims,PI_ptr,CM_ptr,CG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   CentersGroup_ptr=CG_ptr;
}

CenterPickHandler::~CenterPickHandler() 
{
}

// ==========================================================================
// Mouse pick event handling methods
// ==========================================================================

bool CenterPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::pick()" << endl;
   if (get_ModeController_ptr()->getState()==ModeController::SET_CENTER)
   {
      if (GraphicalPickHandler::pick(ea))
      {
// Reset picked 3D world-space point as the physical center of the 2D
// screen space:

         if (get_ndims()==2)
         {
            get_CustomManipulator_ptr()->set_worldspace_center(
               curr_voxel_worldspace_posn);
            get_CentersGroup_ptr()->reset_center(curr_voxel_worldspace_posn);
         }
         else if (get_ndims()==3)
         {
            threevector intercept_point;
//            bool nearby_point_found=
               get_CM_3D_ptr()->get_PointFinder_ptr()->
               find_closest_world_point(
                  get_CM_3D_ptr(),
                  ea.getX(),ea.getY(),intercept_point);
//            cout << "New center location wrt grid origin = "
//                 << intercept_point-get_grid_origin() << endl;

// Reset distance from eye to new worldspace center:

            double d_old=
               get_CustomManipulator_ptr()->get_eye_to_center_distance();
            threevector center_old=
               get_CustomManipulator_ptr()->get_worldspace_center();
            double d_new=d_old+(center_old-intercept_point).get(2);
            get_CustomManipulator_ptr()->set_eye_to_center_distance(d_new);

            get_CustomManipulator_ptr()->set_worldspace_center(
               intercept_point);
            get_CentersGroup_ptr()->reset_center(intercept_point);
            get_CentersGroup_ptr()->update_display();

         }
         return true;
      } // GPH conditional
   } // Mode==SET_CENTER conditional
   return false;
}

bool CenterPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   return false;
}

bool CenterPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::doubleclick()" << endl;
   return false;
}

bool CenterPickHandler::release()
{
   return (get_ModeController_ptr()->getState()==ModeController::SET_CENTER);
}

float CenterPickHandler::get_max_distance_to_Graphical()
{
   return 0;
}

// --------------------------------------------------------------------------
// Member function edit_center_location queries the user to enter the
// latitude and longitude of the center point.  It then resets the
// camera's location to the UTM easting and northing corresponding to
// the input (latitude,longitude) pair, and reorients the camera so
// that it faces straight down the Grid's Z axis.

void CenterPickHandler::edit_center_location()
{
   double lat_degs,lat_mins,lat_secs;
   cout << "Latitude of center location:" << endl << endl;
   cout << "Enter degrees:" << endl;
   cin >> lat_degs;
   cout << "Enter minutes:" << endl;
   cin >> lat_mins;
   cout << "Enter seconds:" << endl;
   cin >> lat_secs;
   double latitude=latlongfunc::dms_to_decimal_degs(
      lat_degs,lat_mins,lat_secs);

   double long_degs,long_mins,long_secs;
   cout << endl;
   cout << "WEST Longitude of center location:" << endl << endl;
   cout << "Enter degrees:" << endl;
   cin >> long_degs;
   cout << "Enter minutes:" << endl;
   cin >> long_mins;
   cout << "Enter seconds:" << endl;
   cin >> long_secs;
   double longitude=latlongfunc::dms_to_decimal_degs(
      -long_degs,-long_mins,-long_secs);

//   double latitude=42.3616;
//   double longitude=-71.0749;

   geopoint curr_geopoint=latlongfunc::compute_geopoint( 
      latitude,longitude,get_grid_origin().get(2));
//   cout << "geopoint = " << curr_geopoint << endl;
   threevector intercept_point(
      curr_geopoint.get_UTM_easting(),
      curr_geopoint.get_UTM_northing(),
      curr_geopoint.get_altitude());
               
   get_CustomManipulator_ptr()->set_worldspace_center(
      intercept_point);
   get_CustomManipulator_ptr()->reset_nadir_view();
   
   get_CentersGroup_ptr()->reset_center(intercept_point);
//   get_ModeController_ptr()->set_prev_State();
}
