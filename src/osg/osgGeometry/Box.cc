// ==========================================================================
// Box class member function definitions
// ==========================================================================
// Last updated on 10/21/07; 3/31/11; 12/2/11
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Group>
#include "osg/osgGeometry/Box.h"
#include "color/colorfuncs.h"
#include "geometry/polygon.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Box::allocate_member_objects()
{
//    material_refptr=new osg::Material;
//   selected_face_ptr=new Rectangle(0,3);
}		       

void Box::initialize_member_objects()
{
   Graphical_name="Box";
   selected_face_number=-2;	// none

   set_size(1.0);
//   const int n_text_labels=1;
//   const int n_text_labels=2;
//   const int n_text_labels=5;
   n_text_messages=6;
   for (unsigned int m=0; m<get_n_text_messages(); m++)
   {
      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   }
}		       

Box::Box(double w,double l,double h,int id):
   Geometrical(3,id)
{	
   b=mybox(w,l,h);
   allocate_member_objects();
   initialize_member_objects();
}		       

Box::Box(double w,double l,double h,double d,int id):
   Geometrical(3,id)
{	
   b=mybox(w,l,h);
   selected_face_displacement=d;
   allocate_member_objects();
   initialize_member_objects();
}		       

Box::~Box()
{
//   cout << "inside Box destructor" << endl;
//   delete selected_face_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Box& box)
{
//   outstream << "inside Box::operator<<" << endl;
   mybox b=box.b;
   outstream << "box.b = " << b << endl;
   outstream << static_cast<const Geometrical&>(box) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_group instantiates an OSG Group
// containing an OSG Box shape and a selected face rectangle.

osg::Group* Box::generate_drawable_group()
{
   group_refptr=new osg::Group;

   osg::Geode* box_geode_ptr=new osg::Geode;
   box_geode_ptr->addDrawable(generate_drawable());

   for (unsigned int i=0; i<get_n_text_messages(); i++)
   {
      box_geode_ptr->addDrawable(generate_text(i));
   }
   group_refptr->addChild(box_geode_ptr);

//   group_refptr->addChild(selected_face_ptr->generate_drawable_geode());
   return group_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_text()

osgText::Text* Box::generate_text(int i)
{
   text_refptr[i]=new osgText::Text;
   text_refptr[i]->setFont("fonts/times.ttf");

//   float char_size=0.7*size[get_ndims()];
//   float char_size=0.85*size[get_ndims()];
//   float char_size=size[get_ndims()];
   float char_size=2*size[get_ndims()];
   text_refptr[i]->setCharacterSize(char_size);
   text_refptr[i]->setAlignment(osgText::Text::CENTER_CENTER);

// Set maximum width of text box to force long text strings to wrap
// around into several smaller, more readable lines:

//   text_refptr[i]->setMaximumWidth(10*char_size);
//   text_refptr[i]->setMaximumWidth(20*char_size);
//   text_refptr[i]->setMaximumWidth(25*char_size);

   double frac=1.0-double(i+1)/double(get_n_text_messages()+1);
   double text_y=(frac-0.5)*b.get_length();

//   cout << "i = " << i << " frac = " << frac 
//        << " text_y = " << text_y << endl;
//   cout << "b.get_length() = " << b.get_length() << endl;

   osg::Vec3 text_position(0 , text_y , 2*b.get_height() );
   text_refptr[i]->setPosition(text_position);
   text_refptr[i]->setColor(colorfunc::get_OSG_color(colorfunc::black));

   return text_refptr[i].get();
}

// ---------------------------------------------------------------------
void Box::reset_text_font_and_size(int i,double charsize_factor)
{
   text_refptr[i]->setFont();
   text_refptr[i]->setCharacterSize(
      text_refptr[i]->getCharacterHeight()*charsize_factor);
}

// ---------------------------------------------------------------------
// Member function generate_drawable

osg::Drawable* Box::generate_drawable()
{
   osg::Vec3 init_position(0,0,0);
   Box_refptr=new osg::Box(init_position,b.get_width(),b.get_length(),
                           b.get_height());
   shape_refptr = new osg::ShapeDrawable(Box_refptr.get());

   osg::StateSet* stateset_ptr=new osg::StateSet;
   shape_refptr->setStateSet(stateset_ptr);

// Enable alpha blending:

   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   osg::BlendFunc* fn = new osg::BlendFunc();
   fn->setFunction(osg::BlendFunc::SRC_ALPHA, 
                   osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
   stateset_ptr->setAttributeAndModes(fn, osg::StateAttribute::ON);
//    stateset_ptr->setAttribute(material_refptr.get(), osg::StateAttribute::OFF);

   return shape_refptr.get();
}

// ---------------------------------------------------------------------
void Box::set_color(const osg::Vec4& color)
{
//   cout << "inside Box::set_color(), r = " << color.r() << " g = " << color.g()
//        << " b = " << color.b() << " a = " << color.a() << endl;

   shape_refptr->setColor(color);
}

// ---------------------------------------------------------------------
// Member function reset_selected_face_drawable generates a
// rectangular OSG geometry corresponding to input box face f.  The
// drawable is radially displaced outward from the box's side by 1% so
// that its rendering does not interfere with that of the box's actual
// side.

void Box::reset_selected_face_drawable(int f,const threevector& Box_posn)
{
   return;

   if (f >=-1 && f <= 4) 
   {
      selected_face_number=f;	 

      polygon selected_polygon;
      if (f==-1)
      {
         selected_polygon=b.get_bottomface();
      }
      else if (f==4)
      {
         selected_polygon=b.get_topface();
      }
      else
      {
         selected_polygon=b.get_sideface(f);
      
      }
      threevector nhat(selected_polygon.get_normal().unitvector());
      threevector ds(selected_face_displacement*nhat);

      threevector V[4];
      for (int i=0; i<4; i++)
      {
         V[i]=threevector(selected_polygon.get_vertex(i).get(0)+ds.get(0),
                          selected_polygon.get_vertex(i).get(1)+ds.get(1),
                          selected_polygon.get_vertex(i).get(2)+ds.get(2));
         V[i] -= Box_posn;
      }

      selected_face_ptr->set_world_vertices(V[0],V[1],V[2],V[3]);
//      selected_face_ptr->set_curr_color(colorfunc::red);
   }
}

// ==========================================================================
// Box manipulation methods
// ==========================================================================

// Member function set_posn takes in a time and pass number along with
// threevector V.  It translates a canonical Box so that it's
// centered on V.  This translation information is stored for later
// callback retrieval.

void Box::set_posn(double curr_t,int pass_number,const threevector& V)
{
//   cout << "inside Box::set_posn()" << endl;
//   cout << "V1 = " << V1 << " V2 = " << V2 << endl;

   set_UVW_coords(curr_t,pass_number,V);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}
