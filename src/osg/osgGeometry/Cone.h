// ==========================================================================
// Header file for Cone class
// ==========================================================================
// Last updated on 1/24/07; 5/6/07; 5/7/07; 6/27/08
// ==========================================================================

#ifndef CONE_H
#define CONE_H

#include <iostream>
#include <osg/Array>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include "osg/osgGeometry/Geometrical.h"
#include "math/threevector.h"

// class osg::Drawable;
// class osg::Geode;

class Cone : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Cone(double radius,double height,int id);
   virtual ~Cone();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Cone& c);

// Set & get methods:

   const threevector& get_tip() const;
   const threevector& get_base() const;
   void set_phi(double phi);
   double get_phi() const;
   void set_theta(double theta);
   double get_theta() const;
   void set_scale(const threevector& s);
   const threevector& get_scale() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   osg::Drawable* generate_drawable();
   virtual void set_color(const osg::Vec4& color);

   void reset_state(osg::Vec4& c);

// Manipulation methods:

   void scale_rotate_and_then_translate(
      double curr_t,int pass_number,const threevector& trans);
   void scale_rotate_and_then_translate(
      double curr_t,int pass_number,
      double theta,double phi,const threevector& scale,
      const threevector& trans);

  protected:

  private:

   double radius,height;
   double phi,theta;
   threevector tip,base,scale;
   osg::ref_ptr<osg::Material> material_refptr;
   osg::ref_ptr<osg::Cone> Cone_refptr;
   osg::ref_ptr<osg::ShapeDrawable> shape_refptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Cone& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline const threevector& Cone::get_tip() const
{
   return tip;
}

inline const threevector& Cone::get_base() const
{
   return base;
}

inline void Cone::set_phi(double phi)
{
   this->phi=phi;
}

inline double Cone::get_phi() const
{
   return phi;
}

inline void Cone::set_theta(double theta)
{
   this->theta=theta;
}

inline double Cone::get_theta() const
{
   return theta;
}

inline void Cone::set_scale(const threevector& s)
{
   scale=s;
}

inline const threevector& Cone::get_scale() const
{
   return scale;
}

#endif // Cone.h



