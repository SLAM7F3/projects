// ========================================================================
// Program SATCITIES is a variant of program VIEWCITIES which can
// display Group 63 Hummer tracks and communication satellite strengths.

/*

 		cd /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga
  		satcities --region_filename boston.pkg

*/
// ========================================================================
// Last updated on 6/15/08; 7/4/08; 6/17/09
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
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/geofuncs.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

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
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate database object to send data to and retrieve data from
// an external Postgres database:

   string hostname="";
   string database_name="";
   string username="";
   if (GISlayer_IDs.size() > 0)
   {
      hostname=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_hostname();
      database_name=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_database_name();
      username=passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_PostGIS_username();
   }
   cout << "hostname = " << hostname << endl;
   cout << "database_name = " << database_name << endl;
   cout << "username = " << username << endl;
   postgis_database postgis_db(hostname,database_name,username);
   
   if (GISlayer_IDs.size() > 0)
   {
      vector<string> GISpoints_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gispoints_tablenames();
      for (unsigned int t=0; t<GISpoints_tablenames.size(); t++)
      {
         postgis_db.pushback_GISpoint_tablename(GISpoints_tablenames[t]);
      }
   }
   
// Instantiate final database object to communicate with SKS:
   
   string SKS_hostname="";
   string SKS_database_name="";
   string SKS_username="";
//   if (GISlayer_IDs.size() > 0)
   if (GISlayer_IDs.size() > 1)
   {
      SKS_hostname=passes_group.get_pass_ptr(GISlayer_IDs.back())->
         get_PassInfo_ptr()->get_PostGIS_hostname();
      SKS_database_name=passes_group.get_pass_ptr(
         GISlayer_IDs.back())->
         get_PassInfo_ptr()->get_PostGIS_database_name();
      SKS_username=passes_group.get_pass_ptr(GISlayer_IDs.back())->
         get_PassInfo_ptr()->get_PostGIS_username();
//   hostname="sks";		// Real hostname for ISDS demo
//   database_name="isdsid_stage_sar";	// 2/7/07 dry run
//   username="sks";

      cout << "SKS_hostname = " << SKS_hostname << endl;
      cout << "SKS_database_name = " << SKS_database_name << endl;
      cout << "SKS_username = " << SKS_username << endl;
   }
   
   postgis_database* SKS_worldmodel_db_ptr=NULL;
   if (SKS_username=="sks")
   {
      SKS_worldmodel_db_ptr=new postgis_database(
         SKS_hostname,SKS_database_name,SKS_username);
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Instantiate people, powerpoint and wiki messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   string broker_URL="tcp://155.34.162.148:61616";
//   string broker_URL="tcp://155.34.162.230:61616";
//   string broker_URL="tcp://155.34.125.216:61616";	// family day

   string people_message_queue_channel_name="people";
   Messenger people_messenger( 
      broker_URL, people_message_queue_channel_name );

   string ppt_message_queue_channel_name="powerpoint";
   Messenger ppt_messenger( broker_URL, ppt_message_queue_channel_name );

   string wiki_message_queue_channel_name="wiki";
   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

// Create a DepthPartitionNode root node to manage partitioning of the
// scene:

   DepthPartitionNode* root = new DepthPartitionNode;
   root->setActive(true);     // Control whether the node analyzes the scene
//   root->setActive(false);   // Control whether the node analyzes the scene
//   root->setMaxTraversalDepth(10000);
//   root->setMaxTraversalDepth(100);
   root->setMaxTraversalDepth(5);
//   cout << "max_depth = " << root->getMaxTraversalDepth() << endl;

// Create black backdrop (?)

   root->setClearColorBuffer(true);

// Create OSG root node:

//   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(passes_group.get_pass_ptr(cloudpass_ID));
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   
   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
//    double zmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMin();
   postgis_db.pushback_gis_bbox(xmin,xmax,ymin,ymax);

// Temporary hack as of 5/3/07: Hardwire min/max height thresholds for
// entire Baghdad ladar map:

//   clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr()->
//      set_min_threshold(-71.5953);
//   clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr()->
//      set_max_threshold(63.9469);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate features, army symbol and sphere segments decoration
// groups:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
//      &earth_regions_group);
//   decorations.add_ArmySymbols(passes_group.get_pass_ptr(cloudpass_ID));

   decorations.add_SphereSegments(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   decorations.get_CylindersGroup_ptr()->set_rh(5,0.35);
   decorations.get_CylinderPickHandler_ptr()->set_text_size(8);
   decorations.get_CylinderPickHandler_ptr()->
      set_text_screen_axis_alignment_flag(false);
   CylindersGroup_ptr->pushback_Messenger_ptr(&people_messenger);

// Instantiate ObsFrusta decoration group:

   ObsFrustaGroup* ObsFrustaGroup_ptr=decorations.add_ObsFrusta(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   ObsFrustaGroup_ptr->set_z_ColorMap_ptr(
      clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr());

   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=true;
   ObsFrustaGroup_ptr->generate_still_imagery_frusta(
      passes_group,multicolor_frusta_flag,initially_mask_all_frusta_flag);
   
// Instantiate PolyLines decoration group:

   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(
         ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(3);
   string satcomm_filename="satcomm.txt";
//   string satcomm_filename="hummer_track_lines.txt";
//   cout << "Enter hummer polyline file name:" << endl;
//   cin >> satcomm_filename;

   PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(satcomm_filename);
   cout << "# polylines = " 
        << decorations.get_PolyLinesGroup_ptr()->get_n_Graphicals()
        << endl;

//   PolyLinePickHandler* PolyLinePickHandler_ptr=
//      decorations.get_PolyLinePickHandler_ptr();
//   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
//   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);

// Instantiate group to hold OSG model information:

   ModelsGroup models_group(passes_group.get_pass_ptr(cloudpass_ID));
   string model_filename="./satellite.osg";
   Model* curr_model_ptr=models_group.generate_new_Model(model_filename);

   const double model_scale_factor=30;	// Blender satellite model
   double satellite_az=206;	// degs
//   double elev_true=36;
   double satellite_elev_eff=39.35;   // degs  (takes refraction into account)
   double satellite_range=9000;	// meters
   threevector model_posn=latlonggrid_ptr->get_world_middle()+
      geofunc::posn_from_az_elev_range(satellite_az,satellite_elev_eff,
                                       satellite_range);
   threevector model_attitude(0,0,PI/2);

   curr_model_ptr->set_scale_attitude_posn(
      models_group.get_curr_t(),models_group.get_passnumber(),
      model_scale_factor,model_attitude,model_posn);
   root->addChild(models_group.get_OSGgroup_ptr());

// Instantiate PowerPoints decoration group:

   PowerPointsGroup* PowerPointsGroup_ptr=
      decorations.add_PowerPoints(passes_group.get_pass_ptr(cloudpass_ID));
   PowerPointsGroup_ptr->pushback_Messenger_ptr(&ppt_messenger);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of a user
// selected center location.  On 2/5/07, we learned (the painful and
// hard way!) that the order in which nodes are added to the
// SpinTransform is important for alpha-blending.  In particular, we
// must add decorations' OSGgroup AFTER adding clouds_group OSGgroup
// if alpha blending of 3D video imagery is to work...

   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      latlonggrids_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());
//   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//      open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   window_mgr_ptr->realize();

//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=window_mgr_ptr->getFrameStamp();

//   cout << "Before entering infinite viewer loop" << endl;
//   outputfunc::enter_continue_char();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();

   postgis_db.disconnect();
   if (SKS_worldmodel_db_ptr != NULL)
   {
      SKS_worldmodel_db_ptr->disconnect();
      delete SKS_worldmodel_db_ptr;
   }

}
