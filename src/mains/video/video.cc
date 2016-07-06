// ========================================================================
// Program VIDEO is an OSG based viewer for photographs and video
// imagery.

// 			    video HAFB.vid

// ========================================================================
// Last updated on 9/24/09; 12/11/09; 1/29/11
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
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
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

//   string wiki_message_queue_channel_name="wiki";
//   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
//   bool display_movie_world_time=true;
   bool display_movie_state=false;		// viewgraphs
   bool display_movie_number=false;		// viewgraphs
   bool display_movie_world_time=false;		// viewgraphs
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

// Instantiate points, poolylines, polygons, linesegments, triangles,
// rectangles and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(
         ndims,passes_group.get_pass_ptr(videopass_ID), 
         AnimationController_ptr);

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

/*
// Experiment with superposing translucent polygon onto image plane:

   vector<threevector> proj_vertices;
   proj_vertices.push_back(threevector(0.2,0.2));
   proj_vertices.push_back(threevector(0.6,0.2));
   proj_vertices.push_back(threevector(0.6,0.7));
   proj_vertices.push_back(threevector(0.2,0.7));
   proj_vertices.push_back(threevector(0.2,0.2));

   colorfunc::Color bbox_color=colorfunc::red;         
   double alpha=0.2;
   PolygonsGroup_ptr->generate_translucent_Polygon(
      bbox_color,proj_vertices,alpha);
*/

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);

// As of 2/12/09, we hard-code geolocation of camera for Jan 6, 2009
// Lobby7 video sequence.  Should eventually read this parameter in
// via package file...

   threevector camera_XYZ(327532.1, 4691760.7, 5.0);	

   movies_group.set_static_camera_posn_offset(camera_XYZ);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);
//   cout << "minU = " << movie_ptr->get_minU() << endl;
//   cout << "maxU = " << movie_ptr->get_maxU() << endl;
//   cout << "minV = " << movie_ptr->get_minV() << endl;
//   cout << "maxV = " << movie_ptr->get_maxV() << endl;

//   Movie* movie2_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

/*
// Chip out sub-image(s) from full video if video_corner_vertices are
// defined in input package and longitude-latitude bbox is specified
// here:

   twovector lower_left_texture_fracs,lower_right_texture_fracs,
      upper_right_texture_fracs,upper_left_texture_fracs;

   lower_left_texture_fracs=twovector(-0.5 , 0.0);
   lower_right_texture_fracs=twovector(0.4 , 0.1);
   upper_right_texture_fracs=twovector(0.6 , 0.4);
   upper_left_texture_fracs=twovector(-0.2 , -0.3);

   movie_ptr>compute_2D_chip(lower_left_texture_fracs,
                              lower_right_texture_fracs,
                              upper_right_texture_fracs,
                              upper_left_texture_fracs);

// 2nd video chip:

   lower_left_texture_fracs=twovector(0.5 , 0.5);
   lower_right_texture_fracs=twovector(1.2 , 0.5);
   upper_right_texture_fracs=twovector(1.1 , 1.0);
   upper_left_texture_fracs=twovector(0.5 , 1.6);

   movie2_ptr->compute_2D_chip(lower_left_texture_fracs,
                               lower_right_texture_fracs,
                               upper_right_texture_fracs,
                               upper_left_texture_fracs);

*/

/*
   double min_long=-101.94;
   double max_long=-101.93;
   double min_lat=33.492;
   double max_lat=33.50;
   movie_ptr->georegister_subtexture_corners(
      min_long,max_long,min_lat,max_lat);
*/

   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate 2D imageplane SignPosts group and pick handler:

   SignPostsGroup* imageplane_SignPostsGroup_ptr=
      decorations.add_SignPosts(2,passes_group.get_pass_ptr(videopass_ID));
   imageplane_SignPostsGroup_ptr->set_AnimationController_ptr(
      AnimationController_ptr);

   SignPostPickHandler* imageplane_SignPostPickHandler_ptr=
      decorations.get_SignPostPickHandler_ptr();

   imageplane_SignPostPickHandler_ptr->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);
   imageplane_SignPostPickHandler_ptr->set_DataNode_ptr(
      movie_ptr->getGeode());

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
      &centers_group,movie_ptr,decorations.get_TrianglesGroup_ptr(),
      decorations.get_LineSegmentsGroup_ptr(),AnimationController_ptr);

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

