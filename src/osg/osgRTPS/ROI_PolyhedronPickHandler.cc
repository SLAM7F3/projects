// ==========================================================================
// ROI_PolyhedronPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 12/21/09
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgRTPS/ROI_Polyhedron.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osgRTPS/ROI_PolyhedronPickHandler.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ROI_PolyhedronPickHandler::allocate_member_objects()
{
}		       

void ROI_PolyhedronPickHandler::initialize_member_objects()
{
   curr_ROI_Polyhedron_ptr=NULL;
}		       

ROI_PolyhedronPickHandler::ROI_PolyhedronPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ROI_PolyhedraGroup* RPG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   PolyhedronPickHandler(PI_ptr,CM_ptr,RPG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   ROI_PolyhedraGroup_ptr=RPG_ptr;
   allocate_member_objects();
   initialize_member_objects();
}

ROI_PolyhedronPickHandler::~ROI_PolyhedronPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool ROI_PolyhedronPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ROI_PolyhedronPickHandler::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_POLYHEDRON ||
       curr_state==ModeController::MANIPULATE_POLYHEDRON)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_POLYHEDRON, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the ROI_Polyhedron whose center lies closest to (X,Y)
// in screen space:

         if (curr_state==ModeController::INSERT_POLYHEDRON)
         {
            return instantiate_ROI_Polyhedron(ea.getX(),ea.getY());
         }
         else
         {
            return select_ROI_Polyhedron();
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
   } // INSERT_POLYHEDRON or MANIPULATE_POLYHEDRON mode conditional
}

// --------------------------------------------------------------------------
bool ROI_PolyhedronPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ROI_PolyhedronPickHandler::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_POLYHEDRON)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         curr_ROI_Polyhedron_ptr=ROI_PolyhedraGroup_ptr->
            get_ID_labeled_ROI_Polyhedron_ptr(get_selected_Graphical_ID());
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
bool ROI_PolyhedronPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ROI_PolyhedronPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

// As of 4/27/08, we experiment with allowing Bluegrass application
// users to double click on cylinders representing vehicle tracks in
// RUN_MOVIE mode:

   if (curr_state==ModeController::MANIPULATE_POLYHEDRON ||
       curr_state==ModeController::RUN_MOVIE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {
         return select_ROI_Polyhedron();
      }
   }

   return false;
}

// --------------------------------------------------------------------------
bool ROI_PolyhedronPickHandler::release()
{
//   cout << "inside ROI_PolyhedronPickHandler::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_POLYHEDRON ||
       curr_state==ModeController::MANIPULATE_POLYHEDRON)
   {
//      ROI_PolyhedraGroup_ptr->broadcast_bbox_corners();

// On 5/2/09, we experiment with resetting Mode to previous state
// after ROI_Polyhedron insertion is complete:

      get_ModeController_ptr()->setState(
         get_ModeController_ptr()->get_prev_State());

      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// ROI_Polyhedron generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_ROI_Polyhedron creates a new ROI_Polyhedron,
// assigns it a unique ID, associates an attendant text label's value
// equal to the ID, sets the selected_ROI_Polyhedron_number equal to
// that ID and adds it to the OSG ROI_Polyhedron group.

bool ROI_PolyhedronPickHandler::instantiate_ROI_Polyhedron(double X,double Y)
{   
//   cout << "inside ROI_PolyhedronPickHandler::instantiate_ROI_Polyhedron()" 
//        << endl;

   bool ROI_Polyhedron_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      int ROI_Polyhedra_subgroup=0;
      ROI_Polyhedron* curr_ROI_Polyhedron_ptr=
         ROI_PolyhedraGroup_ptr->generate_bbox(ROI_Polyhedra_subgroup);
      instantiate_Graphical(curr_ROI_Polyhedron_ptr);

      ROI_Polyhedron_instantiated_flag=true;
   }

   return ROI_Polyhedron_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_ROI_Polyhedron assigns selected_ROI_Polyhedron_number
// equal to the ID of an existing ROI_Polyhedron which lies
// sufficiently close to a 3D ray picked by the user with his mouse
// and eye position.  If no ROI_Polyhedron is nearby the selected
// point, selected_ROI_Polyhedron_number is set equal to -1, and all
// ROI_Polyhedra are effectively de-selected.

bool ROI_PolyhedronPickHandler::select_ROI_Polyhedron()
{   
   int selected_ROI_Polyhedron_ID=select_Graphical();
   return select_ROI_Polyhedron(selected_ROI_Polyhedron_ID);
}

bool ROI_PolyhedronPickHandler::select_ROI_Polyhedron(
   int selected_ROI_Polyhedron_ID)
{   

//   cout << "inside
//   ROI_PolyhedronPickHandler::select_ROI_Polyhedron()" << endl;
//   cout << "selected_ROI_Polyhedron_ID = " <<
//   selected_ROI_Polyhedron_ID << endl;
//   cout << "this = " << this << endl;


/*   
// First retrieve 3D ray corresponding to X & Y screen coords:

   threevector ray_basepoint=get_CM_3D_ptr()->get_eye_world_posn();
   threevector ray_ehat(get_CM_3D_ptr()->get_Transformer_ptr()->
                        compute_ray_into_screen(X,Y));
//   cout << "ray_basepoint = " << ray_basepoint << endl;
//   cout << "ray_ehat = " << ray_ehat << endl;

   int closest_polyline_ID=-1;
   vector<int> close_polyline_IDs;
   double minimal_squared_XY_distance_to_ray=POSITIVEINFINITY;
   const double minimal_distance_threshold=0.01;	// meter
   for (int n=0; n<ROI_PolyhedraGroup_ptr->get_n_Graphicals(); n++)
   {
      ROI_Polyhedron* ROI_Polyhedron_ptr=ROI_PolyhedraGroup_ptr->get_ROI_Polyhedron_ptr(n);
//      cout << "Polyline n = " << n 
//           << " Polyline_ptr = " << ROI_Polyhedron_ptr << endl;

      polyline* polyline_ptr=ROI_Polyhedron_ptr->get_polyline_ptr();
      if (polyline_ptr==NULL)
      {
         polyline_ptr=ROI_Polyhedron_ptr->construct_polyline();
         polyline_ptr->generate_sampled_edge_points_kdtree();
      }

      double z_plane=polyline_ptr->get_vertex(0).get(2);
//      cout << "z_plane = " << z_plane << endl;
      threevector zplane_intercept_posn;
      if (get_CM_3D_ptr()->get_Transformer_ptr()->
          compute_screen_ray_intercept_with_zplane(
             X,Y,z_plane,zplane_intercept_posn))
      {
//         cout << "zplane_intercept_posn = " << zplane_intercept_posn << endl;
      }

//      cout << "approx_range_to_polyline = "
//           << approx_range_to_polyline << endl;
      double curr_sqrd_XY_dist_to_ray=
         polyline_ptr->min_sqrd_distance_to_ray(
            zplane_intercept_posn,ray_basepoint,ray_ehat,
            approx_range_to_polyline);
      
      if (curr_sqrd_XY_dist_to_ray < minimal_squared_XY_distance_to_ray)
      {
         minimal_squared_XY_distance_to_ray=curr_sqrd_XY_dist_to_ray;
         closest_polyline_ID=ROI_Polyhedron_ptr->get_ID();
      }

// If two or more polyline segments lie very close to the input ray,
// add their IDs to STL vector close_polyline_IDs.  Below, we
// systematically cycle through these IDs so that a user can
// eventually pick any polyline (even if it's occluded by some other
// polyline):

      if (curr_sqrd_XY_dist_to_ray < minimal_distance_threshold)
      {
         close_polyline_IDs.push_back(ROI_Polyhedron_ptr->get_ID());
//         cout << "close_polyline_IDs.size() = " 
//              << close_polyline_IDs.size() << endl;
      }

//      cout << "sqrt(curr_sqrd_XY_dist_to_ray) = "
//           << sqrt(curr_sqrd_XY_dist_to_ray)
//           << " sqrt(minimal_squared_XY_dist_to_ray) = "
//           << sqrt(minimal_squared_XY_distance_to_ray) << endl;
//      cout << "closest polyline ID = " << closest_polyline_ID << endl;

   } // loop over index n labeling ROI_Polyhedra

//   cout << "Final minimal dist of ray to polyline = " 
//        << sqrt(minimal_squared_XY_distance_to_ray) << endl;

   if (close_polyline_IDs.size() > 1)
   {
      closest_polyline_ID=close_polyline_IDs[
         modulo(close_polyline_counter++,close_polyline_IDs.size())];
//      cout << "close_polyline_counter = "
//           << close_polyline_counter << endl;
   }

//   cout << "Final closest polyline ID = " << closest_polyline_ID << endl;
   
// Select ROI_Polyhedron if its 2D screen space projection lies within
// 0.5*approx_range_to_polyline of mouse selection point:

   bool select_handled_flag=false;
   if (minimal_squared_XY_distance_to_ray < sqr(0.5*approx_range_to_polyline))
   {
      set_selected_Graphical_ID(closest_polyline_ID);
      select_handled_flag=true;
   }
   else
   {
      set_selected_Graphical_ID(-1);
   }

//   cout << "selected_ROI_Polyhedron_ID = " << get_selected_Graphical_ID()
//        << endl;

   ROI_PolyhedraGroup_ptr->reset_colors();   

// As of 9/3/08, we experiment with reseting mode to its value prior
// to MANIPULATE_POLYLINE if select_handled_flag==true:

   if (select_handled_flag)
   {
      get_ModeController_ptr()->set_prev_State();
   }
*/

   bool select_handled_flag=false;
   return select_handled_flag;
}

