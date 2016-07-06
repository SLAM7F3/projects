// ==========================================================================
// CompassHUD class member function definitions
// ==========================================================================
// Last modified on 6/6/09; 6/7/09; 8/25/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "osg/CompassHUD.h"
#include "math/rotation.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

void CompassHUD::allocate_member_objects()
{
   compass_transform_refptr=new osg::MatrixTransform;
   N_PAT_refptr=new osg::PositionAttitudeTransform();
   E_PAT_refptr=new osg::PositionAttitudeTransform();

/*
   depth_off_refptr=new osg::Depth(osg::Depth::ALWAYS, 0.0, 0.01);
   osg::StateSet* stateset_ptr=compass_transform_refptr->getOrCreateStateSet();
   stateset_ptr->clear();
   stateset_ptr->setAttributeAndModes(
      depth_off_refptr.get(),osg::StateAttribute::ON);
*/
}

void CompassHUD::initialize_member_objects()
{
   nadir_oriented_compass_flag=true;
//   compass_color=colorfunc::white;
   north_az_offset=0;
}

CompassHUD::CompassHUD(colorfunc::Color compass_color):
   GenericHUD( -640, 640, -512, 512 )
{
//   cout << "inside CompassHUD constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();

   this->compass_color=compass_color;
   generate_compass_drawables();
   initialize_compass_transform();
   getProjection()->addChild(compass_transform_refptr.get());

//   set_nodemask(0);	// hide compass
   set_nodemask(1);	// display compass
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void CompassHUD::set_nodemask(int mask_value)
{
   compass_transform_refptr->setNodeMask(mask_value);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_arrow_geode() instantiates cylinder & cone
// drawables.

osg::MatrixTransform* CompassHUD::generate_arrow_geode()
{
//   cout << "inside CompassHUD::generate_arrow_geode()" << endl;
//   cout << "curr_size = " << curr_size << endl;

   double cone_radius=8;
   double cone_height=3*4;

// Recall volume of regular symmetric cone = 1/3 Pi radius**2 height
// and its center of mass is located at 1/4 height.  We translate the
// cone so that its tip point lies at its initial center:

   osg::Vec3 init_center(0,0,-0.25*cone_height+cone_height);
   osg::Cone* Cone_ptr=new osg::Cone(init_center,cone_radius,-cone_height);
   osg::ShapeDrawable* cone_shape_ptr=new osg::ShapeDrawable(Cone_ptr);

   double cylinder_radius=0.25*cone_radius;
   double cylinder_height=6*cone_height;
   osg::Vec3 cylinder_position(0,0,0.5*cylinder_height+cone_height);
   osg::Cylinder* Cylinder_ptr=
      new osg::Cylinder(cylinder_position,cylinder_radius,cylinder_height);
   osg::ShapeDrawable* cylinder_shape_ptr=new osg::ShapeDrawable(
      Cylinder_ptr);

   cylinder_shape_ptr->setColor(colorfunc::get_OSG_color(compass_color));
   cone_shape_ptr->setColor(colorfunc::get_OSG_color(compass_color));

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
// Member function generate_text_label_geode() prepares a new text
// geode to hold "N" or "E" direction labels.

osg::Geode* CompassHUD::generate_text_label_geode(string label)
{
//   cout << "inside CompassHUD::generate_text_label_geode()" << endl;

   osgText::Text* text_ptr=new osgText::Text;
   text_ptr->setFont("fonts/times.ttf");
   text_ptr->setCharacterSize(35);
   text_ptr->setAxisAlignment(osgText::Text::SCREEN);
   text_ptr->setAlignment(osgText::Text::CENTER_CENTER);
   text_ptr->setBackdropType(osgText::Text::OUTLINE);
   text_ptr->setText(label);
   text_ptr->setColor(colorfunc::get_OSG_color(compass_color));

   osg::Geode* text_geode_ptr=new osg::Geode;
   text_geode_ptr->addDrawable(text_ptr);

   return text_geode_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_compass_drawables() instantiates arrow and
// text label geodes.  It adds these geodes to North and East PATs.
// And it adds the PATs to compass_transform_refptr.

void CompassHUD::generate_compass_drawables()
{
//   cout << "inside CompassHUD::generate_drawables()" << endl;
//   cout << "curr_size = " << curr_size << endl;

   osg::MatrixTransform* N_arrow_transform_ptr=generate_arrow_geode();
   N_PAT_refptr->addChild(N_arrow_transform_ptr);
   compass_transform_refptr->addChild(N_PAT_refptr.get());

   osg::MatrixTransform* E_arrow_transform_ptr=generate_arrow_geode();
   E_PAT_refptr->addChild(E_arrow_transform_ptr);
   compass_transform_refptr->addChild(E_PAT_refptr.get());

   N_text_geode_ptr=generate_text_label_geode("N");
   compass_transform_refptr->addChild(N_text_geode_ptr);

   E_text_geode_ptr=generate_text_label_geode("E");
   compass_transform_refptr->addChild(E_text_geode_ptr);
}

// --------------------------------------------------------------------------
// Member function initialize_compass_transform() translates the
// compass to the HUD's lower left corner.

void CompassHUD::initialize_compass_transform() 
{
   compass_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
//   osg::Vec3 posn(0,0,0);
//   osg::Vec3 posn(0.78*m_maxx,0.73*m_maxy,0);
   osg::Vec3 posn(-0.79*m_maxx,-0.73*m_maxy,0);

   osg::Matrixd compass_matrix(
      osg::Matrix::translate( posn ) * 
      osg::Matrixd::scale(1,1,1));
   compass_transform_refptr->setMatrix(compass_matrix);
}

// --------------------------------------------------------------------------
// Member function rotate_compass() takes in an azimuthal angles
// measured in radians.  It rotates the north and east arrows as well
// as the "N" and "E" text labels.

void CompassHUD::rotate_compass(double az)
{
   double el=90*PI/180;
   double roll=0*PI/180;

   az += north_az_offset;

   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll);
   fourvector q=R.quaternion_corresponding_to_rotation();
   osg::Quat quat(q.get(0),q.get(1),q.get(2),q.get(3));
   N_PAT_refptr->setAttitude(quat);

   R=R.rotation_from_az_el_roll(az-90*PI/180.0,el,roll);
   q=R.quaternion_corresponding_to_rotation();
   quat=osg::Quat(q.get(0),q.get(1),q.get(2),q.get(3));
   E_PAT_refptr->setAttitude(quat);

   const double radius=115;

   osgText::Text* text_ptr=static_cast<osgText::Text*>(
      N_text_geode_ptr->getDrawable(0));
   osg::Vec3 text_position(radius*cos(az),radius*sin(az),0);
   text_ptr->setPosition(text_position);

   text_ptr=static_cast<osgText::Text*>(
      E_text_geode_ptr->getDrawable(0));
   text_position=osg::Vec3(radius*cos(az-PI/2),radius*sin(az-PI/2),0);
   text_ptr->setPosition(text_position);
}
