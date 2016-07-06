// ==========================================================================
// PolyhedraKeyHandler class member function definitions.  This class
// does not perform any useful function as of 1/24/07.
// ==========================================================================
// Last modified on 1/11/12; 1/22/12; 1/23/12
// ==========================================================================

#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyhedraKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyhedraKeyHandler::allocate_member_objects()
{
}

void PolyhedraKeyHandler::initialize_member_objects()
{
   OSGsubPAT_number_to_toggle=0;
   PolyhedraGroup_ptr=NULL;
}

PolyhedraKeyHandler::PolyhedraKeyHandler(
   PolyhedraGroup* PHG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(PHG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PolyhedraGroup_ptr=PHG_ptr;
}

PolyhedraKeyHandler::~PolyhedraKeyHandler()
{
}

// ------------------------------------------------------
bool PolyhedraKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==ModeController::RUN_MOVIE)
      {

// For Bluegrass demo, press 't' to toggle on/off Activity Region
// bounding box polyhedra:

         if (ea.getKey()=='t')
         {
            PolyhedraGroup_ptr->toggle_OSGsubPAT_nodemask(
               OSGsubPAT_number_to_toggle);

            cout << "PolyhedraGroup_ptr = " << PolyhedraGroup_ptr << endl;
            cout << "OSGsubPAT_number_to_toggle = "
                 << OSGsubPAT_number_to_toggle << endl;
            return true;
         }

// Press "Delete" key to completely destroy a PolyLine:

//         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
//         {
//            PolyhedraGroup_ptr->destroy_Polyhdron();
//            return true;
//         }

      }
      else if (get_ModeController_ptr()->getState()==
      ModeController::INSERT_POLYHEDRON)
      {
         if (ea.getKey()=='i')
         {
            PolyhedraGroup_ptr->read_OSG_file();
            get_ModeController_ptr()->setState(
               ModeController::VIEW_DATA);
            return true;
         }
         else if (ea.getKey()=='r')
         {
            PolyhedraGroup_ptr->import_new_Polyhedra();
            get_ModeController_ptr()->setState(
               ModeController::VIEW_DATA);
            return true;
         }
      } 
      else if (get_ModeController_ptr()->getState()==
      ModeController::MANIPULATE_POLYHEDRON)
      {
         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
         {
            PolyhedraGroup_ptr->unselect_Polyhedra_vertices();
            PolyhedraGroup_ptr->unselect_Polyhedra_edges();
            if (PolyhedraGroup_ptr->increment_selected_Polyhedron() < 0)
            {
               return false;
            }
            return true;
         }
         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
         {
            PolyhedraGroup_ptr->unselect_Polyhedra_vertices();
            PolyhedraGroup_ptr->unselect_Polyhedra_edges();
            if (PolyhedraGroup_ptr->decrement_selected_Polyhedron() < 0)
            {
               return false;
            }
            return true;
         }
         else if (ea.getKey()=='e')	// e stands for edge
         {
//            cout << "Selected Polyhedron ID = " 
//                 << PolyhedraGroup_ptr->get_selected_Graphical_ID() 
//                 << endl;

            Polyhedron* selected_Polyhedron_ptr=
               PolyhedraGroup_ptr->get_selected_Polyhedron_ptr();
            if (selected_Polyhedron_ptr==NULL) 
            {
               cout << "No selected Polyhedron!" << endl;
               return false;
            }
            
            LineSegmentsGroup* LineSegmentsGroup_ptr=
               selected_Polyhedron_ptr->get_LineSegmentsGroup_ptr();
            int n_edges=LineSegmentsGroup_ptr->get_n_Graphicals();
//            cout << "n_edges = " << n_edges << endl;

            int selected_edge_ID=selected_Polyhedron_ptr->
               get_selected_edge_ID();
            selected_edge_ID=modulo(selected_edge_ID+1,n_edges);
            selected_Polyhedron_ptr->set_selected_edge_ID(
               selected_edge_ID);

            LineSegmentsGroup_ptr->set_selected_Graphical_ID(
               selected_edge_ID);

            LineSegmentsGroup_ptr->reset_colors();
         }
         else if (ea.getKey()=='v')	// v stands for vertex
         {
//            cout << "Selected Polyhedron ID = " 
//                 << PolyhedraGroup_ptr->get_selected_Graphical_ID() 
//                 << endl;

            Polyhedron* selected_Polyhedron_ptr=
               PolyhedraGroup_ptr->get_selected_Polyhedron_ptr();
            if (selected_Polyhedron_ptr==NULL) 
            {
               cout << "No selected Polyhedron!" << endl;
               return false;
            }
            
            osgGeometry::PointsGroup* PointsGroup_ptr=
               selected_Polyhedron_ptr->get_PointsGroup_ptr();
            int n_vertices=PointsGroup_ptr->get_n_Graphicals();
//            cout << "n_vertices = " << n_vertices << endl;

            int selected_vertex_ID=selected_Polyhedron_ptr->
               get_selected_vertex_ID();
            selected_vertex_ID=modulo(selected_vertex_ID+1,n_vertices);
            selected_Polyhedron_ptr->set_selected_vertex_ID(
               selected_vertex_ID);

            PointsGroup_ptr->set_selected_Graphical_ID(selected_vertex_ID);
            PolyhedraGroup_ptr->display_selected_Polyhedron_vertex();
         }

         else if (ea.getKey()=='y')
         {
            PolyhedraGroup_ptr->write_OSG_file();
         }
         else if (ea.getKey()=='z')    
         {
            PolyhedraGroup_ptr->fit_constant_z_ground();
         }

      } // mode conditional

   } // key down conditional
   
   return false;
}


