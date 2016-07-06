// ==========================================================================
// PLANETSGROUP class member function definitions
// ==========================================================================
// Last modified on 3/12/07; 9/5/07; 10/13/07; 5/13/08; 11/27/11
// ==========================================================================

#include <osg/MatrixTransform>

#include <iomanip>
#include <vector>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Material>
#include <osgUtil/Optimizer>
#include <osg/Point>
#include <osg/StateSet>
#include "astro_geo/astrofuncs.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgGrid/EarthGrid.h"
#include "general/filefuncs.h"
#include "astro_geo/moon.h"
#include "numrec/nrfuncs.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "osg/osgSpace/Planet.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "general/stringfuncs.h"

#include "math/constant_vectors.h"
#include "math/mathfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PlanetsGroup::allocate_member_objects()
{

// Instantiate TWO MatrixTransforms which simulate the diurnal
// rotation of the earth.  The first PlanetSpinTransform rotates the
// earth itself about its z-axis.  It is a child of osg::Group
// *solarsystem_refptr.get().  The second EarthSpinTransform is meant
// to hold all objects locked to the surface of the earth (e.g. lines
// of longitude & latitude) which should appear bright even on the
// earth's night-side.  So it is NOT a child of osg::Group
// *solarsystem_refptr.get().

   PlanetSpinTransform_refptr=new osg::MatrixTransform();
   PlanetSpinTransform_refptr->setName("PlanetSpinTransform");

   EarthSpinTransform_refptr=new osg::MatrixTransform();
   EarthSpinTransform_refptr->setName("EarthSpinTransform");
}		       

void PlanetsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PlanetsGroup";

   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";
   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
//   earth_filename=solarsystem_dir+"hires_earth.osga";
   earth_filename=solarsystem_dir+"earth_bright60.osga";

   EarthGraph_ptr=NULL;
   sun_ptr=NULL;

   const double true_sun_dist_from_earth=1.496E11;	// meters
   const double true_sun_radius=6.95E8;			// meters
   computer_graphics_sun_scalefactor=0.01;
   computer_graphics_star_scalefactor=0.1;
//   const double computer_graphics_scalefactor=0.0003;
//   const double computer_graphics_scalefactor=1E-6;

   eff_sun_dist_from_earth=computer_graphics_sun_scalefactor*
      true_sun_dist_from_earth;
   eff_sun_scale=computer_graphics_sun_scalefactor*true_sun_radius;

   eff_star_dist_from_earth=computer_graphics_star_scalefactor*
      true_sun_dist_from_earth;
   eff_star_scale=0.5*eff_sun_scale;

// Note added on 10/13/07:  

//   solarsystem_refptr plays the role of OSGgroup for this
//   PlanetsGroup.  So we attach the update_display callback to
//   solarsystem_refptr within member function generate_solarsystem().

//   get_OSGgroup_ptr()->setUpdateCallback( 
//      new AbstractOSGCallback<PlanetsGroup>(
//         this, &PlanetsGroup::update_display));
}		       

PlanetsGroup::PlanetsGroup(Pass* PI_ptr,Clock* clock_ptr,threevector* GO_ptr):
   DataGraphsGroup(3,PI_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   set_Clock_ptr(clock_ptr);
   compute_earthrotation();
}		       

PlanetsGroup::~PlanetsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PlanetsGroup& P)
{
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      Planet* Planet_ptr=P.get_Planet_ptr(n);
      outstream << "Planet node # " << n << endl;
      outstream << "Planet = " << *Planet_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Planet creation and manipulation methods
// ==========================================================================

// Member function generate_EarthGraph instantiates member DataGraph
// *EarthGraph_ptr with a precalculated .osga file containing a
// textured ellipsoid model for the Earth.

DataGraph* PlanetsGroup::generate_EarthGraph()
{
//   cout << "inside PlanetsGroup::generate_EarthGraph()" << endl;

   EarthGraph_ptr=new DataGraph(
      3,get_next_unused_ID(),pass_ptr,get_LeafNodeVisitor_ptr(),
      get_TreeVisitor_ptr());
   GraphicalsGroup::insert_Graphical_into_list(EarthGraph_ptr);
   insert_graphical_PAT_into_OSGsubPAT(EarthGraph_ptr,0);

   initialize_Graphical(EarthGraph_ptr);

   osg::Node* data_root_ptr=EarthGraph_ptr->ReadGraph(earth_filename);
   EarthGraph_ptr->compute_xyz_and_hyper_bboxes();

// Set material properties of EarthGraph so that it reflects sunlight
// well on its illuminated side:

   osg::StateSet* EarthStateSet = EarthGraph_ptr->get_DataNode_ptr()->
      getOrCreateStateSet();
//   osg::Material* material = new osg::Material;
//   material->setAmbient( osg::Material::BACK,
//                         osg::Vec4( 0.05f, 0.05f, 0.05f, 0.0f ) );
//   material->setDiffuse( osg::Material::BACK,
//                         osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//   material->setSpecular( osg::Material::FRONT,
//                          osg::Vec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
//   material->setSpecular( osg::Material::FRONT_AND_BACK,
//                          osg::Vec4( 0.5f, 0.5f, 0.5f, 0.0f ) );
//   EarthStateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
   EarthStateSet->setMode( GL_NORMALIZE, osg::StateAttribute::ON );

// Optimize earth DataGraph by removing redundant nodes and states:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(data_root_ptr);

   EarthGraph_ptr->get_PAT_ptr()->addChild(data_root_ptr);

// Set label of very top node in EarthGraph so that it can
// subsequently be distinguished from other DataGraphs:

   EarthGraph_ptr->get_DataNode_ptr()->setName("Earth");

// Rotate EarthGraph about its z-axis to simulate diurnal spinning:

   PlanetSpinTransform_refptr->addChild(EarthGraph_ptr->get_DataNode_ptr());

   return EarthGraph_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_solarsystem instantiates the Sun as a
// Planet object.  It also generates member OSG group
// *solarsystem_refptr which holds objects the sun as well as other
// OSG nodes that are illuminated by the sun.

osg::Group* PlanetsGroup::generate_solarsystem(EarthGrid* grid_ptr)
{

/*
// Instantiate moon:

   moon_ptr=generate_new_Planet(Planet::Moon);

   Moon moon(Clock_ptr);
   double eff_moon_dist_from_earth=0.1*moon.get_semimajor_axis();
   double eff_moon_scale=moon.get_equatorial_radius();
//   double eff_moon_dist_from_earth=computer_graphics_sun_scalefactor*
//      moon.get_semimajor_axis();
//   double eff_moon_scale=computer_graphics_sun_scalefactor*
//      moon.get_equatorial_radius();
   threevector moon_direction_ECI=moon.compute_ECI_direction();
   threevector moon_posn=grid_ptr->get_world_middle()
      +eff_moon_dist_from_earth*moon_direction_ECI;
   set_const_scale_and_posn(moon_ptr->get_ID(),eff_moon_scale,moon_posn);
*/

// Instantiate sun:

   sun_ptr=generate_new_Planet(Planet::Sun);
   threevector sun_posn=grid_ptr->get_world_middle()
      +eff_sun_dist_from_earth*Clock_ptr->get_sun_direction_ECI();

// FAKE FAKE: On 9/5/07, we cluged together the following lines in
// order to reorient the sun for Baghdad flythrough movie making
// purposes.  In the future, we should simply choose a different UTC
// hour than 11 for year=2006, month=8, day=24, minutes=secs=0...

//   rotation Rfake;
//   double theta=75*PI/180;
//   Rfake.rotation_about_nhat_by_theta(theta,z_hat);
//   threevector fake_sun_direction_ECI=Rfake*Clock_ptr->
//      get_sun_direction_ECI();
//   threevector sun_posn=grid_ptr->get_world_middle()
//      +eff_sun_dist_from_earth*fake_sun_direction_ECI;
   
   set_const_scale_and_posn(sun_ptr->get_ID(),eff_sun_scale,sun_posn);

// Generate sun's corona:

   double radius=1000;
//   osg::Vec4 corona_color=colorfunc::get_OSG_color(colorfunc::cyan);
//   osg::Vec4 corona_color(0.3,1.0,1.0,1.0);
   osg::Vec4 corona_color(0.5 , 1 , 1 , 1);
   osg::Billboard* corona_ptr=sun_ptr->generate_atmosphere_billboard(
      radius,corona_color);

   double s=2.5E4;
   osg::MatrixTransform* MatrixTransform_ptr=
      osgfunc::generate_scale_and_trans(s,sun_posn);
   MatrixTransform_ptr->addChild(corona_ptr);

   solarsystem_refptr = sun_ptr->createSunLightGroup(sun_posn);
   solarsystem_refptr->addChild(sun_ptr->get_PAT_ptr());
   solarsystem_refptr->addChild(MatrixTransform_ptr);

   solarsystem_refptr->addChild(PlanetSpinTransform_refptr.get());
//   solarsystem_refptr->addChild(moon_ptr->get_PAT_ptr());

// Attach update_display callback to solarsystem_refptr which plays
// the role of OSGgroup for this PlanetsGroup class:

   solarsystem_refptr->setUpdateCallback( 
      new AbstractOSGCallback<PlanetsGroup>(
         this, &PlanetsGroup::update_display));

   return solarsystem_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_starfield instantiates n_iters rounds of
// points of increasing size and adds them to an osg::Geode.  The
// cumulative set of stars is returned by this method within the
// output geode which can be attached to the solar system node.

osg::Geode* PlanetsGroup::generate_starfield(EarthGrid* grid_ptr)
{
   osg::Geode* geode = new osg::Geode;

//   const double default_star_size=1.0;
   const double default_star_size=1.5;
   int n_iters=3;
//   int n_iters=5;
   for (int n=0; n<n_iters; n++)
   {
      int n_stars_per_iter=1500-n*500;
//      int n_stars_per_iter=1500-n*300;
      double star_size=
         default_star_size*pow(2,2*(n/double(n_iters)-0.5));
//      cout << "n = " << n << " star_size = " << star_size 
//           << " n_stars = " << n_stars_per_iter << endl;
      geode->addDrawable(
         generate_starfield_geometry(n_stars_per_iter,star_size,grid_ptr));
   }
   return geode;
}

// ---------------------------------------------------------------------
// Member function generate_starfield_geometry uniformly distributes
// n_stars points over the RA interval 0 < alpha < 2*PI and DEC
// interval -1 < cos(delta) < 1.

osg::Geometry* PlanetsGroup::generate_starfield_geometry(
   int n_stars,double star_size,EarthGrid* grid_ptr)
{
   osg::Geometry* geometry = new osg::Geometry;
   osg::Vec3Array* vertices_ptr=new osg::Vec3Array(n_stars);
   geometry->setVertexArray(vertices_ptr);

   osg::Vec4ubArray* colors_ptr=new osg::Vec4ubArray(n_stars);
   geometry->setColorArray(colors_ptr);
   geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

   for (int i=0; i < n_stars; i++)
   {
      double alpha=(2*PI*nrfunc::ran1())*180/PI;
      double cos_delta=2*(nrfunc::ran1()-0.5);
      double delta=(acos(cos_delta)-PI/2.0)*180/PI;
      threevector star_posn=grid_ptr->get_world_middle()
         +eff_star_dist_from_earth*astrofunc::ECI_vector(alpha,delta);
      vertices_ptr->at(i)=osg::Vec3(star_posn.get(0),star_posn.get(1),
                                    star_posn.get(2));

// On 8/29/06, Ross Anderson pointed out that values for unsigned
// bytes range from 0 to 255 and NOT from 0.0 to 1.0!  If we set the
// values below to fractions less than unity, our starfield appears
// effectively black!

      const unsigned int min_intensity=1;
      unsigned int intensity=basic_math::mytruncate(
         min_intensity+nrfunc::ran1()*(255-min_intensity));
      colors_ptr->at(i)=osg::Vec4ub(intensity,intensity,intensity,255);
   }

//   cout << "vertices_ptr->size() = " << vertices_ptr->size() << endl;
//   cout << "colors_ptr->size() = " << colors_ptr->size() << endl;
   
   osg::DrawArrays* DrawArrays_ptr=new osg::DrawArrays(
      GL_POINTS, 0, vertices_ptr->getNumElements());
   geometry->addPrimitiveSet(DrawArrays_ptr);

   osg::StateSet* StateSet_ptr = new osg::StateSet();
   osg::Point* pt_ptr=new osg::Point;
   StateSet_ptr->setAttribute(pt_ptr);
   pt_ptr->setSize(star_size);
   geometry->setStateSet(StateSet_ptr);

   return geometry;
}

// ---------------------------------------------------------------------
// Member function compute_earthrotation sets member MatrixTransform
// *EarthSpinTransform_refptr.get() to a rotation about the z axis by
// the azimuthal Greenwich angle corresponding to the time stored
// within the input clock.

osg::MatrixTransform* PlanetsGroup::compute_earthrotation()
{
//   cout << "inside PG::compute_earthrotation()" << endl;
   double phi_z=Clock_ptr->get_phi_greenwich();
//   cout << "phi_z = " << phi_z*180/PI << endl;

   osg::Vec3 z_axis(0,0,1);
   osg::Matrix Rz;
   Rz.makeRotate(phi_z,z_axis);
   PlanetSpinTransform_refptr->setMatrix(Rz);
   EarthSpinTransform_refptr->setMatrix(Rz);
   return EarthSpinTransform_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_new_Planet returns a dynamically
// instantiated planet running through the vertices contained within
// input vector V.

Planet* PlanetsGroup::generate_new_Planet(Planet::PlanetType planet_type)
{
//   cout << "inside PlanetsGroup::generate_new_Planet()" << endl;

   int ID=get_next_unused_ID();
   Planet* curr_Planet_ptr=new Planet(
      ID,planet_type,get_LeafNodeVisitor_ptr(),get_TreeVisitor_ptr());

   GraphicalsGroup::insert_Graphical_into_list(curr_Planet_ptr);
   initialize_Graphical(curr_Planet_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Planet_ptr,0);

   osg::Geode* geode_ptr=curr_Planet_ptr->generate_drawable_geode();
   curr_Planet_ptr->get_PAT_ptr()->addChild(geode_ptr);
   
   osg::Billboard* atmosphere_ptr=curr_Planet_ptr->get_atmosphere_ptr();
   if (atmosphere_ptr != NULL)
   {
      curr_Planet_ptr->get_PAT_ptr()->addChild(atmosphere_ptr);
   }
   
   return curr_Planet_ptr;
}

// --------------------------------------------------------------------------
// Member function update_display()

void PlanetsGroup::update_display()
{
//   cout << "inside PlanetsGroup::update_display()" << endl;
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Planet manipulation methods
// ==========================================================================

void PlanetsGroup::set_const_scale_and_posn(
   int ID,double scale,const threevector& V)
{
   Planet* curr_Planet_ptr=get_ID_labeled_Planet_ptr(ID);
   curr_Planet_ptr->set_scale_and_posn(get_curr_t(),get_passnumber(),scale,V);
}

// ---------------------------------------------------------------------
void PlanetsGroup::toggle_ambient_sunlight()
{
   sun_ptr->toggle_ambient_sunlight_intensity();
}

