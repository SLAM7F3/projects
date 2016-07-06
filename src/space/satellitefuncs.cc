// ==========================================================================
// SATELLITEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 7/28/06; 7/31/06; 1/29/12
// ==========================================================================

#include "math/basic_math.h"
#include "astro_geo/geofuncs.h"
#include "geometry/mypoint.h"
#include "math/rotation.h"
#include "space/satellitefuncs.h"
#include "math/statevector.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

namespace satellitefunc
{

// =====================================================================
// Pass initialization methods
// =====================================================================

// Method select_starting_image queries the user to enter the number
// of the first image to process.  We follow the XELIAS numbering
// convention which labels the first image as number 1 rather than 0.
// However, the integer which is returned by this method is
// decremented by one so that the internal number of the first image
// is 0.

   int select_starting_image(
      bool input_param_file,string inputline[],unsigned int& currlinenumber)
      {
         bool valid_response;
         string outputline[1];

         int istart;
         {
            do
            {
               outputline[0]="Enter starting image number:";
               istart=stringfunc::mygetinteger(
                  1,outputline,input_param_file,inputline,currlinenumber);
               if (istart >=1)
               {
                  valid_response=true;
                  istart--;
               }
               else
               {
                  valid_response=false;
                  cout << "Starting image number should be >= 1" << endl;
               }
            }
            while (!valid_response);
         }
         return istart;
      }

// =====================================================================
// Image frame initialization methods
// =====================================================================

// Methods compute_range_unitvector and compute_angular_velocities
// take in the satellite's and radar's state vectors.  The former
// returns the range unit vector, while the latter returns the orbital
// angular velocity vector and the earth stable spin angular velocity
// vector in ECI coordinates.

   void compute_range_unitvector(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,threevector& rhat)
      {
         threevector range(satellite_statevector.get_position()
            -radar_statevector.get_position());
         rhat=range.unitvector();
      }

   void compute_angular_velocities(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      threevector& omega_orbital,threevector& omega_spin_earthstable)
      {
         threevector sat_posn(satellite_statevector.get_position());
         threevector sat_vel(satellite_statevector.get_velocity());
         threevector radar_posn(radar_statevector.get_position());
         threevector radar_vel(radar_statevector.get_velocity());

         threevector range(sat_posn-radar_posn);
         threevector rangedot(sat_vel-radar_vel);
         double r=range.magnitude();
         threevector rhat(range.unitvector());
         double rdot=rangedot.dot(rhat);
         threevector rhat_dot((rangedot-rdot*rhat)/r);

         omega_orbital=rhat_dot.cross(rhat);
         omega_spin_earthstable=		// earth stable motion
            sat_posn.unitvector().cross(sat_vel/sat_posn.magnitude());
      }

// ---------------------------------------------------------------------
// Method compute_nominal_crossrange_vector returns the cross range
// vector in ECI coordinates for either earth stable or sun
// synchronous motion, depending upon input Motion_type
// nominal_spacecraft_motion_type.

   void compute_nominal_crossrange_vector(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& curr_qvec)
      {
         threevector rhat;
         compute_range_unitvector(
            satellite_statevector,radar_statevector,rhat);
         threevector omega_orbital,omega_spin_earthstable;
         compute_angular_velocities(
            satellite_statevector,radar_statevector,
            omega_orbital,omega_spin_earthstable);

         threevector omega_spin;
         if (nominal_spacecraft_motion_type==motionfunc::sunsync)
         {
            omega_spin=threevector(0,0,0);
         }
         else if (nominal_spacecraft_motion_type==motionfunc::earthstable)
         {
            omega_spin=omega_spin_earthstable;
         }
         curr_qvec=rhat.cross(omega_orbital+omega_spin);
      }

// ---------------------------------------------------------------------
// Method compute_qhat_rhat_phat_in_ECI_coords takes in satellite and
// radar state vector information.  It returns the image frame basis
// vectors qhat, rhat and phat in ECI coordinates which correspond to
// the motion indicated by input motionfunc::Motion_type
// nominal_spacecraft_motion_type.

   void compute_qhat_rhat_phat_in_ECI_coords(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& qhat,threevector& rhat,threevector& phat)
      {
         threevector curr_qvec;
         compute_nominal_crossrange_vector(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,curr_qvec);
         qhat=curr_qvec.unitvector();
         compute_range_unitvector(
            satellite_statevector,radar_statevector,rhat);
         phat=qhat.cross(rhat);
      }

// ---------------------------------------------------------------------
// Method construct_imageframe_rotation_matrix forms the CONSTANT
// rotation matrix R0_rotate which is used to manipulate the RH
// satellite wireframe so that the telescope points exactly towards
// the center of the earth, while the primary RH symmetry axis is
// aligned with the orbit normal.

// Note added on 1/10/02: Application of R0_rotate brings the RH
// wireframe model into its nominal orientation in the IMAGE FRAME
// basis (not the ECI basis !)

   void construct_imageframe_rotation_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      rotation& R0_rotate)
      {
         threevector rhat_0,vhat_0,Rhat_0,that_0,shat_0,what_0;
         compute_imageframe_direction_vectors(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            rhat_0,vhat_0,Rhat_0,that_0,shat_0,what_0);
/*
         cout << "inside satellitefunc::construct_imageframe_Rot_matrix()"
              << endl;
         cout << "rhat_0 = " << rhat_0.get(0) << "," << rhat_0.get(1)
              << "," << rhat_0.get(2) << endl;
         cout << "vhat_0 = " << vhat_0.get(0) << "," << vhat_0.get(1)
              << "," << vhat_0.get(2) << endl;
         cout << "Rhat_0 = " << Rhat_0.get(0) << "," << Rhat_0.get(1)
              << "," << Rhat_0.get(2) << endl;
         cout << "that_0 = " << that_0.get(0) << "," << that_0.get(1)
              << "," << that_0.get(2) << endl;
         cout << "shat_0 = " << shat_0.get(0) << "," << shat_0.get(1)
              << "," << shat_0.get(2) << endl;
         cout << "what_0 = " << what_0.get(0) << "," << what_0.get(1)
              << "," << what_0.get(2) << endl;
*/
         construct_imageframe_rotation_matrix(what_0,shat_0,Rhat_0,R0_rotate);
      }

// ---------------------------------------------------------------------
// Method construct_imageframe_rotation_matrix forms the CONSTANT
// matrix R0_rotate which is used to manipulate the A2/AU satellite
// wireframe so that the hinge point beam axis is aligned with vhat_0
// while the telescope axis lies within the plane defined by vhat_0
// and Rhat_0.  Alternatively, this method forms the CONSTANT rotation
// matrix R0_rotate which is used to manipulate the RH satellite
// wireframe so that the telescope points exactly towards the center
// of the earth, while the primary RH symmetry axis is aligned with
// the orbit normal.

// Note added on 1/10/02: Application of R0_rotate brings the A2/AU/RH
// wireframe models into their nominal orientations in the IMAGE FRAME
// basis (not the ECI basis !)

   void construct_imageframe_rotation_matrix(
      const threevector& what_0,const threevector& shat_0,
      const threevector& Rhat_0,rotation& R0_rotate)
      {
         R0_rotate.put_column(0,what_0);
         R0_rotate.put_column(1,shat_0);
         R0_rotate.put_column(2,Rhat_0);
       }

// ---------------------------------------------------------------------
// Xelias returns a dynamically generated genmatrix containing the
// product of its "Image Axes * transpose( Model Axes)" which we need
// to align image planes to IGES models.  On 7/26/06, we empirically
// deduced the relationship between this XELIAS rotation and our
// R0_rotate.

   rotation* XELIAS_imageframe_to_model_rotation_matrix(
      const rotation& R0_rotate)
      {
         rotation* R_XELIAS_ptr=new rotation();
         
         R_XELIAS_ptr->put(0,0,-R0_rotate.get(0,1));
         R_XELIAS_ptr->put(1,0,-R0_rotate.get(1,1));
         R_XELIAS_ptr->put(2,0,-R0_rotate.get(2,1));

         R_XELIAS_ptr->put(0,1,-R0_rotate.get(0,2));
         R_XELIAS_ptr->put(1,1,-R0_rotate.get(1,2));
         R_XELIAS_ptr->put(2,1,-R0_rotate.get(2,2));
         
         R_XELIAS_ptr->put(0,2,R0_rotate.get(0,0));
         R_XELIAS_ptr->put(1,2,R0_rotate.get(1,0));
         R_XELIAS_ptr->put(2,2,R0_rotate.get(2,0));
         
         return R_XELIAS_ptr;
      }

// ---------------------------------------------------------------------
// Member function compute_imageframe_direction_vectors takes in
// satellite and radar statevector information.  It returns the IMAGE
// FRAME (not ECI !) components of the satellite range direction
// vector rhat_0, the nadir direction vector Rhat_0, the velocity
// direction vector vhat_0, the ZERO body roll direction vector that_0
// (which is orthogonal to vhat_0 and which lies as close as possible
// to Rhat_0), the anti-orbit normal direction vector shat_0=that_0 x
// vhat_0, and what_0=shat_0 x Rhat_0.  Note that Rhat_0 is
// numerically close to that_0 and what_0 is numerically close to
// vhat_0 for nearly circular orbits.

// IMPORTANT NOTE: We define Rhat_0 here to be the nadir direction
// vector which points FROM the satellite TO the earth's center.  This
// convention is opposite to that adopted within group 93 imageCDF
// files!  Do not confuse Rhat_0 with the range direction vector
// rhat_0 which we usually take to be pointed in the positive vertical
// direction in radar images.  We introduced rhat_0 on 6/21/01 in
// order to keep track of the range direction when the radar image is
// rotated to canonical form with the velocity axis aligned with the
// horizontal direction.

   void compute_imageframe_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& rhat_0,threevector& vhat_0,threevector& Rhat_0,
      threevector& that_0,threevector& shat_0,threevector& what_0)
      {
         rhat_0=threevector(0,1,0); // true for ANY satellite motion

         threevector V_imageframe(ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            satellite_statevector.get_velocity()));
         vhat_0=V_imageframe.unitvector();

         threevector R_imageframe(ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            satellite_statevector.get_position()));
         Rhat_0=-R_imageframe.unitvector();
   
         double dotproduct=Rhat_0.dot(vhat_0);
         double denom=sqrt(1-sqr(dotproduct));
         that_0=(Rhat_0-dotproduct*vhat_0)/denom;

         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);
         shat_0=-xhat_body_IGES0;
         what_0=shat_0.cross(Rhat_0);
      }

// =====================================================================
// Target elevation member functions
// =====================================================================

// Method compute_target_range_az_el takes in the position vectors for
// the radar and satellite in ECI coordinates.  It then returns the
// target's range in kilometers along with its azimuth and elevation
// with respect to the radar in degrees.

   void compute_target_range_az_el(
      const threevector& radar_posn,const threevector& satellite_posn,
      double& range,double& azimuth,double& elevation)
      {
         double eldot;
         statevector radar_statevector,satellite_statevector;
         radar_statevector.set_position(radar_posn);
         satellite_statevector.set_position(satellite_posn);
         compute_target_range_az_el(
            radar_statevector,satellite_statevector,
            range,azimuth,elevation,eldot);
      }

   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation)
      {
         double eldot;
         compute_target_range_az_el(
            radar_statevector,satellite_statevector,
            range,azimuth,elevation,eldot);
      }
   
   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation,double& elevation_dot)
      {
         double azimuth_dot_mag;
         compute_target_range_az_el(
            radar_statevector,satellite_statevector,range,azimuth,elevation,
            azimuth_dot_mag,elevation_dot);
      }
   
   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation,
      double& azimuth_dot_mag,double& elevation_dot)
      {
         const threevector xhat(1,0,0);
         const threevector yhat(0,1,0);
         const threevector zhat(0,0,1);
   
         threevector rhat_radar(
            radar_statevector.get_position().unitvector());
         double theta_radar=asin(rhat_radar.get(2));
         double phi_radar=atan2(rhat_radar.get(1)/cos(theta_radar),
                                rhat_radar.get(0)/cos(theta_radar));

         double cosphi=cos(phi_radar);
         double sinphi=sin(phi_radar);
         double costheta=cos(theta_radar);
         double sintheta=sin(theta_radar);
         threevector thetahat_radar(
            -sintheta*cosphi*xhat-sintheta*sinphi*yhat+costheta*zhat);
         threevector phihat_radar(-sinphi*xhat+cosphi*yhat);
      
         threevector rho_vec(satellite_statevector.get_position()-
            radar_statevector.get_position());
         threevector rhohat(rho_vec.unitvector());
         range=rho_vec.magnitude();
         elevation=asin(rhohat.dot(rhat_radar));
         azimuth=atan2(rhohat.dot(phihat_radar)/cos(elevation),
                       rhohat.dot(thetahat_radar)/cos(elevation));
         azimuth=basic_math::phase_to_canonical_interval(azimuth,0,2*PI);

// Compute elevation time derivative (degs/sec):

         threevector rho_vec_dot(satellite_statevector.get_velocity()-
            radar_statevector.get_velocity());
         threevector rhat_radar_dot(
            geofunc::omega_earth*zhat.cross(rhat_radar));
         double rho=rho_vec.magnitude();
         double rhodot=-(satellite_statevector.get_position().dot(
            radar_statevector.get_velocity())+
                         radar_statevector.get_position().dot(
               satellite_statevector.get_velocity()))/rho;
         double term1=(rhat_radar.dot(rho_vec_dot)+rho_vec.dot(
            rhat_radar_dot))/rho;
         double term2=rhat_radar.dot(rho_vec)*rhodot/sqr(rho);
         elevation_dot=(term1-term2)/cos(elevation);

// Compute azimuth time derivative (degs/sec):

         threevector rhohat_dot((rho_vec_dot-rhodot*rhohat)/rho);
         azimuth_dot_mag=
            sqrt((sqr(rhohat_dot.magnitude())-sqr(elevation_dot))/
                 sqr(cos(elevation)));

         azimuth *= 180/PI;
         azimuth=basic_math::phase_to_canonical_interval(azimuth,0,360);
         azimuth_dot_mag *= 180/PI;
         elevation *= 180/PI;
         elevation_dot *= 180/PI;
      }


// =====================================================================
// Basis transformation methods
// =====================================================================

// Method imageframe_to_ECI takes in satellite and radar statevector
// information and a motionfunc::Motion_type parameter specifying the
// target's motion (earthstable or sunsync).  It also takes in a
// vector V whose coordinates are specified in the imageframe basis.
// It returns the vector in ECI coordinates.  Method ECI_to_imageframe
// performs the inverse operation.

   threevector imageframe_to_ECI(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      const threevector& V_imageframe)
      {
         threevector curr_qhat,curr_rhat,curr_phat;
         compute_qhat_rhat_phat_in_ECI_coords(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,curr_qhat,curr_rhat,curr_phat);

         double Vq=V_imageframe.get(0);
         double Vr=V_imageframe.get(1);
         double Vp=V_imageframe.get(2);
         return Vq*curr_qhat+Vr*curr_rhat+Vp*curr_phat;
      }

   threevector ECI_to_imageframe(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      const threevector& V_ECI)
      {
         threevector curr_qhat,curr_rhat,curr_phat;
         compute_qhat_rhat_phat_in_ECI_coords(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,curr_qhat,curr_rhat,curr_phat);

         double Vq=V_ECI.dot(curr_qhat);
         double Vr=V_ECI.dot(curr_rhat);
         double Vp=V_ECI.dot(curr_phat);
         return threevector(Vq,Vr,Vp);
      }

// ---------------------------------------------------------------------
// Method IGES_to_imageframe_rot computes rotation matrix RIGES which
// transforms from the IGES (panel or body) basis into the image frame
// basis.  RIGES.transpose() transforms back from the imageframe basis
// to the IGES basis.

   rotation IGES_to_imageframe_rot(
      const threevector& xIGES_hat,const threevector& yIGES_hat,
      const threevector& zIGES_hat) 
      {
         return rotation(zIGES_hat,-xIGES_hat,-yIGES_hat);
      }

// =====================================================================
// Nominal wireframe setup methods
// =====================================================================

// Method nominal_body_IGES_direction_vectors returns the nominally
// oriented IGES body direction vectors in ECI coordinates.  Recall
// xhat_body_IGES points along the orbit normal, yhat_body_IGES points
// in the anti-nadir direction, and zhat_body_IGES=xhat_body_IGES x
// yhat_body_IGES.

   void nominal_body_IGES_direction_vectors(
      const statevector& satellite_statevector,
      threevector& xhat_body_IGES_ECI,threevector& yhat_body_IGES_ECI,
      threevector& zhat_body_IGES_ECI)
      {
         threevector orbitnormal(satellite_statevector.get_position().cross(
            satellite_statevector.get_velocity()));
         xhat_body_IGES_ECI=orbitnormal.unitvector();
         yhat_body_IGES_ECI=satellite_statevector.get_position().unitvector();
         zhat_body_IGES_ECI=xhat_body_IGES_ECI.cross(yhat_body_IGES_ECI);
      }

// ---------------------------------------------------------------------
// This overloaded version of method nominal_body_IGES_direction
// vectors calculates the IMAGE FRAME (not ECI !) components of a
// nominally oriented satellite body's IGES direction vectors.  Recall
// xhat_body_IGES points along the orbit normal, yhat_body_IGES points
// in the anti-nadir direction, and zhat_body_IGES=xhat_body_IGES x
// yhat_body_IGES.

   void nominal_body_IGES_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& xhat_body_IGES0,threevector& yhat_body_IGES0,
      threevector& zhat_body_IGES0)
      {
         threevector xhat_body_IGES_ECI,yhat_body_IGES_ECI,zhat_body_IGES_ECI;
         nominal_body_IGES_direction_vectors(
            satellite_statevector,xhat_body_IGES_ECI,yhat_body_IGES_ECI,
            zhat_body_IGES_ECI);
         xhat_body_IGES0=ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,xhat_body_IGES_ECI);
         yhat_body_IGES0=ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,yhat_body_IGES_ECI);
         zhat_body_IGES0=ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,zhat_body_IGES_ECI);

//         cout << "xhat_body_IGES0 = " << xhat_body_IGES0 << endl;
//         cout << "yhat_body_IGES0 = " << yhat_body_IGES0 << endl;
//         cout << "zhat_body_IGES0 = " << zhat_body_IGES0 << endl;
      }

// ---------------------------------------------------------------------
// Method panelnormal_towards_sun_rotation returns the transformation
// matrix (in IMAGE FRAME coordinates) which rotates the solar panel
// normal vector panelnormal about the primary RH symmetry axis so
// that solar flux impinging upon the solar panel is maximized.  This
// method implements a binary search for the rotation angle which
// maximizes the solar flux.  The results of this method are used to
// bring the RH solar panel into its nominal sun synchronous position.

   void panelnormal_towards_sun_rotation(
      const threevector& panelnormal,const threevector& e_sun0,
      const rotation& R0_rotate,rotation& Rsolar0)
      {
         double sigma_start=0;
         double sigma_stop=360*PI/180;
         double dsigma=20*PI/180;
         do
         {
            double best_sigma=maximize_solar_flux(
               sigma_start,sigma_stop,dsigma,panelnormal,
               e_sun0,R0_rotate,Rsolar0);
            dsigma *= 0.5;
            sigma_start=best_sigma-2*dsigma;
            sigma_stop=best_sigma+2*dsigma;
         }
         while (dsigma > 0.01*PI/180);
      }

// ---------------------------------------------------------------------
   double maximize_solar_flux(
      double sigma_start,double sigma_stop,double dsigma,
      const threevector& panelnormal,const threevector& e_sun0,
      const rotation& R0_rotate,rotation& Rsolar0)
      {
         int nbins=basic_math::round((sigma_stop-sigma_start)/dsigma)+1;
         double max_dotproduct=NEGATIVEINFINITY;
         const rotation R0trans_rotate=R0_rotate.transpose();
   
         double best_sigma=0;
         for (int n=0; n<nbins; n++)
         {
            double sigma=sigma_start+n*dsigma;
            rotation Rsolar(0,sigma,0);
            Rsolar=R0_rotate*Rsolar*R0trans_rotate;
            double dotproduct=e_sun0.dot(Rsolar*panelnormal);
            if (dotproduct > max_dotproduct)
            {
               max_dotproduct=dotproduct;
               Rsolar0=Rsolar;
               best_sigma=sigma;
            }
         }
         return best_sigma;
      }

// ---------------------------------------------------------------------
// Method nominal_panel_IGES_direction_vectors takes in satellite,
// radar and sun state vector information in ECI coordinates.  It
// computes the IMAGE FRAME (not ECI !) components of the nominally
// oriented RH panel's IGES direction vectors.  On 1/23/01, we learned
// from Forrest Hunsberger and George Zogbi that xIGES_hat always
// points along the orbit normal \propto Rsat x Vsat.  For earth
// stable motion, yIGES_hat points along the anti-nadir direction.
// But for sun-sync motion, yIGES_hat is normal to the solar panel and
// points towards the sun.

   void nominal_panel_IGES_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      const threevector& sun_direction_ECI,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& xhat_panel_IGES0,threevector& yhat_panel_IGES0,
      threevector& zhat_panel_IGES0)
      {
// First calculate nominal body axes in image frame coordinates which
// are defined by sun-sync motion.  

         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);

         rotation R0_rotate;
         construct_imageframe_rotation_matrix(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,R0_rotate);

// Convert sun direction vector from ECI to image frame coordinates:

         threevector sun_direction_imageframe=ECI_to_imageframe(
            satellite_statevector,radar_statevector,
            nominal_spacecraft_motion_type,
            sun_direction_ECI);

         rotation Rsolar0;
         threevector yhat_IGES(0,0,-1);	  // body coords
         yhat_IGES=R0_rotate*yhat_IGES; // image frame coords
         panelnormal_towards_sun_rotation(
            yhat_IGES,sun_direction_imageframe,R0_rotate,Rsolar0);

         xhat_panel_IGES0=xhat_body_IGES0;
         yhat_panel_IGES0=Rsolar0*yhat_IGES;
         zhat_panel_IGES0=xhat_body_IGES0.cross(yhat_panel_IGES0);
      }

// =====================================================================
// Motion determination methods
// =====================================================================

// Method IGES_rotation returns a rotation matrix in the image frame
// basis (spanned by the nominal motion qhat, rhat and phat basis
// vectors) which corresponds to right handed rotation about the
// zIGES_hat direction through angle thetaIGES_z, followed by a right
// handed rotation about the NEW yIGES_hat' direction through angle
// thetaIGES_y followed by a right handed rotation about the EVEN
// NEWER xIGES_hat'' direction through angle thetaIGES_x.

   rotation IGES_rotation(
      const mypoint& xIGES_hat,const mypoint& yIGES_hat,
      const mypoint& zIGES_hat,double thetaIGES_x,double thetaIGES_y,
      double thetaIGES_z)
      {
         return IGES_rotation(
            thetaIGES_x,thetaIGES_y,thetaIGES_z,
            IGES_to_imageframe_rot(
               xIGES_hat.get_center(),yIGES_hat.get_center(),
               zIGES_hat.get_center()));
      }

   rotation IGES_rotation(
      double thetaIGES_x,double thetaIGES_y,double thetaIGES_z,
      const rotation& RIGES)
      {
// Note: In our "Extracting IGES rotation angles from an arbitrary
// total rotation matrix" notes dated 1/17/02, we used the variables
// {alpha, beta, gamma} in place of {thetax, thetay, thetaz}.

         double thetax=thetaIGES_z;		
         double thetay=-thetaIGES_x;		
         double thetaz=-thetaIGES_y;		

         double costhetaz=cos(thetaz);
         double sinthetaz=sin(thetaz);
         double costhetay=cos(thetay);
         double sinthetay=sin(thetay);
         double costhetax=cos(thetax);
         double sinthetax=sin(thetax);

// Rotation matrix Kxzy = Rx * Rz * Ry is defined wrt the current
// (panel or body) IGES basis.  In order to reduce execution time, we
// explicitly hardwire in the values of each of the entries within
// Kxzy rather than perform 3 matrix constructions followed by two
// matrix multiplications:

         rotation Kxzy;
         Kxzy.put(0,0,costhetay*costhetaz);
         Kxzy.put(0,1-sinthetaz);
         Kxzy.put(0,2,costhetaz*sinthetay);
         Kxzy.put(1,0,sinthetax*sinthetay+costhetax*costhetay*sinthetaz);
         Kxzy.put(1,1,costhetax*costhetaz);
         Kxzy.put(1,2,-costhetay*sinthetax+costhetax*sinthetay*sinthetaz);
         Kxzy.put(2,0,-costhetax*sinthetay+costhetay*sinthetax*sinthetaz);
         Kxzy.put(2,1,costhetaz*sinthetax);
         Kxzy.put(2,2,costhetax*costhetay+sinthetax*sinthetay*sinthetaz);

// On 1/14/02, we carefully checked that the following product of
// canonical rotation matrices yields a rotation about the IGES zhat,
// yhat' and xhat'' axes by angles thetaz, thetay and thetax
// respectively.  Recall RIGES.transpose() maps from the image frame
// basis to the current IGES (panel or body) basis, Kxzy implements
// the alpha, beta, gamma rotation in current IGES basis, and RIGES
// returns back from the IGES basis to the image frame basis.

         return RIGES*Kxzy*RIGES.transpose();	// = Ktotal

// Test IGES angle extraction routine...

//   double alpha,beta,gamma;
//   IGES_angles(Ktotal,alpha,beta,gamma);
      }

// ---------------------------------------------------------------------
// Given an arbitrary rotation matrix Ktotal in the image frame basis,
// member function IGES_angles returns the values of IGES rotation
// angles thetaIGES_x, thetaIGES_y and thetaIGES_z which correspond to
// Ktotal.  This member function conforms to our empirical observation
// that thetaIGES_y always lies within the interval [-PI/2,PI/2].  For
// the degenerate case where thetaIGES_y precisely equals either +/-
// PI/2, the ambiguous value of thetaIGES_x is set equal to zero.

   void IGES_angles(
      const rotation& Ktotal,const rotation& RIGES,
      double& thetaIGES_x,double& thetaIGES_y,double& thetaIGES_z) 
      {
// In order to extract numerical values for alpha, beta and gamma,
// first transform Ktotal from the image frame basis back to the
// current IGES (panel or body) basis:

         rotation K=RIGES.transpose()*Ktotal*RIGES;

         const double TINY=1E-10;
         double alpha,beta,gamma;
         double cosgamma=sqrt(sqr(K.get(0,0))+sqr(K.get(0,2)));
         if (cosgamma < TINY)
         {
            gamma=-sgn(K.get(0,1))*PI/2;
            beta=0;
            if (sgn(K.get(0,1))==-1)
            {
               alpha=atan2(K.get(2,0),K.get(1,0));
            }
            else
            {
               alpha=atan2(-K.get(1,2),K.get(2,2));
            }
         }
         else
         {
            beta=atan2(K.get(0,2)/cosgamma,K.get(0,0)/cosgamma);
            alpha=atan2(K.get(2,1)/cosgamma,K.get(1,1)/cosgamma);

            if (fabs(cos(beta)) > fabs(sin(beta)))
            {
               gamma=atan(-K.get(0,1)*cos(beta)/K.get(0,0));
            }
            else
            {
               gamma=atan(-K.get(0,1)*sin(beta)/K.get(0,2));
            }
         }
         thetaIGES_x=-beta;
         thetaIGES_y=-gamma;
         thetaIGES_z=alpha;

//   cout << "Inside satellite::IGES_angles()" << endl;
//   cout << "thetaIGES_x = " << thetaIGES_x*180/PI 
//        << " thetaIGES_y = " << thetaIGES_y*180/PI 
//        << " thetaIGES_z = " << thetaIGES_z*180/PI << endl;
//   cout << thetaIGES_x*180/PI << "\t\t"
//        << thetaIGES_y*180/PI << "\t\t"
//        << thetaIGES_z*180/PI << endl;
      }

// ---------------------------------------------------------------------
// Member function compute_roll_pitch_matrices takes in roll and pitch
// measured in degrees.  It returns the roll and pitch rotation
// matrices in IGES body coordinates.

   void compute_roll_pitch_matrices(
      double roll,double pitch,rotation& Rroll,rotation& Rpitch)
      {
         Rpitch.identity();
         double cospitch=cos(pitch*PI/180);
         double sinpitch=sin(pitch*PI/180);
         Rpitch.put(1,1,cospitch);
         Rpitch.put(2,1,-sinpitch);
         Rpitch.put(1,2,sinpitch);
         Rpitch.put(2,2,cospitch);
         
         Rroll.identity();
         double cosroll=cos(roll*PI/180);
         double sinroll=sin(roll*PI/180);
         Rroll.put(0,0,cosroll);
         Rroll.put(1,0,sinroll);
         Rroll.put(0,1,-sinroll);
         Rroll.put(1,1,cosroll);
      }

// ---------------------------------------------------------------------
// Member function compute_roll_pitch_dot_matrices takes in roll and
// pitch measured in degrees along with their rates rolldot and
// pitchdot measured in degs/sec.  It returns the time derivatives of
// the roll and pitch rotation matrices in IGES body coordinates in
// radians/sec.

   void compute_roll_pitch_dot_matrices(
      double roll,double rolldot,double pitch,double pitchdot,
      rotation& Rroll_dot,rotation& Rpitch_dot)
      {
         double cospitch=cos(pitch*PI/180);
         double sinpitch=sin(pitch*PI/180);
         double pdot=pitchdot*PI/180;
         Rpitch_dot.clear_values();

         Rpitch_dot.put(1,1,-pdot*sinpitch);
         Rpitch_dot.put(2,1,-pdot*cospitch);
         Rpitch_dot.put(1,2,pdot*cospitch);
         Rpitch_dot.put(2,2,-pdot*sinpitch);

         double cosroll=cos(roll*PI/180);
         double sinroll=sin(roll*PI/180);
         double rdot=rolldot*PI/180;
         Rroll_dot.clear_values();
         Rroll_dot.put(0,0,-rdot*sinroll);
         Rroll_dot.put(1,0,rdot*cosroll);
         Rroll_dot.put(0,1,-rdot*cosroll);
         Rroll_dot.put(1,1,-rdot*sinroll);
      }

// ---------------------------------------------------------------------
// Member function roll_pitch_maneuver_body_rot takes in roll and
// pitch measured in degrees.  It returns the product of an active
// right handed roll rotation about the approximate velocity axis
// vhat=what x uhat followed by an active right handed pitch rotation
// about the anti orbit normal what.  (Recall uhat points precisely in
// the nadir direction, and vhat points exactly in the velocity
// direction for circular orbits only.)  Pitch is defined so that
// positive values correspond to the satellite looking in its forward
// direction.  Note further that what=-xbody_IGES_hat, uhat =
// -ybody_IGES_hat and vhat = zbody_IGES_hat.  The following roll and
// pitch matrices are defined with respect to the xbody_IGES_hat,
// ybody_IGES_hat and zbody_IGES_hat basis.

   void roll_pitch_maneuver_body_rot(
      double roll,double pitch,rotation& Rmaneuver_body)
      {
         rotation Rroll,Rpitch;
         compute_roll_pitch_matrices(roll,pitch,Rroll,Rpitch);
         Rmaneuver_body=Rroll*Rpitch;
      }

// ---------------------------------------------------------------------
// Member function compute_telescope_direction_and_derivative takes in
// satellite statevector information along with a time measured in
// seconds since midnight on the current pass date.  It also takes in
// satellite roll, pitch, rolldot and pitchdot information (expressed
// in terms of degrees rather than radians).  This method computes the
// telescope direction vector and its time derivative in ECI
// coordinates.

   void compute_telescope_direction(
      const statevector& satellite_statevector,
      double roll,double pitch,threevector& That_ECI)
      {
         double rolldot,pitchdot;
         rolldot=pitchdot=0;
         threevector That_dot_ECI;
         compute_telescope_direction_and_derivative(
            satellite_statevector,roll,rolldot,pitch,pitchdot,
            That_ECI,That_dot_ECI);
      }

   void compute_telescope_direction_and_derivative(
      const statevector& satellite_statevector,
      double roll,double rolldot,double pitch,double pitchdot,
      threevector& That_ECI,threevector& That_dot_ECI)
      {
         threevector Rsat(satellite_statevector.get_position());
         threevector R_hat(Rsat.unitvector());
         threevector Vsat(satellite_statevector.get_velocity());
         threevector Tnominal_hat(-R_hat);
         threevector Tnominal_hat_dot(-(Vsat-Vsat.dot(R_hat)*R_hat)
            /Rsat.magnitude());

         rotation Rroll,Rpitch,Rroll_dot,Rpitch_dot;
         compute_roll_pitch_matrices(roll,pitch,Rroll,Rpitch);
         compute_roll_pitch_dot_matrices(
            roll,rolldot,pitch,pitchdot,Rroll_dot,Rpitch_dot);

// Compute body IGES direction vectors in ECI coordinates:

         threevector xhat_body_IGES_ECI,yhat_body_IGES_ECI,zhat_body_IGES_ECI;
         nominal_body_IGES_direction_vectors(
            satellite_statevector,xhat_body_IGES_ECI,yhat_body_IGES_ECI,
            zhat_body_IGES_ECI);

// Matrix R_ECI transforms RH body coordinates to ECI coordinates -
// e.g. xhat_body_IGES = (1,0,0) in body coords is mapped by R_ECI to
// (xhat_body_IGES_ECI.e[0], xhat_body_IGES_ECI.e[1],
// xhat_body_IGES_ECI.e[2]).

         rotation R_ECI;
         R_ECI.put_column(0,xhat_body_IGES_ECI);
         R_ECI.put_column(1,yhat_body_IGES_ECI);
         R_ECI.put_column(2,zhat_body_IGES_ECI);
         
         That_ECI=R_ECI*Rroll*Rpitch*R_ECI.transpose()*Tnominal_hat;
         That_dot_ECI=R_ECI*Rroll_dot*Rpitch*R_ECI.transpose()*Tnominal_hat
            +R_ECI*Rroll*Rpitch_dot*R_ECI.transpose()*Tnominal_hat
            +R_ECI*Rroll*Rpitch*R_ECI.transpose()*Tnominal_hat_dot;

//   cout << "That_ECI = " << That_ECI << endl;
//   cout << "That_dot_ECI = " << That_dot_ECI << endl;
//   cout << "dotproduct = " << That_ECI.dot(That_dot_ECI) << endl;
      }


} // satellitefunc namespace




