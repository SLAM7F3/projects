// ==========================================================================
// Header file for SATELLITEIMAGE derived class
// ==========================================================================
// Last modified on 4/28/06; 6/7/06; 7/20/06; 7/26/06; 10/8/11
// ==========================================================================

#ifndef SATELLITEIMAGE_H
#define SATELLITEIMAGE_H

#include <set>
#include "image/myimage.h"
#include "math/statevector.h"
#include "math/threevector.h"
class satellite;
class satellitepass;
class ground_radar;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class satelliteimage : public myimage
{

  public:

// Initialization, constructor and destructor methods:

   void allocate_member_objects();
   void initialize_member_objects();
   satelliteimage(satellitepass* satpass_ptr);
   satelliteimage(int Nxbins,int Nybins,satellitepass* satpass_ptr);
   satelliteimage(const twoDarray& T,satellitepass* satpass_ptr);
   satelliteimage(const satelliteimage& m);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~satelliteimage();

   void docopy(const satelliteimage& m);
   satelliteimage& operator= (const satelliteimage& m);

// Set and get member functions:

   void set_times(double start,double stop,double mid,double rel);
   void set_overlap_percentage(double percentage);
   void set_x_conversion(double xconv);
   void set_p_center(double px,double py);
   void set_qnominal_magnitude(double qnom);
   void set_earthstable_to_sunsync_scalefactor(double s);
   void set_sun_direction(const threevector& sun_dir);
   void set_orbit_normal(const threevector& normal);
   void set_target_ptr(satellite* sat_ptr);
   void set_qhat(const threevector& q_hat);
   void set_rhat(const threevector& r_hat);
   void set_angular_velocities(
      const threevector& w_orbit,const threevector& w_ses);

   double get_starttime() const;
   double get_midtime() const;
   double get_stoptime() const;
   double get_reltime() const;
   double get_overlap_percentage() const;
   double get_qnominal_magnitude() const;
   double get_earthstable_to_sunsync_scalefactor() const;
   std::pair<int,int> get_p_center() const;
   threevector& get_sun_direction();
   threevector& get_orbit_normal();
   satellite* get_target_ptr();
   ground_radar* get_ground_radar_ptr();
   const threevector& get_qhat() const;
   const threevector& get_rhat() const;

// Image property member functions:

   void compute_ARIES_image_center(twoDarray* ztwoDarray_ptr);

  protected:

  private:

   double starttime,midtime,stoptime,reltime;
   double overlap_percentage; 	// imagery overlap in %

   double q_nominal_magnitude;	// |q_earthstable| or |q_sunsync|
   double earthstable_to_sunsync_scalefactor; // |q_earthstable|/|q_sunsync|

// Raw freq->cross range length conversion factor (m/Hz).  On 1/16/02,
// we realized that we need to keep track of this factor in order to
// interpret analyst RH findings encoded within paramcdf files:

   double x_conversion;	
   double x_center,y_center;	// Range/cross range image center 
				//  reported by ARIES
   double px_center,py_center;	// Pixel location of radar image center
   double phi,phi_error;	// Azimuthal angle between stable and 
				//   maneuvering image planes (rads)

   threevector rhat;	// ECI range unit vector from radar to satellite 
			//   (dimensionless)
   threevector qhat;	// ECI cross range unit vector (dimensionless)

// Orbital angular velocity and earth stable satellite's spin angular
// velocity (displayed by XELIAS) in ECI coords (rads/sec):

   threevector omega_orbital;
   threevector omega_spin_earthstable; 
   threevector sun_direction;	// sun's dimensionless direction vector 
				//  in ECI coords 
   threevector orbit_normal;	// unitvector in ECI coords

// Each satelliteimage contains a satellite object which holds
// instantaneous COM and attitude information about the physical
// target (and NOT model information!):

   satellite* curr_target_ptr;

// Each satelliteimage contains a ground_radar object which holds
// instantaneous statevector information about the physical ground_radar:

   ground_radar* curr_ground_radar_ptr;

// Each satelliteimage possesses a pointer back to the satellitepass
// with which it is associated:

   satellitepass* pass_ptr;
};

// ==========================================================================
// Inlined methods
// ==========================================================================

// Set and get member functions 

inline void satelliteimage::set_times(
   double start,double stop,double mid,double rel)
{
   starttime=start;
   stoptime=stop;
   midtime=mid;
   reltime=rel;
}

inline void satelliteimage::set_overlap_percentage(double percentage)
{
   overlap_percentage=percentage;
}

inline void satelliteimage::set_x_conversion(double xconv)
{
   x_conversion=xconv;
}

inline void satelliteimage::set_p_center(double px,double py)
{
   px_center=px;
   py_center=py;
}

inline std::pair<int,int> satelliteimage::get_p_center() const
{
   return std::pair<int,int>(
      int(px_center),int(py_center));
}

inline void satelliteimage::set_qnominal_magnitude(double qnom) 
{
   q_nominal_magnitude=qnom;
}

inline void satelliteimage::set_earthstable_to_sunsync_scalefactor(double s)
{
   earthstable_to_sunsync_scalefactor=s;
}

inline void satelliteimage::set_sun_direction(const threevector& sun_dir) 
{
   sun_direction=sun_dir;
}

inline void satelliteimage::set_orbit_normal(const threevector& normal) 
{
   orbit_normal=normal;
}

inline void satelliteimage::set_target_ptr(satellite* sat_ptr)
{
   curr_target_ptr=sat_ptr;
}

inline void satelliteimage::set_qhat(const threevector& q_hat)
{
   qhat=q_hat;
}

inline void satelliteimage::set_rhat(const threevector& r_hat)
{
   rhat=r_hat;
}

inline void satelliteimage::set_angular_velocities(
   const threevector& w_orbit,const threevector& w_ses)
{
   omega_orbital=w_orbit;
   omega_spin_earthstable=w_ses;
}

// ==========================================================================

inline double satelliteimage::get_starttime() const
{
   return starttime;
}

inline double satelliteimage::get_midtime() const
{
   return midtime;
}

inline double satelliteimage::get_stoptime() const
{
   return stoptime;
}

inline double satelliteimage::get_reltime() const
{
   return reltime;
}

inline double satelliteimage::get_overlap_percentage() const
{
   return overlap_percentage;
}

inline double satelliteimage::get_qnominal_magnitude() const
{
   return q_nominal_magnitude;
}

inline double satelliteimage::get_earthstable_to_sunsync_scalefactor() const
{
   return earthstable_to_sunsync_scalefactor;
}

inline threevector& satelliteimage::get_sun_direction()
{
   return sun_direction;
}

inline threevector& satelliteimage::get_orbit_normal() 
{
   return orbit_normal;
}

inline satellite* satelliteimage::get_target_ptr()
{
   return curr_target_ptr;
}

inline ground_radar* satelliteimage::get_ground_radar_ptr()
{
   return curr_ground_radar_ptr;
}

inline const threevector& satelliteimage::get_qhat() const
{
   return qhat;
}

inline const threevector& satelliteimage::get_rhat() const
{
   return rhat;
}

#endif // satelliteimage.h







