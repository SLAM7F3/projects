// ========================================================================
// Program VIEWPOLYHEDA is a testing lab for instantiating and
// manipulating 3D polyhedra.  This program is a variant of BASEMENT,
// and it stands alone from any point cloud input.

// 				viewpolyhedra	

// ========================================================================
// Last updated on 12/17/07; 2/17/08; 4/22/08; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyhedron.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;

// Read input ladar point cloud file:

   PassesGroup passes_group;
   string cloud_filename="empty.xyzp";
   int cloudpass_ID=passes_group.generate_new_pass(cloud_filename);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

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

   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   const double mag_factor=1;
//   const double mag_factor=1000000;
//   const double mag_factor=1000000;

   double min_X=0;
   double max_X=10*mag_factor;
   double min_Y=0;
   double max_Y=10*mag_factor;
   double min_Z=0;

//   double min_X=-200;
//   double max_X=250;
//   double min_Y=-100;
//   double max_Y=250;
//   double min_Z=0;

/*
   double min_X=-1000;
   double max_X=2000;
   double min_Y=0;
   double max_Y=3000;
   double min_Z=-100;
*/

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      min_X,max_X,min_Y,max_Y,min_Z);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   decorations.set_grid_origin_ptr(grid_origin_ptr);

/*
   grid_ptr->set_delta_xy(2,2);
   grid_ptr->set_axis_char_label_size(1.0);
   grid_ptr->set_tick_char_label_size(1.0);
*/

   grid_ptr->set_delta_xy(2*mag_factor,2*mag_factor);
   grid_ptr->set_axis_char_label_size(1.0*mag_factor);
   grid_ptr->set_tick_char_label_size(1.0*mag_factor);

//   grid_ptr->set_delta_xy(500,500);
//   grid_ptr->set_axis_char_label_size(100.0);
//   grid_ptr->set_tick_char_label_size(100.0);

   grid_ptr->update_grid();

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Instantiate boxes decoration group:

   decorations.add_Boxes(
      passes_group.get_pass_ptr(cloudpass_ID));
   root->addChild(decorations.get_BoxesGroup_ptr()->
                  createBoxLight(threevector(20,10,10)));
   decorations.get_BoxesGroup_ptr()->set_wlh(2,2,2);

// Instantiate cones decoration group:

   decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));
   root->addChild(decorations.get_ConesGroup_ptr()->
                  createConeLight(threevector(20,10,10)));

// Instantiate cylinders decoration group:

   decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                             AnimationController_ptr);
   decorations.get_CylindersGroup_ptr()->set_rh(5,0.1);
   root->addChild(decorations.get_CylindersGroup_ptr()->
                  createCylinderLight(threevector(20,10,10)));

// Instantiate polyhedra decoration group:

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));

// Polyhedron P1 is a model for an ObsFrustum:

//   const double X=1*mag_factor;
//   const double Y=2*mag_factor;
   const double Z=3*mag_factor;
   vector<threevector> vertices;
   vertices.push_back(threevector(0,0,Z));
   vertices.push_back(threevector(-Z,-Z,0));
   vertices.push_back(threevector(Z,-Z,0));
   vertices.push_back(threevector(Z,Z,0));
   vertices.push_back(threevector(-Z,Z,0));

   polyhedron P1;
   P1.generate_square_pyramid(vertices);
   P1.absolute_position(*grid_origin_ptr);

   double sx=1;
   double sy=1;
   double sz=10;
   P1.scale(P1.get_origin(),threevector(sx,sy,sz));

   double theta_x=0;
   double theta_y=-45;
   double theta_z=0;
   cout << "Enter thetax in degs:" << endl;
   cin >> theta_x;
   cout << "Enter thetay in degs:" << endl;
   cin >> theta_y;
   cout << "Enter thetaz in degs:" << endl;
   cin >> theta_z;
   theta_x *= PI/180;
   theta_y *= PI/180;
   theta_z *= PI/180;
   P1.rotate(P1.get_origin(),theta_x,theta_y,theta_z);
//   P1.translate(threevector(-1*mag_factor,0,2*mag_factor));

//   double sx,sy,sz;
//   cout << "Enter scale-x factor:" << endl;
//   cin >> sx;
//   cout << "Enter scale-y factor:" << endl;
//   cin >> sy;
//   cout << "Enter scale-z factor:" << endl;
//   cin >> sz;
//   P1.scale(P1.get_origin(),threevector(sx,sy,sz));

//    P1.generate_GNU_triangulated_surface();

   Polyhedron* Polyhedron1_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(&P1);
//   Polyhedron1_ptr->build_current_polyhedron(
//      PolyhedraGroup_ptr->get_curr_t(),PolyhedraGroup_ptr->get_passnumber());
   Polyhedron1_ptr->set_color(
      colorfunc::get_OSG_color(colorfunc::cyan),osg::Vec4(0,0,1,0.5));

/*
   vertices.push_back(threevector(0,0,Z));
   vertices.push_back(threevector(X,0,Z));
   vertices.push_back(threevector(X,Y,Z));
   vertices.push_back(threevector(0,Y,Z));

   vertices.push_back(threevector(0,0,0));
   vertices.push_back(threevector(X,0,0));
   vertices.push_back(threevector(X,Y,0));
   vertices.push_back(threevector(0,Y,0));
*/


// P2 = model for above ground box

   polyhedron P2;
   double minX=0;
   double maxX=1;
   double minY=0;
   double maxY=1;
   double minZ=0;
   double maxZ=10;
   P2.generate_box(minX,maxX,minY,maxY,minZ,maxZ);
//   P2.set_origin(*grid_origin_ptr);
//   P2.translate(*grid_origin_ptr);
   P2.absolute_position(*grid_origin_ptr+threevector(-0.5,-0.5,0));

/*
   double theta_x,theta_y,theta_z;
   cout << "Enter thetax in degs:" << endl;
   cin >> theta_x;
   cout << "Enter thetay in degs:" << endl;
   cin >> theta_y;
   cout << "Enter thetaz in degs:" << endl;
   cin >> theta_z;
   theta_x *= PI/180;
   theta_y *= PI/180;
   theta_z *= PI/180;
   P2.rotate(P2.get_origin(),theta_x,theta_y,theta_z);
*/

/*
   double sx,sy,sz;
   cout << "Enter scale-x factor:" << endl;
   cin >> sx;
   cout << "Enter scale-y factor:" << endl;
   cin >> sy;
   cout << "Enter scale-z factor:" << endl;
   cin >> sz;

*/
   sx=50*mag_factor;
   sy=50*mag_factor;
   sz=1*mag_factor;
   P2.scale(*grid_origin_ptr,threevector(sx,sy,sz));

//    P2.generate_GNU_triangulated_surface();

/*
   threevector origin2(0,0,0);
//   threevector origin2(328945,4690725,25);
   P2.set_origin(origin2);
   P2.translate(P2.get_origin());
   P2.generate_GNU_triangulated_surface();
*/

   Polyhedron* Polyhedron2_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(&P2);
//   Polyhedron2_ptr->build_current_polyhedron(
//      PolyhedraGroup_ptr->get_curr_t(),PolyhedraGroup_ptr->get_passnumber());
   Polyhedron2_ptr->set_color(
      colorfunc::get_OSG_color(colorfunc::yellow),
      osg::Vec4(1,1,0,0.25));

/*
   Polyhedron* Polyhedron3_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(P3);
   Polyhedron3_ptr->set_color(osg::Vec4(1,1,1,0.2));

   Polyhedron* Polyhedron4_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(P4);
   Polyhedron4_ptr->set_color(osg::Vec4(0,1,1,0.2));
*/

/*
   Polyhedron* Polyhedron5_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(P5);
   Polyhedron5_ptr->set_color(osg::Vec4(1,0,0,0));

   Polyhedron* Polyhedron6_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(P6);
   Polyhedron6_ptr->set_color(osg::Vec4(0,1,1,0));

*/

// Expt with renormalizing both P1 and P2 vertices relative to P2
// before computing their intersection:

//   double x_scale=(maxX-minX)*sx;
//   double y_scale=(maxY-minY)*sy;
//   double z_scale=(maxZ-minZ)*sz;
//   cout << "x_scale = " << x_scale << " y_scale = " << y_scale
//        << " z_scale = " << z_scale << endl;

//   Polyhedron* Renorm_intersect_Polyhedron_ptr=
//      PolyhedraGroup_ptr->safely_compute_intersection(&P1,&P2);
//   Renorm_intersect_Polyhedron_ptr->set_color(
//      colorfunc::get_OSG_color(colorfunc::orange),
//      colorfunc::get_OSG_color(colorfunc::cyan));

/*
   Polyhedron* intersection_Polyhedron_ptr=
      PolyhedraGroup_ptr->compute_intersection(
         Polyhedron1_ptr,Polyhedron2_ptr);
   if (intersection_Polyhedron_ptr != NULL)
   {
      cout << "intersection polyhedron = "
           << *(intersection_Polyhedron_ptr->get_polyhedron_ptr())
           << endl;
      intersection_Polyhedron_ptr);
      intersection_Polyhedron_ptr->set_color(
         colorfunc::get_OSG_color(colorfunc::orange),
         colorfunc::get_OSG_color(colorfunc::cyan));
   }
*/

//   root->addChild(decorations.get_PolyhedraGroup_ptr()->
//                  createPolyhedraLight(threevector(20,10,10)));

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(root);
   root->addChild(decorations.get_OSGgroup_ptr());

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// height according to grid's maximal linear dimension:

   CM_3D_ptr->set_worldspace_center(grid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      3*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
}

