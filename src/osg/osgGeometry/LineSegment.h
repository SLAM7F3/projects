// ==========================================================================
// Header file for LineSegment class
// ==========================================================================
// Last updated on 10/21/07; 10/8/09; 11/16/10
// ==========================================================================

#ifndef LineSegment_H
#define LineSegment_H

#include <iostream>
#include <osg/Array>
#include <osg/LineSegment>
#include <osg/LineWidth>
#include <osg/ShapeDrawable>
#include "osg/osgGeometry/Geometrical.h"
#include "geometry/linesegment.h"

class AnimationController;

class LineSegment : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   LineSegment(const int p_ndim,const threevector& V1,const threevector& V2,
               int id,bool draw_arrow=false,AnimationController* AC_ptr=NULL);
   LineSegment(const int p_ndim,const threevector& V1,const threevector& V2,
		   int id,bool draw_endpoint1,bool draw_endpoint2,
		   double endpoint_size_prefactor,
		   AnimationController* AC_ptr=NULL);
   virtual ~LineSegment();
   friend std::ostream& operator<< (
      std::ostream& outstream,const LineSegment& l);

// Set & get methods:

   void set_endpoint_size_prefactor(double prefactor);
   void set_V1(const threevector& v1);
   threevector get_V1() const;
   void set_V2(const threevector& v2);
   threevector get_V2() const;
   threevector get_midpoint() const;
   double get_length() const;
   threevector get_ehat() const;
   osg::LineWidth* get_LineWidth_ptr();
   osg::LineSegment* get_osg_linesegment_ptr();

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   virtual void set_color(const osg::Vec4& color);
   void set_draw_arrow_flag();
   void reset_vertices();
   osg::ShapeDrawable* generate_arrow_tip_drawable(const threevector& n_hat);


// LineSegment manipulation methods

   void set_scale_attitude_posn(
      double curr_t,int pass_number,
      const threevector& V1,const threevector& V2);
   void recover_V1_and_V2(
      double curr_t,int pass_number,threevector& V1,threevector& V2);

  protected:

  private:

   bool draw_arrow_flag,draw_endpoint1_flag,draw_endpoint2_flag;
   double endpoint_size_prefactor;
   linesegment l;
   osg::ref_ptr<osg::Geometry> geom_refptr;
   osg::ref_ptr<osg::LineWidth> linewidth_refptr;
   osg::ref_ptr<osg::LineSegment> osg_linesegment_refptr;

   void allocate_member_objects();
   void initialize_member_objects(); 
   void docopy(const LineSegment& s);

   void reset_our_and_osg_linesegments(
      const threevector& V1,const threevector& V2);
   osg::Geometry* generate_drawable_geom();

   osg::ShapeDrawable* generate_endpoint(int i);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void LineSegment::set_endpoint_size_prefactor(double prefactor)
{
   endpoint_size_prefactor=prefactor;
}

inline osg::LineWidth* LineSegment::get_LineWidth_ptr()
{
   return linewidth_refptr.get();
}

inline void LineSegment::set_V1(const threevector& v1)
{
   l=linesegment(v1,get_V2());
}

inline void LineSegment::set_V2(const threevector& v2)
{
   l=linesegment(get_V1(),v2);
}

inline threevector LineSegment::get_V1() const
{
   return threevector(l.get_v1());
}

inline threevector LineSegment::get_V2() const
{
   return l.get_v2();
}

inline threevector LineSegment::get_midpoint() const 
{
   return l.get_midpoint();
}

inline double LineSegment::get_length() const 
{
   return l.get_length();
}

inline threevector LineSegment::get_ehat() const 
{
   return l.get_ehat();
}

inline osg::LineSegment* LineSegment::get_osg_linesegment_ptr()
{
   return osg_linesegment_refptr.get();
}


#endif // LineSegment.h



