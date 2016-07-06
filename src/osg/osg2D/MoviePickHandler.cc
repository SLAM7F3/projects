// ==========================================================================
// MoviePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 12/29/06; 9/2/08; 2/9/11
// ==========================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <set>
#include "osg/CustomManipulator.h"
#include "osg/osg2D/Movie.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MoviePickHandler.h"
#include "osg/ModeController.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MoviePickHandler::allocate_member_objects()
{
}		       

void MoviePickHandler::initialize_member_objects()
{
}		       

MoviePickHandler::MoviePickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   MoviesGroup* MG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr):
   GraphicalPickHandler(p_ndims,PI_ptr,CM_ptr,MG_ptr,MC_ptr,WCC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   MoviesGroup_ptr=MG_ptr;
}

MoviePickHandler::~MoviePickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool MoviePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
   cout << "inside MPH::pick()" << endl;
   if (GraphicalPickHandler::pick(ea))
   {
      if (get_ModeController_ptr()->getState()==ModeController::RUN_MOVIE)
      {
         return (select_Graphical() > -1);
      }
   }
   return false;
}

// --------------------------------------------------------------------------
bool MoviePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   cout << "inside MoviePickHandler::drag()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::RUN_MOVIE)
   {
      return GraphicalPickHandler::drag(ea);
   }
   else
   {
      return false;
   }
}

// -------------------------------------------------------------------------
bool MoviePickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::RUN_MOVIE)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else
   {
      return false;
   } // RUN_MOVIE mode conditional
}

// --------------------------------------------------------------------------
bool MoviePickHandler::release()
{
//   cout << "inside MPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   return (curr_state==ModeController::RUN_MOVIE);
}

// ==========================================================================
// Movie generation, manipulation and annihilation methods
// ==========================================================================

// Member function get_max_distance_to_Graphical hardwires in
// reasonable world-space distances within which a mouse click must
// lie in order to select some Movie.

float MoviePickHandler::get_max_distance_to_Graphical()
{
   float max_dist=-1.0f;
   if (get_ndims()==2)
   {
      max_dist=1.0f;
   }
   else if (get_ndims()==3)
   {
      max_dist=100.0f;
   }
   return max_dist;
}
