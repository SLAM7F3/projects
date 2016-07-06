// ==========================================================================
// Header file for instantaneous physical SATELLITE target (and NOT
// satellite model) class
// ==========================================================================
// Last modified on 6/7/06; 7/20/06; 1/29/12
// ==========================================================================

#ifndef SATELLITE_H
#define SATELLITE_H

#include <iostream>
#include "math/rotation.h"
#include "math/rpy.h"
#include "math/statevector.h"

class satellite
{

  public:

// Initialization, constructor and destructor methods:

   void allocate_member_objects();
   void initialize_member_objects();
   satellite(void);
   satellite(const satellite& s);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   ~satellite();

   void docopy(const satellite& s);
   satellite& operator= (const satellite& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const satellite& s);

// Set & get member functions

   void set_time(double t);
   void set_position(const threevector& posn);
   void set_velocity(const threevector& vel);
   void set_az_el_range(const threevector& aer);
   void set_az_el_range_dot(const threevector& aer_dot);
   void set_nadir_pnt(const threevector& lla);
   void set_nadir_pnt_dot(const threevector& lla_dot);

   void set_attitude(const rpy& attitude);
   void set_rate(const rpy& rate);
   void set_error(const rpy& error);
   void set_error_dot(const rpy& error_dot);
   void set_max_attitude(const rpy& angle);
   void set_min_attitude(const rpy& angle);
   void set_max_rate(const rpy& rate);
   void set_min_rate(const rpy& rate);
   void set_max_accel(const rpy& accel);

   double get_time() const;
   const statevector& get_statevector() const;
   const threevector& get_position() const;
   const threevector& get_velocity() const;
   const threevector& get_az_el_range() const;
   const threevector& get_az_el_range_dot() const;
   const threevector& get_nadir_pnt() const;
   const threevector& get_nadir_pnt_dot() const;

   rpy* get_attitude_ptr() const;
   rpy* get_rate_ptr() const;
   rpy* get_error_ptr() const;
   rpy* get_error_dot_ptr() const;
   rpy* get_max_attitude_ptr() const;
   rpy* get_min_attitude_ptr() const;
   rpy* get_max_rate_ptr() const;
   rpy* get_min_rate_ptr() const;
   rpy* get_max_accel_ptr() const;

   void set_R0_rotate(const rotation& R);
   const rotation& get_R0_rotate() const;

  protected:

  private:

// Satellite's instantaneous COM position & velocity measured in
// various coordinate systems:

   statevector posn_vel;	// meters 
   statevector az_el_range;	// degs (az,el); meters (r)
   statevector nadir_pnt;	// longitude/latitude of nadir pnt in degs,
				//   altitude of satellite in meters

// Satellite's instantaneous attitude orientation and angular velocity:

// Store spacecraft roll, pitch and yaw angles (in degs) measured
// relative to nominal, earth-stable (NOT sun-sync!) zhat_body_IGES
// (approximately equal to velocity direction vector) and
// -xhat_body_IGES (anti orbit normal) within rpy object
// *attitude_ptr.  When known, save the time derivatives roll_dot,
// pitch_dot and yaw_dot (in degs/sec) in rpy object *rate_ptr as
// well...

   rpy* attitude_ptr;	// degs
   rpy* rate_ptr;	// degs/sec
 
   rpy* error_ptr;	// degs
   rpy* error_dot_ptr;	// degs/sec
   rpy* max_attitude_ptr;
   rpy* min_attitude_ptr;
   rpy* max_rate_ptr;
   rpy* min_rate_ptr;
   rpy* max_accel_ptr;

// Satellite range direction vector rhat_0, nadir direction vector
// Rhat_0, velocity direction vector vhat_0, ZERO body roll direction
// vector that_0 (which is orthogonal to vhat_0 and which lies as
// close as possible to Rhat_0), anti-orbit normal direction vector
// shat_0=that_0 x vhat_0, and what_0=shat_0 x Rhat_0.  Note that
// Rhat_0 is numerically close to that_0 and what_0 is numerically
// close to vhat_0 for nearly circular orbits.

// Zero subscripts on the following vectors indicate that their
// components are measured in IMAGE FRAME (and not in ECI!) coords:

   threevector rhat_0,Rhat_0,vhat_0,what_0,shat_0,that_0;

// Rotation matrix [what_0, shat_0, Rhat_0] which aligns A2/AU/RH
// models into their nominal earthstable orientations in the IMAGE
// FRAME (and not ECI) basis.  xIGES points along what_0, yIGES points
// along shat_0, and zIGES points along Rhat_0.

   rotation R0_rotate;	

};

// ==========================================================================
// Inlined methods
// ==========================================================================

// Set and get member functions 

inline void satellite::set_time(double t)
{
   posn_vel.set_time(t);
   az_el_range.set_time(t);
   nadir_pnt.set_time(t);
}

inline void satellite::set_position(const threevector& posn)
{
   posn_vel.set_position(posn);
}

inline void satellite::set_velocity(const threevector& vel)
{
   posn_vel.set_velocity(vel);
}

inline void satellite::set_az_el_range(const threevector& aer)
{
   az_el_range.set_position(aer);
}

inline void satellite::set_az_el_range_dot(const threevector& aer_dot)
{
   az_el_range.set_position(aer_dot);
}

inline void satellite::set_nadir_pnt(const threevector& lla)
{
   nadir_pnt.set_position(lla);
}

inline void satellite::set_nadir_pnt_dot(const threevector& lla_dot)
{
   nadir_pnt.set_velocity(lla_dot);
}

inline double satellite::get_time() const
{
   return posn_vel.get_time();
}

inline const statevector& satellite::get_statevector() const
{
   return posn_vel;
}

inline const threevector& satellite::get_position() const
{
   return posn_vel.get_position();
}

inline const threevector& satellite::get_velocity() const
{
   return posn_vel.get_velocity();
}

inline const threevector& satellite::get_az_el_range() const
{
   return az_el_range.get_position();
}

inline const threevector& satellite::get_az_el_range_dot() const
{
   return az_el_range.get_velocity();
}

inline const threevector& satellite::get_nadir_pnt() const
{
   return nadir_pnt.get_position();
}

inline const threevector& satellite::get_nadir_pnt_dot() const
{
   return nadir_pnt.get_velocity();
}

// ==========================================================================

inline void satellite::set_attitude(const rpy& attitude) 
{
   *attitude_ptr=attitude;
}

inline void satellite::set_rate(const rpy& rate) 
{
   *rate_ptr=rate;
}

inline void satellite::set_error(const rpy& error) 
{
   *error_ptr=error;
}

inline void satellite::set_error_dot(const rpy& error_dot) 
{
   *error_dot_ptr=error_dot;
}

inline void satellite::set_max_attitude(const rpy& angle) 
{
   *max_attitude_ptr=angle;
}

inline void satellite::set_min_attitude(const rpy& angle) 
{
   *min_attitude_ptr=angle;
}

inline void satellite::set_max_rate(const rpy& rate) 
{
   *max_rate_ptr=rate;
}

inline void satellite::set_min_rate(const rpy& rate) 
{
   *min_rate_ptr=rate;
}

inline void satellite::set_max_accel(const rpy& accel) 
{
   *max_accel_ptr=accel;
}

inline rpy* satellite::get_attitude_ptr() const
{
   return attitude_ptr;
}

inline rpy* satellite::get_rate_ptr() const
{
   return rate_ptr;
}

inline rpy* satellite::get_error_ptr() const
{
   return error_ptr;
}

inline rpy* satellite::get_error_dot_ptr() const
{
   return error_dot_ptr;
}

inline rpy* satellite::get_max_attitude_ptr() const
{
   return max_attitude_ptr;
}

inline rpy* satellite::get_min_attitude_ptr() const
{
   return min_attitude_ptr;
}

inline rpy* satellite::get_max_rate_ptr() const
{
   return max_rate_ptr;
}

inline rpy* satellite::get_min_rate_ptr() const
{
   return min_rate_ptr;
}

inline rpy* satellite::get_max_accel_ptr() const
{
   return max_accel_ptr;
}

inline void satellite::set_R0_rotate(const rotation& R)
{
   R0_rotate=R;
}

inline const rotation& satellite::get_R0_rotate() const
{
   return R0_rotate;
}

#endif // satellite.h

