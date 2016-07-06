// ========================================================================
// This specialized version of program VPLAYER is an OSG based viewer
// for WISP imagery.  This variant is specifically intended to play
// temporal sequences of DIME panels with aerial track features
// superposed for comparison.  It queries the user to enter scene
// and panel IDs for some set of processed WISP & aerial track data.
// ========================================================================
// Last updated on 4/10/13; 6/26/13; 7/18/13
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

#include "osg/osg3D/Terrain_Manipulator.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Instantiate powerpoint and wiki messengers:

//   int pass_ID=passes_group.get_n_passes()-1;
//   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
//      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string wiki_message_queue_channel_name="wiki";
   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool display_movie_state=false;		// viewgraphs
//   bool display_movie_number=false;		// viewgraphs
//   bool display_movie_world_time=false;		// viewgraphs
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());
   
   operations.reset_AnimationController_world_time_params();

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate points, polylines, polygons, linesegments, triangles,
// rectangles and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(
         videopass_ID));
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      decorations.add_LineSegments(
         ndims,passes_group.get_pass_ptr(videopass_ID),
         AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
//      decorations.get_PointsGroup_ptr(),
//      decorations.get_PolygonsGroup_ptr(),
      AnimationController_ptr);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   cout << "movie_filename = " << movie_filename << endl;
   string image_subdir=filefunc::getdirname(movie_filename);
   if (image_subdir.size()==0) image_subdir="./";
   cout << "image_subdir = " << image_subdir << endl;

   AnimationController_ptr->store_ordered_image_filenames(image_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
   cout << "number_of_images = " << number_of_images << endl;

   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   int min_photo_number=-1;
   int max_photo_number=-1;
   videofunc::find_min_max_photo_numbers(
      image_subdir,min_photo_number,max_photo_number);
   int Nimages=max_photo_number-min_photo_number+1;
   if (number_of_images > Nimages)
   {
      Nimages=number_of_images;
      min_photo_number=0;
      max_photo_number=Nimages-1;
   }

   cout << "min_photo_number = " << min_photo_number
        << " max_photo_number = " << max_photo_number
        << " Nimages = " << Nimages << endl;

//   AnimationController_ptr->set_first_framenumber(min_photo_number);
//   AnimationController_ptr->set_curr_framenumber(min_photo_number);
   AnimationController_ptr->set_nframes(Nimages);
   AnimationController_ptr->set_frame_counter_offset(min_photo_number);

   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(
      min_photo_number);
   movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(
      max_photo_number);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);

// Instantiate 3D SignPosts decorations group:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      3,passes_group.get_pass_ptr(videopass_ID));
   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

   SignPostsGroup_ptr->set_AnimationController_ptr(AnimationController_ptr);
   SignPostsGroup_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   SignPostsGroup_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
   SignPostsGroup_ptr->set_MoviesGroup_ptr(&movies_group);

// Instantiate 2D imageplane SignPosts group and pick handler:

   threevector* grid_world_origin_ptr=NULL;
   SignPostsGroup* imageplane_SignPostsGroup_ptr=new SignPostsGroup(
      2,passes_group.get_pass_ptr(videopass_ID),
      grid_world_origin_ptr);
   imageplane_SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);

   SignPostPickHandler* imageplane_SignPostPickHandler_ptr=
      new SignPostPickHandler(
         2,passes_group.get_pass_ptr(videopass_ID),CM_2D_ptr,
         imageplane_SignPostsGroup_ptr,ModeController_ptr,
         window_mgr_ptr,grid_world_origin_ptr);

   imageplane_SignPostPickHandler_ptr->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);
   imageplane_SignPostPickHandler_ptr->set_DataNode_ptr(
      movie_ptr->getGeode());
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      imageplane_SignPostPickHandler_ptr);

   SignPostsGroup_ptr->set_imageplane_SignPostsGroup_ptr(
      imageplane_SignPostsGroup_ptr);
   SignPostsGroup_ptr->set_imageplane_SignPostPickHandler_ptr(
      imageplane_SignPostPickHandler_ptr);

   decorations.get_OSGgroup_ptr()->addChild(imageplane_SignPostsGroup_ptr->
                                            get_OSGgroup_ptr());

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      CM_2D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);

// Import UV feature coordinates for 3D aircraft targets projected
// into 2D WISP image planes:

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      &centers_group,movie_ptr,decorations.get_TrianglesGroup_ptr(),
      decorations.get_LineSegmentsGroup_ptr(),AnimationController_ptr);
   FeaturesGroup_ptr->set_display_feature_scores_flag(true);

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);

   int panel_ID;
   cout << "Enter panel ID:" << endl;
   cin >> panel_ID;
   string panel_ID_str=stringfunc::integer_to_string(panel_ID,1);

   string fieldtest_subdir=
      "/data/DIME/bundler/DIME/May2013_Fieldtest/05202013/";
   string scene_subdir=fieldtest_subdir+"Scene"+scene_ID_str+"/";
   string aerial_tracks_subdir=scene_subdir+"aerial_tracks/";
   string input_features_filename=aerial_tracks_subdir+
      "features_2D_AERIAL_"+panel_ID_str+".txt";
   cout << "input_features_filename = " << input_features_filename << endl;
   FeaturesGroup_ptr->read_feature_info_from_file(
      input_features_filename);


   decorations.set_DataNode_ptr(movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
}

