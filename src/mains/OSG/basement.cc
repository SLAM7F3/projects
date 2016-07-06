// ========================================================================
// Program BASEMENT is a testing lab for instantiating and
// manipulating 3D boxes, cones, cylinders and polylines.  It also
// stands alone from any point cloud input.

//				basement

// ========================================================================
// Last updated on 12/2/11; 8/4/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Read input ladar point cloud file:

   const int ndims=3;
   PassesGroup passes_group;
   string cloud_filename="empty.xyzp";
   int cloudpass_ID=passes_group.generate_new_pass(cloud_filename);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
   bool display_movie_number=false;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
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
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=10;
   double min_Y=0;
   double max_Y=10;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      min_X,max_X,min_Y,max_Y,min_Z);
   CM_3D_ptr->set_Grid_ptr(grid_ptr);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("X (Meters)","Y (Meters)");
   grid_ptr->set_delta_xy(1,1);
   grid_ptr->set_axis_char_label_size(1.0);
   grid_ptr->set_tick_char_label_size(1.0);
   grid_ptr->set_mask(1);

// Instantiate SphereSegments decoration group:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr=
      decorations.add_SphereSegments(
         passes_group.get_pass_ptr(cloudpass_ID));
   
   threevector hemisphere_posn=threevector(3,7,0);
   SphereSegment* SphereSegment_ptr=SphereSegmentsGroup_ptr->
      generate_new_hemisphere(3,hemisphere_posn);
   double alpha=0.5;
   SphereSegment_ptr->set_permanent_color(colorfunc::get_OSG_color(
      colorfunc::red,alpha));

// Instantiate boxes decoration group:

   BoxesGroup* BoxesGroup_ptr=decorations.add_Boxes(
      passes_group.get_pass_ptr(cloudpass_ID));
   BoxesGroup_ptr->set_wlh(1,2,3);

   threevector box_center=grid_ptr->get_world_middle()+threevector(0,0,2);
   Box* Box_ptr=BoxesGroup_ptr->generate_new_Box(box_center);

   Box_ptr->set_permanent_color(colorfunc::get_OSG_color(
      colorfunc::yellow,alpha));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   CylindersGroup_ptr->set_rh(2,2);

   osg::Quat q(0,0,0,1);
   colorfunc::Color cyl_color=colorfunc::orange;
   threevector cyl_center=grid_ptr->get_world_middle()+threevector(3,4,2);
   Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
      cyl_center,q,cyl_color);
   Cylinder_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::cyan,0.5));

// Instantiate cones decoration group:

   ConesGroup* ConesGroup_ptr=
      decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));
   colorfunc::Color cone_color=colorfunc::purple;

   double cone_radius=1.0;
   double cone_height=3;
   ConesGroup_ptr->set_rh(cone_radius,cone_height);
   
   Cone* Cone_ptr=ConesGroup_ptr->generate_new_Cone();
   Cone_ptr->set_theta(PI);

   threevector trans=threevector(5,2,cone_height);
   trans += grid_ptr->get_world_middle() - *grid_origin_ptr;
   
//   Cone_ptr->scale_rotate_and_then_translate(
//      ConesGroup_ptr->get_curr_t(),ConesGroup_ptr->get_passnumber(),
//      trans);

   Cone_ptr->set_permanent_color(
      colorfunc::get_OSG_color(cone_color,0.5));
   Cone_ptr->set_color(
      colorfunc::get_OSG_color(cone_color,0.5));

// Experiment with surfaces of revolution:
   
   vector<twovector> rz_vertices;
   rz_vertices.push_back(twovector(2,0));
   rz_vertices.push_back(twovector(2,1));
   rz_vertices.push_back(twovector(1,2));
   rz_vertices.push_back(twovector(1,3));
   rz_vertices.push_back(twovector(0,4));

   polyhedron SoR;
   SoR.generate_surface_of_revolution(rz_vertices);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));

   Polyhedron* SoRPolyhedron_ptr=PolyhedraGroup_ptr->
      generate_new_Polyhedron(&SoR);

   double theta=0;
   double phi=0;
   threevector scale(1,1,1);
   trans=threevector(0,0,0);
   SoRPolyhedron_ptr->scale_rotate_and_then_translate(
      0,0,theta,phi,scale,trans);

   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::cyan);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::blue);
//   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::yellow);

   SoRPolyhedron_ptr->set_color(edge_color,volume_color);
   SoRPolyhedron_ptr->reset_volume_alpha(0.25);

// Add all decorations to root node:

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

