// ========================================================================
// Program VIEWCITIES is a variant of program VIEWPOINTS which is
// meant to be used for very large urban data sets.

//   viewcities cambridge.osga --GIS_layer ./packages/boston_GIS.pkg

//   viewcities --region_filename ./packages/boston.pkg --GIS_layer ./packages/boston_GIS.pkg

//   viewcities --region_filename ./packages/boston.pkg --GIS_layer ./packages/boston_GIS.pkg --GIS_layer ./packages/SKS_GIS.pkg

//   viewcities --region_filename ./packages/baghdad.pkg --GIS_layer ./packages/baghdad_GIS.pkg

//  viewcities --region_filename ./packages/boston.pkg --region_filename ./packages/copley.pkg 

//  viewcities --region_filename ./packages/boston.pkg --GIS_layer ./packages/boston_GIS.pkg --region_filename ./packages/copley.pkg --region_filename ./packages/copley_north.pkg --GIS_layer ./packages/SKS_GIS.pkg

// ========================================================================
// Last updated on 3/20/09; 5/11/10; 5/12/10; 5/26/10
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
#include "osg/osgEarth/EarthRegionsGroup.h"
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
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
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

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);
   
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

// Create OSG root node:

   osg::Group* root = new osg::Group;

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
//   double zmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMin();

   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;
   postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

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

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);
   SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);
   SignPostsGroup_ptr->set_extra_textmessage_info(" New York City ");

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
   }

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
   PolyLinesGroup_ptr->set_width(8);
   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);

// Instantiate PowerPoints decoration group:

   PowerPointsGroup* PowerPointsGroup_ptr=
      decorations.add_PowerPoints(passes_group.get_pass_ptr(cloudpass_ID));
   PowerPointsGroup_ptr->pushback_Messenger_ptr(&ppt_messenger);

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of a user
// selected center location.  On 2/5/07, we learned (the painful and
// hard way!) that the order in which nodes are added to the
// SpinTransform is important for alpha-blending.  In particular, we
// must add decorations' OSGgroup AFTER adding clouds_group OSGgroup
// if alpha blending of 3D video imagery is to work...

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

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

   delete postgis_databases_group_ptr;
}
