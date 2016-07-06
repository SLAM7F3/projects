// ==========================================================================
// Header file for SignPost class
// ==========================================================================
// Last updated on 1/20/09; 1/30/11; 1/28/12
// ==========================================================================

#ifndef SignPost_H
#define SignPost_H

#include <iostream>
#include <string>
#include <osg/Array>
#include <osg/ShapeDrawable>
#include "osg/osgAnnotators/Annotator.h"
#include "osg/osgGeometry/Geometrical.h"

class SignPost : public Annotator, public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   SignPost(int id,int ndims);
   virtual ~SignPost();
   friend std::ostream& operator<< (
      std::ostream& outstream,const SignPost& s);

// Set & get methods:

   void set_SKS_worldmodel_database_flag(bool flag);
   bool get_SKS_worldmodel_database_flag() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode(
      double curr_size,double height_multiplier=1.0);
   virtual void set_color(const colorfunc::Color& color);
   virtual void set_color(const osg::Vec4& color);
   void set_quasirandom_color();
   void set_label(std::string input_label,double extra_frac_cyl_height=0);
   std::string get_label() const;
   void set_category(std::string input_category);
   std::string get_category() const;
   void set_max_text_width(std::string input_label);

// Signpost manipulation methods:

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

   bool SKS_worldmodel_database_flag;
   float cone_height,cylinder_height;
   std::string label,category;
   osg::Vec3 cone_position,cylinder_position;

   osg::Cone* Cone_ptr;
   osg::ref_ptr<osg::ShapeDrawable> cone_shape_refptr;
   osg::ref_ptr<osg::ShapeDrawable> cylinder_shape_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const SignPost& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline std::string SignPost::get_label() const
{
   return label;
}

inline void SignPost::set_category(std::string input_category)
{
   category=input_category;
}

inline std::string SignPost::get_category() const
{
   return category;
}

inline void SignPost::set_SKS_worldmodel_database_flag(bool flag) 
{
   SKS_worldmodel_database_flag=flag;
}

inline bool SignPost::get_SKS_worldmodel_database_flag() const
{
   return SKS_worldmodel_database_flag;
}

#endif // SignPost.h



