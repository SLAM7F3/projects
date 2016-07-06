// ==========================================================================
// Cone class member function definitions
// ==========================================================================
// Last updated on 1/24/07; 5/6/07; 5/7/07; 6/27/08; 10/27/08
// ==========================================================================

#include <iostream>
#include <osg/BlendFunc>
#include <osg/Geode>
#include "osg/osgGeometry/Cone.h"
#include "color/colorfuncs.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Cone::allocate_member_objects()
{
   material_refptr=new osg::Material;
}		       

void Cone::initialize_member_objects()
{
   Graphical_name="Cone";
   phi=theta=0;
   scale=threevector(1,1,1);
}		       

Cone::Cone(double radius,double height,int id):
   Geometrical(3,id)
{	
   this->radius=radius;
   this->height=height;
   tip=threevector(0,0,height);
   base=threevector(0,0,0);
   allocate_member_objects();
   initialize_member_objects();
}		       

Cone::~Cone()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Cone& c)
{
   outstream << "inside Cone::operator<<" << endl;
   outstream << "radius = " << c.radius << endl;
   outstream << "height = " << c.height << endl;
   outstream << "base = " << c.base << endl;
   outstream << "tip = " << c.tip << endl;
//   outstream << static_cast<const Geometrical&>(c) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_group instantiates an OSG Group
// containing an OSG Cone shape and a selected face rectangle.

osg::Geode* Cone::generate_drawable_geode()
{
   geode_refptr=new osg::Geode;
   geode_refptr->addDrawable(generate_drawable());
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_drawable

osg::Drawable* Cone::generate_drawable()
{

// Recall volume of regular symmetric cone = 1/3 Pi radius**2 height
// and its center of mass is located at 1/4 height.  We translate the
// cone so that its tip point lies at its initial center:

   osg::Vec3 init_center(0,0,0.25*height-height);
   Cone_refptr=new osg::Cone(init_center,radius,height);
   shape_refptr = new osg::ShapeDrawable(Cone_refptr.get());

   tip=threevector(0,0,0);
   base=threevector(0,0,-height);

   osg::StateSet* stateset_ptr=new osg::StateSet;
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   shape_refptr->setStateSet(stateset_ptr);

//   double alpha=0.3;
   double alpha=1.0;
   set_color(colorfunc::get_OSG_color(colorfunc::grey,alpha));

/*
// Enable alpha blending:

   osg::BlendFunc *fn = new osg::BlendFunc();
   fn->setFunction(osg::BlendFunc::SRC_ALPHA, 
                   osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
   stateset_ptr->setAttributeAndModes(fn, osg::StateAttribute::ON);
*/

//   stateset_ptr->setAttribute(material_refptr.get(), osg::StateAttribute::OFF);

   return shape_refptr.get();
}

// ---------------------------------------------------------------------
// Member function set_color

void Cone::set_color(const osg::Vec4& color)
{
   shape_refptr->setColor(color);
}

// ---------------------------------------------------------------------
void Cone::reset_state(osg::Vec4& c)
{
//   cout << "inside Cone::reset_state, r = " << c.r() << " g = " << c.g()
//        << " b = " << c.b() << " a = " << c.a() << endl;
//   material_refptr->setColorMode(osg::Material::SPECULAR);

//   material_refptr->setSpecular(osg::Material::FRONT,osg::Vec4(0.8,0.8,0.8,0.9));

//   material_refptr->setColorMode(osg::Material::EMISSION);
//   material_refptr->setEmission(osg::Material::FRONT,osg::Vec4(0.2,0.4,0.6,0.9));

//   material_refptr->setColorMode(osg::Material::AMBIENT);
//   material_refptr->setColorMode(osg::Material::DIFFUSE);
//   material_refptr->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
//   material_refptr->setAmbient(osg::Material::FRONT,c);

//   material_refptr->setDiffuse(osg::Material::FRONT_AND_BACK,c);



    //   material_refptr->setAmbient(osg::Material::FRONT_AND_BACK,c);

    //   dirtyDisplay();
}

// ==========================================================================
// Manipulation methods
// ==========================================================================

// Method scale_rotate_and_then_translate_cone works with a canonical
// cone with radius=1, height=1, tip location = (0,0,0) and symmetry
// direction = +z_hat.  It first flips the cone so that its direction
// vector points towards -z_hat.  It next rotates the cone so that its
// symmetry direction points [in standard polar coordinates] along
// (sin_theta cos_phi) x_hat + (sin_theta sin_phi) y_hat + cos_theta
// z_hat.  It also rescales the cone's size according to input
// threevector and translates the cone so that its tip lies at input
// threevector trans.

void Cone::scale_rotate_and_then_translate(
   double curr_t,int pass_number,const threevector& trans)
{   
   scale_rotate_and_then_translate(curr_t,pass_number,theta,phi,scale,trans);
}

void Cone::scale_rotate_and_then_translate(
   double curr_t,int pass_number,
   double theta,double phi,const threevector& scale,const threevector& trans)
{   
//   cout << "inside Cone::scale_rotate_and_then_translate()" << endl;
//   cout << "Scale = " << scale << endl;
//   cout << "theta = " << theta*180/PI << endl;
//   cout << "phi = " << phi*180/PI << endl;
//   cout << "trans = " << trans << endl;

   rotation F,Ry,Rz,R,RF;

// First rotate cone about x-axis so that its tip-towards-base
// direction vector is oriented along +zhat:

   double alpha=PI;
   double cos_alpha=cos(alpha);
   double sin_alpha=sin(alpha);

   F.put(1,1,cos_alpha);
   F.put(2,1,sin_alpha);
   F.put(1,2,-sin_alpha);
   F.put(2,2,cos_alpha);

// Next rotate cone about y-axis by angle theta:

   double cos_theta=cos(theta);
   double sin_theta=sin(theta);

   Ry.put(0,0,cos_theta);
   Ry.put(0,2,sin_theta);
   Ry.put(1,1,1);
   Ry.put(2,0,-sin_theta);
   Ry.put(2,2,cos_theta);

// Finally rotate cone about z-axis by angle phi.  Cone's symmetry
// axis is then oriented along (sin_theta cos_phi) x_hat + (sin_theta
// sin_phi) y_hat + cos_theta z_hat:

   double cos_phi=cos(phi);
   double sin_phi=sin(phi);

   Rz.put(0,0,cos_phi);
   Rz.put(0,1,-sin_phi);
   Rz.put(1,0,sin_phi);
   Rz.put(1,1,cos_phi);
   Rz.put(2,2,1);

   R=Rz*Ry;
   RF=R*F;

//   cout << "Ry = " << Ry << endl;
//   cout << "Rz = " << Rz << endl;
//   cout << "R = " << R << endl;
//   cout << "RF = " << RF << endl;
   
   if (!RF.rotation_sanity_check())
   {
      cout << "Error in Cone::scale_rotate_and_then_translate_cone()" << endl;
      cout << "RF is not a proper rotation!" << endl;
      exit(-1);
   }

   const threevector origin(0,0,0);
   Graphical::scale_rotate_and_then_translate(
      curr_t,pass_number,origin,RF,scale,trans);
}

