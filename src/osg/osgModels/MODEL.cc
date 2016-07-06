// ==========================================================================
// MODEL class member function definitions
// ==========================================================================
// Last updated on 9/30/11; 10/9/11; 10/12/11; 4/6/14
// ==========================================================================

#include <iostream>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile> 
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "geometry/face.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODEL.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "passes/Pass.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "image/raster_parser.h"
#include "math/rotation.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgWindow/WindowManager.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MODEL::allocate_member_objects()
{
}		       

void MODEL::initialize_member_objects()
{
   Graphical_name="MODEL";
   speed=-1;	// meters/sec
   path_update_distance=10;	// meters

   OBSFRUSTAGROUP_ptr=NULL;
   dynamically_compute_OBSFRUSTUM_flag=false;
   compute_zface_height_flag=false;
   raytrace_occluded_ground_regions_flag=false;
   raytrace_ROI_flag=true;
   WindowManager_ptr=NULL;
}		       

MODEL::MODEL(threevector* GO_ptr,string filename,int id):
   Geometrical(3,id)
{	
//   cout << "inside MODEL constructor #1" << endl;

   allocate_member_objects();
   initialize_member_objects();
   model_filename=filename;
   model_node_refptr=osgDB::readNodeFile(model_filename);
}		       

MODEL::MODEL(
   Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr,
   osgGA::Terrain_Manipulator* CM_3D_ptr,
   ModeController* ModeController_ptr,WindowManager* WindowManager_ptr,
   string filename,bool instantiate_OBSFRUSTAGROUP_flag,
   bool instantiate_OBSFRUSTUMPickHandler_flag,int id):
   Geometrical(3,id,AC_ptr)
{	
//   cout << "inside MODEL constructor #2" << endl;
//   cout << "filename = " << filename << endl;
//   cout << "instantiate_OBSFRUSTUMPickHandler_flag = "
//        << instantiate_OBSFRUSTUMPickHandler_flag << endl;

   allocate_member_objects();
   initialize_member_objects();
   model_filename=filename;
   model_node_refptr=osgDB::readNodeFile(model_filename);

//   cout << "model_node_refptr.valid() = "
//        << model_node_refptr.valid() << endl;

   if (instantiate_OBSFRUSTAGROUP_flag)
      OBSFRUSTAGROUP_ptr=new OBSFRUSTAGROUP(PI_ptr,CM_3D_ptr,AC_ptr,GO_ptr);
   
   this->WindowManager_ptr=WindowManager_ptr;

   if (WindowManager_ptr != NULL && instantiate_OBSFRUSTUMPickHandler_flag)
   {
      OBSFRUSTUMPickHandler_refptr=new OBSFRUSTUMPickHandler(
         PI_ptr,CM_3D_ptr,OBSFRUSTAGROUP_ptr,ModeController_ptr,
         WindowManager_ptr,GO_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         OBSFRUSTUMPickHandler_refptr.get());
   }
}		       

MODEL::~MODEL()
{
//   cout << "inside MODEL destructor" << endl;
   delete OBSFRUSTAGROUP_ptr;

   OBSFRUSTAGROUP_ptr=NULL;

//   cout << "WindowManager_ptr = " << WindowManager_ptr << endl;
   if (WindowManager_ptr != NULL)
   {
      WindowManager_ptr->remove_EventHandler_refptr(
         dynamic_cast<osgGA::GUIEventHandler*>(
            OBSFRUSTUMPickHandler_refptr.get()));
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const MODEL& m)
{
   outstream << "inside MODEL::operator<<" << endl;
   return(outstream);
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void MODEL::set_raytrace_occluded_ground_regions_flag(bool flag)
{
   cout << "inside MODEL::set_raytrace_occluded_ground_regions_flag()" << endl;
   cout << "this = " << this << endl;
   raytrace_occluded_ground_regions_flag=flag;
//   cout << "flag = " << flag << endl;
//   outputfunc::enter_continue_char();
}

void MODEL::set_OBSFRUSTUM_FOVs(
   unsigned int OBSFRUSTUM_ID,double horiz_FOV,double vert_FOV)
{
   if (OBSFRUSTUM_alpha.size() <= OBSFRUSTUM_ID)
   {
      cout << "Error in set_OBSFRUSTUM_FOVs()!" << endl;
      cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID
           << " OBSFRUSTUM_alpha.size() = " << OBSFRUSTUM_alpha.size()
           << endl;
      return;
   }

   double alpha,beta;
   OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles(
      horiz_FOV,vert_FOV,alpha,beta);

   OBSFRUSTUM_alpha[OBSFRUSTUM_ID]=alpha;
   OBSFRUSTUM_beta[OBSFRUSTUM_ID]=beta;
}

// ==========================================================================
// Model manipulation methods
// ==========================================================================

// This overloaded version of Graphical::set_attitude_posn() performs
// an initial rotation of the (cessna) model so that it points
// northward.  

void MODEL::set_attitude_posn(
   double curr_t,int pass_number,
   const threevector& RPY,const threevector& posn)
{
//   cout << "inside MODEL::set_attitude_posn() #1" << endl;
   double init_yaw_angle=180*PI/180;
   Graphical::set_attitude_posn(
      curr_t,pass_number,RPY,posn,init_yaw_angle);
}

// ==========================================================================
// Observation frusta instantiation and manipulation member functions
// ==========================================================================

// Member function orient_and_position_OBSFRUSTUM rotates the
// OBSFRUSTUM object labeled by input integer OBSFRUSTUM_ID away from
// its canonical orientation along +xhat towards specified azimuth and
// elevation angle settings wrt the model's velocity vector.  It also
// translates the OBSFRUSTUM from its initial (0,0,0) origin to the
// model's current position.

void MODEL::orient_and_position_OBSFRUSTUM(
   unsigned int first_framenumber,unsigned int last_framenumber,
   int pass_number,double delta_phi,double delta_theta,
   double z_base_face,bool sinusoidal_variation_flag,int OBSFRUSTUM_ID)
{     
//   cout << "inside MODEL::orient_and_position_OBSFRUSTUM()" << endl;
//   cout << "delta_phi = " << delta_phi*180/PI
//        << " delta_theta = " << delta_theta*180/PI << endl;
//   cout << "z_base_face = " << z_base_face << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
      get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   OBSFRUSTUM_ptr->set_stationary_Graphical_flag(false);

   const double dt=1.0;
   for (unsigned int i=first_framenumber; i<=last_framenumber; i++)
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
      
      instantaneously_orient_and_position_OBSFRUSTUM(
         curr_t,pass_number,dt,delta_phi,delta_theta,z_base_face,
         OBSFRUSTUM_ptr);
   } // loop over index i labeling frame number
}

// ----------------------------------------------------------------
void MODEL::instantaneously_orient_and_position_OBSFRUSTUM(
   double curr_t,int pass_number,double dt,
   double delta_phi,double delta_theta,double z_base_face,
   OBSFRUSTUM* OBSFRUSTUM_ptr)
{
   cout << "inside MODEL::instantaneously_orient_and_position_OBSFRUSTUM, t = " 
        << curr_t << endl;
   cout << "z_base_face = " << z_base_face << endl;
   
   threevector curr_model_posn;
   if (!get_UVW_coords(curr_t,pass_number,curr_model_posn))
   {
      curr_model_posn=Zero_vector;
   }
//   cout << "curr_model_posn = " << curr_model_posn << endl;

   int n_timesteps=1;
   bool nonzero_speed_flag=false;
   threevector curr_model_velocity;
   while (!nonzero_speed_flag && n_timesteps < 5)
   {
      get_velocity(curr_t,pass_number,n_timesteps*dt,curr_model_velocity);
      if (curr_model_velocity.nearly_equal(Zero_vector))
      {
         n_timesteps++;
      }
      else
      {
         nonzero_speed_flag=true;
      }
//      cout << "curr_model_velocity = " << curr_model_velocity << endl;
   }
   threevector v_hat(x_hat);
   if (!curr_model_velocity.nearly_equal(Zero_vector))
   {
      v_hat=curr_model_velocity.unitvector();
   }
//   cout << "v_hat = " << v_hat << endl;

   double phi=0;
   if (!nearly_equal(fabs(v_hat.get(2)),1))
   {
      phi=atan2(v_hat.get(1),v_hat.get(0));
   }
   double theta=asin(v_hat.get(2));

// Squint observation frustum away from model's velocity vector in
// azimuth by delta_phi and in elevation by delta_theta:
   
   phi += delta_phi;
   theta += delta_theta;
//   cout << "theta = " << theta << " phi = " << phi << endl;

   OBSFRUSTUM_ptr->build_current_frustum(
      curr_t,pass_number,curr_model_posn,phi,theta,z_base_face);
}

// ----------------------------------------------------------------
void MODEL::orient_and_position_instantaneous_dwell_OBSFRUSTUM(
   unsigned int first_framenumber,unsigned int last_framenumber,
   int pass_number,double z_base_face,
   OBSFRUSTUM* dwell_OBSFRUSTUM_ptr,OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,
   const vector<geopoint>& GMTI_targets)
{     
//   cout << "inside MODEL::orient_and_posn_inst_dwell_OBSFRUSTUM" << endl;

   dwell_OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();
   dwell_OBSFRUSTUM_ptr->generate_Pyramid_geodes();

   double dwell_range_extent=4000;		// meters
   double dwell_crossrange_extent=2000;		// meters
   const double max_dwell_time=8;
   double dwell_time=0;
   int GMTI_target_number=0;

   for (unsigned int i=first_framenumber; i<=last_framenumber; i++)
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
            instantaneously_orient_and_position_GMTI_dwell_OBSFRUSTUM(
               curr_t,pass_number,z_base_face,dwell_OBSFRUSTUM_ptr,
               FOV_OBSFRUSTUM_ptr,GMTI_posn,
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
bool MODEL::instantaneously_orient_and_position_GMTI_dwell_OBSFRUSTUM(
   double curr_t,int pass_number,double z_base_face,
   OBSFRUSTUM* dwell_OBSFRUSTUM_ptr,OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,
   const threevector& GMTI_posn,
   double dwell_range_extent,double dwell_crossrange_extent)
{
//   cout << "inside MODEL::instantaneously_orient_and_position_GMTI_dwell"
//        << endl;

   bool GMTI_dwell_handled=false;

   threevector camera_posn;
   if (get_UVW_coords(curr_t,pass_number,camera_posn))
   {
//      cout << "t = " << curr_t << " camera_posn = " << camera_posn << endl;

// First check whether GMTI target lies inside FOV footprint.  We
// learned from Tony Filip on 2/21/07 that LiMIT will schedule a GMTI
// dwell on a ground target provided the target's location lies within
// the FOV footprint (and even if much of the dwell footprint lies
// outside the FOV footprint).

// Retrieve corners for z-plane face saved within
// ViewingPyramidAboveZplane graphical's time-dependent vertices:

      vector<threevector> zplane_face_vertex;
      FOV_OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
         get_vertices(curr_t,pass_number,zplane_face_vertex);
      face FOV_zplane_face(zplane_face_vertex);
      FOV_zplane_face.force_handedness_wrt_direction(
         face::right_handed,z_hat);
//      cout << "FOV_zplane_face = " << FOV_zplane_face << endl;

      polygon dwell_footprint=
         dwell_OBSFRUSTUM_ptr->GMTI_dwell_frustum_footprint(
            camera_posn,GMTI_posn,dwell_range_extent,dwell_crossrange_extent,
            z_base_face);
         
      vector<threevector> UV_corner_dir;
      for (unsigned int c=0; c<4; c++)
      {
         threevector curr_ray=dwell_footprint.get_vertex(c)-camera_posn;
         UV_corner_dir.push_back( curr_ray.unitvector() );
      }

//         bool display_ViewingPyramid_flag=true;
      bool display_ViewingPyramid_flag=false;

      if (!FOV_zplane_face.point_inside_convex_face(GMTI_posn))
      {

// If dwell OBSFRUSTUM should not be seen, render it with a very short
// altitude so that it's effectively invisible:

         dwell_OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
            set_mask(curr_t,pass_number,!display_ViewingPyramid_flag);
      }
      else
      {
         const double max_sidelength=500000;	// max slant range = 500 km
         dwell_OBSFRUSTUM_ptr->generate_or_reset_viewing_pyramid(
            camera_posn,UV_corner_dir,max_sidelength);
         dwell_OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()->
            ensure_faces_handedness(face::right_handed);
         dwell_OBSFRUSTUM_ptr->compute_viewing_pyramid_above_Zplane(
            z_base_face,dwell_OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());

         dwell_OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->
            build_current_pyramid(
               curr_t,pass_number,
               dwell_OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());
         dwell_OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_mask(
            curr_t,pass_number,!display_ViewingPyramid_flag);

         dwell_OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
            build_current_pyramid(
               curr_t,pass_number,
               dwell_OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());

         double volume_alpha=0.5;
         dwell_OBSFRUSTUM_ptr->set_color(
            colorfunc::get_OSG_color(colorfunc::red),volume_alpha);

         dwell_OBSFRUSTUM_ptr->set_typical_pyramid_edge_widths();

         GMTI_dwell_handled=true;
      } // FOV_footprint contains GMTI target conditional
      dwell_OBSFRUSTUM_ptr->set_stationary_Graphical_flag(false);

   } // camera's curr_posn known conditional
   return GMTI_dwell_handled;
}

// ----------------------------------------------------------------
// Member function extract_PointCloudsGroup_ptr() takes in
// EarthRegionsGroup *EarthRegionsGroup_ptr and returns a pointer to
// its PointCloudsGroup if it exists.  Otherwise, this method returns
// NULL.

PointCloudsGroup* MODEL::extract_PointCloudsGroup_ptr(
   EarthRegionsGroup* EarthRegionsGroup_ptr)
{
//   cout << "inside MODEL::extract_PointCloudsGroup_ptr()" << endl;

   PointCloudsGroup* PointCloudsGroup_ptr=NULL;
   if (EarthRegionsGroup_ptr != NULL)
   {
      PointCloudsGroup_ptr=EarthRegionsGroup_ptr->
         get_PointCloudsGroup_ptr();
   }
   return PointCloudsGroup_ptr;
}

// ----------------------------------------------------------------
// Member function extract_PointCloud_ptr() takes in EarthRegionsGroup
// *EarthRegionsGroup_ptr and returns a pointer to its zeroth
// PointCloud if it exists.  Otherwise, this method returns NULL.

PointCloud* MODEL::extract_PointCloud_ptr(
   EarthRegionsGroup* EarthRegionsGroup_ptr)
{
//   cout << "inside MODEL::extract_PointCloud_ptr()" << endl;

   PointCloud* PointCloud_ptr=NULL;
   PointCloudsGroup* PointCloudsGroup_ptr=extract_PointCloudsGroup_ptr(
      EarthRegionsGroup_ptr);
   if (PointCloudsGroup_ptr != NULL)
   {
      PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);
   }

   return PointCloud_ptr;
}

// ==========================================================================
// Dynamic OBSFRUSTA manipulation member functions
// ==========================================================================

// Member function compute_dynamic_OBSFRUSTA() extracts the MODEL's
// current velocity heading angle from the quaternion associated with
// the current time and pass number.  For each pre-defined OBSFRUSTUM,
// this method dynamically builds a Viewing Pyramid whose current
// pointing direction is specified by OBSFRUSTUM_yaw and
// OBSFRUSTUM_pitch and whose constant opening angles were set by
// specified az and el extents at instantiation time.  The OBSFRUSTA
// absolute locations are updated at run-time and do not need to be
// pre-computed.  OBSFRUSTA pointers (which may equal NULL) are
// returned in an STL vector by this method.

vector<OBSFRUSTUM*> MODEL::compute_dynamic_OBSFRUSTA(
   double curr_t,int pass_number,EarthRegionsGroup* EarthRegionsGroup_ptr,
   unsigned int n_future_repeats,bool& OBSFRUSTA_previously_built_flag)
{
//   cout << "inside MODEL::compute_dynamic_OBSFRUSTA()" << endl;
//   cout << "curr_t = " << curr_t << endl;

   vector<OBSFRUSTUM*> OBSFRUSTUM_ptrs_vector;

// First check number if OBSFRUSTA > 0 : 

   unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
   if (n_OBSFRUSTA==0) return OBSFRUSTUM_ptrs_vector;
//   cout << "n_OBSFRUSTA = " << n_OBSFRUSTA << endl;

   for (unsigned int n=0; n<n_OBSFRUSTA; n++)
   {
      OBSFRUSTUM_ptrs_vector.push_back(
         OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n));
   }

   threevector posn;
   bool posn_flag=get_UVW_coords(curr_t,pass_number,posn);
//   cout << "model posn = " << posn << endl;
//   cout << "curr_t = " << curr_t << " posn_flag = " << posn_flag << endl;

// Next perform sanity check on model position.  If it equals
// zero_vector, do not attempt to construct model's OBSFRUSTUM:

   if (posn.nearly_equal(Zero_vector)) posn_flag=false;
   if (!posn_flag) return OBSFRUSTUM_ptrs_vector;

   threevector v_hat(1,0,0);
   osg::Quat curr_q;
   bool quat_flag=get_quaternion(curr_t,pass_number,curr_q);
//   cout << "quat_flag = " << quat_flag << endl;
   
   if (quat_flag)
   {
      fourvector qvec(curr_q._v[0],curr_q._v[1],curr_q._v[2],curr_q._v[3]);
      if (qvec.nearly_equal(fourvector(0,0,0,1))) quat_flag=false;
//      cout << "qvec = " << qvec << endl;
      double az,el,roll;
      mathfunc::az_el_roll_corresponding_to_quaternion(qvec,az,el,roll);
//      cout << "az = " << az*180/PI 
//           << " el = " << el*180/PI 
//           << " roll = " << roll*180/PI << endl;
      v_hat.put(0,cos(roll-PI/2.0));
      v_hat.put(1,sin(roll-PI/2.0));
   }
//   cout << "v_hat = " << v_hat << endl;

//   threevector scale;
//   bool scale_flag=get_scale(curr_t,pass_number,scale);
//   cout << "scale = " << scale << endl;

// If MODEL's position and orientation have not changed since the last
// time this member function was called, we don't bother to recompute
// dynamic OBSFRUSTUM as of 3/19/09...

   OBSFRUSTA_previously_built_flag=false;
   if (posn.nearly_equal(prev_posn) && v_hat.nearly_equal(prev_vhat)) 
   {
      OBSFRUSTA_previously_built_flag=true;
   }

   prev_posn=posn;
   prev_vhat=v_hat;

   for (unsigned int OBSFRUSTUM_ID=0; !OBSFRUSTA_previously_built_flag &&
           OBSFRUSTUM_ID<n_OBSFRUSTA; OBSFRUSTUM_ID++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
         OBSFRUSTUM_ID);
//      cout << "ID = " <<  OBSFRUSTUM_ID 
//           << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
      double z_base_face=get_OBSFRUSTUM_z_base_face(OBSFRUSTUM_ID);

// First construct trial OBSFRUSTUM with its z-plane face located at
// z=z_base_face:

// To avoid flickering in the Real-Time Persistent Surveillance demo,
// we repeat the construction of *OBSFRUSTUM_ptr for multiple time
// steps into the future.

      for (unsigned int r=0; r<n_future_repeats; r++)
      {
//         cout << "r = " << r 
//		<< " n_future_repeats = " << n_future_repeats << endl;
//         cout << "curr_t = " << curr_t << endl;
//         cout << " roll = " << OBSFRUSTUM_roll[OBSFRUSTUM_ID]*180/PI << endl;
//         cout << " pitch = " << OBSFRUSTUM_pitch[OBSFRUSTUM_ID]*180/PI 
//              << endl;
//         cout << "z_base_face = " << z_base_face << endl;
//         cout << "posn = " << posn << endl;
//         cout << "v_hat = " << v_hat << endl;
//         cout << "OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
         OBSFRUSTUM_ptr->build_current_frustum(
            curr_t+r,pass_number,
            OBSFRUSTUM_roll[OBSFRUSTUM_ID],OBSFRUSTUM_pitch[OBSFRUSTUM_ID],
            z_base_face,posn,v_hat);
      }
      
// If PointCloud data is available, recompute average of point cloud's
// height at multiple points along z-plane face.  Reconstruct
// OBSFRUSTUM using improved estimate for ground height:

      PointCloud* PointCloud_ptr=extract_PointCloud_ptr(
         EarthRegionsGroup_ptr);
//      cout << "PointCloud_ptr = " << PointCloud_ptr << endl;

      if (PointCloud_ptr != NULL && compute_zface_height_flag)
      {
         const bool compute_avg_flag=true;
         double z_ground=OBSFRUSTUM_ptr->estimate_z_ground(
            PointCloud_ptr,4,compute_avg_flag);

         if (z_ground > 0.5*NEGATIVEINFINITY)
         {
            OBSFRUSTUM_ptr->build_current_frustum(
               curr_t,pass_number,
               OBSFRUSTUM_roll[OBSFRUSTUM_ID],OBSFRUSTUM_pitch[OBSFRUSTUM_ID],
               z_ground,posn,v_hat);
         }
      } // PointCloud_ptr != NULl && compute_zface_height_flag conditional

   } // loop over OBSFRUSTUM_ID

   for (unsigned int OBSFRUSTUM_ID=0; !OBSFRUSTA_previously_built_flag &&
           OBSFRUSTUM_ID<n_OBSFRUSTA; OBSFRUSTUM_ID++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
         OBSFRUSTUM_ID);
   
      Movie* Movie_ptr = OBSFRUSTUM_ptr->get_Movie_ptr();
//      cout << "Movie_ptr = " << Movie_ptr << endl;
      if (Movie_ptr != NULL)
      {
         camera* camera_ptr=Movie_ptr->get_camera_ptr();
         camera_ptr->set_world_posn(posn);

// Take camera's V_hat direction vector to be set by difference of
// corner rays that maximally overlap with +z_hat.  Compute average of
// corner rays to determine camera's pointing vector W_hat.

         vector<threevector> corner_ray=OBSFRUSTUM_ptr->get_corner_ray();
         camera_ptr->set_UV_corner_world_ray(corner_ray);

         double max_z_dotproduct=NEGATIVEINFINITY;
         threevector V_hat,corner_ray_sum;
         for (unsigned int c=0; c<corner_ray.size(); c++)
         {
            threevector curr_corner_ray=corner_ray[c];
            threevector next_corner_ray=corner_ray[modulo(c+1,4)];
            threevector ray_difference=next_corner_ray-curr_corner_ray;
            double curr_z_dotproduct=ray_difference.get(2);
            if (curr_z_dotproduct > max_z_dotproduct)
            {
               max_z_dotproduct=curr_z_dotproduct;
               V_hat=ray_difference.unitvector();
            }
            corner_ray_sum += curr_corner_ray;
         } // loop over index c labeling corner rays

         threevector W_hat=-corner_ray_sum.unitvector();
         threevector U_hat=V_hat.cross(W_hat);
      
//      cout << "U_hat = " << U_hat 
//           << " V_hat = " << V_hat 
//           << " W_hat = " << W_hat << endl;
//      cout << "U.V = " << U_hat.dot(V_hat) << endl;
//      cout << "V.W = " << V_hat.dot(W_hat) << endl;
//      cout << "W.U = " << W_hat.dot(U_hat) << endl;
      
         camera_ptr->set_Rcamera(U_hat,V_hat);
      } // Movie_ptr != NULL conditional
   } // loop over OBSFRUSTUM_IDs

   return OBSFRUSTUM_ptrs_vector;
}

// ---------------------------------------------------------------------
// Member function recover_or_compute_OBSFRUSTA_occlusion() first
// either generates or recovers OBSFRUSTA based upon the MODEL's
// current position and velocity heading.  It also checks whether
// raytracing results have been previously calculated and stored.  If
// not, this method computes aerial line-of-sight occlusion within a
// Region of Interest on the ground.

bool MODEL::recover_or_compute_OBSFRUSTA_occlusion(
   bool OBSFRUSTA_previously_built_flag,
   double curr_t,int pass_number,EarthRegionsGroup* EarthRegionsGroup_ptr,
   ColorGeodeVisitor* ColorGeodeVisitor_ptr,bounding_box* ground_bbox_ptr,
   Messenger* viewer_Messenger_ptr,double raytrace_cellsize)
{
//   cout << "inside MODEL::recover_or_compute_OBSFRUSTA_occlusions()" << endl;
//   cout << "raytrace_cellsize = " << raytrace_cellsize << endl;
//   cout << "curr_t = " << curr_t << endl;

   bool previously_raytraced_flag=false;
//   cout << "raytrace_occluded_ground_regions_flag = "
//        << raytrace_occluded_ground_regions_flag << endl;

   const double SMALL=0.001;
   double total_occlusion_percentage=0;
   if (raytrace_occluded_ground_regions_flag)
   {
      get_score(curr_t,pass_number,total_occlusion_percentage);
//      cout << "score = " << total_occlusion_percentage << endl;
      if (total_occlusion_percentage > -SMALL) 
         previously_raytraced_flag=true;
//      cout << "total_occlusion_percentage = " 
//           << total_occlusion_percentage << endl;
   } // raytrace_occluded_ground_regions_flag conditional
//   cout << "previously_raytraced_flag = " << previously_raytraced_flag
//        << endl;

   if (OBSFRUSTA_previously_built_flag && 
       !raytrace_occluded_ground_regions_flag)
   {
      return previously_raytraced_flag;
   }
   else if (OBSFRUSTA_previously_built_flag 
            && raytrace_occluded_ground_regions_flag 
            && previously_raytraced_flag)
   {
      return previously_raytraced_flag;
   }
   else if (!OBSFRUSTA_previously_built_flag && previously_raytraced_flag)
   {
//      if (viewer_Messenger_ptr != NULL)
//      {
//         cout << "curr_t = " << curr_t 
//              << " Recovered occlusion percentage = " 
//              << total_occlusion_percentage << endl;
//         broadcast_occlusion_percentage(
//            total_occlusion_percentage,viewer_Messenger_ptr);
//      }
   }

// As of 10/26/09, the QTLOS thin client resets the
// AnimationController's clock to its first framenumber and its mode
// to PLAY when raytracing starts.  After raytracing has been
// performed for all frames, we want the thick client and thin client
// to both show raytracing results for the zeroth framenumber.  So if
// previously_raytraced_flag==true, reset AnimationController's clock
// to beginning and reset its state to PAUSE:

   if (raytrace_occluded_ground_regions_flag && 
       get_AnimationController_ptr()->getState()==AnimationController::PLAY
       && previously_raytraced_flag==true)
   {
      get_AnimationController_ptr()->set_curr_framenumber(
         get_AnimationController_ptr()->get_first_framenumber());
      get_AnimationController_ptr()->setState(AnimationController::PAUSE);
   }
   else if (raytrace_occluded_ground_regions_flag && (
       ( get_AnimationController_ptr()->getState()==AnimationController::PLAY
         && previously_raytraced_flag==false) ||
       ( get_AnimationController_ptr()->getState()==
         AnimationController::PAUSE) ) )
   {
      compute_and_display_OBSFRUSTA_occlusion(
         curr_t,pass_number,previously_raytraced_flag,
         EarthRegionsGroup_ptr,ColorGeodeVisitor_ptr,ground_bbox_ptr,
         viewer_Messenger_ptr,raytrace_cellsize);
   }

   return previously_raytraced_flag;
}

// ---------------------------------------------------------------------
// Member function compute_and_display_OBSFRUSTA_occlusion() loops over the
// current MODEL's OBSFRUSTA and raytraces their fields-of-regard into
// a Region of Interest on the ground.  This method stores occlusion
// score results so that they don't have to be calculated more than
// once.  It also resets ColorGeode coloring to indicate ground
// regions which are visible and occluded.

void MODEL::compute_and_display_OBSFRUSTA_occlusion(
   double curr_t,int pass_number,bool previously_raytraced_flag,
   EarthRegionsGroup* EarthRegionsGroup_ptr,
   ColorGeodeVisitor* ColorGeodeVisitor_ptr,bounding_box* ground_bbox_ptr,
   Messenger* viewer_Messenger_ptr,double raytrace_cellsize)
{
//   cout << "inside MODEL::compute_and_display_OBSFRUSTA_occlusion()" << endl;
//   cout << "curr_t = " << curr_t << endl;
//   cout << "min_raytrace_range = " << min_raytrace_range
//        << " max_raytrace_range = " << max_raytrace_range << endl;
//   cout << "raytrace_cellsize = " << raytrace_cellsize << endl;

   unsigned int n_occluded_rays=0;
   unsigned int n_total_rays=0;
   unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();

// As of 2/5/11, we assume that there exists just a single DTED geotif
// file which needs to be imported if ladar raytracing is being
// performed:

   bool ladar_height_data_flag=
      OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(0)->get_ladar_height_data_flag();
   if (ladar_height_data_flag) n_OBSFRUSTA=1;

   for (unsigned int OBSFRUSTUM_ID=0; OBSFRUSTUM_ID<n_OBSFRUSTA; 
        OBSFRUSTUM_ID++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
         OBSFRUSTUM_ID);
      OBSFRUSTUM_ptr->set_ground_bbox_ptr(ground_bbox_ptr);

//       double curr_occlusion_percentage=
         OBSFRUSTUM_ptr->raytrace_occluded_ground_region(
            min_raytrace_range,max_raytrace_range,raytrace_cellsize,
            n_total_rays,n_occluded_rays);
   } // loop over OBSFRUSTUM_IDs

//   cout << "n_total_rays = " << n_total_rays
//        << " n_occluded_rays = " << n_occluded_rays << endl;
//   cout << "previously_raytraced_flag = " << previously_raytraced_flag
//        << endl;

   if (!previously_raytraced_flag)
   {
         
// Compute total occlusion percentage integrated over all OBSFRUSTA to
// ActiveMQ messenger:

      double total_occlusion_percentage=0;
      if (n_total_rays > 1)
      {
         total_occlusion_percentage=
            double(n_occluded_rays)/double(n_total_rays)*100;
      }
//      cout << "curr_t = " << curr_t << " Saved occlusion percentage = " 
//           << total_occlusion_percentage << endl;
      set_score(curr_t,pass_number,total_occlusion_percentage);

//      if (viewer_Messenger_ptr != NULL)
//      {
//         broadcast_occlusion_percentage(
//            total_occlusion_percentage,viewer_Messenger_ptr);
//      }
   } // !previously_raytraced_flag conditional

// Automatically reset ColorMap to display probabilities and force
// ColorGeodeVisitor to reload latest p values:

   if (ColorGeodeVisitor_ptr != NULL)
   {
      ColorGeodeVisitor_ptr->clear_ptwoDarray_ptrs();

      for (unsigned int n=0; n<n_OBSFRUSTA; n++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
            get_OBSFRUSTUM_ptr(n);

         twoDarray* ptwoDarray_ptr=OBSFRUSTUM_ptr->get_DTED_ptwoDarray_ptr();
         if (ptwoDarray_ptr != NULL)
         {
            ColorGeodeVisitor_ptr->push_back_ptwoDarray_ptr(ptwoDarray_ptr);
         }
      } // loop over index n labeling OBSFRUSTA
      
      PointCloudsGroup* PointCloudsGroup_ptr=
         extract_PointCloudsGroup_ptr(EarthRegionsGroup_ptr);
      if (PointCloudsGroup_ptr != NULL)
      {
         if (PointCloudsGroup_ptr->get_dependent_coloring_var() != 3)
         {
            PointCloudsGroup_ptr->set_dependent_coloring_var(3);
            PointCloudsGroup_ptr->update_dynamic_Grid_color();
         }
         PointCloudsGroup_ptr->reload_all_colors();
      }

   } // ColorGeodeVisitor_ptr != NULL conditional

//   cout << "at end of MODEL::compute_and_display_OBSFRUSTA_occlusion()" << endl;
}

// ----------------------------------------------------------------
// Member function compute_dynamic_OBSFRUSTUM() takes in a specified
// position and velocity direction vector for an aircraft MODEL.  It
// constructs a downward oriented OBSFRUSTUM which intercepts an
// EarthRegion at presumably four locations.  Raytracing operations
// can subsequently be performed using the dynamically altered
// OBSFRUSTUM.

OBSFRUSTUM* MODEL::compute_dynamic_OBSFRUSTUM(
   double curr_t,int pass_number,
   const threevector& posn,const threevector& v_hat,
   double alpha,double beta,double z_base_face,int OBSFRUSTUM_ID)
{
//   cout << "inside MODEL::compute_dynamic_OBSFRUSTUM()" << endl;
//   cout << "z_base_face = " << z_base_face << endl;
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
      OBSFRUSTUM_ID);

   if (OBSFRUSTUM_ptr==NULL) return NULL;

// Construct trial OBSFRUSTUM with its z-plane face located at
// z=z_base_face:

/*
   double alpha,beta;
   OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles(
      OBSFRUSTUM_ptr->get_az_extent(),OBSFRUSTUM_ptr->get_el_extent(),
      alpha,beta);
//   cout << "alpha = " << alpha*180/PI << " beta = " << beta*180/PI << endl;
*/

//    bool all_downward_rays_flag=
      OBSFRUSTUM_ptr->build_current_frustum(
         curr_t,pass_number,posn,v_hat,alpha,beta,
         OBSFRUSTUM_roll[OBSFRUSTUM_ID],OBSFRUSTUM_pitch[OBSFRUSTUM_ID],
         z_base_face);

   return OBSFRUSTUM_ptr;
}

// ----------------------------------------------------------------
// Member function display_average_LOS_results() loads the average
// line-of-sight probabilities stored within longitude/latitude geotif
// tiles into an STL vector of twoDarray pointers.  It then pushes
// these pointers onto ColorGeodeVisitor member STL vector
// ptwoDarray_ptrs.  It also resets the dependent coloring variable to
// the probability channel.

void MODEL::display_average_LOS_results(
   vector<string>& geotif_filenames,
   ColorGeodeVisitor* ColorGeodeVisitor_ptr,
   EarthRegionsGroup* EarthRegionsGroup_ptr,Messenger* viewer_Messenger_ptr)
{
   cout << "inside MODEL::display_average_LOS_results()" << endl;

// Automatically reset ColorMap to display probabilities and force
// ColorGeodeVisitor to reload latest p values:

   if (ColorGeodeVisitor_ptr != NULL)
   {
      vector<twoDarray*> avg_LOS_ptwoDarray_ptrs;
      if (import_avg_LOS_ptwoDarray_contents(
         geotif_filenames,avg_LOS_ptwoDarray_ptrs))
      {
         ColorGeodeVisitor_ptr->clear_ptwoDarray_ptrs();
         ColorGeodeVisitor_ptr->push_back_ptwoDarray_ptrs(
            avg_LOS_ptwoDarray_ptrs);
      }
      cout << "avg_LOS_ptwoDarray_ptrs.size() = "
           << avg_LOS_ptwoDarray_ptrs.size() << endl;

      PointCloudsGroup* PointCloudsGroup_ptr=extract_PointCloudsGroup_ptr(
         EarthRegionsGroup_ptr);
      if (PointCloudsGroup_ptr != NULL)
         PointCloudsGroup_ptr->set_dependent_coloring_var(3);

// Generate coarse LOS occlusion fractions histogram:

      vector<int> nbin;
      vector<double> frac;
      for (unsigned int i=0; i<4; i++)
      {
         nbin.push_back(0);
         frac.push_back(0);
      }
      for (unsigned int a=0; a<avg_LOS_ptwoDarray_ptrs.size(); a++)
      {
         twoDarray* curr_ptwoDarray_ptr=avg_LOS_ptwoDarray_ptrs[a];
         for (unsigned int px=0; px<curr_ptwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<curr_ptwoDarray_ptr->get_ndim(); py++)
            {
               double curr_p=curr_ptwoDarray_ptr->get(px,py);
               if (curr_p >= -.001 && curr_p < 0.25)
               {
                  nbin[0]=nbin[0]+1;
               }
               else if (curr_p >= 0.25 && curr_p < 0.5)
               {
                  nbin[1]=nbin[1]+1;
               }
               else if (curr_p >= 0.5 && curr_p < 0.75)
               {
                  nbin[2]=nbin[2]+1;
               }
               else if (curr_p >= 0.75 && curr_p <= 1.001)
               {
                  nbin[3]=nbin[3]+1;
               }

            } // loop over py index
         } // loop over px index

//         cout << "a = " << a
//              << " nbin[0] = " << nbin[0] 
//              << " nbin[1] = " << nbin[1]
//              << " nbin[2] = " << nbin[2]
//              << " nbin[3] = " << nbin[3] << endl;

      } // loop over index a labeling avg_LOS_ptwoDarrays

      double ntotal=0;
      for (unsigned int i=0; i<4; i++)
      {
         ntotal += nbin[i];
      }
//      cout << "ntotal = " << ntotal << endl;

      for (unsigned int i=0; i<4; i++)
      {
         frac[i]=double(nbin[i])/ntotal;
         cout << "i = " << i << " Occlusion frac = " << frac[i] << endl;
      }
      broadcast_average_occlusion_fractions(frac,viewer_Messenger_ptr);

   } // ColorGeodeVisitor_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function import_avg_LOS_ptwoDarray_contents() recovers all
// geotif files whose names start with "avg_LOS" from the input
// geotif_Ptiles subdirectory.  It purges and regenerates input STL
// vector avg_LOS_ptwoarray_ptrs and fills each dynamically
// instantiated twoDarray* with average LOS data read in from the
// geotif files.  If all the data is read in successfully, this
// boolean method returns true.

bool MODEL::import_avg_LOS_ptwoDarray_contents(
   vector<string> geotif_filenames,vector<twoDarray*>& avg_LOS_ptwoDarray_ptrs)
{
   cout << "inside MODEL::import_avg_LOS_ptwoDarray_contents()" << endl;
   
   if (geotif_filenames.size()==0) return false;

// First purge current contents of avg_LOS_ptwoDarray_ptrs STL vector:

   for (unsigned int a=0; a<avg_LOS_ptwoDarray_ptrs.size(); a++)
   {
      delete avg_LOS_ptwoDarray_ptrs[a];
   }
   avg_LOS_ptwoDarray_ptrs.clear();

// Next generate new set of avg_LOS_ptwoDarray pointers and fill them
// with raster data read in from average LOS geotif files:

   for (unsigned int f=0; f<geotif_filenames.size(); f++)
   {
      cout << "f = " << f
           << " geotiff_filename = " << geotif_filenames[f] << endl;

      raster_parser RasterParser;
      if (!RasterParser.open_image_file(geotif_filenames[f])) continue;

      int channel_ID=0; 
      RasterParser.fetch_raster_band(channel_ID);

      twoDarray* curr_avg_LOS_ptwoDarray_ptr=
         new twoDarray(RasterParser.get_ztwoDarray_ptr());
      curr_avg_LOS_ptwoDarray_ptr->initialize_values(-1);

      RasterParser.read_raster_data(curr_avg_LOS_ptwoDarray_ptr);

      const double p_min=-1;
      const double p_max=1.01;
      RasterParser.convert_GUInts_to_doubles(
         p_min,p_max,curr_avg_LOS_ptwoDarray_ptr);

      avg_LOS_ptwoDarray_ptrs.push_back(curr_avg_LOS_ptwoDarray_ptr);
      

   } // loop over index f labeling avg_LOS geotif files

   return true;
}

// ---------------------------------------------------------------------
// Member function update_NFOV_OBSFRUSTUM_roll_and_pitch() takes in
// the geocoordinates for the narrow field-of-view camera's current
// ground lookpoint.  It computes the lookpoint's direction vector
// relative to the aircraft's instantaneous position.  This method
// subsequently decomposes the look direction vector into a roll about
// the velocity axis v_hat followed by a pitch about w_hat = v_hat x
// z_hat.  It resets OBSFRUSTUM_roll[1] and OBSFRUSTUM_pitch[1] to
// these calculated angles.

void MODEL::update_NFOV_OBSFRUSTUM_roll_and_pitch(
   double curr_t,int pass_number,const geopoint& NFOV_lookpoint)
{
//   cout << "inside MODEL::update_NFOV_OBSFRUSTUM_roll_and_pitch()" << endl;
//   cout << "curr_t = " << curr_t << endl;

   threevector curr_model_posn;
   if (!get_UVW_coords(curr_t,pass_number,curr_model_posn))
   {
      cout << "Current MODEL's position is unknown!" << endl;
      return;
   }
//   cout << "curr_model_posn = " << curr_model_posn << endl;
//   cout << "NFOV_lookpoint = " << NFOV_lookpoint << endl;

   threevector l_hat=	// look direction
      (NFOV_lookpoint.get_UTM_posn()-curr_model_posn).unitvector();
//   cout << "l_hat = " << l_hat << endl;

   threevector curr_model_velocity;
   if (!get_UVW_velocity(curr_t,pass_number,curr_model_velocity))
   {
      cout << "Current MODEL's velocity is unknown!" << endl;
      return;
   }
   if (nearly_equal(curr_model_velocity.sqrd_magnitude(),0))
   {
      cout << "Current MODEL's velocity equals 0!" << endl;
      return;
   }
   
//   cout << "curr_model_velocity = " << curr_model_velocity << endl;
   threevector v_hat(curr_model_velocity.unitvector());
//   cout << "v_hat = " << v_hat << endl;
   threevector w_hat(v_hat.cross(z_hat));
//   cout << "w_hat = " << w_hat << endl;

   double l_z=l_hat.dot(z_hat);
   double l_v=l_hat.dot(v_hat);
//   double l_w=l_hat.dot(w_hat);

//   cout << "l_z = " << l_z << endl;
//   cout << "l_v = " << l_v << endl;
//   cout << "l_w = " << l_w << endl;

   double pitch=atan2(l_v,l_z);
   pitch=basic_math::phase_to_canonical_interval(pitch,0,2*PI);
   pitch=PI-pitch;

//   cout << "pitch = " << pitch*180/PI << endl;
//   cout << "180 - pitch = " << (PI-pitch)*180/PI << endl;

   rotation Rpitch;
   Rpitch.rotation_taking_pqr_to_uvw(
      w_hat,v_hat,z_hat,w_hat,
      cos(pitch)*v_hat+sin(pitch)*z_hat,-sin(pitch)*v_hat+cos(pitch)*z_hat);

   threevector lnew_hat=Rpitch.transpose()*l_hat;
   double lnew_z=lnew_hat.dot(z_hat);
//    double lnew_v=lnew_hat.dot(v_hat);
   double lnew_w=lnew_hat.dot(w_hat);

//   cout << "lnew_z = " << lnew_z << endl;
//   cout << "lnew_v = " << lnew_v << endl;
//   cout << "lnew_w = " << lnew_w << endl;
//   cout << "|lnew|**2 = " << sqr(lnew_z)+sqr(lnew_v)+sqr(lnew_w) << endl;

   double roll=atan(lnew_w/lnew_z);
   roll=basic_math::phase_to_canonical_interval(roll,0,2*PI);
//   cout << "roll = " << roll*180/PI << endl;
   
   rotation Rroll;
   Rroll.rotation_taking_pqr_to_uvw(
      v_hat,z_hat,w_hat,
      v_hat,cos(roll)*z_hat+sin(roll)*w_hat,-sin(roll)*z_hat+cos(roll)*w_hat);

//    threevector lnewnew_hat=Rroll.transpose()*lnew_hat;
//    double lnewnew_z=lnewnew_hat.dot(z_hat);
//    double lnewnew_v=lnewnew_hat.dot(v_hat);
//    double lnewnew_w=lnewnew_hat.dot(w_hat);

//   cout << "lnewnew_z = " << lnewnew_z << endl;
//   cout << "lnewnew_v = " << lnewnew_v << endl;
//   cout << "lnewnew_w = " << lnewnew_w << endl;
//   cout << "|lnewnew|**2 = " 
//        << sqr(lnewnew_z)+sqr(lnewnew_v)+sqr(lnewnew_w) << endl;

   threevector recovered_l_hat=Rpitch*Rroll*(-z_hat);
//   cout << "recovered_l_hat = " << recovered_l_hat << endl;
//   cout << "l_hat = " << l_hat << endl;
//   cout << "l_recovered_hat - l_hat = " << recovered_l_hat - l_hat << endl;

   OBSFRUSTUM_roll[1]=roll;
   OBSFRUSTUM_pitch[1]=pitch;
}

// ==========================================================================
// ActiveMQ broadcast member functions
// ==========================================================================

// Member function broadcast_average_occlusion_fractions()

void MODEL::broadcast_average_occlusion_fractions(
   const vector<double>& frac,Messenger* viewer_Messenger_ptr,
   bool clear_results_flag)
{
//   cout << "inside MODEL::broadcast_average_occlusion_fractions()" << endl;
//   cout << "viewer_Messenger_ptr = " << viewer_Messenger_ptr << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_AVERAGE_OCCLUSION_PERCENTAGES";	

   key="Aircraft ID";
   value=stringfunc::number_to_string(get_ID());
   properties.push_back(Messenger::Property(key,value));

   key="Simulation Time";
   value=get_AnimationController_ptr()->get_world_time_string();
   properties.push_back(Messenger::Property(key,value));
 
   for (unsigned int f=0; f<frac.size(); f++)
   {
      key="AvgOcclusionPercentage"+stringfunc::number_to_string(f);
      if (clear_results_flag)
      {
         value=" ";
      }
      else
      {
         value=stringfunc::number_to_string(basic_math::round(frac[f]*100));
      }
         
      properties.push_back(Messenger::Property(key,value));
   } // loop over index f labeling occlusion fraction bins
   viewer_Messenger_ptr->broadcast_subpacket(command,properties);
}

// ---------------------------------------------------------------------
// Member function broadcast_ground_target_visibility() generates an
// output ActiveMQ message containing visibility information for
// a specified input target at a specified time.  

void MODEL::broadcast_ground_target_visibility(
   double curr_t,int target_ID,int visibility,Messenger* viewer_Messenger_ptr)
{
//   cout << "inside MODEL::broadcast_ground_target_visibilities()" << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_TARGET_VISIBILITY";	

   key="Aircraft ID";
   value=stringfunc::number_to_string(get_ID());
   properties.push_back(Messenger::Property(key,value));

   key="FrameNumber";
   value=stringfunc::number_to_string(curr_t);
   properties.push_back(Messenger::Property(key,value));

   key="TargetID";
   value=stringfunc::number_to_string(target_ID);
   properties.push_back(Messenger::Property(key,value));
 
   key="Visibility";
   value=stringfunc::number_to_string(basic_math::round(visibility));
   properties.push_back(Messenger::Property(key,value));

   viewer_Messenger_ptr->broadcast_subpacket(command,properties);
}

// ==========================================================================
// Human model member functions
// ==========================================================================

// Member function position_and_orient_man_MODEL() takes in a world
// position along with roll, pitch and yaw angles (measured in
// radians).  When r=p=y=0, the human model faces in the -y_hat
// direction.  This method rotates the model about -y_hat by roll,
// rotates the model about -x_hat by pitch, and it rotates the model
// about +z_hat by yaw.

void MODEL::position_and_orient_man_MODEL(
   double curr_t,int pass_number,
   const threevector& posn,double roll,double pitch,double yaw)
{
   threevector RPY(roll,pitch,yaw);
   threevector modified_posn=posn;

// Translate male figure so that its feet correspond to Z=0:

   modified_posn.put(2,posn.get(2)+0.967908);	

   set_attitude_posn(curr_t,pass_number,RPY,modified_posn);
}

// ---------------------------------------------------------------------
// Member function broadcast_MODEL_geoposn() transmits an ActiveMQ
// message containing the geoposition of a reconstructed human based
// upon its face extraction from a 2D photo.

void MODEL::broadcast_human_geoposn(
   const geopoint& human_geopoint,Messenger* GPS_messenger_ptr)
{
//   cout << "inside MODEL::broadcast_human_geoposn()" << endl;

// As of May 2010, Jennifer Drexler's blue force tracker thin client
// expects to receive longitude,latitude rather than easting,northing
// geocoordinates:  

   threevector curr_lla_posn(human_geopoint.get_longitude(),
                             human_geopoint.get_latitude(),
                             human_geopoint.get_altitude());
   threevector human_velocity=Zero_vector;

   tracks_group human_tracks_group;
   int ID=2;
   track* human_track_ptr=human_tracks_group.generate_new_track(ID);
   human_track_ptr->set_description("Reconstructed Man");
   
// Instantiate GPS messenger:

//   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
//   string GPS_message_queue_channel_name="GPS";
//   Messenger GPS_messenger( broker_URL, GPS_message_queue_channel_name );
   Clock clock;
   clock.current_local_time_and_UTC();

   colorfunc::Color human_color=colorfunc::green;
   double curr_time=clock.secs_elapsed_since_reference_date(); 
   human_track_ptr->set_posn_velocity(curr_time,curr_lla_posn,human_velocity);
   string ID_label="Man";
   human_track_ptr->broadcast_statevector(
      curr_time,GPS_messenger_ptr,human_color,ID_label);
}


