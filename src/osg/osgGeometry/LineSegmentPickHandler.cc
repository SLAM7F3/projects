// ==========================================================================
// LineSegmentPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 1/21/07; 6/16/07; 9/21/07; 6/15/08; 9/2/08
// ==========================================================================

#include <iostream>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "osg/osgGeometry/LineSegment.h"
#include "osg/osgGeometry/LineSegmentPickHandler.h"
#include "osg/ModeController.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LineSegmentPickHandler::allocate_member_objects()
{
}		       

void LineSegmentPickHandler::initialize_member_objects()
{
}		       

LineSegmentPickHandler::LineSegmentPickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   LineSegmentsGroup* LSG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr):
   GeometricalPickHandler(p_ndims,PI_ptr,CM_ptr,LSG_ptr,MC_ptr,WCC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   LineSegmentsGroup_ptr=LSG_ptr;
}

LineSegmentPickHandler::~LineSegmentPickHandler() 
{
}

// ---------------------------------------------------------------------
LineSegmentsGroup* LineSegmentPickHandler::get_LineSegmentsGroup_ptr() 
{
   return LineSegmentsGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool LineSegmentPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside LineSegmentPickHandler::pick()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_LINE ||
       curr_state==ModeController::MANIPULATE_LINE)
   {
      if (GraphicalPickHandler::pick(ea))
      {

// If ModeController==INSERT_LINE, pick point whose screen-space
// coordinates lie closest to (X,Y).  Otherwise, select the LineSegment
// whose center lies closest to (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_LINE)
         {
            return instantiate_LineSegment();
         }
         else
         {
            return select_LineSegment();
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
   } // INSERT_LINE or MANIPULATE_LINE mode conditional
}

// --------------------------------------------------------------------------
bool LineSegmentPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside LSPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_LINE ||
       curr_state==ModeController::MANIPULATE_LINE)
   {
//      if (GraphicalPickHandler::drag(ea) && get_ndims()==2)
      if (GraphicalPickHandler::drag(ea))
      {
         LineSegment* LineSegment_ptr=get_linesegment_ptr();
         if (LineSegment_ptr != NULL)
         {
//            threevector UVW;
//            LineSegment_ptr->get_UVW_coords(
//               get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),UVW);
            return true;
         }
      }
   } // INSERT_LINE or MANIPULATE_LINE mode conditional
   return false;
}

// --------------------------------------------------------------------------
bool LineSegmentPickHandler::rotate(const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_LINE)
   {
//      if (GraphicalPickHandler::rotate(ea) && get_ndims()==2)
      if (GraphicalPickHandler::rotate(ea))
      {
         if (get_linesegment_ptr() != NULL)
         {
            return true;
         }
      }
   } // MANIPULATE_LINE mode conditional
   return false;
}

// --------------------------------------------------------------------------
bool LineSegmentPickHandler::release()
{
//   cout << "inside LSPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_LINE ||
       curr_state==ModeController::MANIPULATE_LINE)
   {
      get_LineSegmentsGroup_ptr()->reset_colors();
      return true;
   }
   else
   {
      return false;
   } // INSERT_LINE or MANIPULATE_LINE mode conditional
}

// ==========================================================================
// LineSegment generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_LineSegment creates a new LineSegment, assigns it a
// unique ID, sets the selected_LineSegment_number equal to that ID and
// adds it to the OSG LineSegment group.

bool LineSegmentPickHandler::instantiate_LineSegment()
{   
   cout << "inside LSPH::instantiate_LS()" << endl;
   
// General canonical linesegment from (0,0,0) to (1,0,0).  Then adjust
// its scale, rotation and translation so that the segment runs from
// V1 to V2:

//   bool draw_arrow_flag=true;
   bool draw_arrow_flag=false;
   int segment_ID=-1;
   LineSegment* curr_LineSegment_ptr=get_LineSegmentsGroup_ptr()->
      generate_new_canonical_LineSegment(segment_ID,draw_arrow_flag);

// FAKE FAKE:  Thurs, Aug 17...hardwire in values for V1 and V2...

   threevector V1(0,0,0);
   threevector V2(1,1,0);

   double curr_t=get_LineSegmentsGroup_ptr()->get_curr_t();
   int curr_passnumber=get_LineSegmentsGroup_ptr()->get_passnumber();
   curr_LineSegment_ptr->set_scale_attitude_posn(
      curr_t,curr_passnumber,V1,V2);
   get_LineSegmentsGroup_ptr()->update_display();
   return true;
}

// --------------------------------------------------------------------------
// Method select_LineSegment assigns selected_LineSegment_number equal
// to the ID of an existing segment which lies sufficiently close to a
// point picked by the user with his mouse.  If no LineSegment is
// nearby the selected point, selected_LineSegment_number is set equal
// to -1, and all LineSegments are effectively de-selected.

bool LineSegmentPickHandler::select_LineSegment()
{   
   int closest_linesegment_ID;
   bool linesegment_flag=
      find_closest_vertex_nearby_mouse_posn(closest_linesegment_ID);
   get_LineSegmentsGroup_ptr()->reset_colors();
   return linesegment_flag;
}

// --------------------------------------------------------------------------
// Member function find_closest_vertex_nearby_mouse_posn loops over
// all linesegments within the current data set's Graphicalslist.  It
// computes the distance between the current mouse location and each
// segment's endpoints and midpoint.  If the minimal distance is less
// than some small tolerance value, this boolean method returns true.

// This comment (and indeed this entire method!) needs more work as of
// Monday, Dec 5...

bool LineSegmentPickHandler::find_closest_vertex_nearby_mouse_posn(
   int& closest_LineSegment_ID)
{   
//   cout << "inside LSPH::find_closest_vertex_nearby_mouse_posn()" << endl;

   enum vertex
   {
      no_vertex,v1_vertex,v2_vertex,midpoint_vertex
   };
   vertex closest_vertex=no_vertex;

   closest_LineSegment_ID=-1;
   float min_sqrd_dist=POSITIVEINFINITY;

   for (int n=0; n<get_LineSegmentsGroup_ptr()->get_n_Graphicals(); n++)
   {
      LineSegment* curr_LineSegment_ptr=get_LineSegmentsGroup_ptr()->
         get_LineSegment_ptr(n);

      double curr_t=get_LineSegmentsGroup_ptr()->get_curr_t();
      threevector LineSegment_posn;

      if (curr_LineSegment_ptr->get_UVW_coords(
         curr_t,get_passnumber(),LineSegment_posn))
      {

// If ndims==3, we need to convert LineSegment_posn from world to screen
// space coordinates:

         threevector V1_screenspace_posn=LineSegment_posn;
         if (get_ndims()==3)
         {
            V1_screenspace_posn=
               get_CM_3D_ptr()->get_Transformer_ptr()->
               world_to_screen_transformation(LineSegment_posn);
         }
         threevector V2_screenspace_posn=V1_screenspace_posn+
            (curr_LineSegment_ptr->get_V2()-curr_LineSegment_ptr->get_V1());
         threevector midpoint_screenspace_posn=
            0.5*(V1_screenspace_posn+V2_screenspace_posn);
            
//         cout << "V1_screenspace_posn = "
//              << V1_screenspace_posn << endl;
//         cout << "V2_screenspace_posn = "
//              << V2_screenspace_posn << endl;
//         cout << "curr_voxel_screenspace_posn = "
//              << curr_voxel_screenspace_posn << endl;
         
         float sqrd_dist_to_midpoint=sqrd_screen_dist(
            midpoint_screenspace_posn,curr_voxel_screenspace_posn);
         float sqrd_dist_to_v1=sqrd_screen_dist(
            V1_screenspace_posn,curr_voxel_screenspace_posn);
         float sqrd_dist_to_v2=sqrd_screen_dist(
            V2_screenspace_posn,curr_voxel_screenspace_posn);

         threevector scale;
         curr_LineSegment_ptr->get_scale(curr_t,get_passnumber(),scale);
         double s=max(scale.get(0),scale.get(1),scale.get(2));
         
         if (sqrd_dist_to_midpoint < sqr(s*get_max_distance_to_Graphical()) &&
             sqrd_dist_to_midpoint < min_sqrd_dist)
         {
            min_sqrd_dist=sqrd_dist_to_midpoint;
            closest_LineSegment_ID=curr_LineSegment_ptr->get_ID();
            closest_vertex=midpoint_vertex;
         }
         if (sqrd_dist_to_v1 < sqr(s*get_max_distance_to_Graphical()) &&
             sqrd_dist_to_v1 < min_sqrd_dist)
         {
            min_sqrd_dist=sqrd_dist_to_v1;
            closest_LineSegment_ID=curr_LineSegment_ptr->get_ID();
            closest_vertex=v1_vertex;
         }
         if (sqrd_dist_to_v2 < sqr(s*get_max_distance_to_Graphical()) &&
             sqrd_dist_to_v2 < min_sqrd_dist)
         {
            min_sqrd_dist=sqrd_dist_to_v2;
            closest_LineSegment_ID=curr_LineSegment_ptr->get_ID();
            closest_vertex=v2_vertex;
         }
      } // get_UVW_coords boolean conditional

   } // loop over LineSegmentlist

//   cout << "closest_LineSegment_ID = " << closest_LineSegment_ID << endl;
//   cout << "closest_vertex = " << closest_vertex << endl;
   set_selected_Graphical_ID(closest_LineSegment_ID);

   if (closest_LineSegment_ID > -1 && closest_vertex == midpoint_vertex &&
       get_ndims()==2)
   {
      LineSegment* curr_LineSegment_ptr=get_linesegment_ptr();

      threevector UVW;
      curr_LineSegment_ptr->get_UVW_coords(
         get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),UVW);
      threevector midpoint(UVW+curr_LineSegment_ptr->get_midpoint());
      curr_LineSegment_ptr->set_UVW_coords(
         get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),
         UVW+curr_voxel_screenspace_posn-midpoint);
   }

   if (closest_LineSegment_ID > -1 &&
       closest_vertex != no_vertex && closest_vertex != midpoint_vertex &&
       get_ndims()==2)
   {
      LineSegment* curr_LineSegment_ptr=get_linesegment_ptr();

      threevector UVW;
      curr_LineSegment_ptr->get_UVW_coords(
         get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),UVW);

      if (closest_vertex==v1_vertex)
      {
         curr_LineSegment_ptr->set_V2(
            curr_LineSegment_ptr->get_V2()+
            UVW-curr_voxel_screenspace_posn);
         curr_LineSegment_ptr->set_UVW_coords(
            get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),
            curr_voxel_screenspace_posn);
      }
      else if (closest_vertex==v2_vertex)
      {
         curr_LineSegment_ptr->set_V2(curr_voxel_screenspace_posn-UVW);
      }
      curr_LineSegment_ptr->reset_vertices();
   }

//   LineSegment* curr_LineSegment_ptr=get_linesegment_ptr();
//   if (curr_LineSegment_ptr != NULL)
//   {
//      cout << *curr_LineSegment_ptr << endl;
//      cout << "V1 = " << curr_LineSegment_ptr->get_V1() << endl;
//      cout << "V2 = " << curr_LineSegment_ptr->get_V2() << endl;
//      threevector UVW;
//      curr_LineSegment_ptr->get_UVW_coords(
//         get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),UVW);
//      cout << "UVW = " << UVW << endl;
//   }
   
   return (closest_LineSegment_ID != -1);
}
