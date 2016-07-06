// ========================================================================
// Program PROPAGATOR is a variant of mains/photosynth/PROPAGATOR.  It
// pops open a 3D window displaying a point cloud (ladar and/or
// bundler reconstructed) and georegistered MODEL panel frames'
// OBSFRUSTA. It also opens a 2D window in which the video frame for
// the most recently selected OBSFRUSTUM is displayed.

// ========================================================================
// Last updated on 4/24/12; 6/28/12; 6/29/12; 8/5/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgClipping/Clipping.h"
#include "osg/osgClipping/ClippingKeyHandler.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "models/ParkingLotsGroup.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "models/RoadsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"
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
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   cout << "videopass_ID = " << videopass_ID << endl;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string xyz_points_filename=passes_group.get_xyz_points_filename();
   cout << "xyz_points_filename = " << xyz_points_filename << endl;
   string camera_views_filename=passes_group.get_camera_views_filename();
   cout << "camera_views_filename = " << camera_views_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_2D_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   string window_2D_title="2D imagery";
   string window_3D_title="3D imagery";

   window_mgr_2D_ptr->initialize_dual_windows(
      window_2D_title,window_3D_title,window_mgr_3D_ptr);

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="PROPAGATOR";

// Instantiate photo network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger OBSFRUSTA_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);
   Messenger Movies_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

// Create two OSG root nodes:

   osg::Group* root_2D = new osg::Group;
   osg::Group* root_3D = new osg::Group;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);
   
// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(2,window_mgr_2D_ptr,display_movie_state,
                         display_movie_number,hide_Mode_HUD_flag);

   ModeController* ModeController_2D_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root_2D->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=new ModeController();
   ModeController_3D_ptr->setState(ModeController::MANIPULATE_FUSED_DATA);
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_3D_ptr) );
   root_3D->addChild(osgfunc::create_Mode_HUD(3,ModeController_3D_ptr));

// Add custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_ptr,window_mgr_2D_ptr);
   window_mgr_2D_ptr->set_CameraManipulator(CM_2D_ptr);

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_2D(
      window_mgr_2D_ptr,ModeController_2D_ptr,CM_2D_ptr);
   Decorations decorations_3D(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();

   bool northern_hemisphere_flag=true;
   int UTM_zone=19;	// Boston
   TilesGroup_ptr->set_specified_UTM_zonenumber(UTM_zone);	
   TilesGroup_ptr->set_northern_hemisphere_flag(northern_hemisphere_flag);   
   geopoint lower_left_corner(
      northern_hemisphere_flag,UTM_zone, 325361.000, 4689703.000 , 0);
   geopoint upper_right_corner(
      northern_hemisphere_flag,UTM_zone, 331302.000, 4692873.000 , 0);

   double delta_x=1;	// meter
   double delta_y=1;	// meter
   twoDarray* DTED_ztwoDarray_ptr=
      TilesGroup_ptr->generate_subtile_twoDarray(
         delta_x,delta_y,lower_left_corner,upper_right_corner);

   string geotif_subdir=
      "/data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/tif_files/";
   string geotif_filename=geotif_subdir+"boston_heights_TEC04.tif";
   TilesGroup_ptr->read_geotif_subtile_height_data(
      geotif_filename,DTED_ztwoDarray_ptr);
   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;

// Instantiate group to hold 2D movies:

   MoviesGroup movies_2D_group(
      2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   movies_2D_group.set_photogroup_ptr(photogroup_ptr);
   movies_2D_group.pushback_Messenger_ptr(&Movies_photo_network_messenger);

   Movie* movie_ptr=movies_2D_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());

   root_2D->addChild( movies_2D_group.get_OSGgroup_ptr() );

// Instantiate group to hold 3D movies:

   MoviesGroup movies_3D_group(
      3,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   root_3D->addChild( movies_3D_group.get_OSGgroup_ptr() );

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   decorations_3D.set_PointCloudsGroup_ptr(&clouds_group);
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);

   LatLongGridsGroup latlonggrids_group(
      3,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
//   LatLongGrid_ptr->set_dynamic_grid_flag(false);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations_3D.set_grid_origin_ptr(grid_origin_ptr);

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_3D_ptr,LatLongGrid_ptr));

   root_3D->addChild(clouds_group.get_OSGgroup_ptr());
   root_3D->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   pointfinder.set_minimal_allowed_range(100);	// meters
//   pointfinder.set_maximal_rho(10);
   pointfinder.set_maximal_rho(20);	// meters
//   pointfinder.set_maximal_rho(30);	// meters
//   pointfinder.set_maximal_rho(70);	// meters
   pointfinder.set_max_cone_halfangle(4);	// degs
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate Arrows decoration group:

   ArrowsGroup* ArrowsGroup_ptr=decorations_3D.add_Arrows(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   ArrowsGroup_ptr->set_altitude_dependent_size_flag(false);
//   ArrowsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate 3D & 2D features decorations group:

   FeaturesGroup* FeaturesGroup_3D_ptr=
      decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

   FeaturesGroup* FeaturesGroup_2D_ptr=decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      NULL,movie_ptr,NULL,NULL,AnimationController_ptr);
   decorations_2D.set_DataNode_ptr(movie_ptr->getGeode());

   FeaturesGroup_2D_ptr->set_AnimationController_ptr(AnimationController_ptr);
   FeaturesGroup_2D_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   FeaturesGroup_2D_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
   FeaturesGroup_2D_ptr->set_MoviesGroup_ptr(&movies_2D_group);
   FeaturesGroup_2D_ptr->set_photogroup_ptr(photogroup_ptr);
   FeaturesGroup_2D_ptr->set_PointFinder_ptr(&pointfinder);
   FeaturesGroup_2D_ptr->set_FeaturesGroup_3D_ptr(FeaturesGroup_3D_ptr);
   FeaturesGroup_2D_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);

   FeaturesGroup_2D_ptr->set_display_geocoords_flag(false);
//   FeaturesGroup_2D_ptr->set_display_geocoords_flag(true);
   FeaturesGroup_2D_ptr->set_DTED_ztwoDarray_ptr(DTED_ztwoDarray_ptr);

// Instantiate 3D SignPosts decorations group:

   SignPostsGroup* SignPostsGroup_ptr=decorations_3D.add_SignPosts(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

   for (int n=0; n<decorations_3D.get_n_SignPostsGroups(); n++)
   {
      decorations_3D.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
   }

   SignPostsGroup_ptr->set_photogroup_ptr(photogroup_ptr);
   SignPostsGroup_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   SignPostsGroup_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
//   cout << "package_subdir = " 
//        <<  passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
//      get_package_subdir() << endl;
//   cout << "package_filename_prefix = "
//        <<  passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
//      get_package_filename_prefix() << endl;
   SignPostsGroup_ptr->set_MoviesGroup_ptr(&movies_2D_group);

   SignPostsKeyHandler* SignPostsKeyHandler_ptr=decorations_3D.
      get_SignPostsKeyHandler_ptr();
   SignPostsKeyHandler_ptr->set_AnimationController_ptr(
      AnimationController_ptr);

// Instantiate 2D imageplane SignPosts group and pick handler:

   threevector* grid_world_origin_ptr=NULL;
   SignPostsGroup* imageplane_SignPostsGroup_ptr=new SignPostsGroup(
      2,passes_group.get_pass_ptr(videopass_ID),grid_world_origin_ptr);
   imageplane_SignPostsGroup_ptr->set_AnimationController_ptr(
      AnimationController_ptr);

   SignPostPickHandler* imageplane_SignPostPickHandler_ptr=
      new SignPostPickHandler(
         2,passes_group.get_pass_ptr(videopass_ID),CM_2D_ptr,
         imageplane_SignPostsGroup_ptr,ModeController_2D_ptr,
         window_mgr_2D_ptr,grid_world_origin_ptr);
   imageplane_SignPostPickHandler_ptr->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);
   imageplane_SignPostPickHandler_ptr->set_DataNode_ptr(
      movie_ptr->getGeode());
   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(
      imageplane_SignPostPickHandler_ptr);

   SignPostsGroup_ptr->set_imageplane_SignPostsGroup_ptr(
      imageplane_SignPostsGroup_ptr);
   SignPostsGroup_ptr->set_imageplane_SignPostPickHandler_ptr(
      imageplane_SignPostPickHandler_ptr);

   decorations_2D.get_OSGgroup_ptr()->addChild(imageplane_SignPostsGroup_ptr->
                                               get_OSGgroup_ptr());

// Instantiate BuildingsGroup object:

   BuildingsGroup* BuildingsGroup_ptr=new BuildingsGroup();
   string OFF_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/";
   BuildingsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate ParkingLotsGroup object:

   ParkingLotsGroup* ParkingLotsGroup_ptr=new ParkingLotsGroup();
   ParkingLotsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate RoadsGroup object:

   RoadsGroup* RoadsGroup_ptr=new RoadsGroup();
   RoadsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_DTED_ztwoDarray_ptr(DTED_ztwoDarray_ptr);
//   OBSFRUSTAGROUP_ptr->set_ArrowsGroup_ptr(ArrowsGroup_ptr);
   FeaturesGroup_2D_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

// Instantiate an individual OBSFRUSTUM for every still image:

/*
   double frustum_sidelength=7.5;
   double movie_downrange_distance=-1;
   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);
*/

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations_3D.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_photogroup_ptr(
      photogroup_ptr);

// Instantiate PolyLines, Polygons and Polyhedra decoration groups:

   PolyLinesGroup* imageplane_PolyLinesGroup_ptr=
      decorations_2D.add_PolyLines(
         2,passes_group.get_pass_ptr(videopass_ID));
   osgGeometry::PolygonsGroup* imageplane_PolygonsGroup_ptr=
      decorations_2D.add_Polygons(
         2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   imageplane_PolygonsGroup_ptr->set_PolyLinesGroup_ptr(
      imageplane_PolyLinesGroup_ptr);

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations_3D.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->generate_Building_Polyhedra(BuildingsGroup_ptr);

// Label Buildings with SignPosts:

   bool display_Building_IDs_flag=false;
//   bool display_Building_IDs_flag=true;
   if (display_Building_IDs_flag)
   {
      SignPostsGroup* Building_SignPostsGroup_ptr=decorations_3D.add_SignPosts(
         3,passes_group.get_pass_ptr(cloudpass_ID));

      for (int b=0; b<BuildingsGroup_ptr->get_n_Buildings(); b++)
      {
         Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(b);
         threevector roof_COM=Building_ptr->get_roof_COM();
         double size=0.5;
         double height_multiplier=0.5;
         SignPost* Building_SignPost_ptr=Building_SignPostsGroup_ptr->
            generate_new_SignPost(size,height_multiplier,roof_COM);
      
         string bldg_label=stringfunc::number_to_string(
            Building_ptr->get_ID());
         Building_SignPost_ptr->set_label(bldg_label);
         Building_SignPost_ptr->set_permanent_color(colorfunc::cyan);

      } // loop over index b labeling Buildings
      Building_SignPostsGroup_ptr->reset_colors();
   }

/*
// Import (manually extracted) planar facets from lazerkart data along
// a few building walls:

   PolyLinesGroup* lazerkart_PolyLinesGroup_ptr=decorations_3D.add_PolyLines(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   lazerkart_PolyLinesGroup_ptr->set_width(5);
   
   string facet_polylines_filename="lazerkart_facet_polylines.dat";
   lazerkart_PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(
      facet_polylines_filename);
   for (int p=0; p<lazerkart_PolyLinesGroup_ptr->get_n_Graphicals(); p++)
   {
      PolyLine* lazerkart_PolyLine_ptr=lazerkart_PolyLinesGroup_ptr->
         get_PolyLine_ptr(p);
      string ID_label=lazerkart_PolyLine_ptr->generate_PolyLine_ID_label();
      cout << "p = " << p << " ID_label = " << ID_label << endl;

//      polyline* lazerkart_polyline_ptr=lazerkart_PolyLine_ptr->
//         get_polyline_ptr();
//      threevector COM=lazerkart_polyline_ptr->compute_vertices_COM();
//      const double text_size=100000;
//      lazerkart_PolyLine_ptr->set_label(ID_label,COM,text_size);

      osgGeometry::PointsGroup* PointsGroup_ptr=lazerkart_PolyLine_ptr->
         get_PointsGroup_ptr();
      PointsGroup_ptr->set_OSGgroup_nodemask(0);
   }

   lazerkart_PolyLinesGroup_ptr->set_uniform_color(colorfunc::pink);
   lazerkart_PolyLinesGroup_ptr->reset_colors();
*/

   bool display_lots_and_roads_flag=false;
//   bool display_lots_and_roads_flag=true;
   if (display_lots_and_roads_flag)
   {
      PolyhedraGroup_ptr->generate_ParkingLot_Polyhedra(ParkingLotsGroup_ptr);
      PolyhedraGroup_ptr->generate_Road_Polyhedra(RoadsGroup_ptr);
   }
   
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      PolyhedraGroup_ptr);
   OBSFRUSTAGROUP_ptr->set_PolyhedraGroup_ptr(PolyhedraGroup_ptr);
   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(imageplane_PolygonsGroup_ptr);

// Instantiate Clipping object:

//   bool clipping_flag=true;
   bool clipping_flag=false;
   if (clipping_flag)
   {
      PolyLinesGroup* Clipped_PolyLinesGroup_ptr=decorations_3D.add_PolyLines(
         3,passes_group.get_pass_ptr(cloudpass_ID));
      Clipped_PolyLinesGroup_ptr->set_width(20);

      osgGeometry::PolygonsGroup* Clipped_PolygonsGroup_ptr=
         decorations_3D.add_Polygons(
            3,passes_group.get_pass_ptr(cloudpass_ID),NULL);

      Clipping* Clipping_ptr=new Clipping(OBSFRUSTAGROUP_ptr);
      Clipping_ptr->set_BuildingsGroup_ptr(BuildingsGroup_ptr);
      Clipping_ptr->set_MoviesGroup_ptr(&movies_3D_group);
      Clipping_ptr->set_Clipped_PolyLinesGroup_ptr(Clipped_PolyLinesGroup_ptr);
      Clipping_ptr->set_Clipped_PolygonsGroup_ptr(Clipped_PolygonsGroup_ptr);
      Clipping_ptr->set_PolyhedraGroup_ptr(PolyhedraGroup_ptr);
      Clipping_ptr->set_WindowManager_ptr(window_mgr_3D_ptr);

      Clipping_ptr->set_max_frustum_to_rectangle_distance(60);

      root_3D->addChild(Clipping_ptr->get_OSGgroup_ptr());

      ClippingKeyHandler* ClippingKeyHandler_ptr=new ClippingKeyHandler(
         Clipping_ptr,ModeController_3D_ptr);
      window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back( 
         ClippingKeyHandler_ptr);
   } // clipping_flag 

// Create the windows and run the threads:

   root_2D->addChild(decorations_2D.get_OSGgroup_ptr());
   root_3D->addChild(decorations_3D.get_OSGgroup_ptr());

   window_mgr_2D_ptr->setSceneData(root_2D);
   window_mgr_3D_ptr->setSceneData(root_3D);

   window_mgr_2D_ptr->realize();
   window_mgr_3D_ptr->realize();

// Adjust 3D viewer's field-of-view if some positive virtual_horiz_FOV
// parameter is passed an input argument:

   double virtual_horiz_FOV=passes_group.get_virtual_horiz_FOV();
   if (virtual_horiz_FOV > 0)
   {
      double FOV_h=window_mgr_3D_ptr->get_lens_horizontal_FOV();
      double angular_scale_factor=virtual_horiz_FOV/FOV_h;  
      cout << "virtual_horiz_FOV = " << virtual_horiz_FOV << endl;
//   cout << "angular_scale_factor = " << angular_scale_factor << endl;
      window_mgr_3D_ptr->rescale_viewer_FOV(angular_scale_factor);
   }

//   movies_3D_group.display_decals();

// FAKE FAKE: 6/7/12 

// For viewchart purposes only, we try to suppress point cloud display

   clouds_group.set_OSGgroup_nodemask(0);
//   OBSFRUSTAGROUP_ptr->set_OSGgroup_nodemask(0);

// ========================================================================

//   PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);

   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }
   delete window_mgr_2D_ptr;
   delete window_mgr_3D_ptr;

   delete postgis_databases_group_ptr;

   delete DTED_ztwoDarray_ptr;

   delete photogroup_ptr;
   delete BuildingsGroup_ptr;
   delete ParkingLotsGroup_ptr;
   delete RoadsGroup_ptr;
}
