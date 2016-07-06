// ==========================================================================
// Header file for RayTracer class
// ==========================================================================
// Last updated on 11/1/06; 11/3/06; 11/13/06; 10/13/07; 6/19/08
// ==========================================================================

#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <osg/LineSegment>
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgSceneGraph/MyIntersectVisitor.h"
#include "math/threevector.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

class AnimationController;
class LineSegment;
class LineSegmentsGroup;
class MODELSGROUP;
class PointCloud;
class SignPostsGroup;

class RayTracer : public GraphicalsGroup
{

  public:
    
   typedef TreeVisitor::data_type data_type;

// Initialization, constructor and destructor functions:

   RayTracer(
      Pass* PI_ptr,
      SignPostsGroup* SPG_ptr,MODELSGROUP* MG_ptr,LineSegmentsGroup* LSG_ptr,
      PointCloud* c_ptr,TreeVisitor* TV_ptr,
      AnimationController* AC_ptr,threevector* GO_ptr);
   virtual ~RayTracer();

// Set & get methods:

   MyIntersectVisitor* get_MIV_ptr();

// Tracing methods:

   bool trace_ray();
   threevector get_curr_transmitter_posn();
   threevector get_curr_receiver_posn();
   void reset_ray_endpoints(const threevector& V1,const threevector& V2);
   bool find_first_intercepted_point(threevector& first_intercepted_point);

// Animation methods:

   void update_display();
   void compute_curr_clear_LOS_frac();

  protected:

  private:

   int transmitter_Model_ID,receiver_SignPost_ID;
   int n_LOSs,n_clear_LOSs;
   double prev_update_time;
   threevector first_intercepted_point;
   MyIntersectVisitor* MIV_ptr;

   LineSegment* LineSegment_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   MODELSGROUP* MODELSGROUP_ptr;
   PointCloud* cloud_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   Tree<data_type>* tree_ptr;
   TreeVisitor* TreeVisitor_ptr;

   void allocate_member_objects();
   void initialize_member_objects(); 
   void initialize_ray_linesegment();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline MyIntersectVisitor* RayTracer::get_MIV_ptr()
{
   return MIV_ptr;
}

#endif // RayTracer.h



