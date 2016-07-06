// ==========================================================================
// MODELSGROUP class member function definitions
// ==========================================================================
// Last modified on 6/15/08; 7/1/08; 1/6/11
// ==========================================================================

#include <set>
#include <iomanip>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/constant_vectors.h"
#include "osg/osgEarth/EarthRegion.h"
#include "geometry/ellipse.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mathfuncs.h"
#include "osg/osgModels/ModelsGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ModelsGroup::allocate_member_objects()
{
}		       

void ModelsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ModelsGroup";

   model_filename="";
   model_counter=0;
   PolyLinesGroup_ptr=NULL;
   get_OSGgroup_ptr()->setUpdateCallback(new AbstractOSGCallback<ModelsGroup>(
      this,&ModelsGroup::update_display));
}		       

ModelsGroup::ModelsGroup(Pass* PI_ptr,threevector* GO_ptr,
                         AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ModelsGroup::ModelsGroup(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
                         AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   PolyLinesGroup_ptr=PLG_ptr;
   PolyLinesGroup_ptr->set_width(3);
}		       

ModelsGroup::ModelsGroup(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,threevector* GO_ptr,
   AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   PolyLinesGroup_ptr=PLG_ptr;
   PolyLinesGroup_ptr->set_width(3);
}		       

ModelsGroup::~ModelsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ModelsGroup& M)
{
   int node_counter=0;
   for (unsigned int n=0; n<M.get_n_Graphicals(); n++)
   {
      Model* Model_ptr=M.get_Model_ptr(n);
      outstream << "Model node # " << node_counter++ << endl;
      outstream << "Model = " << *Model_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Model creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Model from all other graphical insertion
// and manipulation methods...

Model* ModelsGroup::generate_new_Model(
   string filename,int OSGsubPAT_number,int ID)
{
   model_filename=filename;
   return generate_new_Model(OSGsubPAT_number,ID);
}

Model* ModelsGroup::generate_new_Model(int OSGsubPAT_number,int ID)
{
   if (ID==-1) ID=get_next_unused_ID();

   if (model_filename=="")
   {
      cout << "Error in ModelsGroup::generate_new_Model()" << endl;
      cout << "No model filename specified!" << endl;
      return NULL;
   }
   
   Model* curr_Model_ptr=new Model(
      get_pass_ptr(),get_grid_world_origin_ptr(),AnimationController_ptr,
      model_filename,ID);
   initialize_Graphical(curr_Model_ptr);
   GraphicalsGroup::insert_Graphical_into_list(curr_Model_ptr);

// Add both model's PAT and OSGgroup for model's ObsFrustum to
// ModelGroup's OSGsubPAT labeled by input OSGsubPAT_number:

   curr_Model_ptr->get_PAT_ptr()->addChild(
      curr_Model_ptr->get_model_node_ptr());
   insert_graphical_PAT_into_OSGsubPAT(curr_Model_ptr,OSGsubPAT_number);

   return curr_Model_ptr;
}

// ---------------------------------------------------------------------
// Member function instantiate_ObsFrustum appends a new ObsFrustum to
// *curr_Model_ptr's ObsFrustaGroup.  It then incorporates the
// ObsFrustaGroup's OSGgroup_ptr to the current ModelsGroup's
// OSGsubPAT whose number is passed as an input parameter.

ObsFrustum* ModelsGroup::instantiate_ObsFrustum(
   Model* Model_ptr,double az_extent,double el_extent,
   int OSGsubPAT_number)
{
   ObsFrustum* ObsFrustum_ptr=Model_ptr->
      get_ObsFrustaGroup_ptr()->generate_new_ObsFrustum(az_extent,el_extent);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->addChild(
      Model_ptr->get_ObsFrustaGroup_ptr()->get_OSGgroup_ptr());
   return ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
// Member function move_z vertically displaces the model in proportion
// to its current Z value.  This allows the cessna model's altitude to
// be rapidly but grossly reset when the aircraft is high above the
// Z=0 plane.  On the other hand, vertical displacement of the
// airplane becomes more fine as its height approaches Z=0.

Model* ModelsGroup::move_z(int sgn)
{
   Model* curr_model_ptr=get_ID_labeled_Model_ptr(
      get_selected_Graphical_ID());

   threevector curr_posn;
   if (curr_model_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),curr_posn))
   {
      double delta_z=0.05*curr_posn.get(2);
      return dynamic_cast<Model*>(GraphicalsGroup::move_z(sgn*delta_z));
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function change_scale

void ModelsGroup::change_scale(double scalefactor)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
//      Model* curr_model_ptr=get_ID_labeled_Model_ptr(
//         get_selected_Graphical_ID());
      Model* curr_model_ptr=get_Model_ptr(n);

      threevector scale;
      if (curr_model_ptr != NULL &&
          curr_model_ptr->get_scale(get_curr_t(),get_passnumber(),scale))
      {
         for (int i=AnimationController_ptr->get_first_framenumber();
              i <= AnimationController_ptr->get_last_framenumber(); i++)
         {
            double curr_t=double(i);
            curr_model_ptr->set_scale(
               curr_t,get_passnumber(),scalefactor*scale);
         }
      }
   } // loop over index n labeling models
}

// ---------------------------------------------------------------------
// Member function unmask_next_model was written for demonstration
// purposes.  For clarity's sake, it's useful to start the San
// Clemente tour with both the LiMIT and predator models masked off.
// Then we can call this method once to turn on the LiMIT model and
// once again to turn on the predator model.  The order in which the
// models are instantiated within the main program determines the
// order in which they are unmasked by this method.

void ModelsGroup::unmask_next_model()
{
   if (model_counter >= get_n_OSGsubPATs()) model_counter=0;
   set_OSGsubPAT_nodemask(model_counter++,1);
   update_display();
}		       

// --------------------------------------------------------------------------
// Member function update_display is repeatedly executed by a callback
// in a main program.

void ModelsGroup::update_display()
{   
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Model path creation methods
// ==========================================================================

void ModelsGroup::record_waypoint()
{
   Model* curr_model_ptr=get_ID_labeled_Model_ptr(
      get_selected_Graphical_ID());

   threevector curr_posn;
   if (curr_model_ptr != NULL && curr_model_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),curr_posn))
   {
      candidate_waypoint.push_back(curr_posn);
      cout << "i = " << candidate_waypoint.size()
           << " waypoint-grid_origin = " 
           << candidate_waypoint.back()-get_grid_world_origin() << endl;
   }
}

// ---------------------------------------------------------------------
void ModelsGroup::finish_waypoint_entry()
{
//   cout << "inside ModelsGroup::finish_waypoint_entry()" << endl;

   waypoint.clear();
   for (unsigned int i=0; i<candidate_waypoint.size(); i++)
   {
      waypoint.push_back(candidate_waypoint[i]);
   }
   candidate_waypoint.clear();

   Model* curr_model_ptr=get_ID_labeled_Model_ptr(
      get_selected_Graphical_ID());

   PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
      waypoint);

   polyline* curr_polyline_ptr=curr_PolyLine_ptr->construct_polyline();
   curr_polyline_ptr->compute_regularly_spaced_edge_points(
      curr_model_ptr->get_path_update_distance(),even_path_point);

   double roll=0;
   double pitch=0;
   const threevector north_hat(y_hat);
   vector<threevector> RPY;
   for (unsigned int i=0; i<even_path_point.size()-1; i++)
   {
      threevector ds(even_path_point[i+1]-even_path_point[i]);

      pair<double,threevector> p=mathfunc::angle_and_axis_between_unitvectors(
         north_hat,ds.unitvector());
      double yaw=p.first;
      double dotproduct=z_hat.dot(p.second);
      yaw *= sgn(dotproduct);

      RPY.push_back(threevector(roll,pitch,yaw));
   }

// Assume aircraft's attitude at last sample point is the same as at
// its next-to-last sample point:

   RPY.push_back(RPY.back());

// On 11/3/06, we learned the painful and hard way that model scale
// information must be explicitly expressed for each temporal sample
// in addition to model attitude and position :

   threevector trivial_scale(1,1,1);
   curr_model_ptr->set_scale_attitude_posn(
      get_passnumber(),trivial_scale,RPY,even_path_point);

// Set number of frames within Animation Controller equal to number of
// evenly sampled path points:

   AnimationController_ptr->set_nframes(even_path_point.size());
}

// ---------------------------------------------------------------------
// Member function generate_racetrack_orbit takes in parameters which
// uniquely define a circular racetrack orbit.  It computes the
// model's XYZ position and roll, pitch, yaw orientation (in radians)
// as functions of orbit position.  This method returns the number of
// frames which the model takes to complete the orbit.

void ModelsGroup::generate_racetrack_orbit(
   Model* curr_model_ptr,double center_longitude,double center_latitude,
   double radius,double height_above_center,
   double orbit_period,double phase_offset)
{
   geopoint racetrack_center(center_longitude,center_latitude);

//   cout << "racetrack_center = " << racetrack_center << endl;
   threevector center_origin(
      racetrack_center.get_UTM_easting(),
      racetrack_center.get_UTM_northing(),0);
   generate_racetrack_orbit(
      curr_model_ptr,center_origin,radius,height_above_center,
      orbit_period,phase_offset);
}

void ModelsGroup::generate_racetrack_orbit(
   Model* curr_model_ptr,const threevector& center_origin,
   double radius,double height_above_center,double orbit_period,
   double phase_offset)
{
   double t_start=0;
   double dt=1;
   double omega=2*PI/orbit_period;

   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double t=t_start+n*dt;
      double theta=omega*t+phase_offset;
      double x=center_origin.get(0)+radius*cos(theta);
      double y=center_origin.get(1)+radius*sin(theta);
      double z=center_origin.get(2)+height_above_center;
      threevector posn(x,y,z);

      double roll=0;	      	// radians
      double pitch=0;		// radians
      double yaw=theta;	// radians
      threevector RPY(roll,pitch,yaw);

      curr_model_ptr->set_attitude_posn(n,get_passnumber(),RPY,posn);
   } // loop over index n labeling angular posn along racetrack orbit
}

// ==========================================================================
// ISDS demo methods
// ==========================================================================

// Member function generate_predator_and_LiMIT_Models is a specialized
// method developed specifically for the ISDS integrated demo in
// February 2007.  It puts predator and AWACS aircraft models into
// notional racetrack orbits.

Model* ModelsGroup::generate_predator_Model(int& OSGsubPAT_number)
{
   string UAV_dir(getenv("OSG_FILE_PATH"));
   UAV_dir += "/Predator/";
   string model_filename=UAV_dir+"mypredator.osg";
   OSGsubPAT_number=get_n_OSGsubPATs();
   Model* predator_ptr=generate_new_Model(model_filename,OSGsubPAT_number);
   return predator_ptr;
}

Model* ModelsGroup::generate_LiMIT_Model(int& OSGsubPAT_number)
{
   string LiMIT_dir(getenv("OSG_FILE_PATH"));
   LiMIT_dir += "/AWACS/MESHES/";
   model_filename=LiMIT_dir+"AWACS.osg";
   OSGsubPAT_number=get_n_OSGsubPATs();
   Model* LiMIT_ptr=generate_new_Model(model_filename,OSGsubPAT_number);
   return LiMIT_ptr;
}

// ---------------------------------------------------------------------
void ModelsGroup::generate_predator_racetrack_orbit(
   int n_total_frames,Model* predator_ptr)
{
   string banner="Geerating predator racetrack orbit";
   outputfunc::write_banner(banner);

   AnimationController_ptr->set_nframes(n_total_frames);

   double prefactor=1.0;
   double radius=prefactor * 6 * 1000;			// meters
   double height_above_center=prefactor * 5 * 1000;	// meters
   double phase_offset=0*PI/180;	// rads
   double orbit_period=0.5*n_total_frames;
   threevector racetrack_center(0,0,0);
   generate_racetrack_orbit(
      predator_ptr,racetrack_center,radius,height_above_center,
      orbit_period,phase_offset);
   const double predator_scale_factor=30;
   set_constant_scale(predator_ptr,predator_scale_factor);
}

// ---------------------------------------------------------------------
void ModelsGroup::generate_predator_racetrack_orbit(
   int n_total_frames,double racetrack_center_longitude,
   double racetrack_center_latitude,Model* predator_ptr)
{
   string banner="Generating predator racetrack orbit";
   outputfunc::write_banner(banner);
   AnimationController_ptr->set_nframes(n_total_frames);

   double prefactor=1.0;
   double radius=prefactor * 6 * 1000;			// meters
   double height_above_center=prefactor * 5 * 1000;	// meters
   double phase_offset=0*PI/180;	// rads
   double orbit_period=0.5*n_total_frames;
   generate_racetrack_orbit(
      predator_ptr,racetrack_center_longitude,racetrack_center_latitude,
      radius,height_above_center,orbit_period,phase_offset);
   const double predator_scale_factor=30;
   set_constant_scale(predator_ptr,predator_scale_factor);
}

// ---------------------------------------------------------------------
// Member function generate_elliptical_LiMIT_racetrack_orbit is a
// specialized method which produces a continuous racetrack orbit for
// LiMIT that closely matches the discontinuous polygonal orbit
// implemented by Virtual Hammer in the Feb 2007 integrated ISDS demo.
// The parameters for the ellipse were calculated using main program
// isds/LiMIT_orbit and hardwired into this method.

void ModelsGroup::generate_elliptical_LiMIT_racetrack_orbit(
   int n_total_frames,const geopoint& racetrack_center,Model* LiMIT_ptr)
{
   threevector center(
      racetrack_center.get_UTM_easting(),racetrack_center.get_UTM_northing(),
      racetrack_center.get_altitude());
   generate_elliptical_LiMIT_racetrack_orbit(n_total_frames,center,LiMIT_ptr);
}

void ModelsGroup::generate_elliptical_LiMIT_racetrack_orbit(
   int n_total_frames,const threevector& racetrack_center,Model* LiMIT_ptr)
{
   string banner="Generating elliptical LiMIT racetrack orbit";
   outputfunc::write_banner(banner);
   AnimationController_ptr->set_nframes(n_total_frames);

   const double a=35851.7620423408;
   const double b=28737.783019816;
   const double theta = 113.919717495022*PI/180;
   ellipse racetrack_orbit(racetrack_center,a,b,theta);

   const double phase_offset=180*PI/180;
   vector<threevector> vertex=racetrack_orbit.generate_vertices(
      n_total_frames,phase_offset);

   const double altitude_above_center=11000;	// meters
   const double omega=2*PI/n_total_frames;

   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double curr_t=n;
      double curr_phi=omega*curr_t+phase_offset;
      threevector posn=vertex[n]+altitude_above_center*z_hat;

      double roll=0;	      	// radians
      double pitch=0;		// radians
      double yaw=atan2(b*cos(curr_phi),-a*sin(curr_phi))-PI/2+theta;
      threevector RPY(roll,pitch,yaw);

      LiMIT_ptr->set_attitude_posn(curr_t,get_passnumber(),RPY,posn);
   } // loop over index n labeling angular posn along racetrack orbit

   const double LiMIT_scale_factor=30;
   set_constant_scale(LiMIT_ptr,LiMIT_scale_factor);
}

// ---------------------------------------------------------------------
// Member function generate_predator_[LiMIT]_ObsFrusta are specialized
// methods developed specifically for the ISDS integrated demo in
// February 2007.  They attach ObsFrusta to models squinted off from
// aircrafts' velocity vectors by some fixed direction.

ObsFrustum* ModelsGroup::generate_predator_ObsFrustrum(
   int predator_OSGsubPAT_number,Model* predator_ptr)
{
   string banner="Generating predator ObsFrustum";
   outputfunc::write_banner(banner);
//   cout << "inside ModelsGroup::generate_predator_ObsFrustrum()" << endl;

   double az_extent=10*PI/180;  // rads
   double el_extent=10*PI/180;  // rads
   ObsFrustum* ObsFrustum_ptr=instantiate_ObsFrustum(
      predator_ptr,az_extent,el_extent,predator_OSGsubPAT_number);
   ObsFrustum_ptr->set_color(colorfunc::get_OSG_color(colorfunc::cyan));
   predator_ptr->get_ObsFrustaGroup_ptr()->reset_colors();

   get_OSGsubPAT_ptr(predator_OSGsubPAT_number)->addChild(
      predator_ptr->get_ObsFrustaGroup_ptr()->get_OSGgroup_ptr());

   double delta_phi=90*PI/180;
   double delta_theta = -45*PI/180;
   double z_offset=100;	// meters
   bool sinusoidal_variation_flag=true;
   predator_ptr->orient_and_position_ObsFrustum(
      get_first_framenumber(),get_last_framenumber(),get_passnumber(),
      delta_phi,delta_theta,z_offset,sinusoidal_variation_flag);
   return ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
ObsFrustum* ModelsGroup::generate_LiMIT_FOV_ObsFrustrum(
   int LiMIT_OSGsubPAT_number,Model* LiMIT_ptr)
{
   string banner="Generating LiMIT FOV ObsFrustum";
   outputfunc::write_banner(banner);
//   cout << "inside ModelsGroup::generate_LiMIT_FOV_ObsFrustrum()" << endl;

// On 2/16/07, Peter Jones told us that the GMTI viewing bounds for
// the electronically steerable array onboard the LiMIT platform
// within the ISDS integrated demo (as of Feb 2007) are +/- 30 degs
// from broadside in azimuth and 16-45 degrees in declination.  

   double FOV_az_extent=60*PI/180;  // rads
   double FOV_el_extent=29*PI/180;  // rads
   ObsFrustum* FOV_ObsFrustum_ptr=
      instantiate_ObsFrustum(LiMIT_ptr,FOV_az_extent,FOV_el_extent,
                             LiMIT_OSGsubPAT_number);
   FOV_ObsFrustum_ptr->set_color(colorfunc::get_OSG_color(colorfunc::pink));
   LiMIT_ptr->get_ObsFrustaGroup_ptr()->reset_colors();

   double delta_phi=90*PI/180;
   double delta_theta = -30.5*PI/180;
   double z_offset=100;   // meters
   bool sinusoidal_variation_flag=false;
   LiMIT_ptr->orient_and_position_ObsFrustum(
      get_first_framenumber(),get_last_framenumber(),
      get_passnumber(),delta_phi,delta_theta,z_offset,
      sinusoidal_variation_flag,FOV_ObsFrustum_ptr->get_ID());

   get_OSGsubPAT_ptr(LiMIT_OSGsubPAT_number)->addChild(
      LiMIT_ptr->get_ObsFrustaGroup_ptr()->get_OSGgroup_ptr());

   update_display();
   return FOV_ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
ObsFrustum* ModelsGroup::generate_LiMIT_instantaneous_dwell_ObsFrustrum(
   int LiMIT_OSGsubPAT_number,Model* LiMIT_ptr,
   ObsFrustum* FOV_ObsFrustum_ptr,EarthRegion* GMTI_region_ptr)
{
   string banner="Generating LiMIT instantaneous dwell ObsFrustum";
   outputfunc::write_banner(banner);
//   cout << "inside ModelsGroup::generate_LiMIT_instantaneous_dwell_ObsFrustrum()"
//        << endl;

   if (GMTI_region_ptr->get_GMTI_targets().size() > 0)
   {
      ObsFrustum* instantaneous_dwell_ObsFrustum_ptr=
         instantiate_ObsFrustum(LiMIT_ptr,0,0,LiMIT_OSGsubPAT_number);

      instantaneous_dwell_ObsFrustum_ptr->set_color(
         colorfunc::get_OSG_color(colorfunc::red));
      LiMIT_ptr->get_ObsFrustaGroup_ptr()->reset_colors();

      double z_offset=50;   // meters

      LiMIT_ptr->orient_and_position_instantaneous_dwell_ObsFrustum(
         get_first_framenumber(),get_last_framenumber(),
         get_passnumber(),z_offset,
         instantaneous_dwell_ObsFrustum_ptr,FOV_ObsFrustum_ptr,
         GMTI_region_ptr->get_GMTI_targets());

      update_display();
      return instantaneous_dwell_ObsFrustum_ptr;
   }
   else
   {
      return NULL;
   }
}

// ==========================================================================
// HAFB video3D methods
// ==========================================================================

// Member function generate_HAFB_video_pass_model is a specialized,
// high-level method which returns a Cessna model with an ObsFrustum
// that displays the Group 99 HAFB video at its base.  Both the Model
// and the ObsFrustum are placed within a new OSGsubPAT of this
// ModelsGroup object.  The flight path is rotated away from its true
// direction over HAFB by input angle z_rot_angle.  (The true flight
// direction made an angle of approximately -60 degs relative to
// east).  The desired, absolute XY location of the aircraft for the
// first frame within the HAFB video pass is contained within input
// threevector first_frame_aircraft_xy_posn.

Model* ModelsGroup::generate_HAFB_video_pass_model(
   double z_rot_angle,const threevector& first_frame_aircraft_posn,
   AnimationController* HAFB_AnimationController_ptr)
{
   string subdir="/home/cho/programs/c++/svn/projects/src/mains/fusion/";
   string model_filename="my_cessna.osg";
   model_filename=subdir+model_filename;

   int OSGsubPAT_number=get_n_OSGsubPATs();
   Model* curr_model_ptr=generate_new_Model(
      model_filename,OSGsubPAT_number);

// Initialize aircraft position and attitude as functions of HAFB
// video frame number:

   vector<threevector> plane_posn,plane_attitude;
   read_HAFB_plane_info(plane_posn,plane_attitude);
   const double cessna_scale_factor=5;
   curr_model_ptr->set_scale_attitude_posn(
      get_passnumber(),cessna_scale_factor,plane_attitude,plane_posn);

// Generate an ObsFrustum for the aircraft model.  Then insert HAFB
// video at its base:

   double z_offset=get_grid_world_origin().get(2);
   instantiate_HAFB_video_ObsFrustum(curr_model_ptr,plane_posn,z_offset,
                                     OSGsubPAT_number);

// Translate model from its relative XY posn within the first frame of
// original HAFB video pass to (0,0):

   osg::Vec3d init_trans(-47172.19,1751.168,0); 
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPivotPoint(init_trans);

// Rotate aircraft model (along with its ObsFrustum and associated
// movie) about (0,0) by angle z_rot_angle:

   const osg::Vec3f Z_hat(0,0,1);
   osg::Quat quat;
   quat.makeRotate(z_rot_angle,Z_hat);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setAttitude(quat);

// Translate model from (0,0) to first_frame_aircraft_posn:

   osg::Vec3d final_trans(
      first_frame_aircraft_posn.get(0),
      first_frame_aircraft_posn.get(1),
      first_frame_aircraft_posn.get(2));
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPosition(final_trans);

   return curr_model_ptr;
}

// ---------------------------------------------------------------------
// Member function instantiate_HAFB_video_ObsFrustum

ObsFrustum* ModelsGroup::instantiate_HAFB_video_ObsFrustum(
   Model* Model_ptr,const vector<threevector>& aircraft_posn,double z_offset,
   int OSGsubPAT_number)
{
   ObsFrustum* ObsFrustum_ptr=Model_ptr->get_ObsFrustaGroup_ptr()->
      generate_HAFB_movie_ObsFrustum(aircraft_posn,z_offset);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->addChild(
      Model_ptr->get_ObsFrustaGroup_ptr()->get_OSGgroup_ptr());

   return ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
void ModelsGroup::read_HAFB_plane_info(
   vector<threevector>& plane_posn,vector<threevector>& plane_attitude)
{

// Read in HAFB aircraft filtered position and attitude as functions
// of time:

   string subdir="/home/cho/programs/c++/svn/projects/src/mains/fusion/";
   string TPA_filename="TPA_filtered.txt";
   TPA_filename=subdir+TPA_filename;
   filefunc::ReadInfile(TPA_filename);

// Recall first image in HAFB_overlap_corrected_grey.vid actually
// corresponds to tiepoint_imagenumber 300 in uncut HAFB.vid file:

   const int imagenumber_offset=300;	

   const int n_fields=8;
   double X[n_fields];
   for (unsigned int i=imagenumber_offset; i<filefunc::text_line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      plane_posn.push_back(threevector(X[2],X[3],X[4]));
      plane_attitude.push_back(
         threevector(X[5]*PI/180,X[6]*PI/180,X[7]*PI/180));
   }
}
