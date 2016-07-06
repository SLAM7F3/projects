// ==========================================================================
// ModeController class member function definitions
// ==========================================================================
// Last modified on 6/17/09; 1/1/11; 11/17/11; 1/22/12
// ==========================================================================

#include <iostream>
#include "osg/ModeController.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ModeController::allocate_member_objects()
{
}		       

void ModeController::initialize_member_objects()
{
   m_state=m_prev_state=VIEW_DATA;
   picking_mode_flag=allow_manipulator_translation_flag=false;
}		       

ModeController::ModeController(bool hide_Mode_HUD_flag)
{
   allocate_member_objects();
   initialize_member_objects();
   this->hide_Mode_HUD_flag=hide_Mode_HUD_flag;
}

// ---------------------------------------------------------------------
void ModeController::setState(eState p_state)
{ 
//   cout << "inside ModeController::setState()" << endl;
   m_prev_state=m_state;
   m_state = p_state; 

//   cout << "prev_state = " << get_state_name(m_prev_state) << endl;
//   cout << "curr_state = " << get_state_name(m_state) << endl;
   
   if (hide_Mode_HUD_flag)
   {
      cout << get_state_name() << endl;
   }

   if (m_state==INSERT_LINE || 
	   m_state==INSERT_POLYLINE || m_state==MANIPULATE_POLYLINE || 
	   m_state==MANIPULATE_POLYLINE_VERTEX ||
	   m_state==INSERT_POLYHEDRON)
//        || m_state==MANIPULATE_POLYHEDRON)
   {
      picking_mode_flag=true;
   }
   else
   {
      picking_mode_flag=false;
   }
//   cout << "picking_mode_flag = " << picking_mode_flag << endl;
}

// ---------------------------------------------------------------------
string ModeController::get_state_name(int state)
{
   if (state == VIEW_DATA)
      return "View Data Mode";
   else if (state == GENERATE_AVI_MOVIE)
      return "Generate AVI Movie Mode";
   else if (state == SET_CENTER)
      return "Set Center Mode";
   else if (state == MANIPULATE_CENTER)
      return "Manipulate Center Mode";
   else if (state == INSERT_FEATURE)
      return "Insert Feature Mode";
   else if (state == MANIPULATE_FEATURE)
      return "Manipulate Feature Mode";
   else if (state == PROPAGATE_FEATURE)
      return "Propagate Feature Mode";
   else if (state == INSERT_ANNOTATION)
      return "Insert Annotation Mode";
   else if (state == MANIPULATE_ANNOTATION)
      return "Manipulate Annotation Mode";
   else if (state == TRACK_FEATURE)
      return "Track Feature Mode";
   else if (state == MANIPULATE_MAPIMAGE)
      return "Manipulate Map Image Mode";
   else if (state == RUN_MOVIE)
      return "Run Movie Mode";
   else if (state == MANIPULATE_MOVIE)
      return "Manipulate Movie Mode";
   else if (state == INSERT_BOX)
      return "Insert Box Mode";
   else if (state == MANIPULATE_BOX)
      return "Manipulate Box Mode";
   else if (state == INSERT_CONE)
      return "Insert Cone Mode";
   else if (state == MANIPULATE_CONE)
      return "Manipulate Cone Mode";
   else if (state == INSERT_RECTANGLE)
      return "Insert Rectangle Mode";
   else if (state == MANIPULATE_RECTANGLE)
      return "Manipulate Rectangle Mode";
   else if (state == INSERT_LINE)
      return "Insert Line Mode";
   else if (state == MANIPULATE_LINE)
      return "Manipulate Line Mode";
   else if (state == INSERT_HEMISPHERE)
      return "Insert Hemisphere Mode";
   else if (state == MANIPULATE_HEMISPHERE)
      return "Manipulate Hemisphere Mode";
   else if (state == INSERT_POINT)
      return "Insert Point Mode";
   else if (state == MANIPULATE_POINT)
      return "Manipulate Point Mode";
   else if (state == INSERT_MODEL)
      return "Insert Model Mode";
   else if (state == MANIPULATE_MODEL)
      return "Manipulate Model Mode";
   else if (state == INSERT_CYLINDER)
      return "Insert Cylinder Mode";
   else if (state == MANIPULATE_CYLINDER)
      return "Manipulate Cylinder Mode";
   else if (state == INSERT_PYRAMID)
      return "Insert Pyramid Mode";
   else if (state == MANIPULATE_PYRAMID)
      return "Manipulate Pyramid Mode";
   else if (state == INSERT_POLYGON)
      return "Insert Polygon Mode";
   else if (state == MANIPULATE_POLYGON)
      return "Manipulate Polygon Mode";
   else if (state == INSERT_POLYLINE)
      return "Insert PolyLine Mode";
   else if (state == MANIPULATE_POLYLINE)
      return "Manipulate PolyLine Mode";
   else if (state == MANIPULATE_POLYLINE_VERTEX)
      return "Manipulate PolyLine Vertex Mode";
   else if (state == INSERT_POLYHEDRON)
      return "Insert Polyhedron Mode";
   else if (state == MANIPULATE_POLYHEDRON)
      return "Manipulate Polyhedron Mode";
   else if (state == FUSE_DATA)
      return "Fuse Data Mode";
   else if (state == MANIPULATE_FUSED_DATA)
      return "Manipulate Fused Data Mode";
   else if (state == MANIPULATE_TRIANGLE)
      return "Manipulate Triangle Mode";
   else if (state == MANIPULATE_PLANE)
      return "Manipulate Plane Mode";
   else if (state == MANIPULATE_GRAPHNODE)
      return "Manipulate GraphNode Mode";
   else if (state == MANIPULATE_EARTH)
      return "Manipulate Earth Mode";
   else if (state == MANIPULATE_OBSFRUSTUM)
      return "Manipulate ObsFrustum Mode";
   else if (state == MANIPULATE_POLYLINE_VERTEX)
      return "Manipulate PolyLine Vertex Mode";
   else if (state == MANIPULATE_FISHNET)
      return "Manipulate Fishnet Mode";
   else
      return "Unknown Mode";
}

