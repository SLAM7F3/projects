// ==========================================================================
// ArrowHUD class member function definitions
// ==========================================================================
// Last modified on 9/11/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osg2D/ArrowHUD.h"
#include "color/colorfuncs.h"

#include "math/rotation.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

void ArrowHUD::allocate_member_objects()
{
   N_arrow_transform_refptr=new osg::MatrixTransform;
   E_arrow_transform_refptr=new osg::MatrixTransform;
   S_arrow_transform_refptr=new osg::MatrixTransform;
   W_arrow_transform_refptr=new osg::MatrixTransform;

   N_PAT_refptr=new osg::PositionAttitudeTransform();
   E_PAT_refptr=new osg::PositionAttitudeTransform();
   S_PAT_refptr=new osg::PositionAttitudeTransform();
   W_PAT_refptr=new osg::PositionAttitudeTransform();
}

void ArrowHUD::initialize_member_objects()
{
//   arrow_color=colorfunc::white;
}

ArrowHUD::ArrowHUD(colorfunc::Color arrow_color):
   GenericHUD( -640, 640, -512, 512 )
{
//   cout << "inside ArrowHUD constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();

   this->arrow_color=arrow_color;
   generate_arrow_drawables();
   initialize_arrow_transforms();
   set_arrow_directions();
   getProjection()->addChild(N_arrow_transform_refptr.get());
   getProjection()->addChild(E_arrow_transform_refptr.get());
   getProjection()->addChild(W_arrow_transform_refptr.get());
   getProjection()->addChild(S_arrow_transform_refptr.get());

//   set_N_nodemask(0);	// hide arrow
//   set_E_nodemask(0);	// hide arrow
//   set_S_nodemask(0);	// hide arrow
//   set_W_nodemask(0);	// hide arrow
   set_N_nodemask(1);	// display arrow
   set_E_nodemask(1);	// display arrow
   set_S_nodemask(1);	// display arrow
   set_W_nodemask(1);	// display arrow
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void ArrowHUD::set_N_nodemask(int mask_value)
{
   N_arrow_transform_refptr->setNodeMask(mask_value);
}

void ArrowHUD::set_E_nodemask(int mask_value)
{
   E_arrow_transform_refptr->setNodeMask(mask_value);
}

void ArrowHUD::set_S_nodemask(int mask_value)
{
   S_arrow_transform_refptr->setNodeMask(mask_value);
}

void ArrowHUD::set_W_nodemask(int mask_value)
{
   W_arrow_transform_refptr->setNodeMask(mask_value);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_arrow_geode() instantiates cylinder & cone
// drawables.

osg::MatrixTransform* ArrowHUD::generate_arrow_geode()
{
//   cout << "inside ArrowHUD::generate_arrow_geode()" << endl;
//   cout << "curr_size = " << curr_size << endl;

   double scalefactor=2;
   double cone_radius=scalefactor*8*4;
   double cone_height=scalefactor*3*4*2;

// Recall volume of regular symmetric cone = 1/3 Pi radius**2 height
// and its center of mass is located at 1/4 height.  We translate the
// cone so that its tip point lies at its initial center:

   osg::Vec3 init_center(0,0,-0.25*cone_height+cone_height);
   osg::Cone* Cone_ptr=new osg::Cone(init_center,cone_radius,-cone_height);
   osg::ShapeDrawable* cone_shape_ptr=new osg::ShapeDrawable(Cone_ptr);

   double cylinder_radius=0.25*cone_radius;
   double cylinder_height=3*cone_height;
   osg::Vec3 cylinder_position(0,0,0.5*cylinder_height+cone_height);
   osg::Cylinder* Cylinder_ptr=
      new osg::Cylinder(cylinder_position,cylinder_radius,cylinder_height);
   osg::ShapeDrawable* cylinder_shape_ptr=new osg::ShapeDrawable(
      Cylinder_ptr);

   cylinder_shape_ptr->setColor(colorfunc::get_OSG_color(arrow_color));
   cone_shape_ptr->setColor(colorfunc::get_OSG_color(arrow_color));

   osg::Geode* arrow_geode_ptr=new osg::Geode;
   arrow_geode_ptr->addDrawable(cone_shape_ptr);
   arrow_geode_ptr->addDrawable(cylinder_shape_ptr);

// Translate Cone & Cylinder s.t. end of Cylinder rather than tip of
// Cone lies at origin

   osg::MatrixTransform* arrow_translation_ptr=new osg::MatrixTransform();
   cylinder_position=osg::Vec3(0,0,cylinder_height+cone_height);
   osg::Matrixd arrow_translation(
      osg::Matrix::translate( -cylinder_position ) );
   arrow_translation_ptr->setMatrix(arrow_translation);

   arrow_translation_ptr->addChild(arrow_geode_ptr);

   return arrow_translation_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_arrow_drawables() instantiates arrow and
// text label geodes.  It adds these geodes to North and East PATs.
// And it adds the PATs to arrow_transform_refptr.

void ArrowHUD::generate_arrow_drawables()
{
//   cout << "inside ArrowHUD::generate_drawables()" << endl;
//   cout << "curr_size = " << curr_size << endl;

   osg::MatrixTransform* N_arrow_transform_ptr=generate_arrow_geode();
   N_PAT_refptr->addChild(N_arrow_transform_ptr);
   N_arrow_transform_refptr->addChild(N_PAT_refptr.get());

   osg::MatrixTransform* E_arrow_transform_ptr=generate_arrow_geode();
   E_PAT_refptr->addChild(E_arrow_transform_ptr);
   E_arrow_transform_refptr->addChild(E_PAT_refptr.get());

   osg::MatrixTransform* S_arrow_transform_ptr=generate_arrow_geode();
   S_PAT_refptr->addChild(S_arrow_transform_ptr);
   S_arrow_transform_refptr->addChild(S_PAT_refptr.get());

   osg::MatrixTransform* W_arrow_transform_ptr=generate_arrow_geode();
   W_PAT_refptr->addChild(W_arrow_transform_ptr);
   W_arrow_transform_refptr->addChild(W_PAT_refptr.get());
}

// --------------------------------------------------------------------------
// Member function initialize_arrow_transforms() translates the N,E,S,W
// arrows to the HUD's upper center, middle right, lower center and
// middle left:

void ArrowHUD::initialize_arrow_transforms() 
{
   N_arrow_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   E_arrow_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   S_arrow_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   W_arrow_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

   double NS_factor=0.65;
   double EW_factor=0.65;
   osg::Vec3 N_posn(0,NS_factor*m_maxy,0);
   osg::Vec3 S_posn(0,-NS_factor*m_maxy,0);
   osg::Vec3 E_posn(EW_factor*m_maxx,0,0);
   osg::Vec3 W_posn(-EW_factor*m_maxx,0,0);

   osg::Matrixd N_arrow_matrix(
      osg::Matrix::translate( N_posn ) * 
      osg::Matrixd::scale(1,1,1));
   N_arrow_transform_refptr->setMatrix(N_arrow_matrix);

   osg::Matrixd E_arrow_matrix(
      osg::Matrix::translate( E_posn ) * 
      osg::Matrixd::scale(1,1,1));
   E_arrow_transform_refptr->setMatrix(E_arrow_matrix);

   osg::Matrixd S_arrow_matrix(
      osg::Matrix::translate( S_posn ) * 
      osg::Matrixd::scale(1,1,1));
   S_arrow_transform_refptr->setMatrix(S_arrow_matrix);

   osg::Matrixd W_arrow_matrix(
      osg::Matrix::translate( W_posn ) * 
      osg::Matrixd::scale(1,1,1));
   W_arrow_transform_refptr->setMatrix(W_arrow_matrix);
}

// --------------------------------------------------------------------------
// Member function set_arrow_directions() takes in an azimuthal angles
// measured in radians.  It rotates the north and east arrows as well
// as the "N" and "E" text labels.

void ArrowHUD::set_arrow_directions()
{
//   double az=0;
//   cout << "Enter az in degs:" << endl;
//   cin >> az;
//   az *= PI/180;

   double az=90*PI/180;
   double el=90*PI/180;
   double roll=0*PI/180;

   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll);
   fourvector q=R.quaternion_corresponding_to_rotation();
   osg::Quat quat(q.get(0),q.get(1),q.get(2),q.get(3));
   N_PAT_refptr->setAttitude(quat);

   R=R.rotation_from_az_el_roll(az-90*PI/180.0,el,roll);
   q=R.quaternion_corresponding_to_rotation();
   quat=osg::Quat(q.get(0),q.get(1),q.get(2),q.get(3));
   E_PAT_refptr->setAttitude(quat);

   R=R.rotation_from_az_el_roll(az-180*PI/180.0,el,roll);
   q=R.quaternion_corresponding_to_rotation();
   quat=osg::Quat(q.get(0),q.get(1),q.get(2),q.get(3));
   S_PAT_refptr->setAttitude(quat);

   R=R.rotation_from_az_el_roll(az-270*PI/180.0,el,roll);
   q=R.quaternion_corresponding_to_rotation();
   quat=osg::Quat(q.get(0),q.get(1),q.get(2),q.get(3));
   W_PAT_refptr->setAttitude(quat);
}
