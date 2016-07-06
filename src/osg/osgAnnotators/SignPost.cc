// ==========================================================================
// SignPost class member function definitions
// ==========================================================================
// Last updated on 1/7/08; 1/20/09; 1/30/11; 10/14/11
// ==========================================================================

#include <iostream>
#include <osgText/Font>
#include <osg/Geode>
#include <osg/Node>
#include <string>
#include "color/colorfuncs.h"
#include "osg/osgAnnotators/SignPost.h"
#include "general/stringfuncs.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SignPost::allocate_member_objects()
{
}		       

void SignPost::initialize_member_objects()
{
   Graphical_name="SignPost";
   set_size(1.0);
   n_text_messages=1;
   label="SignPost Label";
   category="";
   SKS_worldmodel_database_flag=false;

//   set_selected_color(colorfunc::yellow);
   set_selected_color(colorfunc::red);
//   set_permanent_color(colorfunc::red);
   set_permanent_color(colorfunc::white);
}		       

SignPost::SignPost(int id,int ndims):
   Annotator(ndims,id), Geometrical(ndims,id)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

SignPost::~SignPost()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const SignPost& s)
{
   outstream << "inside SignPost::operator<<" << endl;
   outstream << "SignPost label = " << s.get_label() << endl;
   outstream << static_cast<const Geometrical&>(s) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing cylinder, cone and text drawables.

osg::Geode* SignPost::generate_drawable_geode(
   double curr_size,double height_multiplier)
{
//   cout << "inside SignPost::generate_drawable_geode()" << endl;
//   cout << "height_multiplier = " << height_multiplier << endl;
//   cout << "curr_size = " << curr_size << endl;

   float cone_radius=3*curr_size;
   cone_height=3*cone_radius;

// Recall volume of regular symmetric cone = 1/3 Pi radius**2 height
// and its center of mass is located at 1/4 height.  We translate the
// cone so that its tip point lies at its initial center:

   osg::Vec3 init_center(0,0,-0.25*cone_height+cone_height);
   Cone_ptr=new osg::Cone(init_center,cone_radius,-cone_height);
   cone_shape_refptr = new osg::ShapeDrawable(Cone_ptr);

   osg::TessellationHints* hints = new osg::TessellationHints;
   hints->setDetailRatio(0.5f);

   float cylinder_radius=0.25*cone_radius;
   cylinder_height=6*height_multiplier*cone_height;
//   cout << "cylinder_height = " << cylinder_height << endl;
   cylinder_position=osg::Vec3(0,0,0.5*cylinder_height+cone_height);
   cylinder_shape_refptr = new osg::ShapeDrawable(
      new osg::Cylinder(cylinder_position, cylinder_radius,cylinder_height), 
      hints);

   text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   text_refptr[0]=new osgText::Text;
   text_refptr[0]->setFont("fonts/times.ttf");
   text_refptr[0]->setCharacterSize(15*curr_size);
//   text_refptr[0]->setAxisAlignment(osgText::Text::XZ_PLANE);
//   text_refptr[0]->setAxisAlignment(osgText::Text::YZ_PLANE);
//   text_refptr[0]->setAxisAlignment(osgText::Text::REVERSED_YZ_PLANE);
   text_refptr[0]->setAxisAlignment(osgText::Text::SCREEN);

   text_refptr[0]->setAlignment(osgText::Text::CENTER_CENTER);
   text_refptr[0]->setBackdropType(osgText::Text::OUTLINE);
//   text_refptr[0]->setBackdropImplementation(osgText::Text::STENCIL_BUFFER);

   geode_refptr = new osg::Geode();
   geode_refptr->addDrawable(cone_shape_refptr.get());
   geode_refptr->addDrawable(cylinder_shape_refptr.get());
   geode_refptr->addDrawable(text_refptr[0].get());

   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function set_color sets the color of the current object plus
// its attendant text label based upon the input RGBA information.

void SignPost::set_color(const colorfunc::Color& color)
{
   set_color(colorfunc::get_OSG_color(color));
}

void SignPost::set_color(const osg::Vec4& color)
{
//   cout << "inside SignPost::set_color, ID = " << get_ID() << endl;
//   cout << "color = " << endl;
//   osgfunc::print_Vec4(color);

   cone_shape_refptr->setColor(color);
   cylinder_shape_refptr->setColor(color);
   set_text_color(0,color);
}

// --------------------------------------------------------------------------
// Member function set_quasirandom_color() assigns a pure hue based upon 
// the SignPost's ID.

void SignPost::set_quasirandom_color()
{
//   cout << "inside SignPost::set_quasirandom_color() << endl;

   const double hue_start=0;
   const double delta_hue=120+33/3;

   double h=hue_start+get_ID()*delta_hue;
   h = basic_math::phase_to_canonical_interval(h,0,360);
   double s=1;
   double v=1;
   double r,g,b;
   colorfunc::hsv_to_RGB(h,s,v,r,g,b);
   osg::Vec4 signpost_color(r,g,b,1);
         
// FAKE FAKE:  Weds, March 9, 2011 at 8:18 am
// Hardwire colors for lighthawk viewgraph purposes...

   if (get_ID()==0)
   {
      signpost_color=colorfunc::get_OSG_color(colorfunc::cyan);
   }
   else if (get_ID()==1)
   {
      signpost_color=colorfunc::get_OSG_color(colorfunc::yellow);
   }
   else if (get_ID()==2)
   {
      signpost_color=colorfunc::get_OSG_color(colorfunc::green);
   }
   
   set_color(signpost_color);
   set_permanent_color(signpost_color);
   set_selected_color(colorfunc::white);
}

// ---------------------------------------------------------------------
// Member function set_label() positions input_label atop the
// SignPost.  Additional vertical white space separation between the
// top of the SignPost's cylinder and bottom of the input_label text
// can be specified by a non-zero value for extra_frac_cyl_height.

void SignPost::set_label(string input_label,double extra_frac_cyl_height)
{
//   cout << "inside SignPost::set_label()" << endl;
//   cout << "extra_frac_cyl_height = " << extra_frac_cyl_height << endl;
   label=input_label;
   osg::Vec3 text_position = cylinder_position + 
      osg::Vec3(0,0,cone_height+(0.5+extra_frac_cyl_height)*cylinder_height);
   set_text_posn(0,text_position);
   set_text_label(0,label);

//   cout << "text_position = " << threevector(text_position) << endl;
}

// ---------------------------------------------------------------------
// Member function set_max_text_width adjusts the maximum width of
// text box based upon the length of the input label.  It allows long
// text strings to wrap around into several smaller, more readable
// lines,

void SignPost::set_max_text_width(string input_label)
{
//   cout << "inside SignPost::set_max_text_width(), input_label.size() = " 
//        << input_label.size() << endl;

   double width_factor=input_label.size();
   if (input_label.size() > 10 && input_label.size() < 20)
   {
      width_factor=0.5*input_label.size();
   }
   else if (input_label.size() >= 20 && input_label.size() < 30)
   {
      width_factor=0.33*input_label.size();
   }
   else if (input_label.size() >= 30 && input_label.size() < 40)
   {
      width_factor=0.25*input_label.size();
   }
   else if (input_label.size() >= 40)
   {
      width_factor=0.125*input_label.size();
   }
   
   double max_width=width_factor*text_refptr[0]->getCharacterHeight();
//   cout << "max_width = " << max_width << endl;
   text_refptr[0]->setMaximumWidth(max_width);
}

// ==========================================================================
// SignPost manipulation methods
// ==========================================================================

// Member function set_scale_attitude_posn takes in a time and pass
// number along with threevectors V1 and V2.  It scales, rotates and
// translates a canonical linesegment ranging from (0,0,0) to (1,0,0)
// so that it matches the segment ranging from V1 to V2.  This
// transformation information is stored for later callback retrieval.

void SignPost::set_attitude_posn(
   double curr_t,int pass_number,const threevector& V1,const threevector& V2)
{
   threevector r_hat=(V2-V1).unitvector();
   reset_attitude_posn(curr_t,pass_number,V1,r_hat);
}

void SignPost::reset_attitude_posn(
   double curr_t,int pass_number,const threevector& cone_tip,
   const threevector& cone_dir_hat)
{
//   cout << "inside SignPost::set_scale_attitude_posn()" << endl;

   osg::Vec3f Z_hat(0,0,1);
   osg::Quat q;
   if (get_ndims()==3)
   {
      q.makeRotate(Z_hat,osg::Vec3f(
         cone_dir_hat.get(0),cone_dir_hat.get(1),cone_dir_hat.get(2)));
   }
   set_quaternion(curr_t,pass_number,q);

   set_UVW_coords(curr_t,pass_number,cone_tip);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;

}

// ---------------------------------------------------------------------
void SignPost::reset_scale(
   double curr_t,int pass_number,double scale_factor)
{
//   cout << "inside SignPost::reset_scale" << endl;
//   cout << "scale_factor = " << scale_factor << endl;

   set_scale(curr_t,pass_number,
             threevector(scale_factor,scale_factor,scale_factor));
   set_text_size(0,15*scale_factor);
}

