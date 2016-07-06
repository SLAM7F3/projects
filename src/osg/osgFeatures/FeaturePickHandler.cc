// ==========================================================================
// FeaturePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 12/26/10; 2/9/11; 9/9/13; 10/16/13
// ==========================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <vector>
#include "osg/CustomManipulator.h"
#include "osg/osgFeatures/Feature.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturePickHandler.h"
#include "osg/ModeController.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgSceneGraph/ParentVisitor.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FeaturePickHandler::allocate_member_objects()
{
}		       

void FeaturePickHandler::initialize_member_objects()
{
   convert_3D_to_2D_flag=false;
   insert_single_feature_flag=false;
   OBSFRUSTAGROUP_ptr=NULL;
}		       

FeaturePickHandler::FeaturePickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   FeaturesGroup* FG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr,threevector* GO_ptr):
   osgGeometry::PointPickHandler(p_ndims,PI_ptr,CM_ptr,FG_ptr,MC_ptr,WCC_ptr,
                                 GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   FeaturesGroup_ptr=FG_ptr;
}

FeaturePickHandler::FeaturePickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   FeaturesGroup* FG_ptr,OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr,threevector* GO_ptr):
   osgGeometry::PointPickHandler(p_ndims,PI_ptr,CM_ptr,FG_ptr,MC_ptr,WCC_ptr,
                                 GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   FeaturesGroup_ptr=FG_ptr;
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

FeaturePickHandler::~FeaturePickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool FeaturePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside FeaturePickHandler::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
//   cout << "curr_state = " << curr_state << endl;

   if (GraphicalPickHandler::pick(ea))
   {
      if (curr_state==ModeController::INSERT_FEATURE)
      {
         bool flag=instantiate_feature(ea.getX(),ea.getY());
         FeaturesGroup_ptr->set_Geometricals_updated_flag(true);

         if (flag && insert_single_feature_flag)
         {
            get_ModeController_ptr()->setState(
               ModeController::MANIPULATE_FEATURE);
         }
         
         return flag;
      }
      else if (curr_state==ModeController::PROPAGATE_FEATURE)
      {
         return propagate_feature();
      }
      else if (get_ModeController_ptr()->getState()==
               ModeController::MANIPULATE_FEATURE ||
               get_ModeController_ptr()->getState()==
               ModeController::TRACK_FEATURE)
      {

// Select the 3D crosshairs whose center lies closest to (X,Y) in
// screen space:

         return select_feature();
      }
   }
   return false;
}

// --------------------------------------------------------------------------
bool FeaturePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside FeaturePickHandler::drag()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_FEATURE ||
       curr_state==ModeController::MANIPULATE_FEATURE)
   {
      FeaturesGroup_ptr->set_dragging_feature_flag(true);
      Movie* Movie_ptr = FeaturesGroup_ptr->get_Movie_ptr();
      if (Movie_ptr != NULL)
      {
         int xdim = Movie_ptr->getWidth();
         int ydim = Movie_ptr->getHeight();

// Print out pixel coordinates for currently selected Feature:

         Graphical* curr_Graphical_ptr=FeaturesGroup_ptr->
            get_selected_Graphical_ptr();
         if (curr_Graphical_ptr != NULL)
         {
            threevector Graphical_posn;
            if (curr_Graphical_ptr->get_UVW_coords(
                   get_curr_t(),get_passnumber(),Graphical_posn))
            {
               double u = Graphical_posn.get(0);
               double v = Graphical_posn.get(1);
               unsigned int pu,pv;
               Movie_ptr->get_pixel_coords(u,v,pu,pv);
               cout << " pu = " << pu << " pv = " << pv 
                    << " xdim = " << xdim << " ydim = " << ydim << endl;
            }
         }
      }
      
      return GraphicalPickHandler::drag(ea);
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool FeaturePickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_FEATURE)
   {
      rotation_mode = !rotation_mode;
      if (rotation_mode)
      {
         cout << "Feature rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Feature rotation mode toggled off" << endl;
      }
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool FeaturePickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FEATURE)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else       
   {
      return false;
   } // MANIPULATE_FEATURE mode conditional
}

// --------------------------------------------------------------------------
bool FeaturePickHandler::release()
{
//   cout << "inside FeaturePickHandler::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

   if (curr_state==ModeController::INSERT_FEATURE ||
       curr_state==ModeController::MANIPULATE_FEATURE ||
       curr_state==ModeController::TRACK_FEATURE)
       
   {
      FeaturesGroup_ptr->reset_colors();
//   cout << "selected feature number = "
//        << get_selected_Graphical_ID() << endl;

      FeaturesGroup_ptr->set_Geometricals_updated_flag(true);
      FeaturesGroup_ptr->set_dragging_feature_flag(false);





      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Feature generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_feature creates a new feature, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_feature_number equal to that ID and adds it to
// the OSG feature group.

bool FeaturePickHandler::instantiate_feature(double X,double Y)
{   
//   cout << "inside FeaturePickHandler::instantiate_feature()" << endl;
//   cout << "X = " << X << " Y = " << Y << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   bool feature_instantiated_flag=false;
   if (get_ndims()==3)
   {
      feature_instantiated_flag=pick_3D_point(X,Y);
   }
   else
   {
      feature_instantiated_flag=true;
   }

   if (feature_instantiated_flag)
   {
      Feature* curr_feature_ptr=FeaturesGroup_ptr->generate_new_Feature(
         ParentVisitor_refptr->get_earth_flag());
      instantiate_Graphical(curr_feature_ptr);

//      threevector UVW;
//      curr_feature_ptr->get_UVW_coords(
//         FeaturesGroup_ptr->get_curr_t(),
//         FeaturesGroup_ptr->get_passnumber(),UVW);
//      cout << "curr_feature_ptr = " << curr_feature_ptr << endl;
//      cout << "UVW = " << UVW << endl;

      if (convert_3D_to_2D_flag && OBSFRUSTAGROUP_ptr != NULL)
      {
         FeaturesGroup_ptr->set_ndims(2);
         FeaturesGroup_ptr->set_crosshairs_size(1);
         FeaturesGroup_ptr->set_crosshairs_text_size(1);

         curr_feature_ptr->set_ndims(2);
         curr_feature_ptr->set_dim_dependent_colors();
         curr_feature_ptr->set_UVW_dirs(
            FeaturesGroup_ptr->get_curr_t(),
            FeaturesGroup_ptr->get_passnumber(),x_hat,-z_hat);

         int OBSFRUSTUM_ID;
         twovector video_UV;
         OBSFRUSTAGROUP_ptr->convert_screenspace_to_photo_coords(
            curr_voxel_screenspace_posn,OBSFRUSTUM_ID,video_UV);
//      cout << "curr_voxel_screenspace_posn = " << curr_voxel_screenspace_posn
//           << endl;
         cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID 
              << " video_UV = " << video_UV << endl;

         threevector XYZ;
         if (recover_photo_feature_world_coords(OBSFRUSTUM_ID,video_UV,XYZ))
         {
            cout << "Recovered XYZ = " << XYZ << endl;
            threevector stored_UVW;
            curr_feature_ptr->get_UVW_coords(
               FeaturesGroup_ptr->get_curr_t(),
               FeaturesGroup_ptr->get_passnumber(),stored_UVW);
            cout << "FeaturesGroup_ptr->get_passnumber() = "
                 << FeaturesGroup_ptr->get_passnumber() << endl;
            cout << "Stored UVW = " << stored_UVW << endl;
            curr_feature_ptr->set_UVW_coords(
               FeaturesGroup_ptr->get_curr_t(),OBSFRUSTUM_ID,XYZ);

            for (int pass_number=0; pass_number < 20; pass_number++)
            {
               cout << "pass_number = " << pass_number << endl;
               if (curr_feature_ptr->get_UVW_coords(
                  FeaturesGroup_ptr->get_curr_t(),pass_number,stored_UVW))
               {
                  cout << "UVW coords = " << stored_UVW << endl;
               }
            }

            instantaneous_obs* instantaneous_obs_ptr=curr_feature_ptr->
               get_particular_time_obs(
                  FeaturesGroup_ptr->get_curr_t(),
                  FeaturesGroup_ptr->get_passnumber());
            cout << "*instantaneous_obs_ptr = "
                 << *instantaneous_obs_ptr << endl;
         }
      
         if (OBSFRUSTUM_ID==-1)
         {
            FeaturesGroup_ptr->destroy_Graphical(curr_feature_ptr);
         }
         else
         {
//            FeaturesGroup_ptr->get_or_create_feature_geode(
//               FeaturesGroup_ptr->get_curr_t(),
//               curr_feature_ptr,ParentVisitor_refptr->get_earth_flag());
         }
      
         FeaturesGroup_ptr->set_ndims(3);
         curr_feature_ptr->set_ndims(3);
      }
      else
      {
//         FeaturesGroup_ptr->get_or_create_feature_geode(
//            FeaturesGroup_ptr->get_curr_t(),curr_feature_ptr,
//            ParentVisitor_refptr->get_earth_flag());
      } // convert_3D_to_2D_flag && OBSFRUSTAGROUP_ptr != NULL  
   } // feature_instantiated_flag 
   
   return feature_instantiated_flag;	// always true as of 9/2/08
}

// --------------------------------------------------------------------------
// Method propagate_feature creates a new feature, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_feature_number equal to that ID and adds it to
// the OSG feature group.

bool FeaturePickHandler::propagate_feature()
{   
   Feature* curr_feature_ptr=FeaturesGroup_ptr->generate_new_Feature();
   instantiate_Graphical(curr_feature_ptr);

   FeaturesGroup_ptr->mark_feature(curr_feature_ptr->get_ID());
   FeaturesGroup_ptr->propagate_feature_over_pass(curr_feature_ptr);
   FeaturesGroup_ptr->reset_colors();

   return true;
}

// --------------------------------------------------------------------------
// Method select_feature assigns selected_feature_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no feature is nearby the
// selected point, selected_feature_number is set equal to -1, and all
// features are effectively de-selected.

bool FeaturePickHandler::select_feature()
{   
//   cout << "inside FeaturePickHandler::select_feature()" << endl;
//   cout << "get_passnumber() = " << get_passnumber() << endl;
   
   int feature_ID=select_Graphical();

   if (get_ModeController_ptr()->getState()==ModeController::TRACK_FEATURE)
   {
      int curr_ID=get_selected_Graphical_ID();
      FeaturesGroup_ptr->mark_feature(curr_ID);
   }

   FeaturesGroup_ptr->reset_colors();
   return (feature_ID > -1);
}

// ==========================================================================
// 2D photo member functions
// ==========================================================================

// Member function recover_photo_feature_world_coords takes in the ID
// (=passnumber) for some 2D photo which we assume sits within the
// grid's Z-plane.  It also takes in the UV coordinates of some
// feature relative to the photo's UV axes.  This method recovers and
// returns the XYZ world coords of the feature.

bool FeaturePickHandler::recover_photo_feature_world_coords(
   int OBSFRUSTUM_ID,const twovector& video_UV,threevector& XYZ)
{   
//   cout << "inside FPH::recover_photo_feature_world_coords()" << endl;

   if (convert_3D_to_2D_flag && OBSFRUSTAGROUP_ptr != NULL &&
       OBSFRUSTUM_ID >= 0)
   {
      OBSFRUSTAGROUP_ptr->convert_photo_to_screenspace_coords(
         OBSFRUSTUM_ID,video_UV,XYZ);
//      cout << "Recovered screenspace posn = " << XYZ << endl;
      pick_point_on_Zplane(XYZ.get(0),XYZ.get(1),get_grid_origin().get(2));
//      cout << "Recovered curr_voxel_worldspace_posn = "
//           << curr_voxel_worldspace_posn << endl;
      XYZ=curr_voxel_worldspace_posn;
      return true;
   }
   else
   {
      return false;
   }
}
