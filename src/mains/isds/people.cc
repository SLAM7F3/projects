// ========================================================================
// Program PEOPLE is a variant of BASEMENT.  It is meant to be used
// for testing mouse event handling via message passing.

// ActiveMQ running on ISD3D laptop:

// 			people 155.34.162.148:61616

// ActiveMQ running on touchy:

// 			people 155.34.162.230:61616
				
// ========================================================================
// Last updated on 9/21/07; 10/15/07; 10/23/07; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "math/constant_vectors.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize clock for real-time visualization effects
// (e.g. Geometrical blinking):

   timefunc::initialize_timeofday_clock();

// Read input ladar point cloud file:

   const int ndims=3;
   PassesGroup passes_group;
   string cloud_filename="empty.xyzp";
   int cloudpass_ID=passes_group.generate_new_pass(cloud_filename);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Instantiate people, powerpoint and wiki messengers:

   string broker_URL = "tcp://127.0.0.1:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string people_message_queue_channel_name="people";
   Messenger people_messenger( 
      broker_URL, people_message_queue_channel_name);

   string ppt_message_queue_channel_name="powerpoint";
   Messenger ppt_messenger( broker_URL, ppt_message_queue_channel_name );

   string wiki_message_queue_channel_name="wiki";
   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

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

   double min_X=0;
   double max_X=100;
   double min_Y=0;
   double max_Y=100;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      min_X,max_X,min_Y,max_Y,min_Z);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   grid_ptr->set_axes_labels("X (Meters)","Y (Meters)");
   grid_ptr->set_delta_xy(10,10);
   grid_ptr->set_axis_char_label_size(5.0);
   grid_ptr->set_tick_char_label_size(5.0);
   grid_ptr->update_grid();

// Instantiate a transformer in order to convert between screen and
// world space coordinate systems:

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

/*
// Instantiate boxes decoration group:

   decorations.add_Boxes(
      passes_group.get_pass_ptr(cloudpass_ID));
   root->addChild(decorations.get_BoxesGroup_ptr()->
                  createBoxLight(threevector(20,10,10)));
   decorations.get_BoxesGroup_ptr()->set_wlh(2,2,2);
*/

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);

// Instantiate cones decoration group:

   decorations.add_Cones(passes_group.get_pass_ptr(cloudpass_ID));
   root->addChild(decorations.get_ConesGroup_ptr()->
                  createConeLight(threevector(20,10,10)));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(passes_group.get_pass_ptr(cloudpass_ID),
                                AnimationController_ptr);
   decorations.get_CylindersGroup_ptr()->set_rh(5,0.1);
   decorations.get_CylinderPickHandler_ptr()->set_text_size(4);
   decorations.get_CylinderPickHandler_ptr()->
      set_text_screen_axis_alignment_flag(false);
   root->addChild(decorations.get_CylindersGroup_ptr()->
                  createCylinderLight(threevector(20,10,10)));

   CylindersGroup_ptr->pushback_Messenger_ptr(&people_messenger);

// Instantiate pyramids decoration group:

   decorations.add_Pyramids(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate PowerPoints decoration group:

   PowerPointsGroup* PowerPointsGroup_ptr=
      decorations.add_PowerPoints(passes_group.get_pass_ptr(cloudpass_ID));
   PowerPointsGroup_ptr->pushback_Messenger_ptr(&ppt_messenger);

// Instantiate SphereSegments decoration group:

   bool display_spokes_flag=false;
   bool include_blast_flag=false;
   double sphere_segment_radius=50;
   double sphere_segment_az_min=0;
   double sphere_segment_az_max=2*PI;
   double sphere_segment_el_min=0;
   double sphere_segment_el_max=PI/2;

   decorations.add_SphereSegments(
      passes_group.get_pass_ptr(cloudpass_ID),
      NULL,NULL,sphere_segment_radius,
      sphere_segment_az_min,sphere_segment_az_max,
      sphere_segment_el_min,sphere_segment_el_max,
      display_spokes_flag,include_blast_flag);

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

