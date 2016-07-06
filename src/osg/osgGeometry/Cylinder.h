// ==========================================================================
// Header file for Cylinder class
// ==========================================================================
// Last updated on 4/29/08; 6/5/09; 10/12/11
// ==========================================================================

#ifndef CYLINDER_H
#define CYLINDER_H

#include <iostream>
#include <osg/Material>
#include <osg/Quat>
#include <osg/ShapeDrawable>
#include "color/colorfuncs.h"
#include "osg/osgGeometry/Geometrical.h"
#include "math/threevector.h"

// class osg::Drawable;
// class osg::Geode;
class Polyhedron;

class Cylinder : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Cylinder(const threevector& center,const osg::Quat& q,
            double radius,double height,int id);
   Cylinder(
      const threevector& center,const osg::Quat& q,
      double radius,double height,
      osgText::Font* f_ptr,int n_text_messages,
      const threevector& text_displacement,
      double text_size,colorfunc::Color& permanent_color,int id);
   virtual ~Cylinder();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Cylinder& c);

// Set & get methods:

   void set_text_displacement(const threevector& d);
   void set_center(const threevector& center);
   const threevector& get_center() const;

   void set_Polyhedron_ptr(Polyhedron* Polyhedron_ptr);
   Polyhedron* get_Polyhedron_ptr();
   const Polyhedron* get_Polyhedron_ptr() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode(
      double curr_size,bool text_screen_axis_alignment_flag=true);
   void fill_drawable_geode(osg::Geode* geode_ptr,double curr_size,
                            bool text_screen_axis_alignment_flag=true);
   osg::Drawable* generate_drawable(double curr_size);
   void initialize_text(
      int i,double curr_size,bool screen_axis_alignment_flag=true);
   virtual void set_color(const osg::Vec4& color);   

  protected:

  private:

   double radius,height;
   threevector center,text_displacement;
   osg::Quat quaternion;
   osg::ref_ptr<osg::Material> material_refptr;
   osg::ref_ptr<osg::Cylinder> Cylinder_refptr;
   osg::ref_ptr<osg::ShapeDrawable> shape_refptr;

   Polyhedron* Polyhedron_ptr; // just pointer, not object!

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Cylinder& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Cylinder::set_center(const threevector& center)
{
   this->center=center;
}

inline void Cylinder::set_text_displacement(const threevector& d)
{
   text_displacement=d;
}

inline const threevector& Cylinder::get_center() const
{
   return center;
}


#endif // Cylinder.h



