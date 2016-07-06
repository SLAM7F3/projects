// ========================================================================
// Program VIEWMODELS is a variant of VIEWPOINTS.
// ========================================================================
// Last updated on 1/24/12; 1/23/16
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
   CM_3D_ptr->set_enable_underneath_looking_flag(true);	
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
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

   double min_grid_x=clouds_group.get_xyz_bbox().xMin();
   double max_grid_x=clouds_group.get_xyz_bbox().xMax();
   double min_grid_y=clouds_group.get_xyz_bbox().yMin();
   double max_grid_y=clouds_group.get_xyz_bbox().yMax();
   double grid_z=2.55;	// meters
   
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,min_grid_x,max_grid_x,min_grid_y,max_grid_y,grid_z);

   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

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

// Instantiate SignPosts decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   SignPostsGroup_ptr->set_ladar_height_data_flag(true);
//   SignPostsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   SignPostsGroup_ptr->set_altitude_dependent_size_flag(false);

// Instantiate Features decoration groups:

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

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(8);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_z_offset(5);

/*
// Try modeling ground plane via translucent skinny Polyhedron:

   polyhedron* ground_polyhedron_ptr=new polyhedron();
   double minX=grid_origin_ptr->get(0);
   double maxX=minX+max_grid_x-min_grid_x;
   double minY=grid_origin_ptr->get(1);
   double maxY=minY+max_grid_y-min_grid_y;
   double minZ=2.55;	// appropriate for MIT2317
   double maxZ=minZ+0.01;
   ground_polyhedron_ptr->generate_box(minX,maxX,minY,maxY,minZ,maxZ);
   Polyhedron* GroundPolyhedron_ptr=PolyhedraGroup_ptr->
      generate_new_Polyhedron(ground_polyhedron_ptr);

   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::black);
   osg::Vec4 volume_color(92.0/255.0,51.0/255.0,23.0/255.0,1.0); //   brown
//   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::yellow);

//   ground_polyhedron_ptr->write_OFF_file("ground_polyhedron.off");

   GroundPolyhedron_ptr->set_color(edge_color,volume_color);
   GroundPolyhedron_ptr->reset_volume_alpha(0.33);
*/

   polyhedron* coop_polyhedron_ptr=new polyhedron();
   double xmin = 328142.9925;
   double xmax = 328201.7419;
   double ymin = 4692142.018;
   double ymax = 4692180.203;
   double zmin = 2.898496752;
   double zmax = 20.05958576;

   coop_polyhedron_ptr->generate_box(xmin,xmax,ymin,ymax,zmin,zmax);
   Polyhedron* CoopPolyhedron_ptr=PolyhedraGroup_ptr->
      generate_new_Polyhedron(coop_polyhedron_ptr);

   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::cyan);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::blue);
//   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::yellow);

   CoopPolyhedron_ptr->set_color(edge_color,volume_color);
   CoopPolyhedron_ptr->reset_volume_alpha(0.1);

// Instantiate SphereSegments decoration group:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr=decorations.add_SphereSegments(
      passes_group.get_pass_ptr(cloudpass_ID));

   double sphere_radius = 36.0696032521;
   threevector sphere_center(
      328172.367227,4692161.11042,11.4790412543);
//   double sphere_radius = 41.4447613241;
//   threevector sphere_center(328179.599993,4692156.0029,11.4790412543);
   SphereSegment* SphereSegment_ptr=SphereSegmentsGroup_ptr->
      generate_new_hemisphere(sphere_radius,sphere_center);
   

/*
// Instantiate Planes decoration group:

   PlanesGroup* PlanesGroup_ptr=decorations.add_Planes(
      passes_group.get_pass_ptr(cloudpass_ID));

   threevector ground_plane_origin(0,0,2.55);	// Appropriate for MIT2317
   plane ground_plane(z_hat,ground_plane_origin);
   Plane* GroundPlane_ptr=PlanesGroup_ptr->generate_new_Plane(
      ground_plane);
*/

// Instantiate separate RegionPolyLinesGroup to hold ROI skeletons:

   RegionPolyLinesGroup* ROILinesGroup_ptr=decorations.add_RegionPolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);
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

