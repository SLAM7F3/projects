// ========================================================================
// Program QTVIEWMOVIE is an OSG based viewer for 2D movies.

//				qtviewmovie

// ========================================================================
// Last updated on 1/1/12; 1/2/12; 9/15/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/MovieServer.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "postgres/gis_databases_group.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg2D/TOCHUD.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   
   std::set_new_handler(sysfunc::out_of_memory);
   
// Initialize Qt application:

   QApplication app(argc,argv);

// As of 7/19/10, we hardwire the UTM zone for all Movies:

   int UTM_zonenumber=19;	// Boston, MA
   bool daylight_savings_flag=true;

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="QTVIEWMOVIE";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

   message_queue_channel_name="AnimationController";
   Messenger AnimationController_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

// Instantiate MovieServer which receives get calls from web page
// buttons:

   string MovieServer_hostname="127.0.0.1";
   int MovieServer_portnumber=4042;
   string MovieServer_URL;
   if (MovieServer_URL.size() > 0)
   {
      MovieServer_hostname=stringfunc::get_hostname_from_URL(
         MovieServer_URL);
      MovieServer_portnumber=stringfunc::get_portnumber_from_URL(
         MovieServer_URL);
   }
   cout << "MovieServer_hostname = " << MovieServer_hostname
        << " MovieServer_portnumber = " << MovieServer_portnumber
        << endl;
   MovieServer Movie_server(MovieServer_hostname,MovieServer_portnumber);

   string starting_image_subdir="/home/cho/junk/";
//   string starting_image_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/modeling/";
//   string starting_image_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/plume/undistorted_frames/";
//      "/home/cho/programs/c++/svn/projects/src/mains/thunderstorm/video_data/20110511/flight1/";
//      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/HAFB/5-25-flight1-CreditUnion/undistorted_images/";
//      "/home/cho/programs/c++/svn/projects/src/mains/wisp/frames/downsampled_frames/";
//   "/home/cho/georeg/";

   Movie_server.set_starting_image_subdir(starting_image_subdir);
   string tomcat_subdir="/usr/local/apache-tomcat/webapps/video/";

// Generate video pass from first movie filename entered via Qt:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   const int ndims=2;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   gis_databases_group* gis_databases_group_ptr=new gis_databases_group;
   gis_database* gis_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);
   Movie_server.set_gis_database_ptr(gis_database_ptr);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->set_thick_client_window_position();
   window_mgr_ptr->initialize_window("2D imagery");
   window_mgr_ptr->set_auto_generate_movies_flag(true);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_ptr);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_Viewer_Messenger_ptr(&viewer_messenger);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_tomcat_subdir(tomcat_subdir);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      purge_flash_movies();

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Display mission and platform information within upper right corner
// of 2D viewer window:

   TOCHUD* TOCHUD_ptr=new TOCHUD();
   TOCHUD_ptr->reset_text_size(0.925 , 0);
   TOCHUD_ptr->reset_text_size(0.925 , 1);
   root->addChild(TOCHUD_ptr->getProjection());

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=false;		// viewgraphs

   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);
   Movie_server.set_Operations_ptr(&operations);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->set_Messenger_ptr(
      &AnimationController_messenger);
   AnimationController_ptr->get_clock_ptr()->
      compute_UTM_zone_time_offset(UTM_zonenumber);
   AnimationController_ptr->get_clock_ptr()->
      set_daylight_savings_flag(daylight_savings_flag);

// Wait for user to press Select video frames button on thin client in
// bluetracker.  Then load in movie subdir and determine min,max frame
// numbers and Nimages via
// MovieServer::select_movie_frames_subdir_via_GUI().  Wait in loop
// until AC_ptr->get_nframes() >= 1:

   AnimationController_ptr->set_nframes(0);
   while (AnimationController_ptr->get_nframes() < 1)
   {
      app.processEvents();
   }

   string first_frame_filename=Movie_server.get_first_frame_filename();
//   cout << "first_frame_filename = " << first_frame_filename << endl;
      
   passes_group.generate_new_pass(first_frame_filename);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie_server.set_MoviesGroup_ptr(&movies_group);
   Movie* movie_ptr=Movie_server.regenerate_Movie_to_display();

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Display local rather than UTC world time:

   operations.get_ImageNumberHUD_ptr()->set_display_UTC_flag(false);

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate points, polylines, polygons, rectangles and features
// decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   PolyLinesGroup_ptr->set_width(5);
   PolyLinesGroup_ptr->set_n_text_messages(1);
   PolyLinesGroup_ptr->set_CM_2D_ptr(CM_2D_ptr);
   PolyLinesGroup_ptr->set_variable_Point_size_flag(false);
   PolyLinesGroup_ptr->set_altitude_dependent_labels_flag(false);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   cout << "PolygonsGroup_ptr = " << PolygonsGroup_ptr << endl;
   PolyLinesGroup_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   FeaturesGroup_ptr->pushback_Messenger_ptr(&viewer_messenger);

// On 1/2/12, we empirically found that this next set_DataNode_ptr()
// line needs to appear here rather than much earlier within main() in
// order to FeaturePickHandling to work.  We're not sure why...

   decorations.set_DataNode_ptr(movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(operations.get_OSGgroup_ptr());
   root->addChild(movies_group.get_OSGgroup_ptr());

// Movie server variable settings:

   Movie_server.set_CM_ptr(CM_2D_ptr);
   Movie_server.set_FeaturesGroup_ptr(FeaturesGroup_ptr);
   Movie_server.set_FeaturePickHandler_ptr(
      decorations.get_FeaturePickHandler_ptr());
   Movie_server.set_Operations_ptr(&operations);
   Movie_server.set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   Movie_server.set_PolyLinePickHandler_ptr(
      decorations.get_PolyLinePickHandler_ptr());
   Movie_server.set_TOCHUD_ptr(TOCHUD_ptr);
   Movie_server.set_tomcat_subdir(tomcat_subdir);
   Movie_server.set_viewer_messenger_ptr(&viewer_messenger);

// Attach scene graph to viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   string command="SEND_THICKCLIENT_READY_FOR_USER_INPUT";
   viewer_messenger.broadcast_subpacket(command);

   while( !window_mgr_ptr->done() )
   {
      app.processEvents();
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

