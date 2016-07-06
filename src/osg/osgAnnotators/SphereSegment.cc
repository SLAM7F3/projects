// ==========================================================================
// SphereSegment class member function definitions
// ==========================================================================
// Last updated on 1/7/07; 1/21/07; 2/6/07; 12/2/11
// ==========================================================================

#include <iostream>
#include <osgParticle/ExplosionEffect>
//#include <osgParticle/ExplosionDebrisEffect>
//#include <osgParticle/SmokeEffect>
//#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>
#include "osg/osgAnnotators/SphereSegment.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SphereSegment::allocate_member_objects()
{
   group_refptr=new osg::Group();
}		       

void SphereSegment::initialize_member_objects()
{
   Graphical_name="SphereSegment";
   set_permanent_color(colorfunc::get_OSG_color(colorfunc::white,0.5));
   set_size(1.0);
}		       

SphereSegment::SphereSegment(
   int id,double radius,const osg::Vec3& posn,
   double az_min,double az_max,double el_min,double el_max):
   Geometrical(3,id)
{	
   position=posn;
   const int n_sides=60;
   SphereSegment_refptr=new osgSim::SphereSegment(
      position,radius,az_min,az_max,el_min,el_max,n_sides);

   allocate_member_objects();
   initialize_member_objects();
}		       

SphereSegment::~SphereSegment()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const SphereSegment& ss)
{
   outstream << "inside SphereSegment::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(ss) << endl;
   return outstream;
}

// ==========================================================================
// Drawing methods
// ==========================================================================

osg::Group* SphereSegment::generate_drawable_group(
   bool display_spokes_flag,bool include_blast_flag)
{
   if (display_spokes_flag)
   {
      SphereSegment_refptr->setDrawMask(osgSim::SphereSegment::DrawMask(
//         osgSim::SphereSegment::SURFACE|
         osgSim::SphereSegment::SPOKES|
         osgSim::SphereSegment::EDGELINE|
         osgSim::SphereSegment::SIDES));
   }
   else
   {
      SphereSegment_refptr->setDrawMask(osgSim::SphereSegment::DrawMask(
         osgSim::SphereSegment::SURFACE));
   }
   group_refptr->addChild(SphereSegment_refptr.get());

   if (include_blast_flag) GenerateBlast();
   return group_refptr.get();
}

// ---------------------------------------------------------------------
// Member function set_color() sets the color of the current object plus
// its attendant text label based upon the input RGBA information.

void SphereSegment::set_color(const osg::Vec4& color)
{
//   cout << "inside SphereSegment::set_color()" << endl;

//   SphereSegment_refptr->setSurfaceColor(color);
//   SphereSegment_refptr->setSpokeColor(color);
//   SphereSegment_refptr->setEdgeLineColor(color);
//   SphereSegment_refptr->setSideColor(color);

   SphereSegment_refptr->setAllColors(color);
}

// ---------------------------------------------------------------------
void SphereSegment::GenerateBlast()
{
   const float scale=30.0;
   const float intensity=15.0;
   osgParticle::ExplosionEffect* explosion = 
      new osgParticle::ExplosionEffect(position,1.5*scale,1.5*intensity);

//   osgParticle::ExplosionDebrisEffect* explosionDebri = 
//      new osgParticle::ExplosionDebrisEffect(position, 10.0f);
//   osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(
//      position,scale,intensity);
   osgParticle::FireEffect* fire = new osgParticle::FireEffect(
      position,scale,intensity);

   const double fire_duration=1000;
   fire->setEmitterDuration(fire_duration);

   osg::Vec3 wind(0.0f,0.0f,0.0f);
   explosion->setWind(wind);
//   explosionDebri->setWind(wind);
//   smoke->setWind(wind);
   fire->setWind(wind);

   group_refptr->addChild(explosion);
//   group_refptr->addChild(explosionDebri);
//   group_refptr->addChild(smoke);
   group_refptr->addChild(fire);
}
