// ========================================================================
// Program VIEWIMAGES is a variant of VPLAYER.  It can be used
// to scan through all image files contained within the present
// working directory.  If all the input image files do not have
// uniform pixel size, borders will be dynamically added as necessary
// to each input image so that effectively all images end up with the
// same pixel widths and heights.

// VIEWIMAGES enables users to annotate individual images and
// export all such annotations to CSV text files.

//			./viewimages

// ========================================================================
// Last updated on 6/16/14; 6/21/14; 7/1/14; 3/11/16
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "image/imagefuncs.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

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
   cout << "videopass_ID = " << videopass_ID << endl;


// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool display_movie_elapsed_time=false;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time,
      display_movie_elapsed_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
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

// Instantiate points, polylines, polygons, linesegments, triangles,
// rectangles and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(
         videopass_ID));
//   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
//      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
//   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);

/*   
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      decorations.add_LineSegments(
         ndims,passes_group.get_pass_ptr(videopass_ID),
         AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
*/

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),
      AnimationController_ptr);

   string images_subdir="./";
   AnimationController_ptr->store_unordered_image_filenames(images_subdir);
   int Nimages=AnimationController_ptr->get_n_ordered_image_filenames();
   cout << "Nimages = " << Nimages << endl;

// Find maximum pixel width and height among all input images:

   vector<string> image_filenames =filefunc::image_files_in_subdir(
      images_subdir);
   
   string image_filename=AnimationController_ptr->get_curr_image_filename();
   cout << "Initial image filename = " << image_filename << endl;
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(image_filename);

   unsigned int curr_width, curr_height;
   unsigned int max_image_width = 0, max_image_height = 0;
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      imagefunc::get_image_width_height(
         image_filenames[i],curr_width,curr_height);
      max_image_width = basic_math::max(max_image_width, curr_width);
      max_image_height = basic_math::max(max_image_height, curr_height);
   }

   delete texture_rectangle_ptr;
   texture_rectangle_ptr = new texture_rectangle(
      max_image_width, max_image_height, 1, 4, AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

   cout << "n_channels = " << texture_rectangle_ptr->getNchannels() << endl;

   AnimationController_ptr->set_nframes(Nimages);
   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(0);
   movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(
      Nimages-1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      NULL,
      movie_ptr,decorations.get_TrianglesGroup_ptr(),
      decorations.get_LineSegmentsGroup_ptr(),AnimationController_ptr);

   FeaturesGroup_ptr->set_display_selected_bbox_flag(false);
   FeaturesGroup_ptr->set_display_feature_pair_bboxes_flag(true);
//   FeaturesGroup_ptr->set_bbox_sidelength(32);
//   FeaturesGroup_ptr->set_bbox_sidelength(64);
//   FeaturesGroup_ptr->set_bbox_sidelength(128);
   FeaturesGroup_ptr->set_erase_Graphicals_except_at_curr_time_flag(true);
   FeaturesGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);

   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Keyboard control notes:" << endl << endl;
   cout << "Note: Image window must be active before following keys will be recognized" 
        << endl << endl;
   cout << "'p' key toggles image player between 'play' and 'pause' modes " 
        << endl;
   cout << "'r' key reverses image's time direction" << endl;
   cout << "Right and left arrow keys step one image frame forward/backward"
        << endl;
   cout << "'a' key allows user to enter an annotation for current frame"
        << endl;
   cout << "'e' key exports all image annotations to output CSV file" 
        << endl;
   cout << "spacebar 'homes' image window to default settings" << endl;
   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Mouse control notes:" << endl << endl;
   cout << "Left mouse button translates image frame in 2D" << endl;
   cout << "Center mouse button rotates image window in 2D" << endl;
   cout << "Right mouse button zooms image frame in/out" << endl;
   cout << "******************************************************" << endl;
   cout << endl;

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

