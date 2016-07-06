// DANGER DANGER DANGER !!!

// Must move following line

//		    text_refptr[0]->setFont("fonts/times.ttf");

// out of LineSegment::generate_drawable_geode() and into Decorations
// class!

// ==========================================================================
// LineSegment class member function definitions
// ==========================================================================
// Last updated on 6/15/08; 10/8/09; 11/16/10
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/LineSegment.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LineSegment::allocate_member_objects()
{
   osg_linesegment_refptr=new osg::LineSegment();
   geom_refptr=new osg::Geometry;
}		       

void LineSegment::initialize_member_objects()
{
   Graphical_name="LineSegment";
   n_text_messages=1;
   endpoint_size_prefactor=1;
   for (unsigned int m=0; m<get_n_text_messages(); m++)
   {
      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   }
   draw_arrow_flag=draw_endpoint1_flag=draw_endpoint2_flag=false;

   set_permanent_color(colorfunc::white);
//   set_permanent_color(colorfunc::red);
   set_selected_color(colorfunc::green);
}		       

LineSegment::LineSegment(
   const int p_ndims,const threevector& V1,const threevector& V2,int id,
   bool draw_arrow,AnimationController* AC_ptr):
   Geometrical(p_ndims,id,AC_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   draw_arrow_flag=draw_arrow;

   reset_our_and_osg_linesegments(V1,V2);
}		       

LineSegment::LineSegment(
   const int p_ndims,const threevector& V1,const threevector& V2,int id,
   bool draw_endpoint1,bool draw_endpoint2,double endpoint_size_prefactor,
   AnimationController* AC_ptr):
   Geometrical(p_ndims,id,AC_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   draw_endpoint1_flag=draw_endpoint1;
   draw_endpoint2_flag=draw_endpoint2;
   set_endpoint_size_prefactor(endpoint_size_prefactor);

   reset_our_and_osg_linesegments(V1,V2);
}		       

LineSegment::~LineSegment()
{
//   cout << "inside LineSegment destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LineSegment& l)
{
   outstream << "inside LineSegment::operator<<" << endl;
   outstream << "V1 = " << l.get_V1() << endl;
   outstream << "V2 = " << l.get_V2() << endl;
   threevector UVW;
   l.get_UVW_coords(0,0,UVW);
   cout << "UVW = " << UVW << endl;
//   outstream << static_cast<const Geometrical&>(l) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing a linesegment drawable and a possible arrow tip.

osg::Geode* LineSegment::generate_drawable_geode()
{
   geode_refptr = new osg::Geode();
   geode_refptr->addDrawable(generate_drawable_geom());

   text_refptr[0]=new osgText::Text;
   text_refptr[0]->setFont("fonts/times.ttf");
   geode_refptr->addDrawable(text_refptr[0].get());

   reset_vertices();
   get_LineWidth_ptr()->setWidth(1.0);

   if (draw_arrow_flag)
   {
      geode_refptr->addDrawable(generate_arrow_tip_drawable(l.get_ehat()));
   }
   
   if (draw_endpoint1_flag)
   {
      geode_refptr->addDrawable(generate_endpoint(1));
   }

   if (draw_endpoint2_flag)
   {
      geode_refptr->addDrawable(generate_endpoint(2));
   }

   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates Graphical
// member object *geom_ptr which contains an OpenGL Line drawable.

osg::Geometry* LineSegment::generate_drawable_geom()
{
//   cout << "inside LineSegment::generate_drawable_geom()" << endl;
   
   const int n_vertices=2;
   vertices_refptr = new osg::Vec3Array(n_vertices);
   geom_refptr->setVertexArray(vertices_refptr.get());

   geom_refptr->addPrimitiveSet(new osg::DrawArrays(
      osg::PrimitiveSet::LINES,0,n_vertices));

   linewidth_refptr = new osg::LineWidth();
   osg::StateSet* stateset_ptr=geom_refptr->getOrCreateStateSet();
   stateset_ptr->setAttributeAndModes(
      linewidth_refptr.get(),osg::StateAttribute::ON);
//   stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
   geom_refptr->setStateSet(stateset_ptr);

   set_color(get_permanent_color());

   return geom_refptr.get();
}

// ---------------------------------------------------------------------
void LineSegment::set_color(const osg::Vec4& color)
{
//   cout << "inside LineSegment::set_color()" << endl;
//   cout << "this = " << this << endl;
   if (!color_array_refptr.valid())
   {
      color_array_refptr = new osg::Vec4Array(1);
   }
   color_array_refptr->at(0)=color;
   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_OVERALL);
}

// ---------------------------------------------------------------------
void LineSegment::set_draw_arrow_flag()
{
   draw_arrow_flag=true;
   geode_refptr->addDrawable(generate_arrow_tip_drawable(l.get_ehat()));
}

// ---------------------------------------------------------------------
// Member function reset_vertices extracts current endpoint vertex
// positions from member linesegment l.  It then sets the osg::Vec3's
// in member *vertices_refptr equal to these vertex locations.

void LineSegment::reset_vertices()
{
   if (get_ndims()==2)
   {
      vertices_refptr->at(0)=
         osg::Vec3f(get_V1().get(0),get_V1().get(2),get_V1().get(1));
      vertices_refptr->at(1)=
         osg::Vec3f(get_V2().get(0),get_V2().get(2),get_V2().get(1));
   }
   else if (get_ndims()==3)
   {
      vertices_refptr->at(0)=
         osg::Vec3f(get_V1().get(0),get_V1().get(1),get_V1().get(2));
      vertices_refptr->at(1)=
         osg::Vec3f(get_V2().get(0),get_V2().get(1),get_V2().get(2));
   }
}

// ---------------------------------------------------------------------
// Member function generate_arrow_tip_drawable generates an OSG cone,
// positions it at the end of input threevector n_hat, and aligns the
// cone's orientation with n_hat.  It returns an osg::ShapeDrawable*
// which should be added onto an osg::geode along with n_hat.
// Subsequent PAT rotations, translations and scales will then
// jointly affect both the arrow stem and tip.

osg::ShapeDrawable* LineSegment::generate_arrow_tip_drawable(
   const threevector& n_hat)
{
   osg::TessellationHints* hints = new osg::TessellationHints;
   hints->setDetailRatio(0.5f);

   float cone_radius=0.05;
   float cone_height=0.1;
   osg::Vec3 cone_position(n_hat.get(0),n_hat.get(1),n_hat.get(2));
   osg::Cone* cone_ptr=new osg::Cone(
      cone_position, cone_radius,cone_height);

   osg::Quat attitude(0,0,0,1);
   osg::Vec3 Zhat(0,0,1);
   attitude.makeRotate(Zhat,cone_position);

   cone_ptr->setRotation(attitude);
   osg::ShapeDrawable* cone_shape_ptr=new osg::ShapeDrawable(cone_ptr,hints);
   return cone_shape_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_endpoint returns a sphere located at
// linesegment l's V1 or V2 location depending upon input index i =
// 1,2.

osg::ShapeDrawable* LineSegment::generate_endpoint(int i)
{
   float sphere_radius=0.0003*endpoint_size_prefactor;
//   float sphere_radius=0.001;

   threevector V(l.get_v1());
   if (i==2)
   {
      V=l.get_v2();
   }
   osg::Sphere* sphere_ptr=
      new osg::Sphere(osg::Vec3(V.get(0),V.get(1),V.get(2)),sphere_radius);
   
   osg::TessellationHints* hints = new osg::TessellationHints;
   hints->setDetailRatio(0.5f);
   osg::ShapeDrawable* sphere_shape_ptr=
      new osg::ShapeDrawable(sphere_ptr,hints);
   sphere_shape_ptr->setColor(colorfunc::get_OSG_color(colorfunc::white));
   return sphere_shape_ptr;
}

// ==========================================================================
// LineSegment manipulation methods
// ==========================================================================

// Member function set_scale_attitude_posn takes in a time and pass
// number along with threevectors V1 and V2.  It scales, rotates and
// translates a canonical linesegment ranging from (0,0,0) to (1,0,0)
// so that it matches the segment ranging from V1 to V2.  This
// transformation information is stored for later callback retrieval.

void LineSegment::set_scale_attitude_posn(
   double curr_t,int pass_number,const threevector& V1,const threevector& V2)
{
//   cout << "inside LineSegment::set_scale_attitude_posn(), t= " 
//        << curr_t << endl;
   reset_our_and_osg_linesegments(V1,V2);

   double scale=l.get_length();
   if (draw_arrow_flag || draw_endpoint1_flag || draw_endpoint2_flag)
   {
      set_scale(curr_t,pass_number,threevector(scale,scale,scale));
   }
   else
   {
      set_scale(curr_t,pass_number,threevector(scale,1,1));
   }

   osg::Vec3f X_hat(1,0,0);
   threevector r_hat=l.get_ehat();
   osg::Quat q;
   if (get_ndims()==2)
   {
      q.makeRotate(X_hat,osg::Vec3f(r_hat.get(0),r_hat.get(2),r_hat.get(1)));
   }
   else if (get_ndims()==3)
   {
      q.makeRotate(X_hat,osg::Vec3f(r_hat.get(0),r_hat.get(1),r_hat.get(2)));
   }
   set_quaternion(curr_t,pass_number,q);

   set_UVW_coords(curr_t,pass_number,V1);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}

// ---------------------------------------------------------------------
// Member function recover_V1_and_V2 takes in time and pass_number
// inputs and retrieves corresponding LineSegment position, rotation
// and scaling information.  It returns the LineSegment's endpoints V1
// & V2.  (As of Feb 2007, this method will only work for 3D
// LineSegments and not for 2D LineSegments...)

void LineSegment::recover_V1_and_V2(
   double curr_t,int pass_number,threevector& V1,threevector& V2)
{
//   cout << "inside LS::recover_V1_and_V2(), t= " << curr_t << endl;

   get_UVW_coords(curr_t,pass_number,V1);

   double length=1;
   threevector s;
   if (get_scale(curr_t,pass_number,s))
   {
      length=s.get(0);
   }

   osg::Quat q;   
   osg::Vec3d r_hat(1,0,0);
   if (get_quaternion(curr_t,pass_number,q))
   {
      osg::Vec3d X_hat(1,0,0);
      r_hat=q*X_hat;
   }

   V2=V1+length*threevector(r_hat);

//   cout << "V1 = " << V1.get(0) << "," << V1.get(1) << ","
//        << V1.get(2) << endl;
//   cout << "V2 = " << V2.get(0) << "," << V2.get(1) << ","
//        << V2.get(2) << endl << endl;
}

// ---------------------------------------------------------------------
// Member function reset_osg_linesegment switches the starting and
// ending points of P. Cho's linesegment as well as the
// osg::LineSegment members of this LineSegment class.

void LineSegment::reset_our_and_osg_linesegments(
   const threevector& V1,const threevector& V2)
{
//   cout << "inside LineSegment::reset_our_and_osg_linesegments()" << endl;
   
   l=linesegment(V1,V2);   

   osg_linesegment_refptr->set(
      osg::Vec3(V1.get(0),V1.get(1),V1.get(2)),
      osg::Vec3(V2.get(0),V2.get(1),V2.get(2)));
}



