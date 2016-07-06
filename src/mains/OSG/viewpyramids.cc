// ========================================================================
// Program VIEWPYRAMIDS is a testing lab for instantiating and
// manipulating 3D pyramids.  This program is a variant of BASEMENT,
// and it stands alone from any point cloud input.

// 				viewpyramids	

// ========================================================================
// Last updated on 7/14/08; 12/4/10; 5/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
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
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/pyramid.h"
#include "geometry/polyline.h"
#include "geometry/pyramid.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

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
//   AnimationController* AnimationController_ptr=
//      operations.get_AnimationController_ptr();
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
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      min_X,max_X,min_Y,max_Y,min_Z,
      world_origin_precisely_in_lower_left_corner_flag);
   CM_3D_ptr->set_Grid_ptr(grid_ptr);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_delta_xy(2*mag_factor,2*mag_factor);
   grid_ptr->set_axis_char_label_size(1.0*mag_factor);
   grid_ptr->set_tick_char_label_size(1.0*mag_factor);
   grid_ptr->update_grid();

/*
// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate signpost and feature decoration groups:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.get_FeaturesGroup_ptr()->set_TrianglesGroup_ptr(
      decorations.get_TrianglesGroup_ptr());

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
*/

// Instantiate pyramids decoration group:

   PyramidsGroup* PyramidsGroup_ptr=
      decorations.add_Pyramids(passes_group.get_pass_ptr(cloudpass_ID));

// Pyramid P1 is a model for an OBSFRUSTUM:

   const double X=20*mag_factor;
   const double Y=4*mag_factor;
   const double Z=3*mag_factor;

   threevector apex(0,0,0);
   vector<threevector> base_vertices;
   base_vertices.push_back(threevector(X,Y,Z));
   base_vertices.push_back(threevector(X,-Y,Z));
   base_vertices.push_back(threevector(X,-Y,-Z));
   base_vertices.push_back(threevector(X,Y,-Z));

//   pyramid P1(apex,base_vertices);

   pyramid P1;

   double Uscale=2;
   double Vscale=8;
   double altitude=12;   
   threevector scale_factors(Uscale,Vscale,altitude);

//   threevector Uhat=x_hat;
//   threevector Vhat=y_hat;
   threevector Uhat=(x_hat+9*y_hat+5*z_hat).unitvector();
   threevector Vhat=(-9*x_hat+y_hat).unitvector();
   genmatrix R(3,3);
   R.put_column(0,Uhat);
   R.put_column(1,Vhat);
   R.put_column(2,Uhat.cross(Vhat));

//   threevector apex_posn(2,4,6);
   threevector apex_posn(6,2,5);

//   P1.generate_scaled_rotated_translated_square_pyramid(
//      apex_posn,Uscale,Vscale,altitude,Uhat,Vhat);
   P1.generate_scaled_rotated_translated_square_pyramid(
      scale_factors,R,apex_posn);
   
   P1.ensure_faces_handedness(face::right_handed);
//   P1.ensure_faces_handedness(face::left_handed);

/*
   nrfunc::init_time_based_seed();

   double theta_x=0;
//   double theta_x=45;
//   double theta_x=78.5115;
//   double theta_x=180*(nrfunc::ran1()-0.5);

   double theta_y=0;
//   double theta_y=-45;
//   double theta_y=-5.88642;
//   double theta_y=45*(nrfunc::ran1()-0.5);

//   double theta_z=0;
   double theta_z=45;
//   double theta_z=45*(nrfunc::ran1()-0.5);
//   cout << "Enter thetax in degs:" << endl;
//   cin >> theta_x;
//   cout << "Enter thetay in degs:" << endl;
//   cin >> theta_y;
//   cout << "Enter thetaz in degs:" << endl;
//   cin >> theta_z;

   cout << "thetax = " << theta_x << " thetay = " << theta_y
        << " thetaz = " << theta_z << endl;
   theta_x *= PI/180;
   theta_y *= PI/180;
   theta_z *= PI/180;
   P1.rotate(P1.get_origin(),theta_x,theta_y,theta_z);
//   P1.translate(threevector(0*mag_factor,6*mag_factor,1*mag_factor));
*/

//   cout << "Pyramid P1: " << P1 << endl;
//    P1.generate_GNU_triangulated_surface();


   bool display_P1_flag=false;
   if (display_P1_flag)
   {
      Pyramid* Pyramid1_ptr=
         PyramidsGroup_ptr->generate_new_Pyramid(&P1);
      Pyramid1_ptr->build_current_pyramid(
         PyramidsGroup_ptr->get_curr_t(),PyramidsGroup_ptr->get_passnumber(),
         &P1);

      Pyramid1_ptr->set_color(
         &P1,colorfunc::get_OSG_color(colorfunc::green),
         colorfunc::get_OSG_color(colorfunc::blue),
         colorfunc::get_OSG_color(colorfunc::blue),
         osg::Vec4( 0 , 1 , 1 , 0.1 ));
   }
   
   bool display_P2_flag=true;
//   bool display_P2_flag=false;
   if (display_P2_flag)
   {
      vector<vertex> vertices_above_Zplane;
      vector<edge> edges_above_Zplane;
      vector<face> triangles_above_Zplane;
      P1.extract_parts_above_Zplane(
         grid_origin_ptr->get(2),vertices_above_Zplane,edges_above_Zplane,
         triangles_above_Zplane);
//   cout << "vertices_above_Zplane = " << endl;
//   templatefunc::printVector(vertices_above_Zplane);
//   cout << "edges_above_Zplane = " << endl;
//   templatefunc::printVector(edges_above_Zplane);

      if (vertices_above_Zplane.size() > 0)
      {
         pyramid P2(P1.get_apex().get_posn(),vertices_above_Zplane,
                    edges_above_Zplane,triangles_above_Zplane);
         P2.set_zplane_face(*P1.get_zplane_face_ptr());
//      cout << "zplane_face = " << P2.get_zplane_face() << endl;
//         cout << "Above z-plane polyhedron P2: " << P2 << endl;

//          P2.generate_GNU_triangulated_surface();
         Pyramid* Pyramid2_ptr=
            PyramidsGroup_ptr->generate_new_Pyramid(&P2);
         Pyramid2_ptr->build_current_pyramid(
            PyramidsGroup_ptr->get_curr_t(),
            PyramidsGroup_ptr->get_passnumber(),&P2);

         Pyramid2_ptr->reset_edges_width(10);
         Pyramid2_ptr->set_color(
            &P2,colorfunc::get_OSG_color(colorfunc::green),
            colorfunc::get_OSG_color(colorfunc::red),
            colorfunc::get_OSG_color(colorfunc::blue),
            osg::Vec4( 0.5 , 0.5 , 0.5 , 0.5 ));
      } // > 0 vertices above Zplane conditional
   } // display P2 flag
 
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

