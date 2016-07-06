// ==========================================================================
// Plane class member function definitions
// ==========================================================================
// Last updated on 1/4/07; 1/21/07; 10/13/07; 6/15/08
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/Plane.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Plane::allocate_member_objects()
{
   n_vertices=4;
   vertices_refptr = new osg::Vec3Array(n_vertices);
   color_array_refptr = new osg::Vec4Array(n_vertices);
   geom_refptr=new osg::Geometry;
}		       

void Plane::initialize_member_objects()
{
   Graphical_name="Plane";
   
// We initially assume normal points in +z_hat direction.  But this
// choice may later need to be changed to the -z_hat direction:

   ncanonical_hat=threevector(0,0,1);	
   n_segment_ptr=NULL;

   n_text_messages=1;
   for (unsigned int m=0; m<get_n_text_messages(); m++)
   {
      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   }
}		       

Plane::Plane(const plane& p_in,int id,AnimationController* AC_ptr):
   Geometrical(3,id,AC_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   p=p_in;
}		       

Plane::~Plane()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Plane& p_in)
{
   outstream << "inside Plane::operator<<" << endl;
//   outstream << static_cast<const Geometrical&>(l) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing an OpenGL QUAD drawable.

osg::Geode* Plane::generate_drawable_geode()
{
   geode_refptr = new osg::Geode();

   generate_drawable_geom();
   geode_refptr->addDrawable(geom_refptr.get());

   set_ID_text();
   geode_refptr->addDrawable(text_refptr[0].get());

   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom fills member object
// *geom_refptr with an OpenGL QUAD drawable.

osg::Geometry* Plane::generate_drawable_geom()
{
   cout << "inside Plane::generate_drawable_geom()" << endl;
   
   geom_refptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
   geom_refptr->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,n_vertices));

// On 9/19/05, we found that the following StateSet commands enable
// alpha blending for rectangles appearing on top of video imagery...

   osg::StateSet* stateset_ptr=new osg::StateSet;
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   geom_refptr->setStateSet(stateset_ptr);

   set_canonical_vertices();

   set_permanent_color(colorfunc::white);
   set_curr_color(colorfunc::white,0.35);
   return geom_refptr.get();
}

// ---------------------------------------------------------------------
// Member function set_canonical_vertices sets the plane's 4 vertices
// equal to the unit square corners.  It then sets the osg::Vec3's in
// member *vertices_refptr equal to these vertex locations.

void Plane::set_canonical_vertices()
{
   cout << "inside Plane::set_canonical_vertices()" << endl;

   (*vertices_refptr)[0] = osg::Vec3f(-0.5,-0.5,0);
   (*vertices_refptr)[1] = osg::Vec3f(0.5,-0.5,0);
   (*vertices_refptr)[2] = osg::Vec3f(0.5,0.5,0);
   (*vertices_refptr)[3] = osg::Vec3f(-0.5,0.5,0);
   geom_refptr->setVertexArray(vertices_refptr.get());
}

// ---------------------------------------------------------------------
// Member function set_ID_text sets the text label associated with the
// current plane object equal to its ID number.

void Plane::set_ID_text()
{
   text_refptr[0]=new osgText::Text;
   text_refptr[0]->setFont("fonts/times.ttf");
   
   float char_size=0.2;
   text_refptr[0]->setCharacterSize(char_size);
   string label=stringfunc::number_to_string(get_ID());
   text_refptr[0]->setText(label);

   text_refptr[0]->setAxisAlignment(osgText::Text::SCREEN);

   osg::Vec3 text_position(0.40, 0.45,0);
   text_refptr[0]->setPosition(text_position);

   osg::Vec4 text_color=colorfunc::get_OSG_color(colorfunc::grey);
   text_refptr[0]->setColor(text_color);
}

// ---------------------------------------------------------------------
// Member function set_scale_attitude_posn takes in a time and pass
// number.  It scales, rotates and translates a canonical unit plane.
// This transformation information is stored for later callback
// retrieval.

void Plane::set_scale_attitude_posn(
   double curr_t,int pass_number,const plane& p_in)
{
   double scale=1;
   set_scale_attitude_posn(curr_t,pass_number,p_in,scale);
}

void Plane::set_scale_attitude_posn(
   double curr_t,int pass_number,const plane& p_in,double scale)
{
   cout << "inside Plane::set_scale_attitude_posn()" << endl;
   p=p_in;

   set_scale(curr_t,pass_number,threevector(scale,scale,scale));

// First compute quaternion which maps Ncanonical (i.e. Z_hat) into
// n_hat:

   osg::Vec3f Ncanonical(ncanonical_hat.get(0),ncanonical_hat.get(1),
                         ncanonical_hat.get(2));
   osg::Vec3f Nhat(p.get_nhat().get(0),p.get_nhat().get(1),
                   p.get_nhat().get(2));
   osg::Quat q;
   q.makeRotate(Ncanonical,Nhat);

   osg::Vec3f Xhat(1,0,0);
   osg::Vec3f Yhat(0,1,0);
   osg::Vec3f Zhat(0,0,1);
   osg::Vec3f rot_Xhat=q*Xhat;
   osg::Vec3f rot_Yhat=q*Yhat;
   osg::Vec3f rot_Zhat=q*Zhat;
   threevector rot_xhat(rot_Xhat.x(),rot_Xhat.y(),rot_Xhat.z());
   threevector rot_yhat(rot_Yhat.x(),rot_Yhat.y(),rot_Yhat.z());
   threevector rot_zhat(rot_Zhat.x(),rot_Zhat.y(),rot_Zhat.z());
//   cout << "rot_xhat = " << rot_xhat << endl;
//   cout << "rot_yhat = " << rot_yhat << endl;
//   cout << "rot_zhat = " << rot_zhat << endl;

// Next compute quaternion whichi spins rot_xhat about n_hat so that
// it matches a_hat:

   threevector a_hat(p.get_ahat());
   threevector b_hat(p.get_bhat());
   threevector n_hat(p.get_nhat());
   double theta=atan2(rot_xhat.dot(b_hat),rot_xhat.dot(a_hat));

   osg::Quat q_azimuthal;
   q_azimuthal.makeRotate(-theta,Nhat);

// On 3/13/06, we empirically discovered that quaternion
// multiplication appears to be implemented within OSG in LEFT HAND
// ORDER.  So the rotation which maps N_canonical to n_hat followed by
// the rotation which spins a_hat about the n_hat axis is implemented
// in OSG as q*q_azimuthal rather than by the conventional ordering
// q_azimuthal*q:

//   osg::Quat q_total=q_azimuthal*q;
   osg::Quat q_total=q*q_azimuthal;

   osg::Vec3f rot_rot_Xhat=q_total*Xhat;
   osg::Vec3f rot_rot_Yhat=q_total*Yhat;
   osg::Vec3f rot_rot_Zhat=q_total*Zhat;
//   osg::Vec3f rot_rot_Xhat=q_azimuthal*rot_Xhat;
//   osg::Vec3f rot_rot_Yhat=q_azimuthal*rot_Yhat;
//   osg::Vec3f rot_rot_Zhat=q_azimuthal*rot_Zhat;
   threevector rot_rot_xhat(rot_rot_Xhat.x(),rot_rot_Xhat.y(),
                            rot_rot_Xhat.z());
   threevector rot_rot_yhat(rot_rot_Yhat.x(),rot_rot_Yhat.y(),
                            rot_rot_Yhat.z());
   threevector rot_rot_zhat(rot_rot_Zhat.x(),rot_rot_Zhat.y(),
                            rot_rot_Zhat.z());
//   cout << "rot_rot_xhat = " << rot_rot_xhat << endl;
//   cout << "rot_rot_yhat = " << rot_rot_yhat << endl;
//   cout << "rot_rot_zhat = " << rot_rot_zhat << endl;

//   cout << "a_hat = " << a_hat << endl;
//   cout << "b_hat = " << b_hat << endl;
//   cout << "n_hat = " << n_hat << endl;

   set_quaternion(curr_t,pass_number,q_total);

   threevector UVW(p.get_origin());

   set_UVW_coords(curr_t,pass_number,UVW);
//   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
// Member function generate_canonical_normal_segment instantiates
// member *n_segment_ptr as a segment from (0,0,0) to ncanonical_hat.

void Plane::generate_canonical_normal_segment()
{
   bool draw_arrow_flag=true;
   int n_ID=2;
   threevector origin(0,0,0);

   n_segment_ptr=new LineSegment(
      3,origin,ncanonical_hat,n_ID,draw_arrow_flag,AnimationController_ptr);
}
