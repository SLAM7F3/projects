// ========================================================================
// Program PHOTOS2D is a laboratory for working with multiple 2D
// photos.  We hope it will someday emulate Fredo Durand's neato photo
// viewer...

/*
			 photos2d copley.jpg

	    photos2d copley.jpg  --newpass copley_north.jpg

   photos2d copley.jpg --newpass copley_north.jpg --newpass copley.jpg

   photos2d copley.jpg --newpass copley_north.jpg --newpass copley.jpg \
     --newpass copley.jpg --newpass copley_north.jpg --newpass copley.jpg \
     --newpass copley.jpg

   photos2d ./PRU_skywalk/DCP_2008.JPG \
	--newpass ./PRU_skywalk/DCP_2009.JPG \
	--newpass ./PRU_skywalk/DCP_2010.JPG \
	--newpass ./PRU_skywalk/DCP_2011.JPG \
	--newpass ./PRU_skywalk/DCP_2012.JPG \
	--newpass ./PRU_skywalk/DCP_2013.JPG \
	--newpass ./PRU_skywalk/DCP_2014.JPG \
	--newpass ./PRU_skywalk/DCP_2015.JPG \
	--newpass ./PRU_skywalk/DCP_2016.JPG 

*/

// ========================================================================
// Last updated on 10/26/07; 7/14/08; 4/12/09; 4/13/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Read input texture and video files:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   vector<int> videopass_ID;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         videopass_ID.push_back(n);
//         cout << "n = " << n << " videopass_ID = " << videopass_ID.back()
//              << endl;
      }
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   const double mag_factor=10;
   double min_X=0;
   double max_X=10*mag_factor;
   double min_Y=0;
   double max_Y=10*mag_factor;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   bool world_origin_precisely_in_lower_left_corner_flag=true;
//   bool world_origin_precisely_in_lower_left_corner_flag=false;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      min_X,max_X,min_Y,max_Y,min_Z,
      world_origin_precisely_in_lower_left_corner_flag);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

//   cout << "grid_origin = " << *grid_origin_ptr << endl;

   grid_ptr->set_delta_xy(2*mag_factor,2*mag_factor);
   grid_ptr->set_axis_char_label_size(1.0*mag_factor);
   grid_ptr->set_tick_char_label_size(1.0*mag_factor);

   grid_ptr->update_grid();

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(videopass_ID.front()),
      AnimationController_ptr);
   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_twoD_photo_picking_flag(true);

   int n_rows,n_columns;
   OBSFRUSTAGROUP_ptr->compute_tabular_rows_and_columns(
      videopass_ID.size(),n_rows,n_columns);
   int curr_row=0;
   int curr_column=0;

// Instantiate an individual ObsFrustum for every input video.  Each
// contains a separate movie object.

   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         string curr_video_filename=passes_group.get_pass_ptr(n)->
            get_first_filename();

// Group together all still images within common subgroup:

         int OSGsubPAT_number=0;	
//         int OSGsubPAT_number=n;	
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
            generate_movie_OBSFRUSTUM(curr_video_filename,OSGsubPAT_number);
//         OBSFRUSTUM_ptr->generate_viewing_pyramid_ptr();

         OBSFRUSTUM_ptr->set_display_ViewingPyramid_flag(false);
         OBSFRUSTUM_ptr->set_display_ViewingPyramidAboveZplane_flag(true);
         OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();

         OBSFRUSTUM_ptr->set_display_camera_model_flag(
            decorations.get_OBSFRUSTAGROUP_ptr()->get_curr_t(),
            decorations.get_OBSFRUSTAGROUP_ptr()->get_passnumber(),false);
         
// Manipulate camera corresponding to ObsFrustum's movie:

         camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
         double f=-1.5;
         camera_ptr->set_internal_params(
            f,f,camera_ptr->get_u0(),camera_ptr->get_v0());
         camera_ptr->construct_projection_matrix();

         double movie_downrange_distance=50;
         double sidelength=2*movie_downrange_distance;
         OBSFRUSTUM_ptr->initialize_frustum_with_movie(
            sidelength,movie_downrange_distance);

         threevector camera_posn(55*curr_column,40*curr_row,
                                 movie_downrange_distance);
         camera_ptr->set_world_posn(camera_posn);
         curr_column++;
         if (curr_column >= n_columns)
         {
            curr_column=0;
            curr_row++;
         }

         double az=0 * PI/180;
         double el=-90 * PI/180;
         double roll=-90 * PI/180;
         OBSFRUSTUM_ptr->absolute_position_and_orientation(
            decorations.get_OBSFRUSTAGROUP_ptr()->get_curr_t(),
            decorations.get_OBSFRUSTAGROUP_ptr()->get_passnumber(),
            camera_posn,az,el,roll);

// Compute instantaneous above_Zplane pyramid.  Then build
// instantaneous ViewingPyramidAboveZplane graphical from this pyramid
// object:


         double z_ground=0;
         OBSFRUSTUM_ptr->compute_viewing_pyramid_above_Zplane(
            z_ground,OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());

         OBSFRUSTUM_ptr->generate_Pyramid_geodes();
         OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->build_current_pyramid(
            decorations.get_OBSFRUSTAGROUP_ptr()->get_curr_t(),
            decorations.get_OBSFRUSTAGROUP_ptr()->get_passnumber(),
            OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());
         OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_mask(0,0,true);

         OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
            build_current_pyramid(
               decorations.get_OBSFRUSTAGROUP_ptr()->get_curr_t(),
               decorations.get_OBSFRUSTAGROUP_ptr()->get_passnumber(),
               OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());

// Save zplane face's corners within bounding_box STL vector:

//         face* zplane_face_ptr=
//            OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
//            get_pyramid_ptr()->get_zplane_face_ptr();
         
// Color ViewingPyramid and ViewPyramidAboveZplane graphicals.  Then
// set their edge widths:

/*
         OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_color(
            OBSFRUSTUM_ptr->get_viewing_pyramid_ptr(),
            colorfunc::get_OSG_color(colorfunc::green),
            colorfunc::get_OSG_color(colorfunc::blue),
            colorfunc::get_OSG_color(colorfunc::blue),
            osg::Vec4( 0 , 1 , 1 , 0.1 ));
*/
 
         OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->set_color(
            OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr(),
            colorfunc::get_OSG_color(colorfunc::yellow),
            colorfunc::get_OSG_color(colorfunc::orange),
            colorfunc::get_OSG_color(colorfunc::red),
            osg::Vec4( 0.5 , 0.5 , 0.5 , 0.0 ));

         OBSFRUSTUM_ptr->set_typical_pyramid_edge_widths();

      } // video passtype conditional
   } // loop over index n labeling all input passes

// Instantiate features decorations group:

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()));
   FeaturePickHandler* FeaturePickHandler_ptr=decorations.
      get_FeaturePickHandler_ptr();
   FeaturePickHandler_ptr->set_convert_3D_to_2D_flag(true);

// Attach scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_ptr->setSceneData(root);

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//      open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// We do not want the purple grid to appear.  Yet its mask is
// automatically turned off by the Terrain_Manipulator::home() call.
// So we need to reset the grid's mask again:

   grid_ptr->set_mask(0);
   
//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=window_mgr_ptr->getFrameStamp();

//   outputfunc::enter_continue_char();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();

   delete window_mgr_ptr;

}
