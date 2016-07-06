// ==========================================================================
// PolygonPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 2/18/08; 6/15/08; 9/2/08; 4/6/14
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/CustomManipulator.h"
#include "general/stringfuncs.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "geometry/polygon.h"
#include "osg/osgGeometry/Polygon.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolygonPickHandler.h"
#include "osg/osgGeometry/PolyLine.h"
#include "osg/ModeController.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PolygonPickHandler::allocate_member_objects()
      {
      }		       

   void PolygonPickHandler::initialize_member_objects()
      {
         curr_Polygon_ptr=NULL;
         permanent_color=colorfunc::get_OSG_color(colorfunc::white);
         selected_color=colorfunc::get_OSG_color(colorfunc::green);
         text_size=1.0;
      }		       

   PolygonPickHandler::PolygonPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PolygonsGroup* PG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
      GeometricalPickHandler(3,PI_ptr,CM_ptr,PG_ptr,MC_ptr,WCC_ptr,GO_ptr)
      {
         PolygonsGroup_ptr=PG_ptr;
         allocate_member_objects();
         initialize_member_objects();
      }

   PolygonPickHandler::~PolygonPickHandler() 
      {
      }

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

   bool PolygonPickHandler::pick(const osgGA::GUIEventAdapter& ea)
      {
//         cout << "inside PolygonPickHandler::pick()" << endl;

         ModeController::eState curr_state=get_ModeController_ptr()->
            getState();

         if (curr_state==ModeController::MANIPULATE_POLYGON)
         {
            if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
            {

// If ModeController==INSERT_POLYGON, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Polygon whose center lies closest to
// (X,Y) in screen space:

               if (curr_state==ModeController::MANIPULATE_POLYGON)
               {
                  {
                     return select_Polygon(ea.getX(),ea.getY());
                  }
               }
               else
               {
                  return false;
               }
            }
         } // MANIPULATE_POLYGON mode conditional
         return false;
      }
   
// --------------------------------------------------------------------------
   bool PolygonPickHandler::drag(const osgGA::GUIEventAdapter& ea)
      {
//   cout << "inside PolygonPickHandler::drag()" << endl;

         ModeController::eState curr_state=get_ModeController_ptr()->
            getState();
         if (curr_state==ModeController::INSERT_POLYGON ||
             curr_state==ModeController::MANIPULATE_POLYGON)
         {
            if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
            {
               curr_Polygon_ptr=PolygonsGroup_ptr->
                  get_ID_labeled_Polygon_ptr(get_selected_Graphical_ID());
               PolyLine* curr_PolyLine_ptr=curr_Polygon_ptr->
                  get_PolyLine_ptr();
               if (curr_PolyLine_ptr != NULL)
               {
                  double t=PolygonsGroup_ptr->get_curr_t();
                  int passnumber=PolygonsGroup_ptr->get_passnumber();
                  threevector UVW;
                  curr_Polygon_ptr->get_UVW_coords(t,passnumber,UVW);
                  curr_PolyLine_ptr->set_UVW_coords(t,passnumber,UVW);
               }

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
   bool PolygonPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
      {
//         cout << "inside PolygonPickHandler::doubleclick()" << endl;
         return false;
      }

// --------------------------------------------------------------------------
   bool PolygonPickHandler::release()
      {
//   cout << "inside PLPH::release()" << endl;

         ModeController::eState curr_state=get_ModeController_ptr()->
            getState();
         if (curr_state==ModeController::INSERT_POLYGON ||
             curr_state==ModeController::MANIPULATE_POLYGON)
         {
            PolygonsGroup_ptr->reset_colors();
            return true;
         }
         else
         {
            return false;
         }
      }

// ==========================================================================
// Polygon selection, manipulation and annihilation methods
// ==========================================================================

// Method select_Polygon assigns selected_Polygon_number equal to
// the ID of an existing Polygon which lies sufficiently close to a
// point picked by the user with his mouse.  If no Polygon is nearby
// the selected point, selected_Polygon_number is set equal to -1,
// and all Polygons are effectively de-selected.

   bool PolygonPickHandler::select_Polygon(double X,double Y)
      {   
         cout << "inside PolygonPickHandler::select_Polygon(), X = " 
              << X << " Y = " << Y << endl;
         cout << "curr_voxel_screenspace_posn = "
              << curr_voxel_screenspace_posn << endl;

         set_pick_handler_voxel_coords();

         cout << "After calling set_pick_handler_voxel_coords()" << endl;
         cout << "curr_voxel_screenspace_posn = "
              << curr_voxel_screenspace_posn << endl;

         set_selected_Graphical_ID(-1);

         for (unsigned int n=0; n<PolygonsGroup_ptr->get_n_Graphicals(); n++)
         {
            Polygon* Polygon_ptr=PolygonsGroup_ptr->get_Polygon_ptr(n);
            cout << "n = " << n << " Polygon_ptr = " << Polygon_ptr
                 << endl;

            polygon* relative_polygon_ptr=
               Polygon_ptr->get_relative_poly_ptr();

// Project Polygon's vertices from 3D world space into 2D screen
// space:

            vector<threevector> UV_vertices;

            for (unsigned int v=0; v<relative_polygon_ptr->get_nvertices(); 
                 v++)
            {
               threevector relative_vertex=relative_polygon_ptr->
                  get_vertex(v);
               threevector reference_vertex;
               Polygon_ptr->get_UVW_coords(
                  PolygonsGroup_ptr->get_curr_t(),
                  PolygonsGroup_ptr->get_passnumber(),reference_vertex);
               threevector abs_vertex=relative_vertex+reference_vertex;

               threevector UVW = abs_vertex;
               if(get_CM_3D_ptr() != NULL)
               {
                  UVW=get_CM_3D_ptr()->get_Transformer_ptr()->
                     world_to_screen_transformation(abs_vertex);
               }

               UVW.put(2,0);
               UV_vertices.push_back(UVW);
               cout << "Vertex v = " << v << " UVW = " << UVW << endl;
               
            } // loop over v index labeling current polygon vertices

// Form new, projected polygon from 2D screen space coordinates for
// Polygon's vertices:

            polygon projected_polygon(UV_vertices);

// Select Polygon if mouse selection point lies inside perimeter:

            threevector mouse_screenpoint(X,Y,0);
            if (projected_polygon.point_inside_polygon(
               mouse_screenpoint))
            {
               set_selected_Graphical_ID(Polygon_ptr->get_ID());
            }
         } // loop over index n labeling Polygons

         cout << "selected polygon ID = "
              << get_selected_Graphical_ID() << endl;
         
         PolygonsGroup_ptr->reset_colors();   
         return (get_selected_Graphical_ID() > -1);
      }

} // osgGeometry namespace
