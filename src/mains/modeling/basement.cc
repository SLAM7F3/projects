// ========================================================================
// Program BASEMENT is a testing lab for instantiating and
// manipulating 3D boxes, cones, cylinders and polylines.  It also
// stands alone from any point cloud input.

//				basement

// ========================================================================
// Last updated on 1/24/12; 1/26/12; 4/15/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
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

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));

   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::red);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::blue);
   double minX,maxX,minY,maxY,minZ,maxZ;

/*
   polyhedron* polyhedron_ptr=new polyhedron();
   minX=10;
   maxX=minX+5;
   minY=10;
   maxY=minY+5;
   minZ=0;
   maxZ=5;
   polyhedron_ptr->generate_box(minX,maxX,minY,maxY,minZ,maxZ);

   Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->generate_new_Polyhedron(
      polyhedron_ptr);

   Polyhedron_ptr->reset_edges_width(5);
   Polyhedron_ptr->set_color(edge_color,volume_color);
   Polyhedron_ptr->reset_volume_alpha(0.33);

   polyhedron* polyhedron2_ptr=new polyhedron();

   minX=grid_origin_ptr->get(0);
   maxX=minX+1;
   minY=grid_origin_ptr->get(1);
   maxY=minY+1;
   minZ=grid_origin_ptr->get(2);
   maxZ=minZ+1;
   polyhedron2_ptr->generate_box(minX,maxX,minY,maxY,minZ,maxZ);
   Polyhedron* Polyhedron2_ptr=PolyhedraGroup_ptr->generate_new_Polyhedron(
      polyhedron2_ptr);

   Polyhedron2_ptr->reset_edges_width(5);
   Polyhedron2_ptr->set_color(edge_color,volume_color);
   Polyhedron2_ptr->reset_volume_alpha(0.33);
*/

/*
   vector<threevector> bottom_footprint_XYZs;
   bottom_footprint_XYZs.push_back(threevector(5,5,0));
   bottom_footprint_XYZs.push_back(threevector(10,5,0));
   bottom_footprint_XYZs.push_back(threevector(12,11,0));
   bottom_footprint_XYZs.push_back(threevector(7,13,0));
   bottom_footprint_XYZs.push_back(threevector(5,8,0));
   polyline bottom_footprint(bottom_footprint_XYZs);
*/

   vector<threevector> vertices;
   vertices.push_back(threevector(0,0));
   vertices.push_back(threevector(5,0));
   vertices.push_back(threevector(5,1));
   vertices.push_back(threevector(1,1));
   vertices.push_back(threevector(1,4));
   vertices.push_back(threevector(5,4));
   vertices.push_back(threevector(5,5));
   vertices.push_back(threevector(0,5));
   polyline bottom_footprint(vertices);
   polyhedron building_polyhedron;

   const double height=5;
   building_polyhedron.generate_prism_with_rectangular_faces(
      bottom_footprint,height);
   Polyhedron* building_Polyhedron_ptr=PolyhedraGroup_ptr->
      generate_new_Polyhedron(&building_polyhedron);

   building_Polyhedron_ptr->reset_edges_width(5);
   building_Polyhedron_ptr->set_color(edge_color,volume_color);
   building_Polyhedron_ptr->reset_volume_alpha(0.5);


/*
   polyhedron* polyhedron3_ptr=new polyhedron();

   minX=grid_origin_ptr->get(0);
   maxX=minX+25;
   minY=grid_origin_ptr->get(1);
   maxY=minY+25;
   minZ=grid_origin_ptr->get(2);
   maxZ=minZ+0.01;
   polyhedron3_ptr->generate_box(minX,maxX,minY,maxY,minZ,maxZ);
   Polyhedron* Polyhedron3_ptr=PolyhedraGroup_ptr->generate_new_Polyhedron(
      polyhedron3_ptr);

   edge_color=colorfunc::get_OSG_color(colorfunc::black);
   volume_color=colorfunc::get_OSG_color(colorfunc::yellow);

   Polyhedron3_ptr->set_color(edge_color,volume_color);
   Polyhedron3_ptr->reset_volume_alpha(0.1);
*/

/*
   string ply_filename="box1.ply";
   polyhedron_ptr->write_PLY_file(ply_filename);
   ply_filename="box2.ply";
   polyhedron2_ptr->write_PLY_file(ply_filename);
   ply_filename="box3.ply";
   polyhedron3_ptr->write_PLY_file(ply_filename);
*/

/*
// Instantiate PolyLines decoration group:

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(3);

   vector<threevector> vertices;
   vertices.push_back(threevector(0,0));
   vertices.push_back(threevector(5,0));
   vertices.push_back(threevector(5,1));
   vertices.push_back(threevector(1,1));
   vertices.push_back(threevector(1,4));
   vertices.push_back(threevector(5,4));
   vertices.push_back(threevector(5,5));
   vertices.push_back(threevector(0,5));
   contour* contour_ptr=new contour(vertices);
   cout << "contour = " << *contour_ptr << endl;

   vertices.push_back(vertices.front());
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(vertices);

// Instantiate Triangles decoration group:

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   triangles_group* triangles_group_ptr=
      contour_ptr->generate_interior_triangles();
   for (int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
      triangle* curr_triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);

      vector<threevector> vertices;
      vertices.push_back(curr_triangle_ptr->get_vertex(0).get_posn());
      vertices.push_back(curr_triangle_ptr->get_vertex(1).get_posn());
      vertices.push_back(curr_triangle_ptr->get_vertex(2).get_posn());
      polygon tri_poly(vertices);

      threevector reference_origin=Zero_vector;
      osgGeometry::Polygon* TriPolygon_ptr=
         PolygonsGroup_ptr->generate_new_Polygon(reference_origin,tri_poly);
      TriPolygon_ptr->set_permanent_color(colorfunc::get_color(t));
   } // loop over index t labeling triangles
   PolygonsGroup_ptr->reset_colors();
*/

// Instantiate Polygons decoration group:

/*
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   vector<threevector> vertices;
   vertices.push_back(threevector(0,0,0));
   vertices.push_back(threevector(15,0,0));
   vertices.push_back(threevector(15,8,0));
   vertices.push_back(threevector(0,8,0));
   polygon random_poly(vertices);

   threevector reference_origin=Zero_vector;
   osgGeometry::Polygon* GroundPolygon_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(reference_origin,random_poly);

   colorfunc::Color ground_color=colorfunc::yellow;
   double alpha=0.2;
   GroundPolygon_ptr->set_permanent_color(colorfunc::get_OSG_color(
      ground_color,alpha));

//   osg::Vec4 gcolor(0 , 0 , 1 , alpha);
//   GroundPolygon_ptr->set_permanent_color(gcolor);
   PolygonsGroup_ptr->reset_colors();
*/

/*
// Instantiate Planes decoration group:

   PlanesGroup* PlanesGroup_ptr=decorations.add_Planes(
      passes_group.get_pass_ptr(cloudpass_ID));

   threevector ground_plane_origin(0,0,2.55);	// Appropriate for MIT2317
   plane ground_plane(z_hat,ground_plane_origin);
   cout << "ground_plane = " << ground_plane << endl;

   Plane* GroundPlane_ptr=PlanesGroup_ptr->generate_new_canonical_Plane();
//   Plane* GroundPlane_ptr=PlanesGroup_ptr->generate_new_Plane(ground_plane);
   cout << "GroundPlane_ptr = " << GroundPlane_ptr << endl;

   double scale=10;
   GroundPlane_ptr->set_scale_attitude_posn(
      PlanesGroup_ptr->get_curr_t(),
      PlanesGroup_ptr->get_passnumber(),ground_plane,scale);
*/

// Instantiate MODEL decorations group to hold 3D human figure:

   MODELSGROUP* human_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

   int OSGsubPAT_number;
   MODEL* human_MODEL_ptr=human_MODELSGROUP_ptr->generate_man_MODEL(
      OSGsubPAT_number);
   double roll=0;
   double pitch=0;
   double yaw=0;
   threevector posn(3,4,0);
   human_MODEL_ptr->position_and_orient_man_MODEL(
      human_MODELSGROUP_ptr->get_curr_t(),
      human_MODELSGROUP_ptr->get_passnumber(),posn,roll,pitch,yaw);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   string movie_filename="/data/ImageEngine/kermit/kermit000.jpg";
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* Movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);
   
   twovector lower_left_texture_fracs(0.25,0.25);
   twovector lower_right_texture_fracs(0.75,0.25);
   twovector upper_right_texture_fracs(0.75,0.75);
   twovector upper_left_texture_fracs(0.25,0.75);

   Movie_ptr->set_texture_fracs(
      lower_left_texture_fracs,
      lower_right_texture_fracs,
      upper_right_texture_fracs,
      upper_left_texture_fracs);

   threevector video_bottom_left=*grid_origin_ptr+threevector(0,10,0);
   threevector video_bottom_right=*grid_origin_ptr+threevector(10,10,0);
   threevector video_top_right=*grid_origin_ptr+threevector(10,10,10);
   threevector video_top_left=*grid_origin_ptr+threevector(0,10,10);

   Movie_ptr->reset_geom_vertices(
      video_bottom_right,video_bottom_left,
      video_top_left,video_top_right);

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

