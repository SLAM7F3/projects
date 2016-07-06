// ==========================================================================
// Header file for Box class
// ==========================================================================
// Last updated on 12/15/07; 3/31/11; 12/2/11
// ==========================================================================

#ifndef Box_H
#define Box_H

#include <iostream>
#include <osg/Array>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include "osg/osgGeometry/Geometrical.h"
#include "geometry/mybox.h"
#include "osg/osgGeometry/Rectangle.h"

// class osg::Drawable;
// class osg::Group;
class threevector;

class Box : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Box(double w,double l,double h,int id);
   Box(double w,double l,double h,double face_displacement,int id);
   virtual ~Box();
   friend std::ostream& operator<< (std::ostream& outstream,const Box& box);

// Set & get methods:

   void set_selected_face_number(int f);
   int get_selected_face_number() const;
   Rectangle* get_selected_face_ptr();
   void set_mybox(const mybox& bnew);
   mybox* get_mybox_ptr();
   osg::Group* get_group_ptr();
   const osg::Group* get_group_ptr() const;
   
// Drawing methods:

   osg::Group* generate_drawable_group();
   osg::Drawable* generate_drawable();
   osgText::Text* generate_text(int i=0);
   void reset_text_font_and_size(int i,double charsize_factor);
   
   virtual void set_color(const osg::Vec4& color);   
   void reset_selected_face_drawable(int f,const threevector& Box_posn);

// Box manipulation methods:

   void set_posn(double curr_t,int pass_number,const threevector& V);

  protected:

   double selected_face_displacement;	// meter
   Rectangle* selected_face_ptr;

  private:

   int selected_face_number; // -2 = none, -1 = bottom, 4 = top, 0-3 = sides
   mybox b;
   osg::ref_ptr<osg::Group> group_refptr;
   osg::ref_ptr<osg::Material> material_refptr;
   osg::ref_ptr<osg::Box> Box_refptr;
   osg::ref_ptr<osg::ShapeDrawable> shape_refptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Box& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Box::set_selected_face_number(int f)
{
   selected_face_number=4;
   if (f >=-1 && f <= 4)
   {
      selected_face_number=f;
   }
}

inline int Box::get_selected_face_number() const
{
   return selected_face_number;
}

inline Rectangle* Box::get_selected_face_ptr() 
{
   return selected_face_ptr;
}

inline void Box::set_mybox(const mybox& bnew)
{
   b=bnew;
}

inline mybox* Box::get_mybox_ptr()
{
   return &b;
}

inline osg::Group* Box::get_group_ptr()
{
   return group_refptr.get();
}

inline const osg::Group* Box::get_group_ptr() const
{
   return group_refptr.get();
}


#endif // Box.h



