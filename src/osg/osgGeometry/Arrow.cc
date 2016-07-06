// ==========================================================================
// Arrow class member function definitions
// ==========================================================================
// Last updated on 9/9/09; 9/29/09; 11/16/10; 7/31/11
// ==========================================================================

#include <iostream>
#include <osgText/Font>
#include <osg/Geode>
#include <osg/Node>
#include <string>
#include "osg/osgGeometry/Arrow.h"
#include "color/colorfuncs.h"
#include "passes/Pass.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Arrow::allocate_member_objects()
{
   geom_refptr=new osg::Geometry;
   linewidth_refptr = new osg::LineWidth();
}		       

void Arrow::initialize_member_objects()
{
   Graphical_name="Arrow";
   set_size(1.0);
   n_text_messages=1;
   label="Arrow Label";

   ConesGroup_ptr=NULL;

/*
//   set_selected_color(colorfunc::yellow);
   set_selected_color(colorfunc::red);
//   set_permanent_color(colorfunc::red);
   set_permanent_color(colorfunc::white);
*/

}		       

Arrow::Arrow(int ndims,Pass* PI_ptr,threevector* GO_ptr,int ID):
   Geometrical(ndims,ID)
{	
   initialize_member_objects();
   allocate_member_objects();

   if (ndims==3)
   {
      ConesGroup_ptr=new ConesGroup(PI_ptr,GO_ptr);
   }
}		       

Arrow::~Arrow()
{
   delete ConesGroup_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Arrow& a)
{
   outstream << "inside Arrow::operator<<" << endl;
   outstream << "V_base = " << a.get_V_base()
             << " V_tip = " << a.get_V_tip() << endl;
   outstream << "Arrow label = " << a.get_label() << endl;
   outstream << static_cast<const Geometrical&>(a) << endl;
   return outstream;
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode() instantiates an osg::Geode
// containing cylinder, cone and text drawables.

osg::Geode* Arrow::generate_drawable_geode()
{
   geode_refptr = new osg::Geode();
   geode_refptr->addDrawable(generate_drawable_geom());

//   osg::ref_ptr<osgText::Text> curr_text_refptr=new osgText::Text;
//   text_refptr.push_back(curr_text_refptr);
//   initialize_text(i);
//   geode_ptr->addDrawable(text_refptr[i].get());

   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom() instantiates Geometrical
// member object *geom_refptr which contains an OpenGL LINE_STRIP
// drawable.

osg::Geometry* Arrow::generate_drawable_geom()
{
//   cout << "inside Arrow::Generate_drawable_geom()" << endl;

   int n_vertices=2;
   drawarrays_refptr=new osg::DrawArrays(
      osg::PrimitiveSet::LINE_STRIP,0,n_vertices);
   geom_refptr->addPrimitiveSet(drawarrays_refptr.get());
   return geom_refptr.get();
}

// ---------------------------------------------------------------------
void Arrow::set_linewidth(double width)
{
//   cout << "inside Arrow::set_linewidth(), width = " << width << endl;

   osg::StateSet* stateset_ptr=geom_refptr->getOrCreateStateSet();
   stateset_ptr->setAttributeAndModes(
      linewidth_refptr.get(),osg::StateAttribute::ON);
   linewidth_refptr->setWidth(width);
}

// ---------------------------------------------------------------------
// Member function set_magnitude_direction_and_base()

void Arrow::set_magnitude_direction_and_base(
   double magnitude,const threevector& e_hat,const threevector& base,
   double arrowhead_size_prefactor)
{
//   cout << "inside Arrow:set_magnitude_dir_and_base()" << endl;
//   cout << "mag = " << magnitude
//        << " e_hat = " << e_hat 
//        << " center = " << abs_center << endl;

   const double TINY=0.001;
   if (magnitude < TINY) 
   {
      return;
   }
   
   V_base=base;
   V_tip=V_base+magnitude*e_hat;
   set_relative_vertices();
   generate_arrow_head(arrowhead_size_prefactor);
}

// ---------------------------------------------------------------------
// Member function set_magnitude_direction_and_center()

void Arrow::set_magnitude_direction_and_center(
   double magnitude,const threevector& e_hat,const threevector& abs_center,
   double arrowhead_size_prefactor)
{
//   cout << "inside Arrow:set_magnitude_dir_and_center()" << endl;
//   cout << "mag = " << magnitude
//        << " e_hat = " << e_hat 
//        << " center = " << abs_center << endl;

   const double TINY=0.001;
   if (magnitude < TINY) 
   {
      return;
   }
   
   V_base=abs_center-0.5*magnitude*e_hat;
   V_tip=abs_center+0.5*magnitude*e_hat;
   set_relative_vertices();
   generate_arrow_head(arrowhead_size_prefactor);
}

// ---------------------------------------------------------------------
// Member function set_relative_vertices() fills *vertices_refptr with
// relative vertex information with respect to reference_origin.  In
// November 2006, Ross Anderson observed that floating point errors in
// very large absolute vertex positions can lead to noticeable
// polyline flickering.  To avoid roundoff problems, we simply need to
// incorporate the global reference_origin translation within the
// PolyLine's PAT.

void Arrow::set_relative_vertices()
{
//   cout << "inside Arrow::set_relative_vertices()" << endl;

   double magnitude=(V_tip-V_base).magnitude();
   threevector e_hat=(V_tip-V_base).unitvector();

// First either clear & reserve or dynamically allocate
// vertices_refptr.get():

   if (vertices_refptr.valid())
   {
      vertices_refptr->clear();
   }
   else
   {
      vertices_refptr = new osg::Vec3Array;
   }
   geom_refptr->setVertexArray(vertices_refptr.get());

   vertices_refptr->reserve(2);
   for (int n=0; n<2; n++)
   {
      threevector relative_V(V_base-reference_origin);
      if (n==1)
      {
         relative_V=V_base-reference_origin+0.99*magnitude*e_hat;
      }
      
      if (get_ndims()==2)
      {
         vertices_refptr->push_back(
            osg::Vec3f(relative_V.get(0),relative_V.get(2),
                       relative_V.get(1)));
      }
      else if (get_ndims()==3)
      {
         vertices_refptr->push_back(
            osg::Vec3f(relative_V.get(0),relative_V.get(1),
                       relative_V.get(2)));
      }

//      cout << "n = " << n
//           << " vertex = " 
//           << vertices_refptr->back().x() << " , "
//           << vertices_refptr->back().y() << " , "
//           << vertices_refptr->back().z() << endl;

   } // loop over index n labeling vertices
}

// ---------------------------------------------------------------------
// Member function generate_arrow_head()

void Arrow::generate_arrow_head(double arrowhead_size_prefactor)
{
//   cout << "inside Arrow::generate_arrow_head()" << endl;
//   cout << "arrowhead_size_prefactor = " 
//        << arrowhead_size_prefactor << endl;
   
//   cout << "reference_origin = " << reference_origin << endl;

   if (ConesGroup_ptr==NULL) return;

   double arrow_length=(V_tip-V_base).magnitude();
   double radius=arrowhead_size_prefactor*0.025*arrow_length;
   threevector arrow_tip=V_tip-reference_origin;
   threevector arrow_dir=(V_tip-V_base).unitvector();
   threevector arrow_base=arrow_tip-0.05*arrow_length*arrow_dir;

//   cout << "V_tip = " << V_tip << " reference_origin = "
//        << reference_origin << endl;
//   cout << "arrow_length = " << arrow_length << " radius = " << radius
//        << endl;
//   cout << "arrow_base = " << arrow_base
//        << " arrow_tip = " << arrow_tip << endl;

   Cone* curr_Cone_ptr=ConesGroup_ptr->get_Cone_ptr(0);
   ConesGroup_ptr->destroy_Graphical(curr_Cone_ptr);

   ConesGroup_ptr->set_rh(radius,radius);
   curr_Cone_ptr=ConesGroup_ptr->generate_new_Cone(
      arrow_tip,arrow_base,radius);
   curr_Cone_ptr->set_reference_origin(reference_origin);
//   cout << "*curr_Cone_ptr = " << *curr_Cone_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function set_color sets the color of the current object plus
// its attendant text label based upon the input RGBA information.

void Arrow::set_color(const colorfunc::Color& c)
{
   set_color(colorfunc::get_OSG_color(c));
}

void Arrow::set_color(const osg::Vec4& color)
{
//   cout << "inside Arrow::set_color, ID = " << get_ID() << endl;
//   cout << "color = " << endl;
//   osgfunc::print_Vec4(color);

   if (!color_array_refptr.valid())
   {
      color_array_refptr = new osg::Vec4Array(1);
   }
   color_array_refptr->at(0)=color;
   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_OVERALL);

   Cone* Cone_ptr=ConesGroup_ptr->get_Cone_ptr(0);
   if (Cone_ptr != NULL)
   {
      Cone_ptr->set_permanent_color(color);
      Cone_ptr->set_color(color);
      ConesGroup_ptr->reset_colors();
   }

//   set_text_color(0,color);
}

// ---------------------------------------------------------------------
// Member function set_label() positions input_label atop the
// Arrow.  Additional vertical white space separation between the
// top of the Arrow's cylinder and bottom of the input_label text
// can be specified by a non-zero value for extra_frac_cyl_height.

void Arrow::set_label(string input_label,double extra_frac_cyl_height)
{
/*

//   cout << "inside Arrow::set_label()" << endl;
//   cout << "extra_frac_cyl_height = " << extra_frac_cyl_height << endl;
   label=input_label;
   osg::Vec3 text_position = cylinder_position + 
      osg::Vec3(0,0,cone_height+(0.5+extra_frac_cyl_height)*cylinder_height);
   set_text_posn(0,text_position);
   set_text_label(0,label);

//   cout << "text_position = " << threevector(text_position) << endl;

*/

}

// ---------------------------------------------------------------------
// Member function set_max_text_width adjusts the maximum width of
// text box based upon the length of the input label.  It allows long
// text strings to wrap around into several smaller, more readable
// lines,

void Arrow::set_max_text_width(string input_label)
{
//   cout << "inside Arrow::set_max_text_width(), input_label.size() = " 
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
// Arrow manipulation methods
// ==========================================================================

// Member function set_scale_attitude_posn takes in a time and pass
// number along with threevectors V1 and V2.  It scales, rotates and
// translates a canonical linesegment ranging from (0,0,0) to (1,0,0)
// so that it matches the segment ranging from V1 to V2.  This
// transformation information is stored for later callback retrieval.

void Arrow::set_attitude_posn(
   double curr_t,int pass_number,const threevector& V1,const threevector& V2)
{
   threevector r_hat=(V2-V1).unitvector();
   reset_attitude_posn(curr_t,pass_number,V1,r_hat);
}

void Arrow::reset_attitude_posn(
   double curr_t,int pass_number,const threevector& cone_tip,
   const threevector& cone_dir_hat)
{
//   cout << "inside Arrow::reset_attitude_posn()" << endl;
//   cout << "cone_dir_hat = " << cone_dir_hat << endl;

   osg::Vec3f Z_hat(0,0,1);
   osg::Quat q;
   if (get_ndims()==3)
   {
      q.makeRotate(Z_hat,osg::Vec3f(
         cone_dir_hat.get(0),cone_dir_hat.get(1),cone_dir_hat.get(2)));
   }
   set_quaternion(curr_t,pass_number,q);

   set_UVW_coords(curr_t,pass_number,cone_tip);
   
   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}

// ---------------------------------------------------------------------
void Arrow::reset_scale(
   double curr_t,int pass_number,double scale_factor)
{
//   cout << "inside Arrow::reset_scale" << endl;
//   cout << "scale_factor = " << scale_factor << endl;

   set_scale(curr_t,pass_number,
             threevector(scale_factor,scale_factor,scale_factor));
   set_text_size(0,15*scale_factor);
   outputfunc::enter_continue_char();
}
