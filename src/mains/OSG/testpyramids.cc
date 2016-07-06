// ========================================================================
// Program TESTPYRAMIDS is a testing lab for instantiating and
// manipulating 3D pyramids.  This program is a variant of BASEMENT,
// and it stands alone from any point cloud input.

// 			 testpyramids ./images/copley.jpg

// ========================================================================
// Last updated on 8/20/07; 9/21/07; 10/15/07; 12/4/10
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
#include "osg/osgSceneGraph/ColorMap.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "math/mathfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyline.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"

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
   PassesGroup passes_group(&arguments);
   vector<int> videopass_ID;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         videopass_ID.push_back(n);
         cout << "n = " << n << " videopass_ID = " << videopass_ID.back()
              << endl;
      }
   }

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

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   const double mag_factor=1;
   double min_X=0;
   double max_X=10*mag_factor;
   double min_Y=0;
   double max_Y=10*mag_factor;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   bool world_origin_precisely_in_lower_left_corner_flag=true;
//   bool world_origin_precisely_in_lower_left_corner_flag=false;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      min_X,max_X,min_Y,max_Y,min_Z,
      world_origin_precisely_in_lower_left_corner_flag);
   CM_3D_ptr->set_Grid_ptr(grid_ptr);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_delta_xy(2*mag_factor,2*mag_factor);
   grid_ptr->set_axis_char_label_size(1.0*mag_factor);
   grid_ptr->set_tick_char_label_size(1.0*mag_factor);
   grid_ptr->update_grid();

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);

// Instantiate signpost and feature decoration groups:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(
      videopass_ID.front()));
   decorations.get_FeaturesGroup_ptr()->set_TrianglesGroup_ptr(
      decorations.get_TrianglesGroup_ptr());

// Instantiate boxes decoration group:

   decorations.add_Boxes(
      passes_group.get_pass_ptr(videopass_ID.front()));
   root->addChild(decorations.get_BoxesGroup_ptr()->
                  createBoxLight(threevector(20,10,10)));
   decorations.get_BoxesGroup_ptr()->set_wlh(2,2,2);

// Instantiate cones decoration group:

   decorations.add_Cones(passes_group.get_pass_ptr(videopass_ID.front()));
   root->addChild(decorations.get_ConesGroup_ptr()->
                  createConeLight(threevector(20,10,10)));

// Instantiate cylinders decoration group:

   decorations.add_Cylinders(passes_group.get_pass_ptr(videopass_ID.front()),
                             AnimationController_ptr);
   decorations.get_CylindersGroup_ptr()->set_rh(5,0.1);
   root->addChild(decorations.get_CylindersGroup_ptr()->
                  createCylinderLight(threevector(20,10,10)));

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=
      decorations.add_OBSFRUSTA(passes_group.get_pass_ptr(
         videopass_ID.front()),NULL);

   double sidelength=20;
   threevector camera_posn(0,0,0);
   vector<threevector> UV_corner_dir;
   double phi=20*PI/180;
   double theta=15*PI/180;
   UV_corner_dir.push_back(
      mathfunc::construct_direction_vector(phi,-theta));
   UV_corner_dir.push_back(
      mathfunc::construct_direction_vector(-phi,-theta));
   UV_corner_dir.push_back(
      mathfunc::construct_direction_vector(-phi,theta));
   UV_corner_dir.push_back(
      mathfunc::construct_direction_vector(phi,theta));
   
   threevector u_hat((UV_corner_dir[1]-UV_corner_dir[0]).unitvector());
   threevector v_hat((UV_corner_dir[2]-UV_corner_dir[1]).unitvector());
   threevector w_hat=u_hat.cross(v_hat);

//   cout << "u_hat = " << u_hat << endl;
//   cout << "v_hat = " << v_hat << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->generate_test_OBSFRUSTUM(
      grid_origin_ptr->get(2),sidelength,UV_corner_dir);
   OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();

/*
   nrfunc::init_time_based_seed();
//   double theta_x=0;
//   double theta_x=45;
//   double theta_x=78.5115;
   double theta_x=180*(nrfunc::ran1()-0.5);

//   double theta_y=0;
//   double theta_y=-45;
//   double theta_y=-5.88642;
   double theta_y=45*(nrfunc::ran1()-0.5);

//   double theta_z=0;
//   double theta_z=54.0295;
   double theta_z=45*(nrfunc::ran1()-0.5);

//   cout << "Enter thetax in degs:" << endl;
//   cin >> theta_x;
//   cout << "Enter thetay in degs:" << endl;
//   cin >> theta_y;
//   cout << "Enter thetaz in degs:" << endl;
//   cin >> theta_z;

//   double theta_x=-81.0157;
//   double theta_y=-12.3212;
//   double theta_z=16.598;

//   cout << "thetax = " << theta_x << " thetay = " << theta_y
//        << " thetaz = " << theta_z << endl;
   theta_x *= PI/180;
   theta_y *= PI/180;
   theta_z *= PI/180;
//   P1.rotate(P1.get_origin(),theta_x,theta_y,theta_z);

   threevector new_posn=*grid_origin_ptr+threevector(6,6,10);
//   threevector new_posn(1,1,1);
//   threevector new_Uhat(1,-2,-1);
//   threevector new_Vhat(3,1,1);
   threevector new_Uhat(1,0,0);
   threevector new_Vhat(0,1,0);
//   threevector new_Uhat(1,0,0);
//   threevector new_Vhat(0,1,0);
//   OBSFRUSTUM_ptr->absolute_position_and_orientation(
//      0,0,new_posn,new_Uhat.unitvector(),new_Vhat.unitvector());
*/

   threevector new_posn=*grid_origin_ptr+threevector(6,6,10);

   double curr_t=0;
   int pass_number=0;
   double az=60*PI/180;
   double el=-60*PI/180;
   double roll=0;

   OBSFRUSTUM_ptr->absolute_position_and_orientation(
      curr_t,pass_number,new_posn,az,el,roll);

//   cout << "BEFORE modifying apex, viewing pyramid = "
//        << *(OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()) << endl;
   pyramid* viewing_pyramid_ptr=OBSFRUSTUM_ptr->get_viewing_pyramid_ptr();
//   viewing_pyramid_ptr->get_vertices_handler_ptr()->
//      print_vertex_map_contents();


//   threevector newer_posn=*grid_origin_ptr+threevector(12,12,10);
//   viewing_pyramid_ptr->reset_vertex_posn(new_posn,newer_posn);

//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
//        << endl;
//   cout << "AFTER modifying apex, viewing pyramid = "
//        << *(OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()) << endl;
//   viewing_pyramid_ptr->get_vertices_handler_ptr()->
//      print_vertex_map_contents();

   double z_ground=grid_origin_ptr->get(2);
   OBSFRUSTUM_ptr->compute_viewing_pyramid_above_Zplane(
      z_ground,OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());

   threevector new_apex(12,12,10);
   vector<threevector> new_base_vertices;
   new_base_vertices.push_back(threevector(5,5,-10));

//   new_base_vertices.push_back(threevector(5,-5,-10));
   new_base_vertices.push_back(threevector(-5,5,-10));

   new_base_vertices.push_back(threevector(-5,-5,-10));

//   new_base_vertices.push_back(threevector(-5,5,-10));
   new_base_vertices.push_back(threevector(5,-5,-10));

   viewing_pyramid_ptr->reset_square_pyramid_vertices(
      new_apex,new_base_vertices);

/*
   cout << "AFTER compute_viewing_pyr_above_Zplane, viewing_pyr_above_zplane = "
        << *(OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr()) << endl;

   exit(-1);
*/

   OBSFRUSTUM_ptr->generate_Pyramid_geodes();

   OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->build_current_pyramid(
      0,0,OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());
   OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->build_current_pyramid(
      0,0,OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());
   
//   OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_mask(0,0,true);
   OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_mask(0,0,false);

// Color ViewingPyramid and ViewPyramidAboveZplane graphicals.  Then
// set their edge widths:

   OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_color(
      OBSFRUSTUM_ptr->get_viewing_pyramid_ptr(),
      colorfunc::get_OSG_color(colorfunc::green),
      colorfunc::get_OSG_color(colorfunc::blue),
      colorfunc::get_OSG_color(colorfunc::blue),
      osg::Vec4( 0 , 1 , 1 , 0.1 ));
   
   OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->set_color(
      OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr(),
      colorfunc::get_OSG_color(colorfunc::yellow),
      colorfunc::get_OSG_color(colorfunc::orange),
      colorfunc::get_OSG_color(colorfunc::red),
      osg::Vec4( 0.5 , 0.5 , 0.5 , 0.25 ));

   OBSFRUSTUM_ptr->set_typical_pyramid_edge_widths();
//   OBSFRUSTUM_ptr->set_color(colorfunc::get_OSG_color(
//       colorfunc::blue));

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

