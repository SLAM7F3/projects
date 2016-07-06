// ==========================================================================
// SATELLITEPASS class member function definitions
// ==========================================================================
// Last modified on 10/8/11; 10/27/11; 1/29/12
// ==========================================================================

#include <iostream>
#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/astrofuncs.h"
#include "image/compositefuncs.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "space/imagecdf.h"
#include "image/imagefuncs.h"
#include "osg/osg2D/Movie.h"
#include "osg/osg2D/MoviesGroup.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "math/rotation.h"
#include "space/satellite.h"
#include "space/satellitefuncs.h"
#include "space/satellitepass.h"
#include "astro_geo/ground_radar.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::list;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

void satellitepass::initialize_member_objects() 
{
   nominal_pass=true;
   crossover_elevation=NEGATIVEINFINITY;
   nominal_spacecraft_motion_type=motionfunc::earthstable;
}

void satellitepass::allocate_member_objects() 
{
   ground_radar_ptr=new ground_radar;
}

satellitepass::satellitepass(string sat_name) 
{
   initialize_member_objects();
   allocate_member_objects();
   imagecdf_ptr=new imagecdf(sat_name,this);
}		       

// Copy constructor:

satellitepass::satellitepass(const satellitepass& m)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(m);
}

satellitepass::~satellitepass()
{
   delete imagecdf_ptr;
   delete ground_radar_ptr;

   for (unsigned int i=0; i<currimage_ptr.size(); i++)
   {
      delete currimage_ptr[i];
   }
}

// ---------------------------------------------------------------------
void satellitepass::docopy(const satellitepass & m)
{
}

// Overload = operator:

satellitepass& satellitepass::operator= (const satellitepass& m)
{
   if (this==&m) return *this;
   docopy(m);
   return *this;
}

// ==========================================================================
// Pass initialization member functions
// ==========================================================================

// Boolean member function initialize_pass initializes various radar
// and target parameters.  It returns false if the specified imagecdf
// was not successfully opened.

bool satellitepass::initialize_pass(
   string filename,bool regularize_images,bool subsample_oversampled_images,
   bool clip_huge_images)
{
   imagecdf_ptr->select_file(filename);
   if (imagecdf_ptr->readin_file(regularize_images))
   {
      compute_imagery_overlap();

//      istart=satellitefunc::select_starting_image(
//         input_param_file,inputline,currlinenumber);

      compute_physics_params_for_each_image();
      compute_average_orbit_parameters();
      compute_imageplane_params();

      int istart=0;
      
      rotation R0_rotate;
      for (int i=istart; i<imagecdf_ptr->get_nimages(); i++)
      {
         satellitefunc::construct_imageframe_rotation_matrix(
            currimage_ptr[i]->get_target_ptr()->get_statevector(),
            currimage_ptr[i]->get_ground_radar_ptr()->get_statevector(),
            nominal_spacecraft_motion_type,R0_rotate);
         currimage_ptr[i]->get_target_ptr()->set_R0_rotate(R0_rotate);
//         cout << "image i = " << i 
//              << " R0_rotate = " << currimage_ptr[i]->get_target_ptr()->
//            get_R0_rotate() << endl;

//         cout << "i = " << i
//              << " Xelias imageframe to model rot matrix = "
//              << *(satellitefunc::XELIAS_imageframe_to_model_rotation_matrix(
//                 currimage_ptr[i]->get_target_ptr()->get_R0_rotate()))
//              << endl;
      }

      simplify_raw_images(regularize_images,subsample_oversampled_images);
//      adjust_crossrange_scale_for_sunsync_target();
      return true;
   } // imagecdf_exists conditional
   return false;
}

// ---------------------------------------------------------------------
// Member function simplify_raw_images can either regularize or
// downsample raw images after the opened imagecdf file has been
// closed and recompressed.

void satellitepass::simplify_raw_images(
   bool regularize_images,bool subsample_oversampled_images)
{
   if (regularize_images) 
   {
      regularize_imagery_data();
   }
   else if (subsample_oversampled_images)
   {
      const double min_deltax=0.070;	// meter
      const double min_deltay=0.070;	// meter
      downsample_oversampled_images(min_deltax,min_deltay);
   }
   allocate_and_initialize_working_twoDarrays();
}

// ---------------------------------------------------------------------
// Member function regularize_imagery_data first determines the
// extremal values for xlo,xhi,ylo and yhi for all images within the
// current pass.  It also finds the minimum pixel spacing deltax and
// deltay values.  It then regrids each raw image in the pass so that
// they all have the same numbers of pixels in the range and cross
// range directions.  Moreover, the regridded images' xlo,xhi,ylo and
// yhi values are identical. 

void satellitepass::regularize_imagery_data()
{
   outputfunc::write_banner("Regularizing imagery data:");

   unsigned int mdim_max=NEGATIVEINFINITY;
   unsigned int ndim_max=NEGATIVEINFINITY;
   double xlo_min=POSITIVEINFINITY;
   double xhi_max=NEGATIVEINFINITY;
   double deltax_min=POSITIVEINFINITY;
   double ylo_min=POSITIVEINFINITY;
   double yhi_max=NEGATIVEINFINITY;
   double deltay_min=POSITIVEINFINITY;
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      mdim_max=basic_math::max(mdim_max,currimage_ptr[i]->
                               get_z2Darray_orig_ptr()->get_mdim());
      ndim_max=basic_math::max(ndim_max,currimage_ptr[i]->
                               get_z2Darray_orig_ptr()->get_ndim());
      xlo_min=basic_math::min(xlo_min,currimage_ptr[i]->
                              get_z2Darray_orig_ptr()->get_xlo());
      xhi_max=basic_math::max(xhi_max,currimage_ptr[i]->
                              get_z2Darray_orig_ptr()->get_xhi());
      deltax_min=basic_math::min(deltax_min,currimage_ptr[i]->
                                 get_z2Darray_orig_ptr()->get_deltax());
      ylo_min=basic_math::min(ylo_min,currimage_ptr[i]->
                              get_z2Darray_orig_ptr()->get_ylo());
      yhi_max=basic_math::max(yhi_max,currimage_ptr[i]->
                              get_z2Darray_orig_ptr()->get_yhi());
      deltay_min=basic_math::min(deltay_min,currimage_ptr[i]->
                                 get_z2Darray_orig_ptr()->get_deltay());

//         cout << "xlo = " 
//              << currimage_ptr[i]->z2Darray_orig_ptr->get_xlo()
//              << " xcenter = " << currimage_ptr[i]->x_center 
//              << " xhi = " 
//              << currimage_ptr[i]->z2Darray_orig_ptr->get_xhi() << endl;
//         cout << "ylo = " 
//              << currimage_ptr[i]->z2Darray_orig_ptr->get_ylo()
//              << " ycenter = " << currimage_ptr[i]->y_center 
//              << " yhi = " 
//              << currimage_ptr[i]->z2Darray_orig_ptr->get_yhi() << endl;
//         outputfunc::newline();

   } // loop over i

   int mdim_new=basic_math::round((xhi_max-xlo_min)/deltax_min)+1;
   int ndim_new=basic_math::round((yhi_max-ylo_min)/deltay_min)+1;

//   xhi_max=xlo_min+(mdim_new-1)*deltax_min;
   yhi_max=ylo_min+(ndim_new-1)*deltay_min;
//   cout << "xlo_min = " << xlo_min << " xhi_max = " << xhi_max << endl;
//   cout << "ylo_min = " << ylo_min << " yhi_max = " << yhi_max << endl;
//   cout << "mdim_new = " << mdim_new << " ndim_new = " << ndim_new << endl;

   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      cout << i+1 << " " << flush;
      twoDarray* zregular_twoDarray_ptr=new twoDarray(mdim_new,ndim_new);
      compositefunc::regrid_twoDarray(
         xhi_max,xlo_min,yhi_max,ylo_min,
         currimage_ptr[i]->get_z2Darray_orig_ptr(),zregular_twoDarray_ptr);
      delete currimage_ptr[i]->get_z2Darray_orig_ptr();
      currimage_ptr[i]->set_z2Darray_orig_ptr(zregular_twoDarray_ptr);

   } // loop over index i labeling image number
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function downsample_oversampled_images downsamples any raw
// image whose pixel lengths or widths are smaller than input
// parameter min_deltax and min_deltay.  It then replaces the original
// oversampled raw image with its subsampled counterpart.

void satellitepass::downsample_oversampled_images(
   double min_deltax,double min_deltay)
{
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      unsigned int new_mdim=
         currimage_ptr[i]->get_z2Darray_orig_ptr()->get_mdim();
      unsigned int new_ndim=
         currimage_ptr[i]->get_z2Darray_orig_ptr()->get_ndim();
      twoDarray* z2Darray_small_ptr=NULL;

      if (currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltax() 
          < min_deltax)
      {
         outputfunc::newline();
         cout << "X resolution for raw image " << i+1 << " = " 
              << currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltax() 
              << endl;
         cout << "Regridding so that x resolution = " << min_deltax << endl;

         new_mdim=basic_math::round(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_mdim()*
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltax()/
            min_deltax);
         if (!is_odd(new_mdim)) new_mdim++;
         cout << "new mdim = " << new_mdim << endl;
      }
      
      if (currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltay() 
          < min_deltay)
      {
         outputfunc::newline();
         cout << "Y resolution for raw image " << i+1 << " = " 
              << currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltay() 
              << endl;
         cout << "Regridding so that y resolution = " << min_deltay << endl;
         outputfunc::newline();

         new_ndim=basic_math::round(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_ndim()*
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltay()/
            min_deltay);
         if (!is_odd(new_ndim)) new_ndim++;
         cout << "new ndim = " << new_ndim << endl;
      }
      
      if ((new_mdim != currimage_ptr[i]->get_z2Darray_orig_ptr()->get_mdim())
          ||
          (new_ndim != currimage_ptr[i]->get_z2Darray_orig_ptr()->get_ndim()))
      {
         z2Darray_small_ptr=compositefunc::downsample(
            new_mdim,new_ndim,currimage_ptr[i]->get_z2Darray_orig_ptr());
         delete currimage_ptr[i]->get_z2Darray_orig_ptr();
         currimage_ptr[i]->set_z2Darray_orig_ptr(z2Darray_small_ptr);
      }
   } // loop over index i labeling image number
}

// ---------------------------------------------------------------------
// Member function allocate_and_initialize_working_twoDarrays
// dynamically allocates "working" image twoDarray containers so that
// they have the same sizes as their "raw" counterparts.

void satellitepass::allocate_and_initialize_working_twoDarrays()
{
   
// In order to have some idea of image size in pixels, we keep track
// of the total number of pixels summed over all images within a pass:

   n_total_pixels=0;
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      delete currimage_ptr[i]->get_z2Darray_ptr();
      currimage_ptr[i]->set_z2Darray_ptr(
         new twoDarray(*currimage_ptr[i]->get_z2Darray_orig_ptr()));
      n_total_pixels += currimage_ptr[i]->get_z2Darray_ptr()->get_mdim()
         *currimage_ptr[i]->get_z2Darray_ptr()->get_ndim();
   } // loop over index i labeling image number

   cout << "n_total_pixels = " << n_total_pixels << endl;
}

// ---------------------------------------------------------------------
// Member function compute_imagery_overlap calculates the image
// overlap fraction for each image using their starting and stopping
// times:

void satellitepass::compute_imagery_overlap()
{
   for (int i=0; i<imagecdf_ptr->get_nimages()-1; i++)
   {
      double numer=currimage_ptr[i]->get_stoptime()
         -currimage_ptr[i+1]->get_starttime();
      double denom=currimage_ptr[i]->get_stoptime()
         -currimage_ptr[i]->get_starttime();

      if (numer > 0)
      {
         currimage_ptr[i]->set_overlap_percentage(numer/denom*100);
      }
      else
      {
         currimage_ptr[i]->set_overlap_percentage(0);
      }
      cout << "image " << i << " has overlap percentage = " 
           << currimage_ptr[i]->get_overlap_percentage() << endl;
//      cout << "Start time = " << currimage_ptr[i]->get_starttime()
//           << " stop time = " << currimage_ptr[i]->get_stoptime() << endl;

   }
   currimage_ptr[imagecdf_ptr->get_nimages()-1]->set_overlap_percentage(0);
}

// ---------------------------------------------------------------------
// Member function average_time_interval_for_image takes in integer
// index i labeling some image within the current pass.  This method
// returns a simple, average time interval corresponding to the image
// which approximately takes into account the generally uneven
// temporal distribution of images as well as their nonzero overlap.
// The intervals returned by this method summed over i from 0 to
// imagecdf_ptr->get_nimages()-1 equals
// midtime[imagecdf_ptr->get_nimages()-1]-midtime[0].

double satellitepass::average_time_interval_for_image(int i)
{
   double delta_t=0;
   if (i >= imagecdf_ptr->get_nimages() || i < 0)
   {
      cout << "Error inside satellitepass::average_time_interval_for_image()"
           << endl;
      cout << "i = " << i << " lies outside valid image index range!"
          << endl;
   }
   else if (i==0)
   {
      delta_t=0.5*(currimage_ptr[1]->get_midtime()-
      currimage_ptr[0]->get_midtime());
   }
   else if (i==imagecdf_ptr->get_nimages()-1)
   {
      delta_t=0.5*(currimage_ptr[imagecdf_ptr->get_nimages()-1]->get_midtime()
      -currimage_ptr[imagecdf_ptr->get_nimages()-2]->get_midtime());
   }
   else
   {
      delta_t=0.5*(currimage_ptr[i+1]->get_midtime()-
                   currimage_ptr[i-1]->get_midtime());
   }
   return delta_t;
}

// ---------------------------------------------------------------------
// We gather together within member function
// compute_physics_params_for_each_image several different
// initialization tasks which need to be performed for each image in
// the pass.  In particular, this method computes and stores the sun's
// direction vector, the radar's statevector, image plane parameters
// and direction vectors for each image.

void satellitepass::compute_physics_params_for_each_image()
{
   cout << "inside satellitepass::compute_physics_params_for_each_image()"
        << endl;
   
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      cout << "image i = " << i << endl;
      cout << "midtime = " << currimage_ptr[i]->get_midtime() << endl;

// Compute direction vector towards sun in ECI coordinates.  (This sun
// direction information is used to determine the nominal orientation
// of RH):

      currimage_ptr[i]->set_sun_direction(compute_sun_direction(
         currimage_ptr[i]->get_midtime()));

// Compute orbit normal direction vector.  In theory, this should not
// change as a function of image number:

      statevector sat_statevector(currimage_ptr[i]->get_target_ptr()->
                                  get_statevector());
      currimage_ptr[i]->set_orbit_normal(
         sat_statevector.get_position().cross(
            sat_statevector.get_velocity()).unitvector());

      threevector n_hat=currimage_ptr[i]->get_orbit_normal();
      threevector a_hat=sat_statevector.get_position().unitvector();
      threevector b_hat=n_hat.cross(a_hat);
      
      cout << "normal = " << n_hat 
           << " anti-nadir = " << a_hat << endl;

// Calculate radar's position and velocity vectors in ECI coordinates:

      geopoint ground_radar_geopoint(ground_radar_ptr->get_geolocation());
      statevector ground_radar_statevector(compute_ground_radar_statevector(
         currimage_ptr[i]->get_midtime(),ground_radar_geopoint));
      currimage_ptr[i]->get_ground_radar_ptr()->set_statevector(ground_radar_statevector);

// Compute rhat and qhat unitvectors which define the image plane, and
// depend upon nominal_spacecraft_motion_type.  rhat points in the
// range direction from the radar to the satellite.  qhat points in
// the cross range direction along the vector rhat x omega_total:

      threevector qhat,rhat,phat;
      satellitefunc::compute_qhat_rhat_phat_in_ECI_coords(
         sat_statevector,ground_radar_statevector,
         nominal_spacecraft_motion_type,qhat,rhat,phat);
      currimage_ptr[i]->set_qhat(qhat);
      currimage_ptr[i]->set_rhat(rhat);

// Calculate satellite's orbital and spin angular velocity vectors
// with respect to the radar site on the ground for targets executing
// "earth stable" motion.  (Recall spin angular velocity vector
// vanishes for targets executing "inertial" or "sun-sync" motion.)

      threevector w_orbit,w_spin_earthstable;
      satellitefunc::compute_angular_velocities(
         sat_statevector,ground_radar_statevector,
         w_orbit,w_spin_earthstable);
      currimage_ptr[i]->set_angular_velocities(w_orbit,w_spin_earthstable);

   } // loop over index i labeling satellite images
}

// ==========================================================================
// Basic astro and ECI coordinate system member functions:
// ==========================================================================

// Member function compute_sun_direction takes in a time measured in
// seconds since midnight of the current pass object's date.  It
// returns a unit vector in ECI coordinates which points in the
// direction of the sun at the input time.

threevector satellitepass::compute_sun_direction(double currtime)
{
   double sun_RA;   // sun's right ascension (degs)
   double sun_DEC;  // sun's declination (degs)
   astrofunc::sun_RA_DEC(
      imagecdf_ptr->get_pass_date(),currtime,sun_RA,sun_DEC);
   return astrofunc::ECI_vector(sun_RA,sun_DEC);
}

// ---------------------------------------------------------------------
// We wrote this utility primarily for synthetic pass generation
// purposes.

double satellitepass::sidereal_angle(double currtime)
{
   return astrofunc::sidereal_time(
      imagecdf_ptr->get_pass_date(),currtime)/24.0*(2*PI);
}

// ==========================================================================
// Target and radar state vector computation member functions
// ==========================================================================

// Member function compute_average_orbit_parameters uses all state
// vector information for each image in the pass to determine an
// average target angular velocity, orbit radius, semimajor axis,
// eccentricity and Runge-Lenz vector.  The target's orbit is assumed
// to be elliptical.

void satellitepass::compute_average_orbit_parameters()
{
   double midtime[imagecdf_ptr->get_nimages()];
   double R[imagecdf_ptr->get_nimages()];
   threevector curr_omega_vec[imagecdf_ptr->get_nimages()];
   threevector curr_A_vec[imagecdf_ptr->get_nimages()];
   
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      midtime[i]=currimage_ptr[i]->get_midtime();

      statevector posn_vel(currimage_ptr[i]->get_target_ptr()->
                           get_statevector());
      threevector curr_Rvec(posn_vel.get_position());
      R[i]=curr_Rvec.magnitude();
      threevector R_hat(curr_Rvec.unitvector());

      threevector curr_V(posn_vel.get_velocity());
      double Vr=curr_V.dot(R_hat);
      threevector Vtheta_vec(curr_V-Vr*R_hat);
      double Vtheta=Vtheta_vec.magnitude();
      threevector theta_hat(Vtheta_vec.unitvector());
      double omega=Vtheta/R[i];
      threevector omega_hat(R_hat.cross(theta_hat));
      curr_omega_vec[i]=omega*omega_hat;
      curr_A_vec[i]=curr_V.cross(curr_Rvec.cross(curr_V))
         -geofunc::Gnewton_times_mearth*R_hat;
   }
   double Rsat_avg=templatefunc::average(
      imagecdf_ptr->get_nimages(),midtime,R);
   orbit.set_omega_avg(templatefunc::average(
      imagecdf_ptr->get_nimages(),midtime,curr_omega_vec));
//   cout << "Rsat_avg = " << Rsat_avg << endl;
//   cout << "omega_sat_avg = " << orbit.get_omega_avg() << endl;
   
// Recall from section 3.9 in Goldstein's "Classical Mechanics" that
// the Runge-Lenz vector represents a constant of motion for
// elliptical orbits.  It points in the direction of the orbit's
// perigee (=point of closest approach to one of the ellipse's foci).
// Our version of the Runge-Lenz vector has the satellite's squared
// mass divided out:

   orbit.set_Runge_Lenz_vector(templatefunc::average(
      imagecdf_ptr->get_nimages(),midtime,curr_A_vec));

   orbit.set_eccentricity(orbit.get_Runge_Lenz_vector().magnitude()/
                          geofunc::Gnewton_times_mearth);
   orbit.set_a_semimajor(
      pow(geofunc::Gnewton_times_mearth/sqr(
         orbit.get_omega_avg().magnitude()),0.333333));

//   cout.precision(10);
//   cout << "orbit_eccentricity = " << orbit.get_eccentricity() << endl;
//   cout << "a_semimajor = " << orbit.get_a_semimajor() << endl;
//   cout << "<Rsat> = " << Rsat_avg << endl;

// By definition, orbit.a_semimajor must be >= Rsat_avg.  But on
// 10/10/20 we saw numerous examples where imperfect state vector
// propagation leads to Rsat_avg > orbit.a_semimajor.  In this case
// where Kepler's formula fails, we simply set orbit.a_semimajor = Rsat_avg.

   orbit.set_a_semimajor(basic_math::max(Rsat_avg,orbit.get_a_semimajor()));

//   double dotproduct=orbit.get_Runge_Lenz_vector().dot(
//	orbit.get_omega_avg());
//   double cosangle=dotproduct/(orbit.get_Runge_Lenz_vector().magnitude()*
//                               orbit.get_omega_avg().magnitude());
//   cout << "Angle between Runge_Lenz vector and omega_sat = " 
//        << acos(cosangle)*180/PI << endl;
}

// ---------------------------------------------------------------------
// Member function determine_pass_handedness computes the dotproduct
// between the range vector (in ECI coordinates) and shat = vhat x
// that (in ECI coordinates).  A pass is said to be left (right)
// handed if this dotproduct is greater (less than) 0.

void satellitepass::determine_pass_handedness()
{
   outputfunc::newline();
   cout << "Determining pass handedness:   ";
   
   double dotproductsum=0;
   threevector Rhat,vhat,that,shat;
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      statevector posn_vel(currimage_ptr[i]->get_target_ptr()->
                           get_statevector());
      Rhat=-posn_vel.get_position().unitvector(); 
		// points from satellite to earth's center
      vhat=posn_vel.get_velocity().unitvector();

      double dotproduct=Rhat.dot(vhat);
      double denom=sqrt(1-sqr(dotproduct));
      that=(Rhat-dotproduct*vhat)/denom;
      shat=that.cross(vhat);
      dotproductsum += shat.dot(currimage_ptr[i]->get_rhat());
   }

   outputfunc::newline();
   if (sgn(dotproductsum) >= 0)
   {
      left_handed_pass=true;
      cout << "Pass is left handed" << endl;
   }
   else
   {
      left_handed_pass=false;
      cout << "Pass is right handed" << endl;
   }
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function compute_ground_radar_statevector returns the radar's
// absolute position and velocity vectors in ECI coordinates given the
// time currtime in seconds since midnight UTC of the pass day.  This
// method also uses the radar's latitude, longitude east of Greenwich,
// and height above the ellipsoid surface along with the pass date.

statevector satellitepass::compute_ground_radar_statevector(
   double currtime,const geopoint& ground_radar_geoposn)
{

// Earth eccentricity:
//   const double e=sqrt(
//      2*geofunc::f_earth_flatten-sqr(geofunc::f_earth_flatten));	

// First compute radar's location in ECI coordinates assuming that the
// Greenwich meridian lies within the xhat-zhat plane:
   
   double term1=geofunc::R_earth/
      sqrt(1-sqr(geofunc::eccentricity*sin(ground_radar_geoposn.get_latitude()*
                                           PI/180)));
   double x=(term1+ground_radar_geoposn.get_altitude())
      *cos(ground_radar_geoposn.get_latitude()*PI/180);
   double z=(term1*(1-sqr(geofunc::eccentricity))+
             ground_radar_geoposn.get_altitude())*
      sin(ground_radar_geoposn.get_latitude()*PI/180);
   double phi=geofunc::omega_earth*currtime+ground_radar_geoposn.get_longitude()*
      PI/180;
   threevector radar_posn(cos(phi)*x,sin(phi)*x,z);  // meters

// Next compute azimuthal angle measured relative to the xhat-zhat
// plane corresponding to actual location of Greenwich meridian at UTC
// midnight on the date implicitly specified by input parameter
// currtime.  Rotate radar_posn about +zhat by phi_greenwich:

   radar_posn=geofunc::convert_MAXLIK_to_ECI_coords(
      imagecdf_ptr->get_pass_date(),currtime,radar_posn);

   threevector zhat(0,0,1);
   threevector radar_vel(geofunc::omega_earth*zhat.cross(radar_posn));

   return statevector(currtime,radar_posn,radar_vel);
}

// =====================================================================
// Target elevation member functions:
// =====================================================================

// Member function plot_target_orbit_geometry_wrt_sensor() first finds
// all points along the target's orbit whose elevation values are
// positive.  It subsequently plots the range, azimuth and elevation
// of the target between its rising and setting times wrt to the
// ground_radar on the ground.

void satellitepass::plot_target_orbit_geometry_wrt_sensor()
{
   outputfunc::write_banner("Plotting target orbit geometry wrt ground_radar:");

//   linkedlist full_range_list(true);
//   linkedlist full_azimuth_list(true);
//   linkedlist full_elevation_list(true);

   list<pair<double,threevector> > total_az_el_range=
      compute_positive_elevations(
         currimage_ptr[0]->get_ground_radar_ptr()->get_statevector().
         get_position(),
         currimage_ptr[0]->get_target_ptr()->get_statevector().
         get_position());

// Write out positive elevation vs time for entire pass:

//   plot_range_vs_time(full_range_list);
//   plot_azimuth_vs_time(full_azimuth_list);
//   plot_elevation_vs_time(full_elevation_list);
//   satelliteoutputfunc::generate_geometry_script(imagedir);
}

// ---------------------------------------------------------------------
// Member function compute_positive_elevations finds points along the
// elliptical orbit with semi-major axis orbit.a_semimajor,
// eccentricity orbit.eccentricity, Runge-Lenz direction vector
// orbit.Runge_Lenz_vector and orbital angular vector orbit.omega_avg
// whose elevation angles with respect to the radar are positive.  The
// results are returned within the linkedlist full_elevation_list.
// The target's rising and setting times which bracket the interval
// when the target is in view of the radar are also calculated in this
// method.  Finally, this method loads member variable
// crossover_elevation with the crossover elevation in degrees.

list<pair<double,threevector> > satellitepass::compute_positive_elevations(
   const threevector& init_ground_radar_posn,const threevector& init_target_posn)
{

// First find azimuthal angular location gamma_init along orbit path
// corresponding to initial satellite position:

   double gamma_init=orbit.find_azimuthal_orbit_angle_gamma(
      init_target_posn);
   double gamma_lo=gamma_init-PI;
   double gamma_hi=gamma_init+PI;
   double dgamma=0.1*PI/180;
   int nbins=basic_math::round((gamma_hi-gamma_lo)/dgamma)+1;

   t_rising=POSITIVEINFINITY;
   t_setting=NEGATIVEINFINITY;

// Instantiate a ground_radar object which can exist at all times and not
// just image times:

   ground_radar curr_ground_radar;

// Instantiate an STL linked list to hold target azimuth, elevation
// and range information at all times (and not just image times):

   list<pair<double,threevector> > total_az_el_range;

   for (int i=0; i<nbins; i++)
   {
      double gamma=gamma_lo+i*dgamma;
      double t=(gamma-gamma_init)/orbit.get_omega_avg().magnitude();

      double range,azimuth,elevation;
      satellitefunc::compute_target_range_az_el(
         curr_ground_radar.compute_position(t,init_ground_radar_posn),
         orbit.target_location(0,init_target_posn,t),
         range,azimuth,elevation);
      crossover_elevation=basic_math::max(crossover_elevation,elevation);

      if (elevation > 0)
      {
//         cout << "i = " << i
//              << " gamma = " << gamma*180/PI 
//         cout << "t = " << t
//              << " az = " << azimuth
//              << " el = " << elevation << endl;
//              << " crossover = " << crossover_elevation << endl;

         threevector az_el_range(azimuth,elevation,0.001*range);
					         // measure range in kms
         pair<double,threevector> p(t,az_el_range);
         total_az_el_range.push_back(p);

         if (t < t_rising) t_rising=t;
         if (t > t_setting) t_setting=t;
      }
   } // loop over index i labeling elevation bin
   time_target_above_horizon=t_setting-t_rising;

   cout << "Crossover elevation = " << crossover_elevation << " degs" << endl;
   cout << "Time relative to first image when target rises above horizon = " 
        << t_rising << " secs" << endl;
   cout << "Time relative to first image when target sets below horizon = " 
        << t_setting << " secs" << endl;
   cout << "Total time targets spends above horizon = " 
        << time_target_above_horizon << " secs" << endl;
   
   return total_az_el_range;
}

// ---------------------------------------------------------------------
// Member function generate_actual_image_az_el_range_list fills up an
// STL list with target azimuth, elevation and range information
// corresponding to actual image times.

list<pair<double,threevector> > 
satellitepass::generate_actual_image_az_el_range_list() const
{
   list<pair<double,threevector> > actual_image_az_el_range;
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      double currt=currimage_ptr[i]->get_reltime();
      threevector az_el_range(currimage_ptr[i]->get_target_ptr()->
                              get_az_el_range());
      az_el_range.put(2,0.001*az_el_range.get(2)); // measure range in kms

      pair<double,threevector> p(currt,az_el_range);
      actual_image_az_el_range.push_back(p);
   } // loop over index i labeling images
   return actual_image_az_el_range;
}

/*

// ---------------------------------------------------------------------
void satellitepass::plot_range_vs_time(linkedlist& full_range_list) const
{
   linkedlist actual_image_ranges_list(true);
   generate_actual_image_ranges_list(actual_image_ranges_list);
   satelliteoutputfunc::plot_range_vs_time(
      imagedir,t_rising,t_setting,crossover_elevation,
      full_range_list,actual_image_ranges_list);
}

void satellitepass::plot_azimuth_vs_time(linkedlist& full_azimuth_list) const
{
   linkedlist actual_image_azimuths_list(true);
   generate_actual_image_azimuths_list(actual_image_azimuths_list);
   satelliteoutputfunc::plot_azimuth_vs_time(
      imagedir,t_rising,t_setting,crossover_elevation,
      full_azimuth_list,actual_image_azimuths_list);
}

void satellitepass::plot_elevation_vs_time(
   linkedlist& full_elevation_list) const
{
   linkedlist actual_image_elevations_list(true);
   generate_actual_image_elevations_list(actual_image_elevations_list);
   satelliteoutputfunc::plot_elevation_vs_time(
      imagedir,t_rising,t_setting,crossover_elevation,
      full_elevation_list,actual_image_elevations_list);

// For demonstration purposes, dynamically pop open window displaying
// pass geometry results:

   if (pop_open_window)
   {
//      sysfunc::run_xv(full_elevation_list.get_metafile_ptr()->get_filename()
//             +".jpg",594,459,0,0);
      sysfunc::run_xv(full_elevation_list.get_metafile_ptr()->get_filename()
                      +".jpg",display_machine_name);
//      import_root_scrn();
   }
}

// In this overloaded version of member function
// plot_elevation_vs_time, the elevation point corresponding to the
// specified image number is colored red.  We separated off this
// specialized method for doublet synthetic pass movie making purposes:

void satellitepass::plot_elevation_vs_time(
   int imagenumber,linkedlist& full_elevation_list) const
{
   linkedlist actual_images_list(true);
   generate_actual_image_elevations_list(actual_images_list);

   full_elevation_list.get_metafile_ptr()->set_parameters(
      full_elevation_list.get_metafile_ptr()->get_filename(),"Pass Geometry",
      "Time since first radar image collection (secs)",
      "Target elevation angle (degs)",basic_math::min(t_rising,-0.1),t_setting,
      4.5,5.8,-0.001,90,10,5,4.5,2.2);
   plotfunc::writelist_member(full_elevation_list);
   
   actual_images_list.get_metafile_ptr()->set_filename(
      full_elevation_list.get_metafile_ptr()->get_filename());
   actual_images_list.get_node(imagenumber)->get_data().
      set_color(colorfunc::red);
   plotfunc::append_plot(actual_images_list);
}
*/

// ==========================================================================
// Image frame variable computation member functions
// ==========================================================================

// Member function compute_imageplane_params determines the image
// plane based upon state vector information:

void satellitepass::compute_imageplane_params()
{
   project_statevectors_into_imageplane();
   compute_qnom_mags_and_earthstable_to_sunsync_ratios();
   if (nominal_spacecraft_motion_type==motionfunc::sunsync)
      compute_earthstable_to_sunsync_scalefactors();
}

// ---------------------------------------------------------------------
// Member function project_statevectors_into_imageplane calculates the
// components of the satellite position and velocity vectors within
// the phat, qhat and rhat coordinate system.  For purposes of
// comparing kinematic results with XELIAS, we project the satellite's
// spin angular velocity into the image plane as well.  Recall that
// the image plane is defined by rhat (which points in the range
// direction) and qhat=rhat x omega_total (which points in the cross
// range direction).  The state vector components along phat=qhat x
// rhat are orthogonal to the image plane.  Omega_total lies in the
// phat-rhat plane, while omega_orbital = rhatdot x rhat lies in the
// phat-qhat plane.  Only omega_spin_earthstable has a nontrivial
// projection within the qhat-rhat image plane.
// Project_statevectors_into_imageplane computes the angles phi and
// theta which all these vectors respectively make with the qhat axis
// and the phat-qhat plane:

void satellitepass::project_statevectors_into_imageplane()
{
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      statevector target_statevector(
         currimage_ptr[i]->get_target_ptr()->get_statevector());
      statevector ground_radar_statevector(
         currimage_ptr[i]->get_ground_radar_ptr()->get_statevector());
      threevector R_imageframe(satellitefunc::ECI_to_imageframe(
         target_statevector,ground_radar_statevector,
         nominal_spacecraft_motion_type,
         target_statevector.get_position()));
//      double Rq=R_imageframe.get(0);
//      double Rr=R_imageframe.get(1);
//      double Rp=R_imageframe.get(2);

      threevector V_imageframe(satellitefunc::ECI_to_imageframe(
         target_statevector,ground_radar_statevector,
         nominal_spacecraft_motion_type,
         target_statevector.get_velocity()));
//      double Vq=V_imageframe.get(0);
//      double Vr=V_imageframe.get(1);
//      double Vp=V_imageframe.get(2);

      threevector sun_imageframe(satellitefunc::ECI_to_imageframe(
         target_statevector,ground_radar_statevector,
         nominal_spacecraft_motion_type,
         currimage_ptr[i]->get_sun_direction()));
//      double Sq=sun_imageframe.get(0);
//      double Sr=sun_imageframe.get(1);
//      double Sp=sun_imageframe.get(2);

/*
      currimage_ptr[i]->get_target_ptr()->phi_r=atan2(Rp,Rq);
      currimage_ptr[i]->get_target_ptr()->theta_r=
         atan2(Rr,sqrt(sqr(Rp)+sqr(Rq)));
      currimage_ptr[i]->get_target_ptr()->phi_v=atan2(Vp,Vq);
      currimage_ptr[i]->get_target_ptr()->theta_v=
         atan2(Vr,sqrt(sqr(Vp)+sqr(Vq)));
      currimage_ptr[i]->get_target_ptr()->phi_sun=atan2(Sp,Sq);
      currimage_ptr[i]->get_target_ptr()->theta_sun=
         atan2(Sr,sqrt(sqr(Sp)+sqr(Sq)));
*/

// Calculate angle between nadir and image planes.  On 11/20/01, we
// verified that this angle equals 90-maximum target elevation angle
// to very good approximation:

//      threevector crossproduct=currimage_ptr[i]->get_target_ptr()->
//         get_statevector().get_velocity().cross(
//            currimage_ptr[i]->get_target_ptr()->get_statevector().
//            get_position());
//      double theta=acos(phat.dot(crossproduct.unitvector()));
//      cout << "Angle between nadir and image planes = " 
//           << theta*180/PI << endl;
   } // loop over index i labeling image number
}

// ---------------------------------------------------------------------
// Member function compute_qnom_mags_and_earthstable_to_sunsync_ratios
// computes the nominal cross range vectors q_earthstable and
// q_sunsync for nominal earthstable and sun-synchronous satellite
// motion.  It then sets member variable
// satelliteimage::qnominal_magnitude equal to the magnitude of
// whichever cross range vector corresponds to
// nominal_spacecraft_motion_type.  It also saves the ratio of the
// earthstable to sun-sunchronous cross range vector magnitudes within
// satelliteimage::earthstable_to_sunsync_scalefactor.

void satellitepass::compute_qnom_mags_and_earthstable_to_sunsync_ratios()
{
   threevector q_earthstable,q_sunsync;
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      satellitefunc::compute_nominal_crossrange_vector(
         currimage_ptr[i]->get_target_ptr()->get_statevector(),
         currimage_ptr[i]->get_ground_radar_ptr()->get_statevector(),
         motionfunc::earthstable,q_earthstable);
      satellitefunc::compute_nominal_crossrange_vector(
         currimage_ptr[i]->get_target_ptr()->get_statevector(),
         currimage_ptr[i]->get_ground_radar_ptr()->get_statevector(),
         motionfunc::sunsync,q_sunsync);
      if (nominal_spacecraft_motion_type==motionfunc::earthstable)
      {
         currimage_ptr[i]->set_qnominal_magnitude(q_earthstable.magnitude());
      }
      else if (nominal_spacecraft_motion_type==motionfunc::sunsync)
      {
         currimage_ptr[i]->set_qnominal_magnitude(q_sunsync.magnitude());
      }
      currimage_ptr[i]->set_earthstable_to_sunsync_scalefactor(
         q_earthstable.magnitude()/q_sunsync.magnitude());
   } 
}

// ---------------------------------------------------------------------
// Member function compute_earthstable_to_sunsync_scalefactors saves
// the ratio of the earth stable and sun synchronous cross range
// vectors for each image within member variable
// earthstable_to_sunsync_scalefactor[i].

void satellitepass::compute_earthstable_to_sunsync_scalefactors()
{
   if (nominal_spacecraft_motion_type==motionfunc::sunsync)
   {
      threevector q_earthstable,q_sunsync;
      for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
      {
         satellitefunc::compute_nominal_crossrange_vector(
            currimage_ptr[i]->get_target_ptr()->get_statevector(),
            currimage_ptr[i]->get_ground_radar_ptr()->get_statevector(),
            motionfunc::earthstable,q_earthstable);
         satellitefunc::compute_nominal_crossrange_vector(
            currimage_ptr[i]->get_target_ptr()->get_statevector(),
            currimage_ptr[i]->get_ground_radar_ptr()->get_statevector(),
            motionfunc::sunsync,q_sunsync);
         currimage_ptr[i]->set_earthstable_to_sunsync_scalefactor(
            q_earthstable.magnitude()/q_sunsync.magnitude());
      }
   } // nominal_spacecraft_motion_type==motionfunc::sunsync conditional
}

// ---------------------------------------------------------------------
// Adjust cross range vector and scale if radar images were generated
// assuming earth stable motion, yet target actually executed sun
// synchronous motion.  

void satellitepass::adjust_crossrange_scale_for_sunsync_target()
{
   if (nominal_spacecraft_motion_type==motionfunc::sunsync 
       && imagecdf_ptr->get_imagery_motion_type()==motionfunc::general_ypr)
   {
      for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
      {
         currimage_ptr[i]->get_z2Darray_orig_ptr()->set_xlo(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_xlo()*
            currimage_ptr[i]->get_earthstable_to_sunsync_scalefactor());
         currimage_ptr[i]->get_z2Darray_orig_ptr()->set_xhi(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_xhi()*
            currimage_ptr[i]->get_earthstable_to_sunsync_scalefactor());
         currimage_ptr[i]->get_z2Darray_orig_ptr()->set_deltax(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltax()*
            currimage_ptr[i]->get_earthstable_to_sunsync_scalefactor());
         currimage_ptr[i]->get_z2Darray_ptr()->set_xlo(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_xlo());
         currimage_ptr[i]->get_z2Darray_ptr()->set_xhi(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_xhi());
         currimage_ptr[i]->get_z2Darray_ptr()->set_deltax(
            currimage_ptr[i]->get_z2Darray_orig_ptr()->get_deltax());
      }      
   } // nominal_spacecraft_motion_type==motionfunc::sunsync conditional
}

// ==========================================================================
// Pass output member functions
// ==========================================================================

// Member function write_videofile converts the intensity information
// with STL vector currimage_ptr into a Group99 video file which can
// subsequently be viewed via our OSG program video.  This method
// first writes out a video file where are RCS intensities are
// displayed on a greyscale color map.  It subsequently reads the grey
// video file back in and converts its intensity information into RGB
// values using a jet-like color map.  If input
// delete_grey_video_flag==true, the grey video file is deleted at the
// end of this method.

void satellitepass::write_videofile(
   bool delete_grey_video_flag,bool equalize_intensity_histogram_flag)
{

// Instantiate a greyscale VidFile and copy byte data within each
// satellite image of STL vector currimage_ptr into it:

   VidFile grey_vid_out;
   string grey_video_filename=imagecdf_ptr->get_passname()+"_grey.vid";
   
   int width=currimage_ptr[0]->get_z2Darray_ptr()->get_xdim();
   int height=currimage_ptr[0]->get_z2Darray_ptr()->get_ydim();
   int n_images=imagecdf_ptr->get_nimages();
   int n_color_channels=1;
   grey_vid_out.New_8U(grey_video_filename.c_str(),width,height,n_images,
                       n_color_channels);

   unsigned char* data_ptr=new unsigned char[width*height*n_color_channels];
   for (int i=0; i<n_images; i++)
   {
      if (equalize_intensity_histogram_flag)
         imagefunc::equalize_intensity_histogram(
            currimage_ptr[i]->get_z2Darray_ptr());

      grey_vid_out.set_intensity_twoDarray_ptr(
         currimage_ptr[i]->get_z2Darray_ptr());
      int width=currimage_ptr[i]->get_z2Darray_ptr()->get_xdim();
//      int height=currimage_ptr[i]->get_z2Darray_ptr()->get_ydim();
      grey_vid_out.convert_intensity_twoDarray_to_charstar_array(data_ptr);
      grey_vid_out.WriteFrame(data_ptr,width*n_color_channels);
   }
   delete [] data_ptr;

// Read grey scale video file back into a Movie object.  Then convert
// intensities from a grey to a jet color map:
 
   PassesGroup passes_group(grey_video_filename);
   int videopass_ID=0;
   passes_group.generate_new_pass(0,videopass_ID);

   const int ndims=2;

   AnimationController_ptr=new AnimationController(n_images);
   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);

   int map_number=0;
//   cout << "Enter color map number:" << endl;
//   cin >> map_number;
   movie_ptr->change_color_map(map_number);

   const int bytes_per_pixel=3;
   string jet_video_filename=imagecdf_ptr->get_passname()+"_jet.vid";
   cout << "jet_video_filename = " << jet_video_filename << endl;
   VidFile jet_vid_out;
   jet_vid_out.New_8U(jet_video_filename.c_str(),movie_ptr->getWidth(),
                      movie_ptr->getHeight(), movie_ptr->get_Nimages(), 
                      bytes_per_pixel);

   movie_ptr->displayFrame(0);
   for (int i=0; i<movie_ptr->get_Nimages(); i++)
   {
      cout << i << " " << flush;
      jet_vid_out.WriteFrame(
         movie_ptr->get_m_colorimage_ptr(), movie_ptr->getWidth()*
         bytes_per_pixel);
      AnimationController_ptr->increment_frame_counter();
      movie_ptr->display_current_frame();
   }
   delete AnimationController_ptr;

   outputfunc::newline();

   if (delete_grey_video_flag) filefunc::deletefile(grey_video_filename);
}

// ---------------------------------------------------------------------
// Member function write_imagecdf

void satellitepass::write_imagecdf(bool equalize_intensity_histogram_flag)
{
   for (int i=0; i<imagecdf_ptr->get_nimages(); i++)
   {
      if (equalize_intensity_histogram_flag)
         imagefunc::equalize_intensity_histogram(
            currimage_ptr[i]->get_z2Darray_ptr());
   }
   imagecdf_ptr->writeout_file();
}

