// ========================================================================
// Program VIEWPOINTS is an OSG based viewer for 3D point clouds.

// 	viewpoints lowell.xyzp

// 	viewpoints lowell.tdp

// 	viewpoints lowell.osga

// 	viewpoints --region_filename ./packages/lowell.pkg

// 	viewpoints --region_filename ./packages/boston.pkg

// 	viewpoints --region_filename ./packagesbaghdad.pkg

// ========================================================================
// Last updated on 4/9/11; 12/8/11; 1/8/12; 1/23/16
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
#include "osg/osg2D/ColorbarHUD.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

#include "osg/osg2D/ArrowHUD.h"

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
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
//   CM_3D_ptr->set_min_camera_height_above_grid(100);	// meters
   CM_3D_ptr->set_enable_underneath_looking_flag(true);	
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
//    grid_ptr->set_threeD_grid_flag(true);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);


// Instantiate triangle decorations group:

   decorations.add_Triangles(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;

   vector<PointCloud*>* PointCloud_ptrs_ptr=clouds_group.generate_Clouds(
      passes_group,index_tree_flag,decorations.get_TrianglesGroup_ptr());
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

//    Grid::Distance_Scale distance_scale=Grid::meter;
//   Grid::Distance_Scale distance_scale=Grid::kilometer;
//   double delta_s=-1;	// meters
//   double magnification_factor=12;
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());
//   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
//      grid_ptr,clouds_group.get_xyz_bbox(),distance_scale,delta_s,
//      magnification_factor);
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

/*
// FAKE FAKE:  For displaying GPS vs geoaligned GEO camera path for pass 15
// Tues Mar 5, 2013 at 2:35 pm

   grid_ptr->set_delta_xy(1000,1000);
   grid_ptr->set_axis_char_label_size(
      3*grid_ptr->get_axis_char_label_size());
   grid_ptr->set_tick_char_label_size(
      3*grid_ptr->get_tick_char_label_size());
   grid_ptr->update_grid();
*/

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Set height color map:

   PointCloud* PointCloud_ptr=PointCloud_ptrs_ptr->at(0);
   ColorMap* height_ColorMap_ptr=PointCloud_ptr->get_z_ColorMap_ptr();
   height_ColorMap_ptr->set_mapnumber(14); 
		      // reverse large hue value sans white

// Construct a ColorbarHUD if colormap=reverse large hue value sans white

   if (height_ColorMap_ptr->get_map_number()==14)
   {
      double z_max=height_ColorMap_ptr->get_max_threshold(2);
      double z_min=height_ColorMap_ptr->get_min_threshold(2);
      cout << "z_max = " << z_max << " z_min = " << z_min << endl;
   
      double hue_start=300;	// purple
//   double hue_start=240;	// blue
//   double hue_stop=30;	// orange
      double hue_stop=0;	// red
      double scalar_value_start=z_min;
      double scalar_value_stop=z_max;
      string title="Elevation above WGS84 ellipsoid (m)";

      ColorbarHUD* ColorbarHUD_ptr=new ColorbarHUD(
         hue_start,hue_stop,scalar_value_start,scalar_value_stop,title);

      clouds_group.set_ColorbarHUD_ptr(ColorbarHUD_ptr);

//      root->addChild(ColorbarHUD_ptr->getProjection());

   } // height colormap = 14 = reverse large hue value sans white
   
// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate signpost and feature decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   SignPostsGroup_ptr->set_ladar_height_data_flag(true);
//   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   SignPostsGroup_ptr->set_altitude_dependent_size_flag(false);


   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturesGroup_ptr()->set_TrianglesGroup_ptr(
      decorations.get_TrianglesGroup_ptr());
   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->set_OFF_subdir(
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/");

//    osgGeometry::PolygonsGroup* PolygonsGroup_ptr=
      decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(8);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_z_offset(5);

//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);
//   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);

/*
// Instantiate Polyhedra decoration group:

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));

   polyhedron P2;
   double minX=0;
   double maxX=100;
   double minY=0;
   double maxY=100;
   double minZ=0;
   double maxZ=20;
   P2.generate_box(minX,maxX,minY,maxY,minZ,maxZ);
//   P2.set_origin(*grid_origin_ptr);
//   P2.translate(*grid_origin_ptr);
   P2.absolute_position(*grid_origin_ptr+threevector(100,100,0));

   Polyhedron* Polyhedron2_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(&P2);
   PolyhedraGroup_ptr->generate_polyhedron_geode(Polyhedron2_ptr);
   Polyhedron2_ptr->set_color(
      colorfunc::get_OSG_color(colorfunc::yellow),
      osg::Vec4(1,1,0,0.25));
*/

/*
// Instantiate MODEL decorations group:

   MODELSGROUP* MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   int OSGsubPAT_number=0;
   MODEL* human_MODEL_ptr=MODELSGROUP_ptr->generate_man_MODEL(
      OSGsubPAT_number);
   root->addChild(decorations.get_OSGgroup_ptr());

   double X,Y,Z;
   cout << "Enter X:" << endl;
   cin >> X;
   cout << "Enter Y:" << endl;
   cin >> Y;
   cout << "Enter Z:" << endl;
   cin >> Z;
   threevector posn(X,Y,Z);

   double roll=0;
   double pitch=0;
   double yaw=0;
//   cout << "Enter roll in degs:" << endl;
//   cin >> roll;
//   cout << "Enter pitch in degs:" << endl;
//   cin >> pitch;
   cout << "Enter yaw in degs:" << endl;
   cin >> yaw;
   
   roll *= PI/180;	// About -y_hat
   pitch *= PI/180;	// About -x_hat
   yaw *= PI/180;	// About +z_hat

   human_MODEL_ptr->position_and_orient_man_MODEL(
      MODELSGROUP_ptr->get_curr_t(),MODELSGROUP_ptr->get_passnumber(),
      posn,roll,pitch,yaw);
*/

// Instantiate separate RegionPolyLinesGroup to hold ROI skeletons:

   RegionPolyLinesGroup* ROILinesGroup_ptr=decorations.add_RegionPolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID), AnimationController_ptr);
   ROILinesGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
   ROILinesGroup_ptr->set_width(4);	
   ROILinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

   double ROI_skeleton_height=50;	// meters
   ROILinesGroup_ptr->set_skeleton_height(ROI_skeleton_height);

   RegionPolyLinePickHandler* ROILinePickHandler_ptr=
      decorations.get_RegionPolyLinePickHandler_ptr();
   ROILinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   ROILinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   ROILinePickHandler_ptr->set_z_offset(5);
   ROILinePickHandler_ptr->set_min_points_picked(3);
   ROILinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
   ROILinePickHandler_ptr->set_surface_picking_flag(false);
   ROILinePickHandler_ptr->set_Zplane_picking_flag(false);
   ROILinePickHandler_ptr->set_Allow_Manipulation_flag(false);

// Specify color for ROI polyhedron skeleton:

   colorfunc::Color ROI_color=colorfunc::white;
   ROILinePickHandler_ptr->set_permanent_color(ROI_color);

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
// perform global rotation about some axis coming out of user selected
// center location:

   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());

// Attach scene graph to viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// On 2/6/08, Kevin Chen at MIT taught us that the following 3 C lines
// return information about our computer's graphics card.  They need
// to be executed after an OpenGL context has been established.  Kevin
// suggested that we check out glew.sf.net which has a glew_info
// utility which dumps graphics card information.

//   printf("%s\n",glGetString(GL_RENDERER) );
//   printf("%s\n",glGetString(GL_VENDOR));
//   printf("%s\n",glGetString(GL_VERSION));


   
   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

