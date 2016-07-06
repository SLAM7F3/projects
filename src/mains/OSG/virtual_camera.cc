// ========================================================================
// Program VIRTUAL_CAMERA is a variant of VIEWCITIES which opens two
// windows displaying the same point cloud.  In one window, we see the
// view from a virtual OpenGL camera.  In the other window, the
// virtual camera is depicted by an OBSFRUSTUM.  Manipulation of the
// virtual camera in window #1 causes the OBSFRUSTUM to move in window
// #2.

//  virtual_camera cambridge.osga --GIS_layer boston_GIS.pkg

//  virtual_camera --region_filename ./packages/cambridge.pkg --GIS_layer ./packages/boston_GIS.pkg

//  virtual_camera --region_filename ./packages/boston.pkg --GIS_layer ./packages/boston_GIS.pkg

// ========================================================================
// Last updated on 8/25/09; 5/11/10; 5/12/10; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

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

   cout << "cloudpass_ID = " << cloudpass_ID << endl;

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
//   postgis_database* postgis_db_ptr=
      postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Construct the viewers and instantiate ViewerManagers:

   WindowManager* window_mgr_ptr=new ViewerManager();
   WindowManager* window_mgr_global_ptr=new ViewerManager();
   
   string window_title="Virtual camera";
   string window_global_title="Global view";
   window_mgr_ptr->initialize_dual_windows(
      window_title,window_global_title,window_mgr_global_ptr);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   osg::Group* root_global = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_global_ptr=new ModeController();
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_global_ptr) );
   root_global->addChild(osgfunc::create_Mode_HUD(
      ndims,ModeController_global_ptr));

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

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

// Add a custom manipulator to the event handler list:

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

   osgGA::Terrain_Manipulator* CM_3D_global_ptr=
      new osgGA::Terrain_Manipulator(
         ModeController_global_ptr,window_mgr_global_ptr);
   CM_3D_global_ptr->set_Grid_ptr(latlonggrid_ptr);
   window_mgr_global_ptr->set_CameraManipulator(CM_3D_global_ptr);

   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_global_ptr,
                               CM_3D_global_ptr));
   window_mgr_global_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_global_ptr,latlonggrid_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);
   CM_3D_global_ptr->set_PointFinder(&pointfinder);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr,grid_origin_ptr);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);

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

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
   }

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_ArmySymbols(passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_SphereSegments(passes_group.get_pass_ptr(cloudpass_ID));

   decorations.add_ObsFrusta(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   ObsFrustaGroup* ObsFrustaGroup_ptr=decorations.get_ObsFrustaGroup_ptr();

//   ObsFrustum* virtual_ObsFrustum_ptr=
      ObsFrustaGroup_ptr->generate_virtual_camera_ObsFrustum();

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);
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

   root_global->addChild(clouds_group.get_OSGgroup_ptr());
   root_global->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root_global->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);
   window_mgr_global_ptr->setSceneData(root_global);

// Viewer's realize method calls the Custom Manipulator's home() method:

   window_mgr_ptr->realize();
   window_mgr_global_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_3D_ptr->set_worldspace_center(latlonggrid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(
         latlonggrid_ptr->get_xsize(),latlonggrid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   CM_3D_global_ptr->set_worldspace_center(
      latlonggrid_ptr->get_world_middle());
   CM_3D_global_ptr->set_eye_to_center_distance(
      2*basic_math::max(
         latlonggrid_ptr->get_xsize(),latlonggrid_ptr->get_ysize()));
   CM_3D_global_ptr->update_M_and_Minv();

// Reset Producer Viewer's horizontal and vertical FOVs so that they
// match the *CM_3D_ptr->ViewFrustum's:
   
   window_mgr_ptr->process();
   
   window_mgr_ptr->match_viewer_fovs_to_viewfrustum(
      CM_3D_ptr->get_ViewFrustum_ptr());

   while( !window_mgr_ptr->done() && !window_mgr_global_ptr->done() )
   {
      window_mgr_ptr->process();
      window_mgr_global_ptr->process();
   }

// Wait for all cull and draw threads to complete before exiting:

   delete postgis_databases_group_ptr;

   delete window_mgr_ptr;
   delete window_mgr_global_ptr;
}
