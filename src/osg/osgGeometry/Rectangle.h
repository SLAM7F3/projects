// ==========================================================================
// Header file for Rectangle class
// ==========================================================================
// Last updated on 10/21/07; 3/31/11; 7/10/11; 7/17/11
// ==========================================================================

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <iostream>
#include <map>
#include <string>
#include <osg/Array>
#include "geometry/bounding_box.h"
#include "osg/osgGeometry/Geometrical.h"
#include "color/colorfuncs.h"

// class osg::Drawable;
// class osg::StateSet;
// class osg::Texture2D;
class threevector;
class twovector;

class Rectangle : public Geometrical
{

  public:

   typedef std::map<std::string,osg::StateSet*> STATESET_MAP;
    
// Initialization, constructor and destructor functions:

   Rectangle(int id,int ndims=2);
   virtual ~Rectangle();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Rectangle& r);

// Set & get methods:

   double get_canonical_length() const;
   double get_canonical_width() const;
   const bounding_box& get_bbox() const;
   bounding_box* get_bbox_ptr();
   const bounding_box* get_bbox_ptr() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   osg::StateSet* generate_texture_and_stateset(std::string image_filename);
   
   void set_canonical_local_vertices();
   void reset_bbox(double t,int pass_number);
   void set_world_vertices(
      const twovector& V0,const twovector& V1,const twovector& V2,
      const twovector& V3);
   void set_world_vertices(
      const threevector& V0,const threevector& V1,const threevector& V2,
      const threevector& V3);
   void compute_screenspace_vertex_posns(threevector screenspace_vertex[]);
   
  protected:

  private:

   double canonical_length,canonical_width;
   bounding_box bbox;
   osg::ref_ptr<osg::Geometry> geom_refptr;
   osg::ref_ptr<osg::StateSet> stateset_refptr;
   osg::ref_ptr<osg::Texture2D> face_texture_refptr;
   STATESET_MAP* Stateset_map_ptr;

   void allocate_member_objects(); 
   void initialize_member_objects();
   void docopy(const Rectangle& s);

   osg::Geometry* generate_drawable_geom();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double Rectangle::get_canonical_length() const
{
   return canonical_length;
}

inline double Rectangle::get_canonical_width() const
{
   return canonical_width;
}

inline const bounding_box& Rectangle::get_bbox() const
{
   return bbox;
}

inline bounding_box* Rectangle::get_bbox_ptr()
{
   return &bbox;
}

inline const bounding_box* Rectangle::get_bbox_ptr() const
{
   return &bbox;
}


#endif // Rectangle.h



