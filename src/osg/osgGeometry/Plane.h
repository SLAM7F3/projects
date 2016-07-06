// ==========================================================================
// Header file for OSG Plane class
// ==========================================================================
// Last updated on 1/4/07; 1/21/07; 10/13/07; 10/21/07; 6/15/08
// ==========================================================================

#ifndef Plane_H
#define Plane_H

#include <iostream>
#include <osg/Array>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGeometry/LineSegment.h"
#include "geometry/plane.h"

class AnimationController;
// class osg::Geode;
// class osg::Geometry;

class Plane : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Plane(const plane& p_in, int id,AnimationController* AC_ptr=NULL);
   virtual ~Plane();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Plane& p_in);

// Set & get methods:

   threevector get_nhat() const;
   void set_plane(const plane& p_in);
   plane* get_plane_ptr();
   void set_n_segment_ptr(LineSegment* n_ptr);
   LineSegment* get_n_segment_ptr();

// Drawing methods:

   osg::Geode* generate_drawable_geode();

   void set_ID_text();
   void set_scale_attitude_posn(double curr_t,int pass_number,const plane& p);
   void set_scale_attitude_posn(double curr_t,int pass_number,const plane& p,
      double scale);
   void generate_canonical_normal_segment();
   
  protected:

  private:

   int n_vertices;
   threevector ncanonical_hat;
   plane p;
   LineSegment* n_segment_ptr;
   osg::ref_ptr<osg::Geometry> geom_refptr;

   void allocate_member_objects();
   void initialize_member_objects(); 
   void docopy(const Plane& p_in);

   void set_canonical_vertices();
   osg::Geometry* generate_drawable_geom();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline threevector Plane::get_nhat() const
{
   return p.get_nhat();
}

inline void Plane::set_plane(const plane& p_in)
{
   p=p_in;
}

inline plane* Plane::get_plane_ptr()
{
   return &p;
}

inline void Plane::set_n_segment_ptr(LineSegment* n_ptr)
{
   n_segment_ptr=n_ptr;
}

inline LineSegment* Plane::get_n_segment_ptr()
{
   return n_segment_ptr;
}


#endif // Plane.h



