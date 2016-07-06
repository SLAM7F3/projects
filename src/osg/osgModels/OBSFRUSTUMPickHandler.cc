// ==========================================================================
// OBSFRUSTUMPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 3/19/10; 12/4/10; 9/13/12
// ==========================================================================

#include <iostream>
#include <vector>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/OBSFRUSTUMPickHandler.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/Transformer.h"
#include "math/twovector.h"
#include "osg/osgWindow/WindowManager.h"

#include "geometry/face.h"
#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void OBSFRUSTUMPickHandler::allocate_member_objects()
{
}		       

void OBSFRUSTUMPickHandler::initialize_member_objects()
{
   twoD_photo_picking_flag=false;
   mask_nonselected_OSGsubPATs_flag=true;
   disallow_OBSFRUSTUM_doubleclicking_flag=false;
   Grid_ptr=NULL;
}		       

OBSFRUSTUMPickHandler::OBSFRUSTUMPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,OBSFRUSTAGROUP* OFG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   GeometricalPickHandler(
      3,PI_ptr,CM_ptr,OFG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

OBSFRUSTUMPickHandler::~OBSFRUSTUMPickHandler() 
{
//   cout << "inside OBSFRUSTUMPickHandler destructor" << endl;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool OBSFRUSTUMPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside OFPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// Select the 3D OBSFRUSTUM whose center lies closest to (X,Y) in
// screen space:

         if (twoD_photo_picking_flag)
         {
            return select_OBSFRUSTUM(ea.getX(),ea.getY());
         }
         else
         {
            return select_OBSFRUSTUM();
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
   } // MANIPULATE_FUSED_DATA mode conditional
}

// --------------------------------------------------------------------------
bool OBSFRUSTUMPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   return false;
}

// --------------------------------------------------------------------------
bool OBSFRUSTUMPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state != ModeController::MANIPULATE_FUSED_DATA) return false;

//   cout << "inside OBSFRUSTUMPickHandler::doubleclick()" << endl;
//   cout << "disallow_OBSFRUSTUM_doubleclicking_flag = "
//        << disallow_OBSFRUSTUM_doubleclicking_flag << endl;

   if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
   {
      if (twoD_photo_picking_flag)
      {
         return select_OBSFRUSTUM(ea.getX(),ea.getY());
      }
      else
      {
         if (!disallow_OBSFRUSTUM_doubleclicking_flag && select_OBSFRUSTUM())
         {
            return OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
               get_selected_Graphical_ID());
         }
      }
   }
   return false;
}

// --------------------------------------------------------------------------
bool OBSFRUSTUMPickHandler::release()
{
//   cout << "inside OFPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      OBSFRUSTAGROUP_ptr->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// OBSFRUSTUM generation, manipulation and annihilation methods
// ==========================================================================

// Method select_OBSFRUSTUM projects the viewing pyramids 3D sides
// into the 2D screen's image plane.  It computes the minimum and
// maximum angles for the 4 projected sides.  This method next
// computes the angle which the point manually selected via a mouse
// click makes wrt the viewing pyramid's apex.  The selected point
// must lie within the "fan" defined by the projected sides by an
// OBSFRUSTUM in order for it to be selected.  It must also lie
// sufficiently close to the OBSFRUSTUM's apex.  

// If no OBSFRUSTUM is nearby the selected point,
// selected_OBSFRUSTUM_ID is set equal to -1, and all OBSFRUSTA
// are effectively de-selected.

bool OBSFRUSTUMPickHandler::select_OBSFRUSTUM()
{   
//   cout << "inside OBSFRUSTUMPickHandler::select_OBSFRUSTUM()" << endl;

   int selected_OBSFRUSTUM_ID=-1;
//   double min_p_scrn_distance=POSITIVEINFINITY;
//   double min_p_scrn_distance=0.5;
   double min_p_scrn_distance=0.65;
//    double max_p_world_distance=1000;	// meters

   for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
//      cout << "n = " << n << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;

// First check whether *OBSFRUSTUM_ptr is masked.  If so, it should
// not be selected!

      bool mask_flag=OBSFRUSTUM_ptr->get_mask(
         OBSFRUSTAGROUP_ptr->get_curr_t(),
         OBSFRUSTAGROUP_ptr->get_passnumber());
      if (mask_flag) continue;

      Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
//      cout << "Movie_ptr = " << Movie_ptr << endl;
      if (Movie_ptr==NULL) continue;


      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      threevector camera_world_posn=camera_ptr->get_world_posn();
//      cout << "camera_world_posn = " << camera_world_posn << endl;
  
// Do not attempt to select an OBSFRUSTUM if virtual camera is already
// located at its vertex!
    
      threevector curr_eye_world_posn=get_CM_3D_ptr()->get_eye_world_posn();
      if (curr_eye_world_posn.nearly_equal(camera_world_posn))
      {
//         cout << "Ignoring nearly equal OBSFRUSTUM!" << endl;
//         outputfunc::enter_continue_char();
         continue;
      }

      threevector camera_screen_posn=
         get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(camera_world_posn);
//      cout << "camera_screen_posn = " << camera_screen_posn << endl;
      
      threevector sym_world_posn=camera_world_posn-camera_ptr->get_What();

      double theta_s=ray_angle_wrt_camera_posn(
         sym_world_posn,camera_screen_posn);

// Project corner world rays into screen space.  Then compute their
// projections angles wrt 2D screenspace.  Store minimal and maximal
// angle values.  In order for an OBSFRUSTUM to be picked, we'll
// require that the picked point's screen space location must have
// angle angle wrt the OBSFRUSTUM's apex position within this extremal
// angular range:

      vector<threevector> UV_corner_world_ray=
         camera_ptr->get_UV_corner_world_ray();
      twovector theta_limit(POSITIVEINFINITY,NEGATIVEINFINITY);
      for (unsigned int c=0; c<UV_corner_world_ray.size(); c++)
      {
         threevector ray_world_posn=camera_world_posn+
            UV_corner_world_ray[c];
         double theta_r=ray_angle_wrt_camera_posn(
            ray_world_posn,camera_screen_posn);
         theta_r=basic_math::phase_to_canonical_interval(
            theta_r,theta_s-PI,theta_s+PI);

         theta_limit.put(0,basic_math::min(theta_limit.get(0),theta_r));
         theta_limit.put(1,basic_math::max(theta_limit.get(1),theta_r));
      } // loop over index c labeling corner world rays

// Form ray phat emanating from camera's to current voxel's screen
// positions.  Then compute its 2D angle theta_p in screen space:

      threevector p_scrn=curr_voxel_screenspace_posn-camera_screen_posn;
      p_scrn.put(2,0);
      threevector phat=p_scrn.unitvector();
      double theta_p=atan2(phat.get(1),phat.get(0));
      theta_p=basic_math::phase_to_canonical_interval(
         theta_p,theta_s-PI,theta_s+PI);

//      cout << "min_theta = " << theta_limit.get(0)*180/PI
//           << " theta_p = " << theta_p*180/PI 
//           << " max_theta = " << theta_limit.get(1)*180/PI << endl;

//      cout << "n = " << n 
//           << " min_p_scrn_distance = " << min_p_scrn_distance << endl;
//      cout << "p_scrn.magnitude() = " << p_scrn.magnitude() << endl;
    
      if (theta_p > theta_limit.get(0) && theta_p < theta_limit.get(1) &&
          p_scrn.magnitude() < min_p_scrn_distance)
      {
         min_p_scrn_distance=p_scrn.magnitude();
         selected_OBSFRUSTUM_ID=n;
      }
   } // loop over index n labeling OBSFRUSTA

   set_selected_Graphical_ID(selected_OBSFRUSTUM_ID);
//   cout << "Selected OBSFRUSTUM ID = " << selected_OBSFRUSTUM_ID << endl;
//   cout << "Selected OSGsubPAT = " 
//        << OBSFRUSTAGROUP_ptr->get_selected_OSGsubPAT_ID()
//        << endl;

   if (selected_OBSFRUSTUM_ID >= 0 && 
       OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr() != NULL)
   {

// As of 10/15/07, we choose to pause every movie that is playing
// within a non-selected OBSFRUSTUM.  We play only the movie within a
// selected OBSFRUSTUM.  We also mask the ImageNumberHUD if the
// selected OBSFRUSTUM corresponds to a single photo rather than a
// video with multiple frames:

      OBSFRUSTAGROUP_ptr->pause_all_videos();

      OBSFRUSTUM* selected_OBSFRUSTUM_ptr=
         OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(
            selected_OBSFRUSTUM_ID);

      if (selected_OBSFRUSTUM_ptr->get_Movie_ptr()->get_Nimages() > 1)
      {
         OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
            set_display_movie_number_flag(true);
         OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
            set_display_movie_state_flag(true);
         selected_OBSFRUSTUM_ptr->get_AnimationController_ptr()->
            setState(AnimationController::PLAY);
      }
      else
      {
         OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
            set_display_movie_number_flag(false);
         OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
            set_display_movie_state_flag(false);
      }
   } // selected_OBSFRUSTUM_ID >= 0 && ImageNumberHUD_ptr != NULL conditional

// For NYC demo, we want to only display related clusters of OBSFRUSTA
// at any given time.  But for video panorama demo, we want to display
// different OBSFRUSTA OSGsubPATs simultaneously:

//   cout << "mask_nonselected_OSGsubPATs_flag = "
//        << mask_nonselected_OSGsubPATs_flag << endl;
   if (mask_nonselected_OSGsubPATs_flag)
   {
      OBSFRUSTAGROUP_ptr->mask_nonselected_OSGsubPATs();
   }
   else
   {
      OBSFRUSTAGROUP_ptr->reset_frustum_colors_based_on_Zcolormap();
   }
   
   if (selected_OBSFRUSTUM_ID==-1 && Grid_ptr != NULL) 
   {
      Grid_ptr->set_mask(1);
   }

   cout << endl;
   cout << "selected_OBSFRUSTUM_ID = " << selected_OBSFRUSTUM_ID << endl;
   if (selected_OBSFRUSTUM_ID >= 0)
   {
      OBSFRUSTUM* selected_OBSFRUSTUM_ptr=
         OBSFRUSTAGROUP_ptr->get_selected_OBSFRUSTUM_ptr();
      string image_filename=selected_OBSFRUSTUM_ptr->
         get_Movie_ptr()->get_video_filename();
      cout << "Selected image filename = " << image_filename << endl;
      string package_filename=OBSFRUSTAGROUP_ptr->get_package_filename(
         image_filename);
      if (package_filename.size() > 0)
      {
         cout << "Package file corresponding to image = " << endl;
         cout << package_filename << endl;
      }
      cout << endl;
   }

   return (selected_OBSFRUSTUM_ID > -1);
}

// --------------------------------------------------------------------------
// Member function ray_angle_wrt_camera_posn is a little private
// utility method which computes the angle which V_world_posn
// projected into the screen's image plane makes wrt
// camera_screen_posn.

double OBSFRUSTUMPickHandler::ray_angle_wrt_camera_posn(
   const threevector& V_world_posn,const threevector& camera_screen_posn)
{   
//   cout << "inside OBSFRUSTUMPickHandler::ray_angle_wrt_camera_posn()" 
//        << endl;
   
   threevector V_screen_posn=
      get_CM_3D_ptr()->get_Transformer_ptr()->
      world_to_screen_transformation(V_world_posn);

   threevector v_hat=V_screen_posn-camera_screen_posn;
   v_hat.put(2,0);

   v_hat=v_hat.unitvector();
   double theta_v=atan2(v_hat.get(1),v_hat.get(0));

   return theta_v;
}

// --------------------------------------------------------------------------
// This specialized overloaded version of select_OBSFRUSTUM was
// written for 2D photo picking.  It takes in the screen coordinates
// (U,V) of the current mouse click.  It assumes that all OBSFRUSTA
// are oriented with their symmetry axes aligned with world Z_hat.  It
// further assumes that the photos lie within the Z=0 plane with their
// horizontal and vertical edges aligned with world X_hat and world
// Y_hat.  This method computes the UV bounding box for each photo in
// the Z=0 plane.  If the mouse click coords lie within a photo's UV
// bbox, that photo's OBSFRUSTUM is selected by this method.

bool OBSFRUSTUMPickHandler::select_OBSFRUSTUM(double U,double V)
{   
//   cout << "inside OFPH::select_OBSFRUSTUM()" << endl;

   OBSFRUSTAGROUP_ptr->compute_current_zplane_UV_bboxes();

   int selected_OBSFRUSTUM_ID=-1;
   for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      bounding_box curr_bbox=
         OBSFRUSTAGROUP_ptr->get_photo_zplane_bboxes().at(n);
      if (curr_bbox.point_inside(U,V))
      {
         selected_OBSFRUSTUM_ID=n;
      }
   } // loop over index n labeling OBSFRUSTA
   set_selected_Graphical_ID(selected_OBSFRUSTUM_ID);
   cout << "Selected OBSFRUSTUM ID = " << selected_OBSFRUSTUM_ID << endl;

   OBSFRUSTAGROUP_ptr->reset_zface_color();
   if (mask_nonselected_OSGsubPATs_flag)
   {
      OBSFRUSTAGROUP_ptr->mask_nonselected_OSGsubPATs();
   }
   return (selected_OBSFRUSTUM_ID > -1);
}
