// ==========================================================================
// CONESGROUP class member function definitions
// ==========================================================================
// Last modified on 6/15/08; 9/7/09; 9/8/09; 12/4/10; 4/5/14
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include <osg/Light>
#include <osg/LightModel>
#include <osg/LightSource>

#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/ConesGroup.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ConesGroup::allocate_member_objects()
{
}		       

void ConesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ConesGroup";
//   radius=0.1;
//   height=0.1;
//   size[3]=0.1;
   radius=1.0;
   height=1.0;
   size[3]=1.0;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ConesGroup>(
         this, &ConesGroup::update_display));
}		       

ConesGroup::ConesGroup(Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ConesGroup::ConesGroup(
   Pass* PI_ptr,osgGA::Custom3DManipulator* CM_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   CM_3D_refptr=CM_ptr;
}		       

ConesGroup::~ConesGroup()
{
//    cout << "inside ConesGroup destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ConesGroup& C)
{
   int node_counter=0;
   for (unsigned int n=0; n<C.get_n_Graphicals(); n++)
   {
      Cone* Cone_ptr=C.get_Cone_ptr(n);
      outstream << "Cone node # " << node_counter++ << endl;
      outstream << "Cone = " << *Cone_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Cone creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Cone from all other graphical insertion
// and manipulation methods...

Cone* ConesGroup::generate_new_Cone(
   const threevector& tip,const threevector& base,double radius,
   int ID,unsigned int OSGsubPAT_number)
{
//    cout << "inside ConesGroup::generate_new_Cone() #1" << endl;

//   this->radius=radius;
//   this->height = radius;
//   size[3]=radius;

   Cone* curr_Cone_ptr=generate_new_Cone(ID,OSGsubPAT_number);
   
//   cout << "radius = " << radius
//        << " height = " << height << endl;

   transform_cone(curr_Cone_ptr,tip,base,radius);
   return curr_Cone_ptr;
}

Cone* ConesGroup::generate_new_Cone(int ID,unsigned int OSGsubPAT_number)
{
//   cout << "inside ConesGroup::generate_new_Cone() #2" << endl;
   
   if (ID==-1) ID=get_next_unused_ID();
   Cone* curr_Cone_ptr=new Cone(radius,height,ID);
   curr_Cone_ptr->set_reference_origin(get_grid_world_origin());
   initialize_new_Cone(curr_Cone_ptr,OSGsubPAT_number);
   return curr_Cone_ptr;
}

// ---------------------------------------------------------------------
void ConesGroup::initialize_new_Cone(
   Cone* curr_Cone_ptr,unsigned int OSGsubPAT_number)
{
//   cout << "inside ConesGroup::initialize_new_Cone" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_Cone_ptr);
   initialize_Graphical(curr_Cone_ptr);

// Recall that as of Sep 2009, we store relative vertex information
// with respect to an average reference_origin point to avoid floating
// point problems.  So we need to translate the Cone by its reference
// origin in order to globally position it:

   curr_Cone_ptr->set_UVW_coords(
      get_curr_t(),get_passnumber(),
      curr_Cone_ptr->get_reference_origin());

   osg::Geode* geode_ptr=curr_Cone_ptr->generate_drawable_geode();
   curr_Cone_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Cone_ptr,OSGsubPAT_number);

   reset_colors();
}

// --------------------------------------------------------------------------
// Method transform_cone takes in some canonical cone with radius=1,
// height=1, tip location = (0,0,0) and symmetry direction = +z_hat.
// It first scales and rotates the cone so that its direction vector
// points towards from the input base to the input tip.  It also
// translates the cone so that its tip point coincides with the tip
// input.

void ConesGroup::transform_cone(
   Cone* Cone_ptr,const threevector& tip,const threevector& base,
   double radius)
{   
//   cout << "inside ConesGroup::transform_cone()" << endl;

//   cout << "tip = " << tip << endl;
//   cout << "base = " << base << endl;
   threevector h_hat=(base-tip).unitvector();
   double height=(base-tip).magnitude();
   threevector scale(radius,radius,height);
//   cout << "radius = " << radius << endl;
//   cout << "height = " << height << endl;
//   cout << "h_hat = " << h_hat << endl;

   Cone_ptr->set_theta(acos(h_hat.get(2)));
   Cone_ptr->set_phi(0);
   if (!nearly_equal(Cone_ptr->get_theta(),0))
   {
      Cone_ptr->set_phi(atan2(h_hat.get(1),h_hat.get(0)));
   }
   Cone_ptr->scale_rotate_and_then_translate(
      get_curr_t(),get_passnumber(),
      Cone_ptr->get_theta(),Cone_ptr->get_phi(),scale,tip);
}

// --------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for
// Cone objects corresponding to the current dimension by input
// parameter factor.

void ConesGroup::change_size(double factor)
{   
   size[get_ndims()] *= factor;
   GraphicalsGroup::change_size(factor);
}

// --------------------------------------------------------------------------
// Member function set_altitude_dependent_size() retrieves the current
// altitude of the virtual camera.  If CM_3D_ptr != NULL, it resets
// the size for Cones as a linear function of the altitude.  We wrote
// this method in Sep 2009 for automatically resizing vector flow
// field arrow heads for the LOST project.

void ConesGroup::set_altitude_dependent_size(
   double prefactor,double max_size,double min_size,double z_min,double z_max)
{
//   cout << "inside ConesGroup::set_altitude_dependent_size()"  << endl;

   double size=prefactor*compute_altitude_dependent_size(z_min,z_max);
   size=basic_math::min(size,max_size);
   size=basic_math::max(size,min_size);
//   cout << "size = " << size << endl;

   if (size > 0)
   {
      set_size(size);
   }
}

// --------------------------------------------------------------------------
// Member function reset_colors loops over all entries within the
// GraphicalsList.  It temporarily colors blue the Graphical whose ID
// equals selected_Graphical_ID.  All other Graphicals are colored
// according to their permanent, intrinsic colors.

void ConesGroup::reset_colors()
{   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Cone* cone_ptr=get_Cone_ptr(n);
      colorfunc::Color curr_cone_color;
      if (get_selected_Graphical_ID()==cone_ptr->get_ID())
      {
         curr_cone_color=colorfunc::blue;
      }
      else
      {
         curr_cone_color=colorfunc::get_colorfunc_color(
            cone_ptr->get_permanent_color());
      }
      osg::Vec4 osg_color=colorfunc::get_OSG_color(curr_cone_color);
//      cout << "osg_color: r = " << osg_color.r() 
//           << " g = " << osg_color.g() << " b = " << osg_color.b()
//           << " a = " << osg_color.a() << endl;
      cone_ptr->reset_state(osg_color);
   } // loop over conees in conelist
}

// --------------------------------------------------------------------------
// Member function change_color retrieves the ID for the currently
// selected Cone graphical.  It then queries the user to enter a new
// color string for the selected Cone in the console window.  This
// method changes the selected Cone's permanent, intrinsic color to the
// user entered one.

void ConesGroup::change_color()
{   
   cout << "inside ConesGroup::change_color()" << endl;
   
   int selected_ID=get_selected_Graphical_ID();
   Cone* selected_Cone_ptr=get_ID_labeled_Cone_ptr(selected_ID);

   if (selected_Cone_ptr != NULL)
   {
//      osg::Vec4 curr_color=selected_Cone_ptr->get_color();
//      cout << "Selected cone " << selected_ID 
//           << " has color = " << curr_color.x() << ","
//           << curr_color.y() << "," << curr_color.z() << endl;

      string colorstring;
      cout << "Enter new color for selected cone " << selected_ID
           << endl;
      cin >> colorstring;

      double alpha=1.0;
      cout << "Enter alpha for selected cone" << endl;
      cin >> alpha;
      
      colorfunc::Color c=colorfunc::string_to_color(colorstring);
      selected_Cone_ptr->set_permanent_color(c);
      reset_colors();
   } // selected_Cone_ptr != NULL conditional
}

// ---------------------------------------------------------------------
osg::Group* ConesGroup::createConeLight(const threevector& light_posn)
{

    osg::LightSource* ConeLightSource = new osg::LightSource;

    osg::Light* ConeLight = ConeLightSource->getLight();
    ConeLight->setPosition( osg::Vec4( light_posn.get(0),light_posn.get(1),
                                      light_posn.get(2),1.0));
//    ConeLight->setAmbient( osg::Vec4( 0.35f, 0.35f, 0.35f, 1.0f ) );
//    ConeLight->setAmbient( osg::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ) );
//    ConeLight->setAmbient( osg::Vec4( 0.65f, 0.65f, 0.65f, 1.0f ) );
    ConeLight->setAmbient( osg::Vec4( 0.7f, 0.7f, 0.7f, 1.0f ) );
//    ConeLight->setAmbient( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    ConeLight->setAmbient( osg::Vec4( 0.9f, 0.9f, 0.9f, 1.0f ) );

//    ConeLight->setDiffuse( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    ConeLight->setDiffuse( osg::Vec4( 0.95f, 0.95f, 0.95f, 1.0f ) );

    ConeLightSource->setLight( ConeLight );
    ConeLightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    ConeLightSource->getOrCreateStateSet()->setMode(
       GL_LIGHTING, osg::StateAttribute::ON);

    osg::LightModel* lightModel = new osg::LightModel;
//    lightModel->setAmbientIntensity(osg::Vec4(0.25f,0.25f,0.25f,0.5f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.75f,0.75f,0.75f,1.0f));
    lightModel->setAmbientIntensity(osg::Vec4(0.9f,0.9f,0.9f,1.0f));
    ConeLightSource->getOrCreateStateSet()->setAttribute(lightModel);


    return ConeLightSource;
} 

// --------------------------------------------------------------------------
// Member function update_display()

void ConesGroup::update_display()
{
//   cout << "inside ConesGroup::update_display()" << endl;

   GraphicalsGroup::update_display();
}
   
// ==========================================================================
// View frustum cone methods
// ==========================================================================

void ConesGroup::draw_FOV_cone()
{
   cout << "inside ConesGroup::draw_FOV_cone()" << endl;
   
   double radius=6378137.0;  // meters  (true size)

   threevector tip(CM_3D_refptr->get_eye_world_posn());
   threevector base(tip-5*radius*CM_3D_refptr->get_camera_Zhat());
   cout << "tip = " << tip << " base = " << base << endl;
   cout << "camera_Zhat = " << CM_3D_refptr->get_camera_Zhat() << endl;
   
//   Cone* cone_ptr=
      generate_new_Cone(tip,base,radius);
}

