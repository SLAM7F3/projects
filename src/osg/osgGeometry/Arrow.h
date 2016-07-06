// ==========================================================================
// Header file for Arrow class
// ==========================================================================
// Last updated on 9/9/09; 9/29/09; 11/16/10; 7/31/11
// ==========================================================================

#ifndef ARROW_H
#define ARROW_H

#include <iostream>
#include <string>
#include <osg/Array>
#include <osg/LineWidth>
#include <osg/ShapeDrawable>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGeometry/ConesGroup.h"

class Pass;

class Arrow : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Arrow(int ndims,Pass* PI_ptr,threevector* GO_ptr,int ID);
   virtual ~Arrow();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Arrow& s);

// Set & get methods:

   threevector& get_V_tip();
   const threevector& get_V_tip() const;
   threevector& get_V_base();
   const threevector& get_V_base() const;
   
   ConesGroup* get_ConesGroup_ptr();
   const ConesGroup* get_ConesGroup_ptr() const;
   void set_label(std::string input_label,double extra_frac_cyl_height=0);
   std::string get_label() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   void set_magnitude_direction_and_base(
      double magnitude,const threevector& e_hat,const threevector& base,
      double arrowhead_size_prefactor=1);
   void set_magnitude_direction_and_center(
      double magnitude,const threevector& e_hat,
      const threevector& abs_center,double arrowhead_size_prefactor);
   void set_linewidth(double width);
   virtual void set_color(const colorfunc::Color& c);
   virtual void set_color(const osg::Vec4& color);
   void set_max_text_width(std::string input_label);

// Arrow manipulation methods:

   void set_attitude_posn(
      double curr_t,int pass_number,
      const threevector& V1,const threevector& V2);
   void reset_attitude_posn(
      double curr_t,int pass_number,const threevector& cone_tip,
      const threevector& cone_dir_hat);
   void reset_scale(
      double curr_t,int pass_number,double scale_factor);

  protected:

  private:

   std::string label;
   threevector V_base,V_tip;
   ConesGroup* ConesGroup_ptr;

   osg::ref_ptr<osg::LineWidth> linewidth_refptr;
   osg::ref_ptr<osg::Geometry> geom_refptr;
   osg::ref_ptr<osg::DrawArrays> drawarrays_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Arrow& a);

   osg::Geometry* generate_drawable_geom();
   void set_relative_vertices();
   void generate_arrow_head(double arrowhead_size_prefactor);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline threevector& Arrow::get_V_tip()
{
   return V_tip;
}

inline const threevector& Arrow::get_V_tip() const
{
   return V_tip;
}

inline threevector& Arrow::get_V_base()
{
   return V_base;
}

inline const threevector& Arrow::get_V_base() const
{
   return V_base;
}

inline ConesGroup* Arrow::get_ConesGroup_ptr()
{
   return ConesGroup_ptr;
}

inline const ConesGroup* Arrow::get_ConesGroup_ptr() const
{
   return ConesGroup_ptr;
}


inline std::string Arrow::get_label() const
{
   return label;
}

#endif // Arrow.h



