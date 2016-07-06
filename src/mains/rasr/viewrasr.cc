// ========================================================================
// Program VIEWRASR is a specialized variant of VIEWPOINTS which
// superposes abstracted floorplan contours (distilled from raw G76 2D
// ladar map data) on some OSGA background.  We wrote this program for
// RASR map viewer development and display purposes.

// 			viewrasr ./south_lab/floorplan.osga

// ========================================================================
// Last updated on 1/16/10; 1/17/10; 1/18/10; 5/10/10
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
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
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
//   Pass* pass_ptr=passes_group.get_pass_ptr(cloudpass_ID);

//   int p_map=2; 	// small_hue_value
//   int p_map=4;    	// large_hue_value_sans_white for Baghdad ground mask
//   int p_map=6;     	// grey scale
//   int p_map=8;	// wrap1
//   int p_map=9;	// wrap2 
//   int p_map=10;	// wrap3 for ALIRT-A and RTV New York map
//   cout << "Enter map number of height colormap:" << endl;
//   cin >> p_map;

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

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;
   clouds_group.generate_Clouds(passes_group,index_tree_flag);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());
   grid_ptr->set_delta_xy(10,10);
   grid_ptr->set_axis_char_label_size(10);
   grid_ptr->set_tick_char_label_size(2.5);
   grid_ptr->update_grid();

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
   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

//   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations.add_Polyhedra(
         passes_group.get_pass_ptr(cloudpass_ID));
//   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=
      decorations.add_Polygons(
         ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(4);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_z_offset(5);

//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);
   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);

// Read in pre-calculated contours:

   string subdir="./south_lab/";
   string components_subdir=subdir+"components/";

   int c_start=0;
//   int n_contours=1;
//   int n_contours=4;
//   int n_contours=10;
//   int n_contours=100;
   int n_contours=198;
   for (int c=c_start; c<n_contours; c++)
   {
      cout << c << " " << flush;
      string contour_filename=components_subdir+
         "contour_"+stringfunc::integer_to_string(c,3)+".contour";
      filefunc::ReadInfile(contour_filename);

      polygon bottom_poly;
      unsigned int linenumber=0;
      bottom_poly.read_from_text_lines(linenumber,filefunc::text_line);
//      cout << "bottom_poly = " << bottom_poly << endl;

      vector<threevector> vertices;
      for (unsigned int n=0; n<bottom_poly.get_nvertices(); n++)
      {
         threevector curr_vertex(bottom_poly.get_vertex(n));
         curr_vertex.put(2,1);
         vertices.push_back(curr_vertex);
      }

// As of Sunday, Jan 17, 2010 at 1 pm, we're basically able to render
// S-building walls as 3D prisms.  But the results do not look
// sufficiently compelling to justify the longer rendering time.  So
// we content ourselves with drawing just a 2D floor plan based upon
// the G76 ladar data:

/*
      polyhedron prism;
      double height=2;	// meters
      prism.generate_prism_with_rectangular_faces(vertices,height);
//      cout << "prism = " << prism << endl;
      Polyhedron* Prism_ptr=PolyhedraGroup_ptr->generate_new_Polyhedron(
         &prism);
//      double alpha=0.5;
//      double alpha=0.75;
      double alpha=0.9;
      colorfunc::Color bbox_color=colorfunc::grey;
      Prism_ptr->set_color(
         colorfunc::get_OSG_color(bbox_color),
         colorfunc::get_OSG_color(bbox_color,alpha));
*/

      vertices.push_back(vertices.at(0));
//      PolyLine* PolyLine_ptr=
         PolyLinesGroup_ptr->generate_new_PolyLine(vertices);

   } // loop over index c labeling contours
   
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

