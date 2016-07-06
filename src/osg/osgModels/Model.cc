// ==========================================================================
// Model class member function definitions
// ==========================================================================
// Last updated on 2/20/07; 2/21/07; 8/27/07; 12/23/07; 2/10/08
// ==========================================================================

#include <iostream>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile> 
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "osg/osgModels/Model.h"
#include "passes/Pass.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Model::allocate_member_objects()
{
   group_refptr=new osg::Group();
}		       

void Model::initialize_member_objects()
{
   Graphical_name="Model";
   path_update_distance=10;	// meters
//   path_update_distance=100;	// meters
   ObsFrustaGroup_ptr=NULL;
}		       

Model::Model(threevector* GO_ptr,string filename,int id):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();
   model_filename=filename;
   model_node_refptr=osgDB::readNodeFile(model_filename);
}		       

Model::Model(Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr,
             string filename,int id):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();
   model_filename=filename;
   model_node_refptr=osgDB::readNodeFile(model_filename);

   ObsFrustaGroup_ptr=new ObsFrustaGroup(PI_ptr,AC_ptr,GO_ptr);
}		       

Model::~Model()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Model& m)
{
   outstream << "inside Model::operator<<" << endl;
   return(outstream);
}


// ==========================================================================
// Model manipulation methods
// ==========================================================================

// Member function set_scale_attitude_posn takes in a time and pass
// number along with a corresponding scale, roll, pitch, yaw and XYZ
// position for the model.  This information is stored for later
// callback retrieval.

void Model::set_attitude_posn(
   double curr_t,int pass_number,
   const threevector& RPY,const threevector& posn)
{
//   cout << "inside Model::set_attitude_posn()" << endl;
   double init_yaw_angle=180*PI/180;
   Graphical::set_attitude_posn(
      curr_t,pass_number,RPY,posn,init_yaw_angle);
}

// ==========================================================================
// Observation frusta instantiation and manipulation member functions
// ==========================================================================

// Member function orient_and_position_ObsFrustum rotates the
// ObsFrustum object labeled by input integer ObsFrustum_ID away from
// its canonical orientation along +xhat towards specified azimuth and
// elevation angle settings wrt the model's velocity vector.  It also
// translates the ObsFrustum from its initial (0,0,0) origin to the
// model's current position.

void Model::orient_and_position_ObsFrustum(
   int first_framenumber,int last_framenumber,
   int pass_number,double delta_phi,double delta_theta,
   double z_offset,bool sinusoidal_variation_flag,int ObsFrustum_ID)
{     
//   cout << "inside Model::orient_and_position_ObsFrustum()" << endl;

   const double dt=1.0;
   for (int i=first_framenumber; i<=last_framenumber; i++)
   {
      double curr_t=static_cast<double>(i);

// If sinusoidal_variation_flag==true, let delta_phi and delta_theta
// vary sinusoidally with time in order to emulate quasi-random
// camera slewing onboard a UAV:

      if (sinusoidal_variation_flag)
      {
         const double phi_max=110*PI/180;
         const double phi_min=60*PI/180;
         const double theta_max=-35*PI/180;
         const double theta_min=-60*PI/180;

         int n_frames=last_framenumber-first_framenumber+1;
         double omega=2*PI/double(n_frames);
         delta_phi=phi_min+(phi_max-phi_min)*
            0.5*(1+sin(3*omega*curr_t));
         delta_theta=theta_min+(theta_max-theta_min)*
            0.5*(1+sin(3*omega*curr_t));
      }
      
      instantaneously_orient_and_position_ObsFrustum(
         curr_t,pass_number,dt,delta_phi,delta_theta,
         z_offset,ObsFrustum_ID);
   } // loop over index i labeling frame number
}

// ----------------------------------------------------------------
void Model::instantaneously_orient_and_position_ObsFrustum(
   double curr_t,int pass_number,double dt,
   double delta_phi,double delta_theta,double z_offset,int ObsFrustum_ID)
{
//   cout << "inside Model::instantaneously_orient_and_posn_OF, this = "
//        << this << endl;
//   cout << "z_offset = " << z_offset << endl;
   
   ObsFrustum* ObsFrustum_ptr=ObsFrustaGroup_ptr->
      get_ID_labeled_ObsFrustum_ptr(ObsFrustum_ID);
   
   threevector curr_model_posn;
   if (!get_UVW_coords(curr_t,pass_number,curr_model_posn))
   {
      curr_model_posn=Zero_vector;
   }
   
   threevector curr_model_velocity;
   if (!get_velocity(curr_t,pass_number,dt,curr_model_velocity))
   {
      curr_model_velocity=x_hat;
   }

// First rotate x_hat, y_hat and z_hat so that x_hat is aligned with
// current velocity vector:

   threevector v_hat=curr_model_velocity.unitvector();
   threevector w_hat,u_hat;
   mathfunc::generate_orthogonal_basis(v_hat,w_hat,u_hat);

   double theta=asin(v_hat.get(2));
   double phi=0;
   if (!nearly_equal(fabs(v_hat.get(2)),1))
   {
      phi=atan2(v_hat.get(1),v_hat.get(0));
   }

// Squint observation frustum away from model's velocity vector in
// azimuth by delta_phi and in elevation by delta_theta:
   
   phi += delta_phi;
   theta += delta_theta;

   double cos_theta=cos(theta);
   double sin_theta=sin(theta);
   double cos_phi=cos(phi);
   double sin_phi=sin(phi);
   threevector n_hat(cos_theta*cos_phi,cos_theta*sin_phi,sin_theta);
   ObsFrustum_ptr->build_current_frustum(
      curr_t,pass_number,curr_model_posn,n_hat,z_offset);
   ObsFrustum_ptr->set_stationary_Graphical_flag(false);
}

// ----------------------------------------------------------------
void Model::orient_and_position_instantaneous_dwell_ObsFrustum(
   int first_framenumber,int last_framenumber,int pass_number,double z_offset,
   ObsFrustum* dwell_ObsFrustum_ptr,ObsFrustum* FOV_ObsFrustum_ptr,
   const vector<geopoint>& GMTI_targets)
{     
//   cout << "inside Model::orient_and_posn_inst_dwell_ObsFrustum_2"
//        << endl;

   double dwell_range_extent=4000;		// meters
   double dwell_crossrange_extent=2000;		// meters

   const double max_dwell_time=8;
   double dwell_time=0;

   int GMTI_target_number=0;
   for (int i=first_framenumber; i<=last_framenumber; i++)
   {
      double curr_t=static_cast<double>(i);
      if (dwell_time > max_dwell_time)
      {
         dwell_time=0;
         GMTI_target_number=modulo(GMTI_target_number+1,GMTI_targets.size());
      }

      bool GMTI_dwell_handled=false;
      int n_GMTI_target_dwells_attempted=0;
      do
      {
         geopoint GMTI_target=GMTI_targets[GMTI_target_number];
         threevector GMTI_posn(GMTI_target.get_UTM_easting(),GMTI_target.
                               get_UTM_northing(),0);
         GMTI_dwell_handled=
            instantaneously_orient_and_position_GMTI_dwell_ObsFrustum(
               curr_t,pass_number,z_offset,dwell_ObsFrustum_ptr,
               FOV_ObsFrustum_ptr,GMTI_posn,
               dwell_range_extent,dwell_crossrange_extent);
         if (!GMTI_dwell_handled)
         {
            GMTI_target_number=modulo(
               GMTI_target_number+1,GMTI_targets.size());
            n_GMTI_target_dwells_attempted++;
         }
      }
      while (!GMTI_dwell_handled && 
             n_GMTI_target_dwells_attempted < int(GMTI_targets.size()));
      
      dwell_time += 1;

   } // loop over index i labeling frame number
}

// ----------------------------------------------------------------
bool Model::instantaneously_orient_and_position_GMTI_dwell_ObsFrustum(
   double curr_t,int pass_number,double z_offset,
   ObsFrustum* dwell_ObsFrustum_ptr,ObsFrustum* FOV_ObsFrustum_ptr,
   const threevector& GMTI_posn,
   double dwell_range_extent,double dwell_crossrange_extent)
{
//   cout << "inside Model::instantaneously_orient_and_position_GMTI_dwell"
//        << endl;

   bool GMTI_dwell_handled=false;

   threevector camera_posn;
   if (get_UVW_coords(curr_t,pass_number,camera_posn))
   {
      dwell_ObsFrustum_ptr->set_UVW_coords(curr_t,pass_number,camera_posn);

// First check whether GMTI target lies inside FOV footprint.  We
// learned from Tony Filip on 2/21/07 that LiMIT will schedule a GMTI
// dwell on a ground target provided the target's location lies within
// the FOV footprint (and even if much of the dwell footprint lies
// outside the FOV footprint).

      polygon FOV_footprint=FOV_ObsFrustum_ptr->reconstruct_footprint(
         curr_t,pass_number);

      polygon dwell_footprint=
         dwell_ObsFrustum_ptr->GMTI_dwell_frustum_footprint(
            curr_t,pass_number,camera_posn,GMTI_posn,
            dwell_range_extent,dwell_crossrange_extent,z_offset);

      vector<threevector> UV_corner_dir;
      for (int c=0; c<4; c++)
      {
         threevector curr_ray=dwell_footprint.get_vertex(c)-camera_posn;
         UV_corner_dir.push_back( curr_ray.unitvector() );
      }

      if (!FOV_footprint.point_inside_polygon(GMTI_posn))
      {

// If dwell ObsFrustum should not be seen, render it with a very short
// altitude so that it's effectively invisible:

         dwell_ObsFrustum_ptr->build_current_frustum(
            curr_t,pass_number,camera_posn.get(2)-20,camera_posn,
            UV_corner_dir);
      }
      else
      {
         const double max_lambda=500000;	// max slant range = 500 km
         dwell_ObsFrustum_ptr->build_current_frustum(
            curr_t,pass_number,z_offset,camera_posn,UV_corner_dir,
            max_lambda);
         GMTI_dwell_handled=true;
      } // FOV_footprint contains GMTI target conditional
      dwell_ObsFrustum_ptr->set_stationary_Graphical_flag(false);

   } // camera's curr_posn known conditional

   return GMTI_dwell_handled;
}

   
