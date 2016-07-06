// ========================================================================
// Program MATCHER pops open two windows containing 2D images.  It
// imports and displays a set of previously calculated feature
// tiepoints for both images.  The user may then enter new features in
// either window.  MATCHER performs affine fitting to the previously
// calculated tiepoints in order to estimate and display the
// corresponding location for the new feature in the other window.
// ========================================================================
// Last updated on 12/2/12; 5/24/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "video/image_matcher.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_XY_ID=passes_group.get_videopass_ID();
   int videopass_UV_ID=videopass_XY_ID+1;
//   cout << "videopass_XY_ID = " << videopass_XY_ID << endl;
//   cout << "videopass_UV_ID = " << videopass_UV_ID << endl;
//   cout << "n_passes = " << passes_group.get_n_passes() << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
//   cout << "n_photos = " << n_photos << endl;

   vector<string> photo_filenames,features_filenames;
   for (int p=0; p<n_photos; p++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(p);
      photo_filenames.push_back(photo_ptr->get_filename());
//      cout << "p = " << p << " photo_filename = " 
//           << photo_filenames.back() << endl;
//      string subdir=filefunc::getdirname(photo_filenames.back());
//      string basename=filefunc::getbasename(photo_filenames.back());
//      string prefix=stringfunc::prefix(basename);
//      string curr_features_filename=subdir+"features_2D_"+prefix+".txt";
//      cout << "Features_filename = " << curr_features_filename << endl;
//      features_filenames.push_back(curr_features_filename);
   }

// Construct dual viewers and instantiate 2 ViewerManagers:

   WindowManager* window_mgr_2D_XY_ptr=new ViewerManager();
   WindowManager* window_mgr_2D_UV_ptr=new ViewerManager();
   string window_2D_XY_title="XY image plane";
   string window_2D_UV_title="UV image plane";
   window_mgr_2D_UV_ptr->initialize_dual_windows(
      window_2D_UV_title,window_2D_XY_title,window_mgr_2D_XY_ptr);

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="MATCHER";

// Instantiate photo network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger Movies_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

// Create two OSG root nodes:

   osg::Group* root_2D_XY = new osg::Group;
   osg::Group* root_2D_UV = new osg::Group;
   
// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(ndims,window_mgr_2D_XY_ptr,display_movie_state,
                         display_movie_number,hide_Mode_HUD_flag);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();

   ModeController* ModeController_2D_XY_ptr=
      operations.get_ModeController_ptr();
   ModeController_2D_XY_ptr->setState(ModeController::MANIPULATE_FEATURE);
   window_mgr_2D_XY_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_2D_XY_ptr) );
   root_2D_XY->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_2D_UV_ptr=new ModeController();
   ModeController_2D_UV_ptr->setState(ModeController::MANIPULATE_FEATURE);
   window_mgr_2D_UV_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_2D_UV_ptr) );
   root_2D_UV->addChild(osgfunc::create_Mode_HUD(
      ndims,ModeController_2D_UV_ptr));

// Add custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_XY_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_XY_ptr,
      window_mgr_2D_XY_ptr);
   window_mgr_2D_XY_ptr->set_CameraManipulator(CM_2D_XY_ptr);

   osgGA::Custom2DManipulator* CM_2D_UV_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_UV_ptr,
      window_mgr_2D_UV_ptr);
   window_mgr_2D_UV_ptr->set_CameraManipulator(CM_2D_UV_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_2D_XY(
      window_mgr_2D_XY_ptr,ModeController_2D_XY_ptr,CM_2D_XY_ptr);
   Decorations decorations_2D_UV(
      window_mgr_2D_UV_ptr,ModeController_2D_UV_ptr,CM_2D_UV_ptr);

// Instantiate group to hold 2D movies in XY image plane:

   MoviesGroup movies_2D_XY_group(
      ndims,passes_group.get_pass_ptr(videopass_XY_ID),
      AnimationController_ptr);
   movies_2D_XY_group.set_photogroup_ptr(photogroup_ptr);
   movies_2D_XY_group.pushback_Messenger_ptr(&Movies_photo_network_messenger);
   Movie* movie_XY_ptr=movies_2D_XY_group.generate_new_Movie(
      photo_filenames[0]);
   root_2D_XY->addChild( movies_2D_XY_group.get_OSGgroup_ptr() );

   AnimationController_ptr->set_nframes(movie_XY_ptr->get_Nimages());

// Instantiate group to hold 2D movies in UV image plane:

   MoviesGroup movies_2D_UV_group(
      ndims,passes_group.get_pass_ptr(videopass_UV_ID),
      AnimationController_ptr);
   movies_2D_UV_group.set_photogroup_ptr(photogroup_ptr);
   movies_2D_UV_group.pushback_Messenger_ptr(&Movies_photo_network_messenger);
   Movie* movie_UV_ptr=movies_2D_UV_group.generate_new_Movie(
      photo_filenames[1]);
   root_2D_UV->addChild( movies_2D_UV_group.get_OSGgroup_ptr() );

// Instantiate 2D_XY and 2D_UV feature decorations group:

   FeaturesGroup* FeaturesGroup_2D_XY_ptr=decorations_2D_XY.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_XY_ID),
      NULL,movie_XY_ptr,NULL,NULL,AnimationController_ptr);
   decorations_2D_XY.set_DataNode_ptr(movie_XY_ptr->getGeode());

   FeaturesGroup* FeaturesGroup_2D_UV_ptr=decorations_2D_UV.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_UV_ID),
      NULL,movie_UV_ptr,NULL,NULL,AnimationController_ptr);
   decorations_2D_UV.set_DataNode_ptr(movie_UV_ptr->getGeode());

   FeaturesGroup_2D_XY_ptr->set_AnimationController_ptr(
      AnimationController_ptr);
   FeaturesGroup_2D_XY_ptr->set_MoviesGroup_ptr(&movies_2D_XY_group);
   FeaturesGroup_2D_XY_ptr->set_photogroup_ptr(photogroup_ptr);

   FeaturesGroup_2D_UV_ptr->set_AnimationController_ptr(
      AnimationController_ptr);
   FeaturesGroup_2D_UV_ptr->set_MoviesGroup_ptr(&movies_2D_UV_group);
   FeaturesGroup_2D_UV_ptr->set_photogroup_ptr(photogroup_ptr);

   FeaturesGroup_2D_XY_ptr->set_counterpart_FeaturesGroup_2D_ptr(
      FeaturesGroup_2D_UV_ptr);
   FeaturesGroup_2D_UV_ptr->set_counterpart_FeaturesGroup_2D_ptr(
      FeaturesGroup_2D_XY_ptr);

// Import matching XY and UV features:

   string features_subdir="./features/";
   bool display_OSG_features_flag=true;
   bool hide_singleton_features_flag=true;

   FeaturesGroup_2D_XY_ptr->read_in_photo_features(
      photogroup_ptr,features_subdir,display_OSG_features_flag,
      hide_singleton_features_flag);
   FeaturesGroup_2D_UV_ptr->read_in_photo_features(
      photogroup_ptr,features_subdir,display_OSG_features_flag,
      hide_singleton_features_flag);

// Instantiate image_matcher which can be used to dynamically compute
// new pairs of XY and UV tiepoints:

   image_matcher* image_matcher_ptr=new image_matcher(photogroup_ptr);
   image_matcher_ptr->reset_inlier_tiepoint_pairs(
      FeaturesGroup_2D_XY_ptr->get_features_map_ptr());
   image_matcher_ptr->fill_feature_coords_ID_maps();
   image_matcher_ptr->generate_VP_trees();

   FeaturesGroup_2D_XY_ptr->set_image_matcher_ptr(image_matcher_ptr);
   FeaturesGroup_2D_UV_ptr->set_image_matcher_ptr(image_matcher_ptr);

// Create the windows and run the threads:

   root_2D_XY->addChild(decorations_2D_XY.get_OSGgroup_ptr());
   root_2D_UV->addChild(decorations_2D_UV.get_OSGgroup_ptr());

   window_mgr_2D_XY_ptr->setSceneData(root_2D_XY);
   window_mgr_2D_UV_ptr->setSceneData(root_2D_UV);

   window_mgr_2D_XY_ptr->realize();
   window_mgr_2D_UV_ptr->realize();

// ========================================================================

   while( !window_mgr_2D_XY_ptr->done() && !window_mgr_2D_UV_ptr->done() )
   {
      window_mgr_2D_XY_ptr->process();
      window_mgr_2D_UV_ptr->process();
   }

   delete window_mgr_2D_XY_ptr;
   delete window_mgr_2D_UV_ptr;
   delete image_matcher_ptr;
}
