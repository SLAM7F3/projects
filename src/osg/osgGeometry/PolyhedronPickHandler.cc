// ==========================================================================
// PolyhedronPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 4/26/09; 4/27/09; 5/2/09; 1/22/12
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "track/mover_funcs.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "geometry/polygon.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/Polyhedron.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyhedronPickHandler.h"

#include "general/stringfuncs.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"
#include "geometry/polyhedron.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyhedronPickHandler::allocate_member_objects()
{
}		       

void PolyhedronPickHandler::initialize_member_objects()
{
   curr_Polyhedron_ptr=NULL;

   permanent_color=colorfunc::get_OSG_color(colorfunc::white);
   selected_color=colorfunc::get_OSG_color(colorfunc::green);
   Polyhedron_color=colorfunc::get_OSG_color(colorfunc::red);
   text_size=1.0;
   z_offset=0;
}		       

PolyhedronPickHandler::PolyhedronPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PolyhedraGroup* PG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,PG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   PolyhedraGroup_ptr=PG_ptr;
   allocate_member_objects();
   initialize_member_objects();
}

PolyhedronPickHandler::~PolyhedronPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool PolyhedronPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyhedronPickHandler::pick()" << endl;
//   cout << "PolyhedronPickHandler this = " << this << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_POLYHEDRON)
//   if (curr_state==ModeController::INSERT_POLYHEDRON ||
//       curr_state==ModeController::MANIPULATE_POLYHEDRON)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_POLYHEDRON, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the Polyhedron whose center lies closest to (X,Y)
// in screen space:

         if (curr_state==ModeController::INSERT_POLYHEDRON)
         {
            return instantiate_Polyhedron(ea.getX(),ea.getY());
         }
         else
         {
            return select_Polyhedron();
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
bool PolyhedronPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyhedronPickHandler::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_POLYHEDRON)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         curr_Polyhedron_ptr=PolyhedraGroup_ptr->
            get_ID_labeled_Polyhedron_ptr(get_selected_Graphical_ID());
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
bool PolyhedronPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyhedronPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

// As of 4/27/08, we experiment with allowing Bluegrass application
// users to double click on cylinders representing vehicle tracks in
// RUN_MOVIE mode:

   if (curr_state==ModeController::MANIPULATE_POLYHEDRON ||
       curr_state==ModeController::RUN_MOVIE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {
         return select_Polyhedron();
      }
   }

   return false;
}

// --------------------------------------------------------------------------
bool PolyhedronPickHandler::release()
{
//   cout << "inside PolyhedronPickHandler::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_POLYHEDRON)
   {
      PolyhedraGroup_ptr->broadcast_bbox_corners();

// On 5/2/09, we experiment with resetting Mode to previous state
// after Polyhedron insertion is complete:

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
// Polyhedron generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Polyhedron creates a new Polyhedron, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Polyhedron_number equal to that ID and adds it to
// the OSG Polyhedron group.

bool PolyhedronPickHandler::instantiate_Polyhedron(double X,double Y)
{   
//   cout << "inside PolyhedronPickHandler::instantiate_Polyhedron()" << endl;

   bool Polyhedron_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      int Polyhedra_subgroup=0;
      Polyhedron* curr_Polyhedron_ptr=PolyhedraGroup_ptr->generate_bbox(
         Polyhedra_subgroup);
      instantiate_Graphical(curr_Polyhedron_ptr);

/*
  double theta=135*PI/180;
  double phi=45*PI/180;
  threevector scale(1,1,15);
  threevector trans(0,0,0);
  PolyhedronsGroup_ptr->scale_rotate_and_then_translate_Polyhedron(
  curr_Polyhedron_ptr,theta,phi,scale,trans);
*/

      Polyhedron_instantiated_flag=true;
   }

   return Polyhedron_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_Polyhedron assigns selected_Polyhedron_number equal to
// the ID of an existing Polyhedron which lies sufficiently close to a
// 3D ray picked by the user with his mouse and eye position.  If no
// Polyhedron is nearby the selected point, selected_Polyhedron_number is
// set equal to -1, and all Polyhedra are effectively de-selected.

bool PolyhedronPickHandler::select_Polyhedron()
{   
   int selected_Polyhedron_ID=select_Graphical();
   return select_Polyhedron(selected_Polyhedron_ID);
}

bool PolyhedronPickHandler::select_Polyhedron(int selected_Polyhedron_ID)
{   
//   cout << "inside PolyhedronPickHandler::select_Polyhedron()" << endl;
//   cout << "selected_Polyhedron_ID = " << selected_Polyhedron_ID << endl;
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
   for (int n=0; n<PolyhedraGroup_ptr->get_n_Graphicals(); n++)
   {
      Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(n);
//      cout << "Polyline n = " << n 
//           << " Polyline_ptr = " << Polyhedron_ptr << endl;

      polyline* polyline_ptr=Polyhedron_ptr->get_polyline_ptr();
      if (polyline_ptr==NULL)
      {
         polyline_ptr=Polyhedron_ptr->construct_polyline();
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
         closest_polyline_ID=Polyhedron_ptr->get_ID();
      }

// If two or more polyline segments lie very close to the input ray,
// add their IDs to STL vector close_polyline_IDs.  Below, we
// systematically cycle through these IDs so that a user can
// eventually pick any polyline (even if it's occluded by some other
// polyline):

      if (curr_sqrd_XY_dist_to_ray < minimal_distance_threshold)
      {
         close_polyline_IDs.push_back(Polyhedron_ptr->get_ID());
//         cout << "close_polyline_IDs.size() = " 
//              << close_polyline_IDs.size() << endl;
      }

//      cout << "sqrt(curr_sqrd_XY_dist_to_ray) = "
//           << sqrt(curr_sqrd_XY_dist_to_ray)
//           << " sqrt(minimal_squared_XY_dist_to_ray) = "
//           << sqrt(minimal_squared_XY_distance_to_ray) << endl;
//      cout << "closest polyline ID = " << closest_polyline_ID << endl;

   } // loop over index n labeling Polyhedra

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
   
// Select Polyhedron if its 2D screen space projection lies within
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

//   cout << "selected_Polyhedron_ID = " << get_selected_Graphical_ID()
//        << endl;

   PolyhedraGroup_ptr->reset_colors();   

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

