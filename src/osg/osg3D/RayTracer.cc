// ==========================================================================
// RayTracer class member function definitions
// ==========================================================================
// Last updated on 7/18/07; 10/13/07; 6/19/08
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osg3D/RayTracer.h"
#include "osg/osgAnnotators/SignPost.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "time/timefuncs.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RayTracer::initialize_member_objects()
{
   transmitter_Model_ID=0;
   receiver_SignPost_ID=0;

   cloud_ptr=NULL;
   LineSegment_ptr=NULL;
   LineSegmentsGroup_ptr=NULL;
   MODELSGROUP_ptr=NULL;
   SignPostsGroup_ptr=NULL;

   n_LOSs=n_clear_LOSs=0;

   prev_update_time=NEGATIVEINFINITY;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<RayTracer>(
         this, &RayTracer::update_display));
}		       

void RayTracer::allocate_member_objects()
{
   MIV_ptr=new MyIntersectVisitor(tree_ptr);
}		       

RayTracer::RayTracer(
   Pass* PI_ptr,
   SignPostsGroup* SPG_ptr,MODELSGROUP* MG_ptr,LineSegmentsGroup* LSG_ptr,
   PointCloud* c_ptr,TreeVisitor* TV_ptr,
   AnimationController* AC_ptr,threevector* GO_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{
   initialize_member_objects();
   SignPostsGroup_ptr=SPG_ptr;
   MODELSGROUP_ptr=MG_ptr;
   LineSegmentsGroup_ptr=LSG_ptr;
   cloud_ptr=c_ptr;
   TreeVisitor_ptr=TV_ptr;
   tree_ptr=TreeVisitor_ptr->get_tree_ptr();

   allocate_member_objects();
   initialize_ray_linesegment();

   SignPostsGroup_ptr->set_fixed_label_to_SignPost_ID(
      receiver_SignPost_ID,"Receiver");
}

void RayTracer::initialize_ray_linesegment()
{
   bool draw_endpoint1_flag=true;
   bool draw_endpoint2_flag=true;
   LineSegment_ptr=LineSegmentsGroup_ptr->generate_new_canonical_LineSegment(
      -1,draw_endpoint1_flag,draw_endpoint2_flag);
   LineSegment_ptr->set_curr_color(colorfunc::white);
   LineSegment_ptr->get_LineWidth_ptr()->setWidth(4);
   MIV_ptr->addLineSegment(LineSegment_ptr->get_osg_linesegment_ptr());
}

RayTracer::~RayTracer()
{
}

// ==========================================================================
// Tracing methods
// ==========================================================================

bool RayTracer::trace_ray()
{
//   cout << "inside RayTracer::trace_ray()" << endl;
   
   bool clear_LOS_flag=false;

   threevector intercepted_point=get_curr_receiver_posn();
   if (cloud_ptr != NULL && SignPostsGroup_ptr->get_n_Graphicals() > 0)
   {
      reset_ray_endpoints(
         get_curr_transmitter_posn(),get_curr_receiver_posn());

      MIV_ptr->purge_traversal_history();
      cloud_ptr->get_DataNode_ptr()->accept(*MIV_ptr);
//      MIV_ptr->print_traversal_history();

      if (!find_first_intercepted_point(intercepted_point))
      {
         clear_LOS_flag=true;
      }
      else
      {
//         cout << "Intercept wrt grid origin = "
//              << intercepted_point-get_grid_world_origin()
//              << endl;

         double interception_receiver_separation_distance=
            (intercepted_point-get_curr_receiver_posn()).magnitude();

//         const double max_allowed_separation=5;	// meters
         const double max_allowed_separation=1;	// meters

//         cout << "|receiver posn - intercept pt| = " 
//              << interception_receiver_separation_distance << endl;
         if (interception_receiver_separation_distance < 
             max_allowed_separation)
         {
            clear_LOS_flag=true;
         }
      }
   }

   reset_ray_endpoints(get_curr_transmitter_posn(),intercepted_point);
   if (clear_LOS_flag)
   {
      LineSegment_ptr->set_curr_color(colorfunc::green);
//      cout << "Clear LOS" << endl;
   }
   else
   {
      LineSegment_ptr->set_curr_color(colorfunc::red);
//      cout << "LOS is blocked" << endl;
   }

   return clear_LOS_flag;
}

// ---------------------------------------------------------------------
threevector RayTracer::get_curr_transmitter_posn()
{
//   cout << "inside RayTracer::get_curr_transmitter_posn()" << endl;
   
   threevector transmitter_posn(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   if (MODELSGROUP_ptr != NULL &&
       MODELSGROUP_ptr->get_n_Graphicals() > 0)
   {
      MODEL* transmitter_model_ptr=MODELSGROUP_ptr->get_MODEL_ptr(
         transmitter_Model_ID);

      transmitter_model_ptr->get_UVW_coords(
         MODELSGROUP_ptr->get_curr_t(),
         MODELSGROUP_ptr->get_passnumber(),transmitter_posn);

//      cout << "current transmitter posn wrt grid origin = " 
//           << transmitter_posn-get_grid_world_origin()
//           << endl;
   }
   return transmitter_posn;
}

// ---------------------------------------------------------------------
threevector RayTracer::get_curr_receiver_posn()
{
   threevector receiver_posn(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   
   if (SignPostsGroup_ptr != NULL &&
       SignPostsGroup_ptr->get_n_Graphicals() > 0)
   {
      SignPost* receiver_signpost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(
         receiver_SignPost_ID);

      receiver_signpost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),receiver_posn);

//      cout << "current receiver posn wrt grid origin = " 
//           << receiver_posn-get_grid_world_origin()
//           << endl;
   }
   return receiver_posn;
}

// ---------------------------------------------------------------------
void RayTracer::reset_ray_endpoints(
   const threevector& V1,const threevector& V2)
{
   LineSegment_ptr->set_scale_attitude_posn(
      LineSegmentsGroup_ptr->get_curr_t(),
      LineSegmentsGroup_ptr->get_passnumber(),V1,V2);
}

// ---------------------------------------------------------------------
// Boolean member function find_first_intercepted_point() returns
// false if the current LineSegment object does not pierce the DTED
// surface before it terminates at endpoint V2.  Otherwise, it returns
// the first encountered intercept within output threevector
// first_intercepted_point.

bool RayTracer::find_first_intercepted_point(
   threevector& first_intercepted_point)
{
   osgUtil::IntersectVisitor::HitList hlist=MIV_ptr->getHitList(
      LineSegment_ptr->get_osg_linesegment_ptr());
//   cout << "# hits = " << hlist.size() << endl;

   bool intercept_point_found=(hlist.size() > 0);
   if (intercept_point_found)
   {
      osgUtil::Hit hit = hlist.front();
      string drawable_classname=hit._drawable->className();
      if (drawable_classname=="Geometry")
      {
         first_intercepted_point=threevector(hit.getWorldIntersectPoint());
         MIV_ptr->getHitList(LineSegment_ptr->get_osg_linesegment_ptr()).
            clear();
      }
   } // intercept_point_found conditional

   return intercept_point_found;
}		       

// ==========================================================================
// Animation methods:
// ==========================================================================

// Member function update_display()

void RayTracer::update_display()
{
//   cout << "inside RayTracer::update_display()" << endl;
   
   if (AnimationController_ptr->getState()==AnimationController::PLAY)
   {
      compute_curr_clear_LOS_frac();
   }
   
   LineSegmentsGroup_ptr->update_display();

   const double min_elapsed_time_interval=5;	// secs
//   const double min_elapsed_time_interval=10;	// secs
   double curr_update_time=timefunc::elapsed_timeofday_time();
   if (curr_update_time-prev_update_time > min_elapsed_time_interval)
   {
      prev_update_time=curr_update_time;

      TreeVisitor_ptr->purge_traversal_history();
      TreeVisitor_ptr->get_DataNode_ptr()->accept(*TreeVisitor_ptr);
//      TreeVisitor_ptr->print_traversal_history();

      TreeVisitor_ptr->set_scenegraph_updated_flag(false);
   }
   GraphicalsGroup::update_display();
}

// --------------------------------------------------------------------------
void RayTracer::compute_curr_clear_LOS_frac()
{
   n_LOSs++;
   if (trace_ray())
   {
      n_clear_LOSs++;
   }

//   double clear_LOS_frac=double(n_clear_LOSs)/double(n_LOSs);
//   cout << "Clear LOS percentage for current flight path = "
//        << clear_LOS_frac*100 << " %" << endl;
}
