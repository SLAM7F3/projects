// ========================================================================
// Program VIDEOTEXTURE generates a Quickbird static surface along with
// a LatLongGrid underneath a dynamic Constant Hawk movie texture in
// 3D worldspace.  It also instantiates model aircraft which orbit
// above the texture in prescribed racetracks.

//	 videotexture --region_filename ./packages/lubbock_fused.pkg --surface_texture ./packages/ar1_texture_frames_all_orig.pkg

// ========================================================================
// Last updated on 12/8/07; 12/9/07; 2/19/08; 5/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "geometry/ellipse.h"
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/stringfuncs.h"
#include "passes/TextDialogBox.h"
#include "osg/osg3D/Terrain_Manipulator.h"
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

// Read input texture file:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
//   cout << "texturepass_ID = " << texturepass_ID << endl;
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Create a DepthPartitionNode to manage partitioning of the scene

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Create OSG root node:

//   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder;
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),AnimationController_ptr);
   root->addChild( movies_group.get_OSGgroup_ptr() );

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   root->addChild(clouds_group.get_OSGgroup_ptr());

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
//   bool force_display_flag=true;
//   latlonggrid_ptr->adjust_depth_buffering(
//      force_display_flag,latlonggrid_ptr->get_geode_ptr());
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate surface texture movie:

   Movie* movie_ptr=movies_group.get_Movie_ptr(0);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(texturepass_ID));
   SignPostsGroup* SignPostsGroup_ptr=decorations.get_SignPostsGroup_ptr(0);
   SignPostsGroup_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   SignPostsGroup_ptr->set_Clock_ptr(&clock);
   SignPostsGroup_ptr->set_common_geometrical_size(100);
   
   vector<twovector> signpost_long_lat;
   signpost_long_lat.push_back(twovector(-118.385100,33.185000));
   signpost_long_lat.push_back(twovector(-118.799800,33.226800));
   signpost_long_lat.push_back(twovector(-118.849000,33.176000));
   signpost_long_lat.push_back(twovector(-118.803200,32.796700));
   signpost_long_lat.push_back(twovector(-118.726000,32.715800));
   signpost_long_lat.push_back(twovector(-118.359900,32.709400));
   signpost_long_lat.push_back(twovector(-118.290800,32.770400));
   signpost_long_lat.push_back(twovector(-118.326200,33.129900));

   signpost_long_lat.push_back(twovector(-118.5650902978,33.0206022036));
   // site 2
   signpost_long_lat.push_back(twovector(-118.488025099,32.9150077336)); 
   // site 6
   signpost_long_lat.push_back(twovector(-118.583973101,33.0212484756));
   // site 0
   signpost_long_lat.push_back(twovector(-118.51523921225,32.9084069935));
   // site 12

   for (unsigned int s=0; s<signpost_long_lat.size(); s++)
   {
      twovector long_lat(signpost_long_lat[s]);
      SignPost* curr_SignPost_ptr=
         SignPostsGroup_ptr->generate_new_SignPost_on_earth(
            long_lat.get(0),long_lat.get(1),0);
      string label=stringfunc::number_to_string(long_lat.get(0),2)+" , "+
         stringfunc::number_to_string(long_lat.get(1),2);

      if (s==signpost_long_lat.size()-4)
      {
         label="Test site 002";
      }
      else if (s==signpost_long_lat.size()-3)      
      {
         label="Test site 006";
      }
      else if (s==signpost_long_lat.size()-2)      
      {
         label="Test site 000";
      }
      else if (s==signpost_long_lat.size()-1)      
      {
         label="Test site 012";
      }

      curr_SignPost_ptr->set_label(label);
   }

// Draw elliptical racetrack orbit for LiMIT model:

   double major_axis=35851.7620423408;
   double minor_axis=28737.783019816;
   double theta = 113.919717495022*PI/180;
   threevector racetrack_center(353512.872107285,3648382.98542531,0);
   ellipse racetrack_orbit(racetrack_center,major_axis,minor_axis,theta);
   vector<threevector> vertex=racetrack_orbit.generate_vertices(360);

   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(3,passes_group.get_pass_ptr(texturepass_ID));
   PolyLinesGroup_ptr->generate_new_PolyLine(vertex);

   decorations.add_Features(ndims,passes_group.get_pass_ptr(texturepass_ID));
   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
      &earth_regions_group);
   decorations.add_ArmySymbols(passes_group.get_pass_ptr(texturepass_ID));

   decorations.add_SphereSegments(passes_group.get_pass_ptr(texturepass_ID));
   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
//      decorations.get_SphereSegmentsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
//         &earth_regions_group);
   }

   if (videopass_ID >= 0)
   {
	OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=
           decorations.add_OBSFRUSTA(
              passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   }

// Add GMTI targets into San Clemente earth_region:

   EarthRegion* GMTI_region_ptr=earth_regions_group.get_EarthRegion_ptr(0);
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.5650902978,33.0206022036));
   // site 2
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.488025099,32.9150077336)); 
   // site 6
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.583973101,33.0212484756));
   // site 0
   GMTI_region_ptr->add_GMTI_target(geopoint(-118.51523921225,32.9084069935));
   // site 12
 
// Instantiate model decorations group:

   decorations.add_MODELS(
      passes_group.get_pass_ptr(texturepass_ID),AnimationController_ptr);
//   MODELSGROUP* MODELSGROUP_ptr=decorations.get_MODELSGROUP_ptr();

/*

//   int n_animation_frames=40;
   int n_animation_frames=200;
//   int n_animation_frames=720*2;

// Generate LiMIT model, racetrack orbit and TWO OBSFRUSTA:

   int LiMIT_OSGsubPAT_number;
   MODEL* LiMIT_ptr=MODELSGROUP_ptr->generate_LiMIT_Model(
      LiMIT_OSGsubPAT_number);
   threevector LiMIT_ellipse_center(353512.872107285,3648382.98542531,0);
   MODELSGROUP_ptr->generate_elliptical_LiMIT_racetrack_orbit(
      n_animation_frames,LiMIT_ellipse_center,LiMIT_ptr);
   OBSFRUSTUM* LiMIT_FOV_OBSFRUSTUM_ptr=
      MODELSGROUP_ptr->generate_LiMIT_FOV_OBSFRUSTUM(
         LiMIT_OSGsubPAT_number,LiMIT_ptr);
   MODELSGROUP_ptr->generate_LiMIT_instantaneous_dwell_OBSFRUSTUM(
      LiMIT_OSGsubPAT_number,LiMIT_ptr,LiMIT_FOV_OBSFRUSTUM_ptr,
      GMTI_region_ptr);

// Generate predator model, racetrack orbit and OBSFRUSTUM:

   int predator_OSGsubPAT_number;
   MODEL* predator_ptr=MODELSGROUP_ptr->generate_predator_Model(
      predator_OSGsubPAT_number);

   double predator_racetrack_center_longitude=-118.5;	// San Clemente
   double predator_racetrack_center_latitude=32.91;	// San Clemente
   MODELSGROUP_ptr->generate_predator_racetrack_orbit(
      n_animation_frames,predator_racetrack_center_longitude,
      predator_racetrack_center_latitude,predator_ptr);
   MODELSGROUP_ptr->generate_predator_OBSFRUSTUM(
      predator_OSGsubPAT_number,predator_ptr);

// For clarity's sake, initially start with all model OSGsubPATs
// masked off.  Also enlarge the LiMIT and predator models beyond
// their default sizes.

   for (int n=0; n<MODELSGROUP_ptr->get_n_OSGsubPATs(); n++)
   {
      MODELSGROUP_ptr->set_OSGsubPAT_mask(n,0);      
   }
   MODELSGROUP_ptr->change_scale(3);
*/

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

//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=window_mgr_ptr->getFrameStamp();

//   cout << "Before entering infinite viewer loop" << endl;
//   outputfunc::enter_continue_char();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();

   delete window_mgr_ptr;
}
