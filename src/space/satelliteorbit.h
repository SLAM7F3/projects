// =========================================================================
// Header file for SATELLITEORBIT class
// =========================================================================
// Last modified on 4/28/06; 8/29/06
// =========================================================================

#ifndef SATELLITEORBIT_H
#define SATELLITEORBIT_H

#include <vector>
#include "math/statevector.h"
#include "math/threevector.h"

class satelliteorbit
{

  public:

// Initialization, constructor and destructor methods:

   void allocate_member_objects();
   void initialize_member_objects();
   satelliteorbit(void);
   satelliteorbit(const satelliteorbit& o);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~satelliteorbit();

   void docopy(const satelliteorbit& o);
   satelliteorbit& operator= (const satelliteorbit& o);

// Set and get member functions:

   void set_a_semimajor(double a);
   void set_eccentricity(double e);
   void set_omega_avg(const threevector& omega);
   void set_Runge_Lenz_vector(const threevector& RL);
   void set_reference_posn_and_time(const threevector& posn0,double t0);
   double get_a_semimajor() const;
   double get_eccentricity() const;
   const threevector& get_omega_avg() const;
   const threevector& get_Runge_Lenz_vector() const;

// Target state vector computation member functions:

   threevector target_location_given_gamma(double gamma);
   std::vector<threevector> target_locations();
   std::vector<threevector> target_locations(double d_gamma);
   threevector target_location(double currtime);
   threevector target_location(
      double reference_time,const threevector& reference_satellite_posn,
      double currtime);
   threevector target_velocity_given_position(
      const threevector& target_position);
   double find_azimuthal_orbit_angle_gamma(
      const threevector& input_satellite_posn);
   void compute_target_statevector(
      double currtime,statevector& target_statevector);
   statevector compute_target_statevector(
      double reference_time,const threevector& reference_satellite_posn,
      double currtime);

  protected:

  private:

   double a_semimajor; 	   // Average semimajor axis for assumed
			   //  elliptical satellite orbit in meters
   double eccentricity;  // slightly > 0 for nearly circular orbits
   double t_reference;	 // Time corresponding to reference position along 
			 //  satellite's orbit
   threevector position_reference; // Some "origin" point along orbit
   threevector omega_avg;  	// Average satellite angular velocity vector
				//  in ECI coords
   threevector Runge_Lenz_vector; // Average Runge-Lenz vector for assumed 
			       //  elliptical satellite orbit (points in 
			       //  perigee direction)
};

// ==========================================================================
// Inlined methods
// ==========================================================================

// Set and get member functions

inline void satelliteorbit::set_a_semimajor(double a)
{
   a_semimajor=a;
}

inline void satelliteorbit::set_eccentricity(double e)
{
   eccentricity=e;
}

inline void satelliteorbit::set_omega_avg(const threevector& omega)
{
   omega_avg=omega;
}

inline void satelliteorbit::set_Runge_Lenz_vector(const threevector& RL)
{
   Runge_Lenz_vector=RL;
}

inline void satelliteorbit::set_reference_posn_and_time(
   const threevector& posn0,double t0)
{
   position_reference=posn0;
   t_reference=t0;
}

inline double satelliteorbit::get_a_semimajor() const
{
   return a_semimajor;
}

inline double satelliteorbit::get_eccentricity() const
{
   return eccentricity;
}

inline const threevector& satelliteorbit::get_omega_avg() const
{
   return omega_avg;
}

inline const threevector& satelliteorbit::get_Runge_Lenz_vector() const
{
   return Runge_Lenz_vector;
}

#endif // satelliteorbit.h

