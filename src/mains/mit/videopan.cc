// ========================================================================
// Program VIDEOPAN is a testing grounds for visualizing dynamic video
// superposed on top of a static panorama image.

//		videopan --region_filename ./packages/lobby7.pkg 

// ========================================================================
// Last updated on 9/28/08; 9/29/08; 10/1/08
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "passes/TextDialogBox.h"
#include "osg/osgWindow/ViewerManager.h"

#include "filter/filterfuncs.h"
#include "time/timefuncs.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;
   cout << "n_passes = " << passes_group.get_n_passes() << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
//   bool display_movie_world_time=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Utilize any frame-to-worldtime information entered via input
// package files:

   PassInfo* video_passinfo_ptr=
      passes_group.get_passinfo_ptr(videopass_ID);
   if (video_passinfo_ptr != NULL)
   {
      AnimationController_ptr->correlate_frames_to_world_times(
         video_passinfo_ptr->get_frame_times());
   }

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate points, polygons, linesegments, triangles, rectangles
// and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      decorations.add_LineSegments(
         ndims,passes_group.get_pass_ptr(videopass_ID),
         AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));

/*
   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   SignPostsGroup_ptr->set_common_geometrical_size(0.003);
   double curr_size=SignPostsGroup_ptr->get_common_geometrical_size();
   double height_multiplier=0.03;

   twovector UV(1.77927682012 , 0.775438185791);
   SignPost* SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost_on_video(
      UV,curr_size,height_multiplier);
   SignPost_ptr->set_label("Rogers Building");
   SignPost_ptr->set_max_text_width("Buildi");
   SignPost_ptr->set_permanent_color(colorfunc::red);
   SignPost_ptr->change_text_size(0,1.7);

   UV=twovector(2.29368531131,  0.624565827628);
   SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost_on_video(
      UV,curr_size,height_multiplier);
   SignPost_ptr->set_label("Pratt School");
   SignPost_ptr->set_max_text_width("School");
   SignPost_ptr->set_permanent_color(colorfunc::red);
   SignPost_ptr->change_text_size(0,1.7);

   UV=twovector( 0.78478234994 , 0.650580470071);
   SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost_on_video(
      UV,curr_size,height_multiplier);
   SignPost_ptr->set_label("Advanced Educational Services");
   SignPost_ptr->set_max_text_width("Educational");
   SignPost_ptr->set_permanent_color(colorfunc::red);
   SignPost_ptr->change_text_size(0,1.7);

   UV=twovector( 0.23954892348 , 0.646593980391);
   SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost_on_video(
      UV,curr_size,height_multiplier);
   SignPost_ptr->set_label("Guggenheim Lab");
   SignPost_ptr->set_max_text_width("Guggen");
   SignPost_ptr->set_permanent_color(colorfunc::red);
   SignPost_ptr->change_text_size(0,1.7);

   UV=twovector(  1.21705430614 , 0.301736625374);
   SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost_on_video(
      UV,curr_size,height_multiplier);
   SignPost_ptr->set_label("Massachusetts Avenue");
   SignPost_ptr->set_max_text_width("Massachusetts");
   SignPost_ptr->set_permanent_color(colorfunc::red);
   SignPost_ptr->change_text_size(0,1.7);

   SignPostsGroup_ptr->reset_colors();
*/

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),AnimationController_ptr);
   root->addChild( movies_group.get_OSGgroup_ptr() );

// Instantiate first movie to hold background panorama:

   string panorama_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   Movie* panorama_movie_ptr=movies_group.generate_new_Movie(
      panorama_filename);

// Instantiate second movie to hold dynamic video frames:

   Pass* video_pass_ptr=passes_group.get_pass_ptr(1);
   string video_filename=video_pass_ptr->get_first_filename();
//   cout << "video_filename = " << video_filename << endl;
   Movie* video_movie_ptr=movies_group.generate_new_Movie(video_filename);

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      CM_2D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      &centers_group,panorama_movie_ptr,decorations.get_TrianglesGroup_ptr(),
      decorations.get_LineSegmentsGroup_ptr(),AnimationController_ptr);

   decorations.set_DataNode_ptr(panorama_movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Open text dialog box to display feature information:

//   FeaturesGroup_ptr->get_TextDialogBox_ptr()->open("Feature Information");
//   FeaturesGroup_ptr->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   timefunc::initialize_timeofday_clock();

// Store (U,V) coords for video frame corners within STL vector
// video_frame_corners:

   vector<threevector> video_frame_corners=
      video_movie_ptr->construct_video_frame_corners();
   
// Parse Soonmin's homography results for MIT main entrance sequence:

   homography H;
   string input_filename="homographies.txt";
   vector<homography*> homography_ptrs=
      H.parse_homography_results_file(input_filename);

   vector<threevector> prev_filtered_corner_coords;
   prev_filtered_corner_coords.push_back(Zero_vector);
   prev_filtered_corner_coords.push_back(Zero_vector);
   prev_filtered_corner_coords.push_back(Zero_vector);
   prev_filtered_corner_coords.push_back(Zero_vector);
   
   double alpha1;
   cout << "Enter alpha parameter value for temporal filtering:" << endl;
   cout << "(alpha=1 [0.003] yields no [significant] smoothing)" << endl;
   cin >> alpha1;

/*
   vector<threevector> XY_corners;
   XY_corners.push_back(
      threevector(1.46022911034822,-0.01,0.310868422911544));
   XY_corners.push_back(
      threevector(0.377961140316129,-0.01,0.284424855481621));
   XY_corners.push_back(
      threevector(0.352018329430367,-0.01,0.980944597898946));
   XY_corners.push_back(
      threevector(1.49492812528604,-0.01,0.777517067304944));
*/


   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();

      const int video_width=320;
      const int video_height=180;
      const int panorama_width=935;
      const int panorama_height=308;

      homography* curr_H_ptr=homography_ptrs[
         AnimationController_ptr->get_curr_framenumber()];

//      cout << "*curr_H_ptr = " << *curr_H_ptr << endl;

      vector<threevector> panorama_frame_coords=
         video_movie_ptr->compute_video_frame_corners_in_panorama(
            video_width,video_height,panorama_width,panorama_height,
            curr_H_ptr,video_frame_corners);

// Perform alpha-filtering on video frame corner coordinates
// calculated in panorama frame to remove worst of temporal jittering:

      double alpha=1.0;	// exclusively use raw input
      if (AnimationController_ptr->get_curr_framenumber() > 1)
      {
         alpha=alpha1;
      }

      for (int c=0; c<int(panorama_frame_coords.size()); c++)
      {
         threevector raw_corner_coords=panorama_frame_coords[c];

         double filtered_X=filterfunc::alpha_filter(
            raw_corner_coords.get(0),prev_filtered_corner_coords[c].get(0),
            alpha);
         double filtered_Y=filterfunc::alpha_filter(
            raw_corner_coords.get(1),prev_filtered_corner_coords[c].get(1),
            alpha);
         double filtered_Z=filterfunc::alpha_filter(
            raw_corner_coords.get(2),prev_filtered_corner_coords[c].get(2),
            alpha);

         prev_filtered_corner_coords[c]=threevector(
            filtered_X,filtered_Y,filtered_Z);

//         cout << "c = " << c
//              <<  " prev_filtered_corner_coords = "
//              << prev_filtered_corner_coords[c] << endl;
         
      } // loop over index c labeling panorama frame coords


      video_movie_ptr->superpose_video_frame_on_panorama(
         prev_filtered_corner_coords);
//      video_movie_ptr->superpose_video_frame_on_panorama(XY_corners);
   }

//   FeaturesGroup_ptr->get_TextDialogBox_ptr()->close();
}

