// ==========================================================================
// PolyLinePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 2/8/11; 2/9/11; 3/20/11; 1/22/16
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
#include "osg/osgGeometry/PolyLine.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"

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

void PolyLinePickHandler::allocate_member_objects()
{
}		       

void PolyLinePickHandler::initialize_member_objects()
{
   PolyLine_rather_than_Line_mode=true;
   form_polygon_flag=form_polyhedron_skeleton_flag=
      fix_PolyLine_altitudes_flag=false;
   generate_polyline_kdtree_flag=true;

   min_points_picked=2;
   close_polyline_counter=0;

   PolyLine_continuing_flag=false;
   curr_PolyLine_ptr=NULL;

   permanent_color=colorfunc::get_OSG_color(colorfunc::purple);
//   permanent_color=colorfunc::get_OSG_color(colorfunc::white);
   selected_color=colorfunc::get_OSG_color(colorfunc::red);
   Polygon_color=colorfunc::get_OSG_color(colorfunc::purple);
   Polyhedron_color=colorfunc::get_OSG_color(colorfunc::purple);
   z_offset=0;
   approx_range_to_polyline=10;		// meters
   regular_vertex_spacing=10; // meter
}		       

PolyLinePickHandler::PolyLinePickHandler(
   const int p_ndims,Pass* PI_ptr,
   osgGA::CustomManipulator* CM_ptr,PolyLinesGroup* PLG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   GeometricalPickHandler(p_ndims,PI_ptr,CM_ptr,PLG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   PolyLinesGroup_ptr=PLG_ptr;
   allocate_member_objects();
   initialize_member_objects();
}

PolyLinePickHandler::~PolyLinePickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool PolyLinePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyLinePickHandler::pick()" << endl;
//   cout << "PolyLinePickHandler this = " << this << endl;
//   cout << "PolyLine_rather_than_Line_mode = "
//        << PolyLine_rather_than_Line_mode << endl;
//   cout << "Allow_Insertion_flag = " << Allow_Insertion_flag << endl;
//   cout << "Allow_Manipulation_flag = " << Allow_Manipulation_flag << endl;

   if (get_disable_input_flag()) return false;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

   if ( (curr_state==ModeController::INSERT_LINE && Allow_Insertion_flag 
         && !PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::MANIPULATE_LINE &&
         Allow_Manipulation_flag && !PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::INSERT_POLYLINE && Allow_Insertion_flag 
         && PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::MANIPULATE_POLYLINE &&
        Allow_Manipulation_flag && PolyLine_rather_than_Line_mode) 
      )
   {
      if (GraphicalPickHandler::pick(ea))
      {

// If ModeController==INSERT_LINE or INSERT_POLYLINE, pick point
// within the 3D cloud whose screen-space coordinates lie closest to
// (X,Y).  Otherwise, select the 3D PolyLine whose center lies closest
// to (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_LINE ||
             curr_state==ModeController::INSERT_POLYLINE)
         {
            if (PolyLine_continuing_flag)
            {
//               cout << "get_dX() = " << get_dX()
//                    << " get_dY() = " << get_dY() << endl;
               const double TINY=1E-6;
               if (fabs(get_dX()) > TINY && fabs(get_dY()) > TINY)
               {
                  return continue_PolyLine(ea.getX(),ea.getY());
               }
               else
               {
                  return true;
               }
            }
            else
            {
               return instantiate_PolyLine(ea.getX(),ea.getY());
            }
         }
         else 	// curr_state=ModeController::MANIPULATE_LINE ||
		//  ModeController::MANIPULATE_POLYLINE ||
         {
            return select_PolyLine(ea.getX(),ea.getY());
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
   } // INSERT_[POLY]LINE or MANIPULATE_[POLY]LINE mode conditional
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   if (get_disable_input_flag()) return false;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_POLYLINE_VERTEX ||
       curr_state==ModeController::MANIPULATE_POLYLINE)
   {
//      cout << "inside PolyLinePickHandler::drag()" << endl;
      return GraphicalPickHandler::drag(ea);
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Member function doubleclick signals user termination of PolyLine
// input.

bool PolyLinePickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyLinePickHandler::doubleclick()" << endl;
//   cout << "n_PolyLines = " << PolyLinesGroup_ptr->get_n_Graphicals()
//        << endl;
//   cout << "PolyLinesGroup.get_name() = " << PolyLinesGroup_ptr->get_name()
//        << endl;

   if (get_disable_input_flag()) return false;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if ( (curr_state != ModeController::INSERT_LINE &&
         !PolyLine_rather_than_Line_mode) ||
        (curr_state != ModeController::INSERT_POLYLINE &&
         PolyLine_rather_than_Line_mode) ) return false;
   if (!Allow_Insertion_flag) return false;
   if (!PolyLine_continuing_flag) return false;

   PolyLine_continuing_flag=false;
   if (curr_PolyLine_ptr != NULL)
   {

// Check whether number of points in *curr_PolyLine_ptr is less than
// min_points_picked:

//      cout << "curr_PolyLine n_vertices = " 
//           << curr_PolyLine_ptr->get_n_vertices() << endl;
//      cout << "min_points_picked = " << min_points_picked << endl;

      if (curr_PolyLine_ptr->get_n_vertices() < min_points_picked) 
         return false;

      osgGeometry::PointsGroup* PointsGroup_ptr=
         curr_PolyLine_ptr->get_PointsGroup_ptr();
      if (PointsGroup_ptr != NULL)
      {
         PointsGroup_ptr->set_selected_Graphical_ID(-1);
         PointsGroup_ptr->reset_colors();
      }

//      cout << "form_polyhedron_skeleton_flag = " 
//           << form_polyhedron_skeleton_flag << endl;
      if (form_polyhedron_skeleton_flag) 
      {
         PolyLinesGroup_ptr->form_polyhedron_skeleton(
            curr_PolyLine_ptr,permanent_color,selected_color);
         colorfunc::Color mover_color=colorfunc::white;

// Send messages about new ROIs and KOZs to Michael Yee's social
// network tool via urban_network messenger:

         PolyLinesGroup_ptr->issue_add_vertex_message(
            curr_PolyLine_ptr,mover_color);
      } // form_polyhedron_skeleton_flag

// Note added on 5/1/08: For reasons we do not understand, we cannot
// instantiate Polyhedra within 3D point clouds without their shortly
// causing our 3D viewer programs to core dump.  So we comment out
// this next subsection...

/*
      if (form_polyhedron_flag)
      {
         polyline* polyline_ptr=curr_PolyLine_ptr->construct_polyline();
         vector<threevector> poly_vertices;
         for (int v=0; v<polyline_ptr->get_n_vertices(); v++)
         {
            poly_vertices.push_back(polyline_ptr->get_vertex(v));
         }

         double height=50;	// meters
         polyhedron prism;
         prism.generate_prism_with_rectangular_faces(poly_vertices,height);
//         cout << "prism = " << prism << endl;


         PolyhedraGroup* PolyhedraGroup_ptr=
            PolyLinesGroup_ptr->get_PolyhedraGroup_ptr();
         if (PolyhedraGroup_ptr != NULL)
         {   
            Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->
               generate_new_Polyhedron(&prism);
//            Polyhedron_ptr->build_current_polyhedron(
//               PolyhedraGroup_ptr->get_curr_t(),
//               PolyhedraGroup_ptr->get_passnumber());

//            cout << "*Polyhedron_ptr = " << *Polyhedron_ptr << endl;

            Polyhedron_ptr->set_permanent_color(Polyhedron_color);
            Polyhedron_ptr->reset_edges_width(8);
            PolyhedraGroup_ptr->reset_colors();
         }
      } // form_polyhedron_flag conditional
*/

// Attempt to form closed polygon from entered polyline vertices:

//      cout << "form_polygon_flag = " << form_polygon_flag << endl;

      if (form_polygon_flag)
      {
         osgGeometry::PolygonsGroup* PolygonsGroup_ptr=
            PolyLinesGroup_ptr->get_PolygonsGroup_ptr();
         osgGeometry::Polygon* Polygon_ptr=PolygonsGroup_ptr->
            generate_new_Polygon(curr_PolyLine_ptr);

//         cout << "relative poly = " 
//              << *(Polygon_ptr->get_relative_poly_ptr()) << endl;
//         cout << "reference origin = " << Polygon_ptr->get_reference_origin()
//              << endl;

         Polygon_ptr->set_permanent_color(Polygon_color);
         PolygonsGroup_ptr->reset_colors();

         V.push_back(V.front());

// As of 1/22/16, we do not want to display vertices on final polygon
// after user has finished entering polyline:

         bool display_vertices_flag = false;
         curr_PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
            V,curr_PolyLine_ptr,permanent_color,selected_color,
            display_vertices_flag);
      } // form_polygon_flag conditional
      
      V.clear();

   } // curr_PolyLine_ptr != NULL conditional

/*
// FAKE FAKE: 5/30/08 at 9:45 am...  experiment with blinking PolyLine
// just entered for alg development only...

   curr_PolyLine_ptr->set_blinking_flag(true);
   curr_PolyLine_ptr->set_blinking_start_time(
      timefunc::elapsed_timeofday_time());
   curr_PolyLine_ptr->set_max_blink_period(5);
*/

/*
// FAKE FAKE: 6/21/08 at 5:30 am...experiment with multicoloring
// PolyLine just entered for alg development only...

   vector<osg::Vec4> colors;
   for (int i=0; i<curr_PolyLine_ptr->get_n_vertices(); i++)
   {
      colors.push_back(colorfunc::get_OSG_color(colorfunc::get_color(i)));
   }
   curr_PolyLine_ptr->set_local_colors(colors);
   curr_PolyLine_ptr->set_multicolor_flag(true);
*/

// FAKE FAKE:  Fri Jun 27 at 7 pm
// Experiment with adding arrows to middle parts of PolyLines:

//   vector<double> arrow_posn_fracs;
//   arrow_posn_fracs.push_back(0.25);
//   arrow_posn_fracs.push_back(0.5);
//   arrow_posn_fracs.push_back(0.75);
//   curr_PolyLine_ptr->add_flow_direction_arrows(
//      arrow_posn_fracs,PolyLinesGroup_ptr->get_width());

//   double distance_between_arrows=30.0;
//   curr_PolyLine_ptr->add_flow_direction_arrows(
//      distance_between_arrows,PolyLinesGroup_ptr->get_width());

// FAKE FAKE:  Sun Jun 29 at 5:45 pm
// Experiment with selecting PolyLines via kdtrees...

/*
   polyline* polyline_ptr=curr_PolyLine_ptr->get_polyline_ptr();
   if (polyline_ptr==NULL)
   {
      polyline_ptr=curr_PolyLine_ptr->construct_polyline();
   }
*/

// For flight path entry purposes, a user picks XY waypoints on the
// ground surface.  But if fix_PolyLine_altitudes_flag==true, reset
// the z-value of each waypoint to equal PolyLinesGroup member
// variable constant_vertices_altitude:

   if (fix_PolyLine_altitudes_flag) 
   {
//      PolyLinesGroup_ptr->lowpass_filter_polyline(curr_PolyLine_ptr);
      PolyLinesGroup_ptr->reset_PolyLine_altitudes(curr_PolyLine_ptr);
//      PolyLinesGroup_ptr->issue_last_track_message();
   }

//   double polyline_length=curr_PolyLine_ptr->get_or_set_polyline_ptr()->
//      compute_total_length();
//   cout << "polyline length = " << polyline_length << " meters" << endl;

   if (generate_polyline_kdtree_flag)
   {
      curr_PolyLine_ptr->get_or_set_polyline_ptr()->
         generate_sampled_edge_points_kdtree(regular_vertex_spacing);
   }
   
   PolyLinesGroup_ptr->set_selected_Graphical_ID(-1);
   PolyLinesGroup_ptr->reset_colors();

// On 5/1/08, we experiment with resetting Mode to previous state
// after PolyLine insertion is complete:

   get_ModeController_ptr()->setState(
      get_ModeController_ptr()->get_prev_State());

//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//   cout << "At end of PolyLinePickHandler::doubleclick()" << endl;
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr << endl;
//   cout << "PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << PolyLinesGroup_ptr->get_n_Graphicals() << endl;
//   for (int n=0; n<PolyLinesGroup_ptr->get_n_Graphicals(); n++)
//   {
//      Geometrical* Geometrical_ptr=PolyLinesGroup_ptr->get_Geometrical_ptr(n);
//      int curr_ID=Geometrical_ptr->get_ID();
//      cout << "n = " << n 
//           << " Geometrical_ptr = " << Geometrical_ptr 
//           << " Geometrical_ptr->get_ID = " << curr_ID
//           << endl;
//   }
//   for (int n=0; n<8; n++)
//   {
//      cout << "n = " << n 
//           << " get_ID_labeled_PolyLine_ptr(n) = "
//           << PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(n)
//           << endl;
//   }

   curr_PolyLine_ptr->set_entry_finished_flag(true);
   PolyLinesGroup_ptr->set_Geometricals_updated_flag(true);

   return true;
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::release()
{
   if (get_disable_input_flag()) return false;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if ( (curr_state==ModeController::INSERT_LINE && Allow_Insertion_flag &&
         !PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::MANIPULATE_LINE 
         && Allow_Manipulation_flag && !PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::INSERT_POLYLINE && 
         Allow_Insertion_flag && PolyLine_rather_than_Line_mode) ||
        (curr_state==ModeController::MANIPULATE_POLYLINE 
         && Allow_Manipulation_flag && PolyLine_rather_than_Line_mode) 
      )
   {
//      cout << "inside PolyLinePickHandler::release()" << endl;
      PolyLinesGroup_ptr->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// This overridden version of move_Graphical() repositions the
// selected Point to the current location of the mouse.

bool PolyLinePickHandler::move_Graphical()
{   
//   cout << "inside PolyLinePickHandler::move_Graphical()" << endl;

   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_selected_PolyLine_ptr();
   osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
      get_PointsGroup_ptr();
   osgGeometry::Point* curr_Point_ptr=
      PointsGroup_ptr->get_selected_Point_ptr();

// If curr_Point_ptr==NULL, we assume that the user intends to move
// the entire PolyLine and not simply alter an individual PolyLine
// vertex location:

   if (curr_Point_ptr==NULL) return move_PolyLine(PolyLine_ptr);

   bool Point_moved_flag=false;
   if (curr_Point_ptr != NULL)
   {
      threevector Point_posn;
      if (curr_Point_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Point_posn))
      {
//         cout << "ID = " << curr_Point_ptr->get_ID() << endl;
//         cout << "name = " << curr_Point_ptr->get_name() << endl;
//         cout << "Point_posn = " << Point_posn << endl;

// If ndims==3, we need to convert Point_posn from world to screen
// space coordinates:

         threevector Point_screenspace_posn=Point_posn;

         if (get_ndims()==3)
         {
            Point_screenspace_posn=
               get_CM_3D_ptr()->get_Transformer_ptr()->
               world_to_screen_transformation(Point_posn);
         }

//         cout << "Point_screenspace_posn = " 
//              << Point_screenspace_posn << endl;
//         cout << "curr_voxel_screenspace_posn = "
//              << curr_voxel_screenspace_posn << endl;
//         cout << "max_dist_to_graphicals = "
//              << get_max_distance_to_Graphical() << endl;

         if (sqrd_screen_dist(
            Point_screenspace_posn,curr_voxel_screenspace_posn) < 
             sqr(get_max_distance_to_Graphical())) 
         {
            threevector curr_voxel_posn(curr_voxel_screenspace_posn);

// If ndims==3, we form a new Graphical screen space position using the
// current X and Y screen coordinates obtained from the mouse and the
// Graphical's existing Z screen coordinate.  We then transform this new
// Graphical screen space position back to world-space coordinates.  We
// next restore the Graphical's original world-space z value.  Mouse
// dragging of 3D Graphical's consequently affects only their
// world-space x and y coordinates.  (Manipulation of world-space z
// values is much better performed using the key-driven move_z
// method!)  The final modified x and y world-space coordinates are
// stored within the Graphical's UVW coordinates:

            if (get_ndims()==3)
            {
               threevector new_Point_screenspace_posn(
                  curr_voxel_screenspace_posn.get(0),
                  curr_voxel_screenspace_posn.get(1),
                  Point_screenspace_posn.get(2));
               threevector new_Point_worldspace_posn=
                  get_CM_3D_ptr()->get_Transformer_ptr()->
                  screen_to_world_transformation(
                     new_Point_screenspace_posn);

               curr_voxel_posn=new_Point_worldspace_posn;
//               cout << "curr_voxel_posn = " << curr_voxel_posn << endl;
//               cout << "Graphical worldspace posn = " 
//                    << new_Point_worldspace_posn << endl;

               curr_voxel_posn.put(2,Point_posn.get(2));
            } // ndims==3 conditional
            
            curr_Point_ptr->set_UVW_coords(
               get_curr_t(),get_passnumber(),curr_voxel_posn);
            Point_moved_flag=true;
               
// Make a note that we have manually manipulated the Graphical's UVW
// coordinates for one particular time in one particular pass:

            curr_Point_ptr->set_coords_manually_manipulated(
               get_curr_t(),get_passnumber());

         } // dist < max_dist_to_crosshairs conditional
      } // get_UVW_coords boolean conditional
   } // currPoint_ptr != NULL conditional

   return Point_moved_flag;
}

// ==========================================================================
// PolyLine generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_PolyLine creates a new PolyLine, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_PolyLine_number equal to that ID and adds it to
// the OSG PolyLine group.

bool PolyLinePickHandler::instantiate_PolyLine(double X,double Y)
{   
//   cout << "inside PolyLinePickHandler::instantiate_PolyLine()" << endl;
//   cout << "X = " << X << " Y = " << Y << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   bool PolyLine_instantiated_flag=false;
   if (get_ndims()==3)
   {
      PolyLine_instantiated_flag=GraphicalPickHandler::pick_3D_point(X,Y);
      threevector curr_vertex=curr_voxel_worldspace_posn+z_offset*z_hat;
      V.push_back(curr_vertex);
   }
   else
   {
//      cout << "curr_voxel_screenspace_posn = "
//           << curr_voxel_screenspace_posn << endl;
      V.push_back(curr_voxel_screenspace_posn);
      PolyLine_instantiated_flag=true;
   } // ndims==3 conditional

   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;
   
   curr_PolyLine_ID=PolyLinesGroup_ptr->get_next_unused_ID();
//   cout << "curr_PolyLine_ID = " << curr_PolyLine_ID << endl;
      
   curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
      V.back(),force_display_flag,single_polyline_per_geode_flag,
      PolyLinesGroup_ptr->get_n_text_messages(),curr_PolyLine_ID);
//      cout << "n_text_messages = " << curr_PolyLine_ptr->get_n_text_messages()
//           << endl;

   PolyLinesGroup_ptr->set_selected_Graphical_ID(curr_PolyLine_ID);
   curr_PolyLine_ptr->set_selected_color(selected_color);

// Base size of first vertex point upon virtual camera's height above
// grid.  Reset size of 2nd and all subsequent vertex points based
// upon PolyLine's length:

   double PolyLine_sizefactor=1;
   if (get_ndims()==2)
   {
      PolyLine_sizefactor=1.0/8.0;
   }

   if (PolyLinesGroup_ptr->get_variable_Point_size_flag())
   {
      double polyline_length=curr_PolyLine_ptr->get_polyline_ptr()->
         get_total_length();
      PolyLine_sizefactor=curr_PolyLine_ptr->get_length_sizefactor();
      if (get_CM_3D_ptr() != NULL && nearly_equal(polyline_length,0))
      {
         double height_above_grid=get_CM_3D_ptr()->
            get_camera_height_above_grid();
//         cout << "height_above_grid = " << height_above_grid << endl;
         double logheight=log10(height_above_grid);
         PolyLine_sizefactor=0.5*pow(4,logheight-2);
      }
   }

//   PolyLinesGroup_ptr->set_Pointsize_scalefactor(
//      0.5*PolyLinesGroup_ptr->get_Pointsize_scalefactor());

   osgGeometry::PointsGroup* PointsGroup_ptr=curr_PolyLine_ptr->
      get_PointsGroup_ptr();
   PointsGroup_ptr->set_crosshairs_size(
      PolyLine_sizefactor*PolyLinesGroup_ptr->get_Pointsize_scalefactor()*
      PointsGroup_ptr->get_crosshairs_size());
   PointsGroup_ptr->set_crosshairs_text_size(
      PolyLine_sizefactor*PolyLinesGroup_ptr->get_textsize_scalefactor()*
      PointsGroup_ptr->get_crosshairs_text_size());
   
   curr_PolyLine_ptr->add_vertex_points(V,selected_color);
   
   PolyLine_continuing_flag=true;

   return PolyLine_instantiated_flag;
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::continue_PolyLine(double X,double Y)
{   
//   cout << "inside PolyLinePickHandler::continue_PolyLine()" << endl;
//   cout << "ndims = " << get_ndims()  << endl;
//   cout << "Mouse posn: X = " << X << " Y = " << Y << endl;
//   cout << "curr_voxel_screenspace_posn = " << curr_voxel_screenspace_posn
//        << endl;

   if (get_ndims()==3)
   {
      GraphicalPickHandler::pick_3D_point(X,Y);
      threevector curr_vertex=curr_voxel_worldspace_posn+z_offset*z_hat;
      V.push_back(curr_vertex);
   }
   else
   {
      V.push_back(curr_voxel_screenspace_posn);
   } // ndims==3 conditional

   curr_PolyLine_ptr=
      PolyLinesGroup_ptr->regenerate_PolyLine(
         V,curr_PolyLine_ptr,permanent_color,selected_color);
   
   return PolyLine_continuing_flag;
}

// --------------------------------------------------------------------------
// Method select_PolyLine assigns selected_PolyLine_number equal to
// the ID of an existing PolyLine which lies sufficiently close to a
// 3D ray picked by the user with his mouse and eye position.  If no
// PolyLine is nearby the selected point, selected_PolyLine_number is
// set equal to -1, and all PolyLines are effectively de-selected.

bool PolyLinePickHandler::select_PolyLine(double X,double Y)
{   
//   cout << "inside PolyLinePickHandler::select_PolyLine(), X = " 
//        << X << " Y = " << Y << endl;
//   cout << "this = " << this << endl;

   bool select_handled_flag=false;
   if (get_ndims()==2)
   {
      int closest_PolyLine_ID = -1;
      double min_pt_to_polyline_dist = POSITIVEINFINITY;
      const double distance_threshold = 0.01;  // In screenspace UV coords

// Find PolyLine closest to mouse point (X,Y).  If separation distance
// is less than distance_threshold, regard closest PolyLine as
// selected:

      for (unsigned int n=0; n<PolyLinesGroup_ptr->get_n_Graphicals(); n++)
      {
         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
         polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
         double curr_pt_to_polyline_dist = 
            polyline_ptr->min_distance_to_point(curr_voxel_screenspace_posn);
         if(curr_pt_to_polyline_dist < min_pt_to_polyline_dist &&
            curr_pt_to_polyline_dist < distance_threshold)
         {
            min_pt_to_polyline_dist = curr_pt_to_polyline_dist;
            closest_PolyLine_ID = PolyLine_ptr->get_ID();
         }
      }

      int selected_PolyLine_ID = closest_PolyLine_ID;
      PolyLinesGroup_ptr->set_selected_Graphical_ID(selected_PolyLine_ID);
      PolyLinesGroup_ptr->reset_colors();

      select_handled_flag=true;
      return select_handled_flag;
   }

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
   for (unsigned int n=0; n<PolyLinesGroup_ptr->get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
//      cout << "Polyline n = " << n 
//           << " Polyline_ptr = " << PolyLine_ptr << endl;

      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      if (polyline_ptr==NULL)
      {
         polyline_ptr=PolyLine_ptr->construct_polyline();
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
         closest_polyline_ID=PolyLine_ptr->get_ID();
      }

// If two or more polyline segments lie very close to the input ray,
// add their IDs to STL vector close_polyline_IDs.  Below, we
// systematically cycle through these IDs so that a user can
// eventually pick any polyline (even if it's occluded by some other
// polyline):

      if (curr_sqrd_XY_dist_to_ray < minimal_distance_threshold)
      {
         close_polyline_IDs.push_back(PolyLine_ptr->get_ID());
//         cout << "close_polyline_IDs.size() = " 
//              << close_polyline_IDs.size() << endl;
      }

//      cout << "sqrt(curr_sqrd_XY_dist_to_ray) = "
//           << sqrt(curr_sqrd_XY_dist_to_ray)
//           << " sqrt(minimal_squared_XY_dist_to_ray) = "
//           << sqrt(minimal_squared_XY_distance_to_ray) << endl;
//      cout << "closest polyline ID = " << closest_polyline_ID << endl;

   } // loop over index n labeling PolyLines

//   cout << "Final minimal dist of ray to polyline = " 
//        << sqrt(minimal_squared_XY_distance_to_ray) << endl;

   if (close_polyline_IDs.size() > 1)
   {
      closest_polyline_ID=close_polyline_IDs[
         modulo(close_polyline_counter++,close_polyline_IDs.size())];
//      cout << "close_polyline_counter = "
//           << close_polyline_counter << endl;
   }

   cout << "Final closest polyline ID = " << closest_polyline_ID << endl;
   
// Select PolyLine if its 2D screen space projection lies within
// 0.5*approx_range_to_polyline of mouse selection point:

   if (minimal_squared_XY_distance_to_ray < sqr(0.5*approx_range_to_polyline))
   {
      set_selected_Graphical_ID(closest_polyline_ID);
      select_handled_flag=true;
   }
   else
   {
      set_selected_Graphical_ID(-1);
   }

   cout << "selected_PolyLine_ID = " << get_selected_Graphical_ID()
        << endl;

   PolyLinesGroup_ptr->reset_colors();   

// As of 9/3/08, we experiment with reseting mode to its value prior
// to MANIPULATE_POLYLINE if select_handled_flag==true:

   if (select_handled_flag)
   {
      get_ModeController_ptr()->set_prev_State();
   }

   return select_handled_flag;
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::move_PolyLine(PolyLine* PolyLine_ptr)
{   
//   cout << "inside PolyLinePickHandler::move_PolyLine()" << endl;

   bool PolyLine_moved_flag=false;
   if (PolyLine_ptr==NULL) return PolyLine_moved_flag;

   threevector PolyLine_posn;
   if (PolyLine_ptr->get_UVW_coords(
       get_curr_t(),get_passnumber(),PolyLine_posn))
   {
//      cout << "ID = " << PolyLine_ptr->get_ID() << endl;
//      cout << "name = " << PolyLine_ptr->get_name() << endl;
//      cout << "PolyLine_posn = " << PolyLine_posn << endl;

// If ndims==3, we need to convert PolyLine_posn from world to screen
// space coordinates:

      threevector PolyLine_screenspace_posn=PolyLine_posn;

      if (get_ndims()==3)
      {
         PolyLine_screenspace_posn=
            get_CM_3D_ptr()->get_Transformer_ptr()->
            world_to_screen_transformation(PolyLine_posn);
      }

//      cout << "PolyLine_screenspace_posn = " 
//           << PolyLine_screenspace_posn << endl;
//      cout << "curr_voxel_screenspace_posn = "
//           << curr_voxel_screenspace_posn << endl;
//      cout << "max_dist_to_graphicals = "
//           << get_max_distance_to_Graphical() << endl;

      if (sqrd_screen_dist(
         PolyLine_screenspace_posn,curr_voxel_screenspace_posn) < 
         sqr(get_max_distance_to_Graphical())) 
      {
         threevector curr_voxel_posn(curr_voxel_screenspace_posn);

// If ndims==3, we form a new Graphical screen space position using the
// current X and Y screen coordinates obtained from the mouse and the
// Graphical's existing Z screen coordinate.  We then transform this new
// Graphical screen space position back to world-space coordinates.  We
// next restore the Graphical's original world-space z value.  Mouse
// dragging of 3D Graphical's consequently affects only their
// world-space x and y coordinates.  (Manipulation of world-space z
// values is much better performed using the key-driven move_z
// method!)  The final modified x and y world-space coordinates are
// stored within the Graphical's UVW coordinates:

         if (get_ndims()==3)
         {
            threevector new_PolyLine_screenspace_posn(
               curr_voxel_screenspace_posn.get(0),
               curr_voxel_screenspace_posn.get(1),
               PolyLine_screenspace_posn.get(2));
            threevector new_PolyLine_worldspace_posn=
               get_CM_3D_ptr()->get_Transformer_ptr()->
               screen_to_world_transformation(
                  new_PolyLine_screenspace_posn);

            curr_voxel_posn=new_PolyLine_worldspace_posn;
//               cout << "curr_voxel_posn = " << curr_voxel_posn << endl;
//               cout << "Graphical worldspace posn = " 
//                    << new_PolyLine_worldspace_posn << endl;

            curr_voxel_posn.put(2,PolyLine_posn.get(2));
         } // ndims==3 conditional
//         cout << "curr_voxel_posn = " << curr_voxel_posn << endl;
 
         PolyLine_ptr->set_UVW_coords(
            get_curr_t(),get_passnumber(),curr_voxel_posn);
         PolyLine_moved_flag=true;
               
// Make a note that we have manually manipulated the PolyLine's UVW
// coordinates for one particular time in one particular pass:

         PolyLine_ptr->set_coords_manually_manipulated(
            get_curr_t(),get_passnumber());

      } // dist < max_dist_to_crosshairs conditional
   } // get_UVW_coords boolean conditional

   return PolyLine_moved_flag;
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyLinePickHandler::rotate()" << endl;

   PolyLine* selected_PolyLine_ptr=PolyLinesGroup_ptr->
      get_selected_PolyLine_ptr();
   if (selected_PolyLine_ptr==NULL) return false;
   
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_POLYLINE)
   {
      double dY=ea.getY()-oldY;
      PolyLinesGroup_ptr->rotate(selected_PolyLine_ptr,dY);
      return true;
//      return GraphicalPickHandler::rotate(ea);
   }
   else
   {
      return false;
   } // MANIPULATE_POLYLINE mode conditional
}

// --------------------------------------------------------------------------
bool PolyLinePickHandler::scale(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PolyLinePickHandler::scale()" << endl;

   PolyLine* selected_PolyLine_ptr=PolyLinesGroup_ptr->
      get_selected_PolyLine_ptr();
   if (selected_PolyLine_ptr==NULL) return false;
   
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_POLYLINE)
   {
      double dY=ea.getY()-oldY;
      PolyLinesGroup_ptr->rescale(selected_PolyLine_ptr,dY);
      return true;
   }
   else
   {
      return false;
   } // MANIPULATE_POLYLINE mode conditional
}
