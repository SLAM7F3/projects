// ========================================================================
// Program IMAGECLIENTTEST opens a 2D OpenSceneGraph movie player
// window.  It displays within this window 1000x1000 chips returned by
// the ImageServer in response to the ImageClient's GET_NEXT_IMAGE
// request.

// This program emulates receipt of Real-Time Persistent Surveillance
// imagery in the form of JPEG chips coming from the air down to the
// ground.

// ========================================================================
// Last updated on 5/27/09; 7/30/09; 5/10/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "Qt/web/ImageClient.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"
#include "osg/osgWindow/ViewerManager.h"


#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

   string ImageServer_URL=passes_group.get_LogicServer_URL();
   cout << "ImageServer_URL = " << ImageServer_URL << endl;
// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
//   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
   bool display_movie_world_time=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* Movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(Movie_ptr->get_Nimages());
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);
   decorations.set_DataNode_ptr(Movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Instantiate ImageClient:

   int ID=0;
//   int ID=3;
//   bool iPhone_beacon_flag=true;
   bool iPhone_beacon_flag=false;
   ImageClient* ImageClient_ptr=new ImageClient(ImageServer_URL,ID,
                                                iPhone_beacon_flag);
   ImageClient_ptr->set_save_images_to_JPEG_files_flag(true);
   ImageClient_ptr->set_Movie_ptr(Movie_ptr);
   ImageClient_ptr->set_output_image_subdir("/tmp/recovered_images/");

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   if (!iPhone_beacon_flag) window_mgr_ptr->realize();

   while (true)
//   while( !window_mgr_ptr->done() )
   {
      if (!iPhone_beacon_flag) window_mgr_ptr->process();
      app.processEvents();
   }

   delete window_mgr_ptr;
}



