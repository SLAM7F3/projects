// ========================================================================
// Program INSPOINTS is a variant of VIEWPOINTS which takes in XSENS
// INS readings.

//			inspoints lowell.osga

// ========================================================================
// Last updated on 3/18/11; 3/22/11; 4/3/11
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
#include "ins/xsens_ins.h"

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

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
//   CM_3D_ptr->set_min_camera_height_above_grid(100);	// meters
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate an XSENS_INS object to detect physical motions:

   xsens_ins INS;
   INS.initialize_xsens();
   INS.initialize_xsens_2();
   INS.set_CM_3D_ptr(CM_3D_ptr);

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

//   Grid::Distance_Scale distance_scale=Grid::meter;
//   double delta_s=2;	// meters
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());
//   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
//      grid_ptr,clouds_group.get_xyz_bbox(),distance_scale,delta_s);
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate signpost and feature decoration groups:

   decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
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
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
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
      INS.update_ins_metadata();
      INS.update_ins_metadata_2();
      INS.update_avg_az_el_roll();
//      INS.update_median_az_el_roll();
   }

   delete window_mgr_ptr;
}

