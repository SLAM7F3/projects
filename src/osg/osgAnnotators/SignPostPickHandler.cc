// ==========================================================================
// SignPostPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 12/27/10; 1/29/11; 1/30/11
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgAnnotators/SignPostPickHandler.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SignPostPickHandler::allocate_member_objects()
{
}		       

void SignPostPickHandler::initialize_member_objects()
{
   allow_doubleclick_in_manipulate_fused_data_mode=false;
}		       

SignPostPickHandler::SignPostPickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   SignPostsGroup* SPG_ptr,ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(p_ndims,PI_ptr,CM_ptr,SPG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   SignPostsGroup_ptr=SPG_ptr;
}

SignPostPickHandler::~SignPostPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool SignPostPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside SignPostPickHandler::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_ANNOTATION ||
       curr_state==ModeController::MANIPULATE_ANNOTATION)
   {
      if (GraphicalPickHandler::pick(ea))
      {

// If ModeController==INSERT_ANNOTATION, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D SignPost whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_ANNOTATION)
         {
            bool flag=instantiate_SignPost(ea.getX(),ea.getY());

// On 5/4/09, we experiment with resetting Mode to previous state
// after SignPost insertion is complete:

            get_ModeController_ptr()->setState(
               get_ModeController_ptr()->get_prev_State());
            if (get_ndims()==3) SignPostsGroup_ptr->update_colors();
            SignPostsGroup_ptr->reset_colors();

            SignPostsGroup_ptr->broadcast_NFOV_aimpoint();
            SignPostsGroup_ptr->set_Geometricals_updated_flag(true);

            return flag;
         }
         else
         {
            return select_SignPost();
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
   } // INSERT_ANNOTATION or MANIPULATE_ANNOTATION mode conditional
}

// --------------------------------------------------------------------------
bool SignPostPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside SignPostPickHandler::drag()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();

   bool drag_handled=false;
   if (curr_state==ModeController::INSERT_ANNOTATION ||
      curr_state==ModeController::MANIPULATE_ANNOTATION)
   {
      drag_handled=GraphicalPickHandler::drag(ea);

      if (pass_ptr->get_UTM_zonenumber() > 0)
      {
         SignPost* curr_SignPost_ptr=SignPostsGroup_ptr->
            get_ID_labeled_SignPost_ptr(get_selected_Graphical_ID());
         if (curr_SignPost_ptr != NULL)
         {
            threevector UTM_coords;
            if (curr_SignPost_ptr->get_transformed_UVW_coords(
               SignPostsGroup_ptr->get_curr_t(),
               SignPostsGroup_ptr->get_passnumber(),UTM_coords))
            {
               if (SignPostsGroup_ptr->convert_UTM_to_LLA_coords(
                  UTM_coords,curr_SignPost_ptr))
               {
//                     SignPostsGroup_ptr->update_SKS_worldmodel_database_entry(
//                        curr_SignPost_ptr);
               }
            }
         } // curr_SignPost_ptr != NULL conditional
      } // UTM_zonenumber > 0 conditional

   } // INSERT_ANNOTATION or MANIPULATE_ANNOTATION mode conditional
   return drag_handled;
}

// --------------------------------------------------------------------------
bool SignPostPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside SignPostPickHandler::doubleclick()" << endl;
//   cout << "this = " << this << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

// In order to simplify the NYC touch table demo, we allow users to
// double click on GIS annotations if they are within
// MANIPULATE_FUSED_DATA as well as MANIPULATE_ANNOTATION mode:

   if ( curr_state != ModeController::MANIPULATE_ANNOTATION &&
        curr_state != ModeController::RUN_MOVIE &&
        curr_state != ModeController::MANIPULATE_FUSED_DATA )
   {
      return false;
   }
   
//   cout << "allow_doubleclick_in_manipulate_fused_data_mode = "
//        << allow_doubleclick_in_manipulate_fused_data_mode << endl;
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA &&
       !allow_doubleclick_in_manipulate_fused_data_mode)
   {
      return false;
   }

   if (GraphicalPickHandler::pick(ea))
   {
      bool selection_flag=select_SignPost();
      bool message_flag=SignPostsGroup_ptr->issue_invocation_message();
      return selection_flag && message_flag;
   }
   return false;
}

// --------------------------------------------------------------------------
bool SignPostPickHandler::release()
{
//   cout << "inside SignPostPickHandler::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_ANNOTATION ||
       curr_state==ModeController::MANIPULATE_ANNOTATION)
   {
      SignPostsGroup_ptr->update_colors();
      SignPostsGroup_ptr->reset_colors();

// On 5/4/09, we experiment with resetting Mode to previous state
// after SignPost insertion is complete:

//      get_ModeController_ptr()->setState(
//         get_ModeController_ptr()->get_prev_State());

      SignPostsGroup_ptr->set_Geometricals_updated_flag(true);
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// SignPost generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_SignPost creates a new SignPost, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_SignPost_number equal to that ID and adds it to
// the OSG SignPost group.

bool SignPostPickHandler::instantiate_SignPost(double X,double Y)
{   
//   cout << "inside SignPostPickHandler::instantiate_SignPost()" << endl;

   bool signpost_instantiated_flag=false;
   if (get_ndims()==3)
   {
      signpost_instantiated_flag=GraphicalPickHandler::pick_3D_point(X,Y);
   }
   else
   {
      signpost_instantiated_flag=true;
   }

   if (signpost_instantiated_flag)
   {
      double common_SignPost_size=SignPostsGroup_ptr->
         get_common_geometrical_size();
      SignPost* curr_SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost(
         common_SignPost_size,1);
      instantiate_Graphical(curr_SignPost_ptr);

      SignPostsGroup_ptr->edit_SignPost_label();

// As of 1/30/2011, we prefer for SignPosts appearing against 2D
// photos to be pure-hue colored rather than white as soon as they are
// instantiated.  So we deselect any new 2D SignPost as soon as it's
// instantiated:

      if (get_ndims()==2)
      {
         set_selected_Graphical_ID(-1);
      }
      
      SignPostsGroup_ptr->reset_colors();

   } // signpost_instantiated_flag conditional
   return signpost_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_SignPost assigns selected_SignPost_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no SignPost is nearby the
// selected point, selected_SignPost_number is set equal to -1, and all
// SignPosts are effectively de-selected.

bool SignPostPickHandler::select_SignPost()
{   
//   cout << "inside SignPostPickHandler::select_SignPost()" << endl;
   int selected_signpost_ID=select_Graphical();
//   cout << "selected_signpost_ID = " << selected_signpost_ID << endl;

   SignPostsGroup_ptr->reset_colors();
   return (selected_signpost_ID > -1);
}

// --------------------------------------------------------------------------
// Member function get_max_distance_to_Graphical hardwires in
// reasonable screen-space distances within which a mouse click must
// lie in order to select some Geometrical.

float SignPostPickHandler::get_max_distance_to_Graphical()
{
//   cout << "inside SPPH::get_max_dist_to_Graphical()" << endl;
   double max_dist=0.1;
//   cout << "max_dist = " << max_dist << endl;
   return max_dist;
}
