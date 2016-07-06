// ==========================================================================
// Cylinder class member function definitions
// ==========================================================================
// Last updated on 4/29/08; 6/5/09; 10/12/11
// ==========================================================================

#include <iostream>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Quat>
#include "osg/osgGeometry/Cylinder.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/Polyhedron.h"

#include "osg/osgfuncs.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Cylinder::allocate_member_objects()
{
   material_refptr=new osg::Material;
}		       

void Cylinder::initialize_member_objects()
{
   Graphical_name="Cylinder";
   Polyhedron_ptr=NULL;
}		       

Cylinder::Cylinder(
   const threevector& center,const osg::Quat& q,
   double radius,double height,int id):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();

   this->center=center;
   quaternion=q;
   this->radius=radius;
   this->height=height;
}		       

Cylinder::Cylinder(
   const threevector& center,const osg::Quat& q,double radius,double height,
   osgText::Font* f_ptr,int n_text_messages,
   const threevector& text_displacement,
   double text_size,colorfunc::Color& permanent_color,int id):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();

   this->center=center;
   quaternion=q;
   this->radius=radius;
   this->height=height;

   font_refptr=f_ptr;
   this->n_text_messages=n_text_messages;
   for (unsigned int i=0; i<get_n_text_messages(); i++)
   {
      osg::ref_ptr<osgText::Text> curr_text_refptr=new osgText::Text;
      text_refptr.push_back(curr_text_refptr);
      set_text_size(i,text_size);
   }
   this->text_displacement=text_displacement;

   set_permanent_color(colorfunc::get_OSG_color(permanent_color));
}		       

Cylinder::~Cylinder()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Cylinder& c)
{
   outstream << "inside Cylinder::operator<<" << endl;
//   outstream << static_cast<const Geometrical&>(c) << endl;
   return(outstream);
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void Cylinder::set_Polyhedron_ptr(Polyhedron* Polyhedron_ptr)
{
   this->Polyhedron_ptr=Polyhedron_ptr;
}

Polyhedron* Cylinder::get_Polyhedron_ptr()
{
   return Polyhedron_ptr;
}

const Polyhedron* Cylinder::get_Polyhedron_ptr() const
{
   return Polyhedron_ptr;
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Member function generate_drawable_geode() instantiates an OSG Geode

osg::Geode* Cylinder::generate_drawable_geode(
   double curr_size,bool text_screen_axis_alignment_flag)
{
//   cout << "inside Cylinder::generate_drawable_geode()" << endl;
   geode_refptr=new osg::Geode;
   fill_drawable_geode(geode_refptr.get(),curr_size,
                       text_screen_axis_alignment_flag);
   return geode_refptr.get();
}

void Cylinder::fill_drawable_geode(
   osg::Geode* geode_ptr,double curr_size,
   bool text_screen_axis_alignment_flag)
{
//   cout << "inside Cylinder::fill_drawable_geode()" << endl;

   geode_ptr->addDrawable(generate_drawable(curr_size));
   for (unsigned int i=0; i<get_n_text_messages(); i++)
   {
      initialize_text(i,curr_size,text_screen_axis_alignment_flag);
      geode_ptr->addDrawable(text_refptr[i].get());
   }
}

// ---------------------------------------------------------------------
// Member function generate_drawable

osg::Drawable* Cylinder::generate_drawable(double curr_size)
{

// We translate the cylinder so that its base lies at its initial
// center:

   osg::Vec3 modified_center(
      center.get(0),center.get(1),center.get(2)+0.5*height*curr_size);
   Cylinder_refptr=new osg::Cylinder(
      modified_center,radius*curr_size,height*curr_size);
   Cylinder_refptr->setRotation(quaternion);
   shape_refptr = new osg::ShapeDrawable(Cylinder_refptr.get());

   osg::StateSet* stateset_ptr=new osg::StateSet;
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);

// Helpful hint from Ross on 2/28/08:  setRenderingHint draws cylinder last 
// into depth buffer:

   stateset_ptr->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

   stateset_ptr->setMode(GL_DEPTH_WRITEMASK,osg::StateAttribute::OFF);
   stateset_ptr->setAttributeAndModes(
      new osg::Depth(osg::Depth::LESS,0.0, 1.0, false),
      osg::StateAttribute::ON);

   shape_refptr->setStateSet(stateset_ptr);
   return shape_refptr.get();
}

// ---------------------------------------------------------------------
// Member function initialize_text

void Cylinder::initialize_text(
   int i,double curr_size,bool text_screen_axis_alignment_flag)
{
//   cout << "inside Cylinder::init_text, text_scrn_axis_align_flag = "
//        << text_screen_axis_alignment_flag << endl;

   Geometrical::initialize_text(i);
   osg::Vec3 init_text_posn(
      text_displacement.get(0)*curr_size,
      text_displacement.get(1)*curr_size,
      text_displacement.get(2)*curr_size);
   set_text_posn(0,center+threevector(quaternion*init_text_posn));

// If text_screen_axis_alignment_flag==true, cylinder text is always
// oriented horizontally wrt the screen independent of the camera's
// attitude:

   if (text_screen_axis_alignment_flag)
   {
      text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);
   }
   else
   {
      text_refptr[i]->setAxisAlignment(osgText::Text::XY_PLANE);
   }
}

// ---------------------------------------------------------------------
// Member function set_color sets the color of the current object plus
// its attendant text label based upon the input RGBA information.

void Cylinder::set_color(const osg::Vec4& color)
{
//   cout << "inside Cylinder::set_color()" << endl;
//   cout << "shape_refptr.valid() = " << shape_refptr.valid() << endl;

   shape_refptr->setColor(color);

//   cout << "text_refptr.size() = " << text_refptr.size() << endl;
//   if (text_refptr.size() > 0 && text_refptr[0] != NULL) 
   if (text_refptr.size() > 0)
   {
//      if (text_refptr[0].valid()) set_text_color(0,get_text_color());
      if (text_refptr[0].valid())
      {
         set_permanent_text_color(color);
         set_text_color(get_permanent_text_color());
      }
   }

//   cout << "at end of Cylinder::set_color()" << endl;
}
