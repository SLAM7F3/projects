// ==========================================================================
// CYLINDERSGROUP class member function definitions
// ==========================================================================
// Last modified on 6/5/09; 12/4/10; 10/12/11; 4/5/14
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include <osg/Light>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/Quat>

#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "osg/osgGeometry/Cylinder.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "astro_geo/Ellipsoid_model.h"
#include "astro_geo/geopoint.h"
#include "messenger/Messenger.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

#include "math/constant_vectors.h"
#include "osg/osgfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CylindersGroup::allocate_member_objects()
{
}		       

void CylindersGroup::initialize_member_objects()
{
   GraphicalsGroup_name="CylindersGroup";
   tall_RTPS_size_flag=false;
   radius=1.0;
   height=1.0;
   size[3]=1.0;

   initial_jump_flag=false;
   theta_U=theta_V=NEGATIVEINFINITY;
   movers_group_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<CylindersGroup>(
         this, &CylindersGroup::update_display));
}		       

CylindersGroup::CylindersGroup(
   Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

CylindersGroup::CylindersGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,clock_ptr,EM_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

CylindersGroup::CylindersGroup(
   Pass* PI_ptr,AnimationController* AC_ptr,
   osgGA::Terrain_Manipulator* CM_3D_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   set_CM_3D_ptr(CM_3D_ptr);
}		       

CylindersGroup::~CylindersGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const CylindersGroup& C)
{
   int node_counter=0;
   for (unsigned int n=0; n<C.get_n_Graphicals(); n++)
   {
      Cylinder* Cylinder_ptr=C.get_Cylinder_ptr(n);
      outstream << "Cylinder node # " << node_counter++ << endl;
      outstream << "Cylinder = " << *Cylinder_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Cylinder creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Cylinder from all other graphical insertion
// and manipulation methods...

Cylinder* CylindersGroup::generate_new_Cylinder(
   const threevector& center,const osg::Quat& q,
   colorfunc::Color& permanent_color,
   int n_text_messages,double text_displacement,double text_size,
   bool text_screen_axis_alignment_flag,int ID,unsigned int OSGsubPAT_number)
{
   threevector text_3D_displacement(0,0,text_displacement);
   return generate_new_Cylinder(
      center,q,permanent_color,text_3D_displacement,n_text_messages,text_size,
      text_screen_axis_alignment_flag,ID,OSGsubPAT_number);
}

Cylinder* CylindersGroup::generate_new_Cylinder(
   const threevector& center,const osg::Quat& q,
   colorfunc::Color& permanent_color,
   const threevector& text_displacement,int n_text_messages,double text_size,
   bool text_screen_axis_alignment_flag,int ID,unsigned int OSGsubPAT_number)
{
//   cout << "inside CylindersGroup::generate_new_Cylinder()" << endl;
//   cout << "center = " << center << endl;
//   cout << "permanent_color = " << permanent_color << endl;
//   cout << "n_text_messages = " << n_text_messages << endl;
//   cout << "text_displacement = " << text_displacement << endl;
//   cout << "text_size = " << text_size << endl;
//   cout << "text_screen_axis_alignment_flag = "
//        << text_screen_axis_alignment_flag << endl;
//   cout << "ID = " << ID << endl;

   if (ID==-1) ID=get_next_unused_ID();
   Cylinder* curr_Cylinder_ptr=new Cylinder(
      center,q,radius,height,
      font_refptr.get(),n_text_messages,text_displacement,text_size,
      permanent_color,ID);

   initialize_new_Cylinder(curr_Cylinder_ptr,text_screen_axis_alignment_flag,
                           OSGsubPAT_number);
   return curr_Cylinder_ptr;
}

// ---------------------------------------------------------------------
void CylindersGroup::initialize_new_Cylinder(
   Cylinder* curr_Cylinder_ptr,bool text_screen_axis_alignment_flag,
   unsigned int OSGsubPAT_number)
{
//   cout << "inside CylindersGroup::initialize_new_Cylinder" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_Cylinder_ptr);
   initialize_Graphical(curr_Cylinder_ptr);

   osg::Geode* geode_ptr=curr_Cylinder_ptr->generate_drawable_geode(
      common_geometrical_size,text_screen_axis_alignment_flag);
   curr_Cylinder_ptr->get_PAT_ptr()->addChild(geode_ptr);

   insert_graphical_PAT_into_OSGsubPAT(curr_Cylinder_ptr,OSGsubPAT_number);
   reset_colors();
}

// --------------------------------------------------------------------------
// Member function orient_cylinder_with_ellipsoid_radial_dir align's
// the input cylinder's symmetry axis with the radial direction at the
// point where the cylinder is instantiated.

void CylindersGroup::orient_cylinder_with_ellipsoid_radial_dir(
   Cylinder* cylinder_ptr)
{
   osg::Vec3f Z_hat(0,0,1);
   threevector r_hat(Ellipsoid_model_ptr->get_radial_ECI_hat());
   osg::Quat q;
   q.makeRotate(Z_hat,osg::Vec3f(r_hat.get(0),r_hat.get(1),r_hat.get(2)));
   cylinder_ptr->set_quaternion(get_curr_t(),get_passnumber(),q);
}

// --------------------------------------------------------------------------
// Method scale_rotate_and_then_translate_cylinder takes in some
// canonical cylinder with radius=1, height=1, center = (0,0,0) and
// symmetry direction = +z_hat.  It first orients the cylinder so that
// its direction vector points towards -z_hat.  It next rotates the
// cylinder so that its symmetry direction points [in standard polar
// coordinates] along (sin_theta cos_phi) x_hat + (sin_theta sin_phi)
// y_hat + cos_theta z_hat.  It also rescales the cylinder's size
// according to input threevector and translates the cylinder so that
// its tip lies at input threevector trans.

void CylindersGroup::scale_rotate_and_then_translate_cylinder(
   Cylinder* Cylinder_ptr,double theta,double phi,const threevector& scale,
   const threevector& trans)
{   
   rotation F,Ry,Rz,R,RF;

// First rotate cylinder about x-axis so that its tip points towards -z_hat:

   double alpha=PI;
   double cos_alpha=cos(alpha);
   double sin_alpha=sin(alpha);

   F.put(1,1,cos_alpha);
   F.put(2,1,sin_alpha);
   F.put(1,2,-sin_alpha);
   F.put(2,2,cos_alpha);

// Next rotate cylinder about y-axis by angle theta:

   double cos_theta=cos(theta);
   double sin_theta=sin(theta);

   Ry.put(0,0,cos_theta);
   Ry.put(0,2,sin_theta);
   Ry.put(1,1,1);
   Ry.put(2,0,-sin_theta);
   Ry.put(2,2,cos_theta);

// Finally rotate cylinder about z-axis by angle phi.  Cylinder's symmetry
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
      cout << "Error in CylindersGroup::scale_rotate_and_then_translate_cylinder()"
           << endl;
      cout << "RF is not a proper rotation!" << endl;
      exit(-1);
   }

   const threevector origin(0,0,0);
   Cylinder_ptr->scale_rotate_and_then_translate(
      get_curr_t(),get_passnumber(),origin,RF,scale,trans);
}

// --------------------------------------------------------------------------
// Member function change_color retrieves the ID for the currently
// selected Cylinder graphical.  It then queries the user to enter a new
// color string for the selected Cylinder in the console window.  This
// method changes the selected Cylinder's permanent, intrinsic color to the
// user entered one.

void CylindersGroup::change_color()
{   
   cout << "inside CylindersGroup::change_color()" << endl;
   
   int selected_ID=get_selected_Graphical_ID();
   Cylinder* selected_Cylinder_ptr=get_ID_labeled_Cylinder_ptr(selected_ID);

   if (selected_Cylinder_ptr != NULL)
   {
//      osg::Vec4 curr_color=selected_Cylinder_ptr->get_color();
//      cout << "Selected cylinder " << selected_ID 
//           << " has color = " << curr_color.x() << ","
//           << curr_color.y() << "," << curr_color.z() << endl;

      string colorstring;
      cout << "Enter new color for selected cylinder " << selected_ID
           << endl;
      cin >> colorstring;

      double alpha=1.0;
      cout << "Enter alpha for selected cylinder" << endl;
      cin >> alpha;
      
      colorfunc::Color c=colorfunc::string_to_color(colorstring);
      selected_Cylinder_ptr->set_permanent_color(c);
      reset_colors();
   } // selected_Cylinder_ptr != NULL conditional
}

// ---------------------------------------------------------------------
osg::Group* CylindersGroup::createCylinderLight(const threevector& light_posn)
{
    osg::LightSource* CylinderLightSource = new osg::LightSource;

    osg::Light* CylinderLight = CylinderLightSource->getLight();
    CylinderLight->setPosition( 
       osg::Vec4( light_posn.get(0),light_posn.get(1),
                  light_posn.get(2),1.0));
//    CylinderLight->setAmbient( osg::Vec4( 0.35f, 0.35f, 0.35f, 1.0f ) );
//    CylinderLight->setAmbient( osg::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ) );
//    CylinderLight->setAmbient( osg::Vec4( 0.65f, 0.65f, 0.65f, 1.0f ) );
    CylinderLight->setAmbient( osg::Vec4( 0.7f, 0.7f, 0.7f, 1.0f ) );
//    CylinderLight->setAmbient( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    CylinderLight->setAmbient( osg::Vec4( 0.9f, 0.9f, 0.9f, 1.0f ) );

//    CylinderLight->setDiffuse( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    CylinderLight->setDiffuse( osg::Vec4( 0.95f, 0.95f, 0.95f, 1.0f ) );

    CylinderLightSource->setLight( CylinderLight );
    CylinderLightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    CylinderLightSource->getOrCreateStateSet()->setMode(
       GL_LIGHTING, osg::StateAttribute::ON);

    osg::LightModel* lightModel = new osg::LightModel;
//    lightModel->setAmbientIntensity(osg::Vec4(0.25f,0.25f,0.25f,0.5f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.75f,0.75f,0.75f,1.0f));
    lightModel->setAmbientIntensity(osg::Vec4(0.9f,0.9f,0.9f,1.0f));
    CylinderLightSource->getOrCreateStateSet()->setAttribute(lightModel);

    return CylinderLightSource;
}

// ==========================================================================
// Update member functions
// ==========================================================================

// Member function recolor_encountered_vehicle_Cylinders()

void CylindersGroup::recolor_encountered_vehicle_Cylinders()
{
//   cout << "inside CylindersGroup::recolor_encountered_vehicle_Cylinders()"
//        << endl;
   if (movers_group_ptr==NULL) return;
  
   vector<int> encountered_vehicle_IDs=movers_group_ptr->
      get_encountered_vehicle_IDs();
   for (unsigned int r=0; r<encountered_vehicle_IDs.size(); r++)
   {
//      cout << "VEHICLE ID = " << encountered_vehicle_IDs[r] 
//           << " previously encountered" << endl;
      Cylinder* Cylinder_ptr=
         get_ID_labeled_Cylinder_ptr(encountered_vehicle_IDs[r]);

      colorfunc::Color encountered_color=colorfunc::pink;
      Cylinder_ptr->set_permanent_color(encountered_color);
   }
}
    
// --------------------------------------------------------------------------
// Member function update_display()

void CylindersGroup::update_display()
{
//   cout << "inside CylindersGroup::update_display()" << endl;
//   cout << "get_curr_t() = " << get_curr_t() << endl;
//   cout << "n_cyls = " << get_n_Graphicals() << endl;

   parse_latest_messages();

   reset_colors();

//   follow_selected_cylinder();
   follow_selected_Geometrical();

   recolor_encountered_vehicle_Cylinders();

   GraphicalsGroup::update_display();
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool CylindersGroup::parse_next_message_in_queue(message& curr_message)
{
   cout << "inside CylindersGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message = " << curr_message << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   string value;
   if (curr_message.get_text_message()=="UPDATE_TRACK_POSN")
   {
      cout << "Received UPDATE_TRACK_POSN message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();

      value=curr_message.get_property_value("ID");  
      int track_ID=stringfunc::string_to_integer(value);

      value=curr_message.get_property_value("Time");
      double time=stringfunc::string_to_number(value);
      
      value=curr_message.get_property_value("X Y");
      vector<double> XY=stringfunc::string_to_numbers(value);
      double Z=0;

      update_track_posn(track_ID,time,XY[0],XY[1],Z);
      message_handled_flag=true;
   }

   return message_handled_flag;
}

// ==========================================================================
// Cylinder tracking member functions
// ==========================================================================

/*
// Member function follow_selected_cylinder locks the
// TerrainManipulator virtual camera's lateral position to the
// instantaneous X & Y coordinates of the cylinder specified by the
// input ID.  We wrote this method in order to facilitate user
// monitoring of particular vehicles within Bluegrass applications.
// The virtual camera's height above the cylinder can be dynamically
// adjusted.

void CylindersGroup::follow_selected_cylinder(
   double min_height_above_cylinder)
{
//   cout << "inside CylindersGroup::follow_selected_cylinder()" << endl;

// Test lab for centering a particular cylinder and moving map
// underneath it:

   int ID=get_selected_Graphical_ID();
   if (ID < 0) return;
//   cout << "ID = " << ID << endl;
   Cylinder* curr_Cylinder_ptr=get_ID_labeled_Cylinder_ptr(ID);
//   cout << "curr_Cyl_ptr = " << curr_Cylinder_ptr << endl;

   track* track_ptr=curr_Cylinder_ptr->get_track_ptr();
//   cout << "track_ptr = " << track_ptr << endl;
   if (track_ptr==NULL) return;
   
   string vehicle_label=track_ptr->get_label();
   int vehicle_track_ID=stringfunc::string_to_integer(
      stringfunc::suffix(vehicle_label,"V"));
//   cout << "ID = " << ID
//        << " vehicle track # = " << vehicle_track_ID << endl;

   threevector cyl_posn;
   curr_Cylinder_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),cyl_posn);
   if (CM_3D_refptr.valid() && !cyl_posn.nearly_equal(Zero_vector))
   {
      osg::Matrixd M=CM_3D_refptr->getMatrix();
//      osgfunc::print_matrix(M);

      double min_eye_posn_altitude=cyl_posn.get(2)+min_height_above_cylinder;
      double eye_posn_altitude=basic_math::max(min_eye_posn_altitude,M(3,2));

      osg::Matrixd new_M;
      new_M.set(M(0,0) , M(0,1) , M(0,2) , M(0,3) , 
                M(1,0) , M(1,1) , M(1,2) , M(1,3) ,
                M(2,0) , M(2,1) , M(2,2) , M(2,3) ,
                cyl_posn.get(0) , cyl_posn.get(1) , eye_posn_altitude, 1 );
      CM_3D_refptr->setMatrices(new_M);
   } // CM_3D_refptr.valid() conditional
}
*/

    
// --------------------------------------------------------------------------
// Member function find_Cylinder_ID_given_track_label takes in integer
// track_label.  It performs a brute force search over all Cylinders
// associated with tracks.  If some Cylinder's track label matches the
// input track_label, this method returns the Cylinder's ID.
// Otherwise, it returns -1.

int CylindersGroup::find_Cylinder_ID_given_track_label(int track_label)
{
//   cout << "inside CylindersGroup::find_Cylinder_ID_given_track_label()" 
//        << endl;

   for (unsigned int c=0; c<get_n_Graphicals(); c++)
   {
      Cylinder* curr_Cylinder_ptr=get_Cylinder_ptr(c);
      track* curr_track_ptr=curr_Cylinder_ptr->get_track_ptr();
      if (curr_track_ptr != NULL)
      {
         string curr_track_label=curr_track_ptr->get_label();
         int curr_track_ID=stringfunc::string_to_integer(
            stringfunc::suffix(curr_track_label,"V"));
         if (curr_track_ID==track_label)
         {
            return curr_Cylinder_ptr->get_ID();
         }
      }
   } // loop over index c labeling Cylinders
   return -1;
}

// ==========================================================================
// Real time persistent surveillance specific member functions
// ==========================================================================

// Member function set_tall_Cylinder_size() rescales all Cylinders'
// X, Y and Z coordinates along with their text sizes so that they can
// easily be seen within the Milwaukee map.  set_short_Cylinder_size()
// undoes this magnifications and returns the Cylinders to sizes
// reasonably appropriate for tracking cars when the virtual camera
// has zoomed in to the city block level.

void CylindersGroup::set_tall_Cylinder_size()
{

// In order to avoid flickering, we repeat *Cylinder_ptr's current
// position for a few time steps into the future:

   int n_repeats=3;
   set_size(10.0, 10.0 , 60.0, 60.0 , n_repeats);
}

void CylindersGroup::set_short_Cylinder_size()
{

// In order to avoid flickering, we repeat *Cylinder_ptr's current
// position for a few time steps into the future:

   int n_repeats=3;
   set_size(1.0 , 1.0 , 1.0 , 10.0, n_repeats);
}

// --------------------------------------------------------------------------
// Member function update_track_posn() takes in the track_ID, time and
// posn coordinates for some ground mover.  If a Cylinder
// corresponding to the track ID doesn't already exist, this method
// generates a new one.  It then updates the Cylinder's height based
// upon member boolean tall_RTPS_size_flag.  It also resets the
// Cylinder's and corresponding track's UTM position.

void CylindersGroup::update_track_posn(
   int track_ID,double time,double X,double Y,double Z)
{
//   cout << "inside CylindersGroup::update_track_posn()" << endl;

   Cylinder* Cylinder_ptr=get_ID_labeled_Cylinder_ptr(track_ID);
   if (Cylinder_ptr==NULL)
   {
      osg::Quat trivial_q(0,0,0,1);
      int n_text_messages=1;
      threevector text_displacement(
         get_radius()+20,get_radius()+5,get_height()+10);
      double text_size=5*get_radius();
      bool text_screen_axis_alignment_flag=true;

      double alpha_permanent=0.20;
      double alpha_selected=0.20;
      colorfunc::Color permanent_cylinder_color=colorfunc::red;
      colorfunc::Color selected_cylinder_color=colorfunc::pink;

      Cylinder_ptr=generate_new_Cylinder(
         Zero_vector,trivial_q,permanent_cylinder_color,
         text_displacement,n_text_messages,text_size,
         text_screen_axis_alignment_flag,track_ID); 

      Cylinder_ptr->set_permanent_color(
         colorfunc::get_OSG_color(
            permanent_cylinder_color,alpha_permanent));
      Cylinder_ptr->set_selected_color(
         colorfunc::get_OSG_color(
            selected_cylinder_color,alpha_selected));

      string text_label=stringfunc::number_to_string(Cylinder_ptr->get_ID());
      Cylinder_ptr->set_text_label(0,text_label);
      Cylinder_ptr->set_text_color(0,permanent_cylinder_color);

      Cylinder_ptr->set_stationary_Graphical_flag(false);
   } // Cylinder_ptr==NULL conditional

   if (tall_RTPS_size_flag)
   {
      set_tall_Cylinder_size();
   }
   else
   {
      set_short_Cylinder_size();
   }

   threevector curr_track_posn(X,Y,Z);
   threevector curr_velocity(0,0,0);

   track* track_ptr=Cylinder_ptr->get_track_ptr();
   track_ptr->set_posn_velocity(time,curr_track_posn,curr_velocity);

   double curr_t=AnimationController_ptr->
      get_frame_corresponding_to_elapsed_secs();

// In order to avoid flickering, we repeat *Cylinder_ptr's current
// position for a few time steps into the future:

   const int n_repeats=3;
//      const int n_repeats=4;
   for (int r=0; r<n_repeats; r++)
   {
      Cylinder_ptr->set_UVW_coords(
         curr_t+r,get_passnumber(),curr_track_posn);
   }

//      cout << "track_ID = " << track_ID 
//           << " time = " << time
//           << " X = " << X << " Y = " << Y << endl;
//      cout << "n_cyls = " << get_n_Graphicals() 
//      cout << " curr_t = " << curr_t 
//           << " get_curr_t() = " << get_curr_t() << endl;

} 

// --------------------------------------------------------------------------
// Member function update_blueforce_car_posn() takes in time and
// position metadata presumably transmited by an iPhone.  If a (blue
// colored) Cylinder corresponding to the iPhone doesn't already
// exist, this method generates one.  It then updates the position of
// the Cylinder and its corresponding track.

void CylindersGroup::update_blueforce_car_posn(
   int blueforce_track_ID,double elapsed_secs,
   double longitude,double latitude,double altitude,
   int specified_UTM_zonenumber)
{
//   cout << "inside CylindersGroup::update_blueforce_car_posn()" << endl;
//   cout << "blueforce_track_ID = " << blueforce_track_ID << endl;
//   cout << "elapsed_secs = " << elapsed_secs
//        << " longitude = " << longitude << " latitude = " << latitude
//        << endl;
//   cout << "specified_UTM_zonenumber = " << specified_UTM_zonenumber
//        << endl;

   double time=elapsed_secs;
   geopoint curr_blueforce_geopoint(
      longitude,latitude,altitude,specified_UTM_zonenumber);
//   cout << "curr_blueforce_geopoint = " << curr_blueforce_geopoint << endl;

   Cylinder* Cylinder_ptr=get_ID_labeled_Cylinder_ptr(blueforce_track_ID);
   if (Cylinder_ptr==NULL)
   {
      osg::Quat trivial_q(0,0,0,1);
      int n_text_messages=1;
      threevector text_displacement(
         get_radius()+20,get_radius()+5,get_height()+10);
      double text_size=5*get_radius();
      bool text_screen_axis_alignment_flag=true;

      double alpha_permanent=0.20;
      double alpha_selected=0.20;
      colorfunc::Color permanent_cylinder_color=colorfunc::blue;
      colorfunc::Color selected_cylinder_color=colorfunc::cyan;

      Cylinder_ptr=generate_new_Cylinder(
         Zero_vector,trivial_q,permanent_cylinder_color,
         text_displacement,n_text_messages,text_size,
         text_screen_axis_alignment_flag,blueforce_track_ID); 

      Cylinder_ptr->set_permanent_color(colorfunc::get_OSG_color(
         permanent_cylinder_color,alpha_permanent));
      Cylinder_ptr->set_selected_color(colorfunc::get_OSG_color(
         selected_cylinder_color,alpha_selected));

      string text_label=stringfunc::number_to_string(Cylinder_ptr->get_ID());
      Cylinder_ptr->set_text_label(0,text_label);
      Cylinder_ptr->set_text_color(0,permanent_cylinder_color);

      Cylinder_ptr->set_stationary_Graphical_flag(false);
   } // Cylinder_ptr==NULL conditional

   if (tall_RTPS_size_flag)
   {
      set_tall_Cylinder_size();
   }
   else
   {
      set_short_Cylinder_size();
   }
   
   threevector curr_track_posn(
      curr_blueforce_geopoint.get_UTM_posn());
   threevector curr_velocity(0,0,0);

   track* track_ptr=Cylinder_ptr->get_track_ptr();
//   cout << "track_ptr = " << track_ptr << endl;
   track_ptr->set_posn_velocity(time,curr_track_posn,curr_velocity);

   double curr_t=AnimationController_ptr->
      get_frame_corresponding_to_elapsed_secs();

// In order to avoid flickering, we repeat *Cylinder_ptr's current
// position for a few time steps into the future:

   const int n_repeats=3;
//   const int n_repeats=4;
   for (int r=0; r<n_repeats; r++)
   {
      Cylinder_ptr->set_UVW_coords(curr_t+r,get_passnumber(),curr_track_posn);
   }

//      cout << "blueforce_track_ID = " << blueforce_track_ID 
//           << " time = " << time
//           << " X = " << X << " Y = " << Y << endl;
//      cout << "n_cyls = " << get_n_Graphicals()  
//      cout << " curr_t = " << curr_t 
//           << " get_curr_t() = " << get_curr_t() 
//           << endl;

}
