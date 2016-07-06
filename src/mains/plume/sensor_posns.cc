// ========================================================================
// Program SENSOR_POSNS sets up 3D Geometrical objects (hemispheres,
// boxes, cylinders, cones) in the Z=0 plane according to the November
// 2011 sensor grid plan.  It also displays the locations of 4x3=12
// wooden posts on the South Carolina experiment site.

//				sensor_posns

// ========================================================================
// Last updated on 12/13/11; 12/14/11; 1/3/13
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

// Constants

   const double feet_per_meter=3.2808;
   double alpha=0.5;

// Repeated variable declarations

   double dz;
   vector<threevector> XYZ;

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
   double max_X=50;
   double min_Y=0;
   double max_Y=50;
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

//   threevector sensor_grid_center=*grid_origin_ptr+threevector(15,10);
   threevector sensor_grid_center=grid_ptr->get_world_middle();
   double thetax=0;
   double thetay=0;
   double thetaz=0*PI/180;
//   double thetaz=30*PI/180;
   rotation R(thetax,thetay,thetaz);

// Instantiate SphereSegments decoration group:

   SphereSegmentsGroup* SphereSegmentsGroup_ptr=
      decorations.add_SphereSegments(
         passes_group.get_pass_ptr(cloudpass_ID));
   
   dz=0;

   XYZ.clear();
   XYZ.push_back(threevector(-6,-6,dz));
   XYZ.push_back(threevector(0,-6,dz));
   XYZ.push_back(threevector(6,-6,dz));
   XYZ.push_back(threevector(-6,0,dz));
   XYZ.push_back(threevector(6,0,dz));
   XYZ.push_back(threevector(-6,6,dz));
   XYZ.push_back(threevector(0,6,dz));
   XYZ.push_back(threevector(6,6,dz));
   
   colorfunc::Color hemisphere_color=colorfunc::yegr;
   double hemisphere_radius=0.5;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_hemisphere_posn(R*XYZ[i]);
      threevector hemisphere_posn=sensor_grid_center+rel_hemisphere_posn;
      SphereSegment* SphereSegment_ptr=SphereSegmentsGroup_ptr->
         generate_new_hemisphere(hemisphere_radius,hemisphere_posn);
      SphereSegment_ptr->set_permanent_color(colorfunc::get_OSG_color(
         hemisphere_color,alpha));
   }

// Instantiate boxes decoration group:

   BoxesGroup* BoxesGroup_ptr=decorations.add_Boxes(
      passes_group.get_pass_ptr(cloudpass_ID));
   BoxesGroup_ptr->set_wlh(0.5,0.5,0.5);

   dz=0 /feet_per_meter;	// meters

   XYZ.clear();
   XYZ.push_back(threevector(-16,-16,dz));
   XYZ.push_back(threevector(16,-16,dz));

   XYZ.push_back(threevector(-10,0,dz));
   XYZ.push_back(threevector(0,10,dz));
   XYZ.push_back(threevector(0,-10,dz));
   XYZ.push_back(threevector(10,0,dz));

   XYZ.push_back(threevector(16,16,dz));
   XYZ.push_back(threevector(-16,16,dz));

   XYZ.push_back(threevector(3,0,dz));
   XYZ.push_back(threevector(-3,0,dz));
   XYZ.push_back(threevector(0,3,dz));
   XYZ.push_back(threevector(0,-3,dz));

   colorfunc::Color box_color=colorfunc::red;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_box_posn(R*XYZ[i]);
      threevector box_center=sensor_grid_center+rel_box_posn;
      Box* Box_ptr=BoxesGroup_ptr->generate_new_Box(box_center);
      Box_ptr->set_permanent_color(colorfunc::get_OSG_color(
         box_color,alpha));
   }

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   osg::Quat q(0,0,0,1);

   double cyl_height=10 /feet_per_meter;	// meters
   CylindersGroup_ptr->set_rh(0.5,0.5*cyl_height);

   XYZ.clear();
   XYZ.push_back(threevector(-12,-12,0));
   XYZ.push_back(threevector(0,-12,0));
   XYZ.push_back(threevector(12,-12,0));
   XYZ.push_back(threevector(-12,0,0));
   XYZ.push_back(threevector(12,0,0));
   XYZ.push_back(threevector(12,12,0));
   XYZ.push_back(threevector(0,12,0));
   XYZ.push_back(threevector(-12,12,0));

   colorfunc::Color cyl_color=colorfunc::blgr;
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector rel_cyl_posn(R*XYZ[i]);
      threevector cyl_center=sensor_grid_center+rel_cyl_posn+0.5*cyl_height*
         z_hat;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         cyl_center,q,cyl_color);
      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(cyl_color,alpha));
   }

// Depict 4 sets of wooden posts via Cylinders:

   XYZ.clear();
   XYZ.push_back(threevector(19.8848,   22.3228,   1.524));
   XYZ.push_back(threevector(22.1238,   22.392,   1.5494));
   XYZ.push_back(threevector(22.2444,   20.0636,   1.5113));
   XYZ.push_back(threevector(23.135,   -20.2825,   1.4097));
   XYZ.push_back(threevector(23.0408,   -22.2898,   1.5748));
   XYZ.push_back(threevector(20.6738,  -22.3807,   1.524));
   XYZ.push_back(threevector(-20.0271,   -22.5936,   1.4859));
   XYZ.push_back(threevector(-22.1606,   -22.6965,   1.5748));
   XYZ.push_back(threevector(-22.4681,   -20.4006,   1.5494));
   XYZ.push_back(threevector(-21.7022,   24.2473,   1.4605));
   XYZ.push_back(threevector(-21.6222,   26.5879,   1.5113));
   XYZ.push_back(threevector(-19.2814,   26.3858,   1.4478));

   cyl_color=colorfunc::brown;
   cyl_height=1.5;	// meter
   for (int i=0; i<XYZ.size(); i++)
   {
      threevector curr_XY(XYZ[i].get(0),XYZ[i].get(1));
//      threevector rel_cyl_posn(R*XYZ[i]);
      threevector rel_cyl_posn(R*curr_XY);
      cout << "i = " << i << " XY = " << curr_XY << " rel_cyl_posn = "
           << rel_cyl_posn << endl;
      
      threevector cyl_center=sensor_grid_center+rel_cyl_posn;
//         +0.5*cyl_height*z_hat;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         cyl_center,q,cyl_color);
      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(cyl_color,alpha));
   }


// Instantiate cones decoration group:

   ConesGroup* ConesGroup_ptr=
      decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));

   double cone_height=5 /feet_per_meter;	// meters
   double cone_radius=0.5;
   ConesGroup_ptr->set_rh(cone_radius,cone_height);

   XYZ.clear();
   XYZ.push_back(threevector(0,0,0));		// Marks sensor grid center

   XYZ.push_back(threevector(-20,-20,0));
   XYZ.push_back(threevector(-7,-20,0));
   XYZ.push_back(threevector(7,-20,0));
   XYZ.push_back(threevector(20,-20,0));

   XYZ.push_back(threevector(-12,12,0));
   XYZ.push_back(threevector(0,12,0));
   XYZ.push_back(threevector(12,12,0));

   XYZ.push_back(threevector(-20,7,0));
   XYZ.push_back(threevector(20,7,0));

   XYZ.push_back(threevector(-12,0,0));
   XYZ.push_back(threevector(12,0,0));

   XYZ.push_back(threevector(-20,-7,0));
   XYZ.push_back(threevector(20,-7,0));

   XYZ.push_back(threevector(-12,-12,0));
   XYZ.push_back(threevector(0,-12,0));
   XYZ.push_back(threevector(12,-12,0));

   XYZ.push_back(threevector(20,20,0));
   XYZ.push_back(threevector(-7,20,0));
   XYZ.push_back(threevector(7,20,0));
   XYZ.push_back(threevector(-20,20,0));

   colorfunc::Color cone_color=colorfunc::yellow;
   alpha=1;
   for (int i=0; i<XYZ.size(); i++)
   {
      if (i > 0)
      {
         cone_color=colorfunc::blue;	// 5' particle counters
         alpha=0.9;
      }

      threevector rel_cone_posn(R*XYZ[i]);
      threevector cone_base=rel_cone_posn;
      threevector cone_tip=cone_base+threevector(0,0,cone_height);

      Cone* Cone_ptr=ConesGroup_ptr->generate_new_Cone();
      Cone_ptr->set_theta(PI);

      threevector trans=cone_tip;
      trans += sensor_grid_center - *grid_origin_ptr;

      Cone_ptr->scale_rotate_and_then_translate(
         ConesGroup_ptr->get_curr_t(),ConesGroup_ptr->get_passnumber(),trans);

      Cone_ptr->set_color(
         colorfunc::get_OSG_color(cone_color,alpha));
   } 
   
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

