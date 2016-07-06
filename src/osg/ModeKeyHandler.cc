// ==========================================================================
// ModeKeyHandler class member function definitions
// ==========================================================================
// Last modified on 1/10/11; 11/17/11; 1/4/13; 1/22/16
// ==========================================================================

#include <iostream>
#include "osg/ModeKeyHandler.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ModeKeyHandler::allocate_member_objects()
{
}		       

void ModeKeyHandler::initialize_member_objects()
{
}		       

ModeKeyHandler::ModeKeyHandler( ModeController* p_controller ):
   m_controller( p_controller )
{
   allocate_member_objects();
   initialize_member_objects();
}

// ------------------------------------------------------
bool ModeKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   switch (ea.getEventType() )
   {
      case osgGA::GUIEventAdapter::KEYDOWN:
      {
         ModeController::eState curr_Mode=m_controller->getState();

         switch ( ea.getKey() )
         {

            case 'A' :
               if (curr_Mode==ModeController::INSERT_ANNOTATION)
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_ANNOTATION);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_ANNOTATION);
               }
               return true;
               break;

            case 'B' :
               if (curr_Mode==ModeController::INSERT_POLYHEDRON)
//               if (curr_Mode==ModeController::INSERT_BOX)
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_POLYHEDRON);
//                     ModeController::MANIPULATE_BOX);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_POLYHEDRON);
//                  m_controller->setState( ModeController::INSERT_BOX);
               }
               return true;
               break;

            case 'C' :
               if (curr_Mode==ModeController::INSERT_CONE)
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_CONE);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_CONE);
               }
               return true;
               break;

            case 'D' :
               if (curr_Mode==ModeController::MANIPULATE_OBSFRUSTUM)
               {
                  m_controller->setState( m_controller->get_prev_State() );
               }
               else
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_OBSFRUSTUM);
               }
               return true;
               break;

            case 'E' :
               if (curr_Mode==ModeController::MANIPULATE_EARTH)
               {
                  m_controller->setState( m_controller->get_prev_State() );
               }
               else
               {
                  m_controller->setState(ModeController::MANIPULATE_EARTH);
               }
               return true;
               break;	    

            case 'F' :
               if (curr_Mode==ModeController::MANIPULATE_FISHNET)
               {
                  m_controller->setState( m_controller->get_prev_State() );
               }
               else
               {
                  m_controller->setState(ModeController::MANIPULATE_FISHNET);
               }
               return true;
               break;	    

            case 'G' :
               if (curr_Mode==ModeController::INSERT_POLYGON)
               {
                  m_controller->setState(ModeController::MANIPULATE_POLYGON);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_POLYGON);
               }
               return true;
               break;

//            case 'G' :
//               if (curr_Mode==ModeController::INSERT_POLYLINE)
//               if (curr_Mode==ModeController::INSERT_POLYGON)
//               if (curr_Mode==ModeController::INSERT_PYRAMID)
//               if (curr_Mode==ModeController::MANIPULATE_GRAPHNODE)
//               {
//                  m_controller->setState( 
//                     ModeController::MANIPULATE_POLYLINE);
//                     ModeController::MANIPULATE_POLYGON);
//                     ModeController::MANIPULATE_PYRAMID);
//                     m_controller->get_prev_State() );
//               }
//               else
//               {
//                  m_controller->setState(
//                     ModeController::INSERT_POLYLINE);
//                     ModeController::INSERT_POLYGON);
//                     ModeController::INSERT_PYRAMID);
//                     ModeController::MANIPULATE_GRAPHNODE);
//               }
//               return true;
//               return false;
//               break;	    

            case 'H' :
               if (curr_Mode==ModeController::INSERT_HEMISPHERE)
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_HEMISPHERE);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_HEMISPHERE);
               }
               return true;
               break;

            case 'I' :
               if (curr_Mode==ModeController::INSERT_FEATURE)
               {
                  m_controller->setState(ModeController::MANIPULATE_FEATURE);
               }
//               else if (curr_Mode==ModeController::MANIPULATE_FEATURE)
//               {
//                  m_controller->setState( ModeController::PROPAGATE_FEATURE);
//               }
               else 
               {
                  m_controller->setState( ModeController::INSERT_FEATURE );
               }
               return true;
               break;

            case 'J' :
               if (curr_Mode==ModeController::INSERT_MODEL)
               {
                  m_controller->setState(ModeController::MANIPULATE_MODEL);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_MODEL);
               }
               return true;

            case 'K' :
               if (curr_Mode==ModeController::MANIPULATE_PLANE)
               {
                  m_controller->setState( m_controller->get_prev_State() );
               }
               else
               {
                  m_controller->setState(ModeController::MANIPULATE_PLANE);
               }
               return true;
               break;	    


            case 'L' :
               if (curr_Mode==ModeController::INSERT_POLYLINE)
               {
                  m_controller->setState(ModeController::MANIPULATE_POLYLINE);
               }
               else
               {
                  m_controller->setState(ModeController::INSERT_POLYLINE);
               }
               return true;
               break;

            case 'M' :
               cout << "M selected" << endl;
               if (curr_Mode==ModeController::SET_CENTER)
               {
                  m_controller->setState( ModeController::MANIPULATE_CENTER );
               }
               else
               {
                  m_controller->setState( ModeController::SET_CENTER );
               }
               return true;
               break;	    

// 'O' is reserved for exporting screen image to an output RGB file:

            case 'P' :
               if (curr_Mode==ModeController::INSERT_POINT)
               {
                  m_controller->setState(ModeController::MANIPULATE_POINT);
               }
               else
               {
                  m_controller->setState(ModeController::INSERT_POINT);
               }
               return true;
               break;

            case 'Q' :
               if (curr_Mode==ModeController::INSERT_RECTANGLE)
               {
                  m_controller->setState(
                     ModeController::MANIPULATE_RECTANGLE);
               }
               else
               {
                  m_controller->setState( ModeController::INSERT_RECTANGLE);
               }
               return true;
               break;

            case 'R' :
               if (curr_Mode==ModeController::RUN_MOVIE)
               {
                  m_controller->setState(ModeController::MANIPULATE_MOVIE);
               }
               else 
               {
                  m_controller->setState(ModeController::RUN_MOVIE);
               }
               return true;
               break;

            case 'T' :
               if (curr_Mode==ModeController::MANIPULATE_TRIANGLE)
               {
                  m_controller->setState(ModeController::VIEW_DATA);
               }
               else
               {
                  m_controller->setState(ModeController::MANIPULATE_TRIANGLE);
               }
               return true;
               break;

            case 'U' :
               if (curr_Mode==ModeController::FUSE_DATA)
               {
                  m_controller->setState( 
                     ModeController::MANIPULATE_FUSED_DATA);
               }
               else
               {
                  m_controller->setState( ModeController::FUSE_DATA );
               }
               return true;
               break;	    

            case 'V' :
               if (curr_Mode==ModeController::VIEW_DATA)
               {
                  m_controller->setState(ModeController::GENERATE_AVI_MOVIE);
               }
               else
               {
                  m_controller->setState( ModeController::VIEW_DATA );
               }
               return true;
               break;

// 'W' is reserved for ending waypoint entry

            case 'X' :
               if (curr_Mode==ModeController::INSERT_CYLINDER)
               {
                  m_controller->setState(ModeController::MANIPULATE_CYLINDER);
               }
               else
               {
                  m_controller->setState(ModeController::INSERT_CYLINDER);
               }
               return true;
               break;

// 'Z' is reserved for playing animation path files

            default :
               return false;
         }
      }
      default :
         return false;
   }
}


