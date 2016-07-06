// ==========================================================================
// SATELLITEORBIT class member function definitions
// ==========================================================================
// Last modified on 4/28/06; 8/29/06
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "space/satelliteorbit.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

void satelliteorbit::allocate_member_objects() 
{
}

void satelliteorbit::initialize_member_objects() 
{
}

satelliteorbit::satelliteorbit(void) // Null constructor function 
{
   allocate_member_objects();
   initialize_member_objects();
}		       

// Copy constructor:

satelliteorbit::satelliteorbit(const satelliteorbit& o)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(o);
}

satelliteorbit::~satelliteorbit()
{
}

// ---------------------------------------------------------------------
void satelliteorbit::docopy(const satelliteorbit& o)
{
   a_semimajor=o.a_semimajor;
   eccentricity=o.eccentricity;
   t_reference=o.t_reference;
   position_reference=o.position_reference;
   omega_avg=o.omega_avg;
   Runge_Lenz_vector=o.Runge_Lenz_vector; 
}

// Overload = operator:

satelliteorbit& satelliteorbit::operator= (const satelliteorbit& o)
{
   if (this==&o) return *this;
   docopy(o);
   return *this;
}

// ==========================================================================
// Target state vector computation member functions
// ==========================================================================

// Member function target_location_given_gamma computes the position
// of the satellite in ECI coordinates assuming that it travels along
// an elliptical orbit.  [The ellipse parameters are set in
// satelliteorbit::compute_average_orbit_parameters().]  This method
// takes in angle gamma which parameterizes the target's azimuthal
// location along the orbit relative to the perigee point.

threevector satelliteorbit::target_location_given_gamma(double gamma)
{
   threevector Ahat(Runge_Lenz_vector.unitvector());
   threevector crossproduct(omega_avg.cross(Ahat));	
   threevector Bhat(crossproduct.unitvector());
   double r=a_semimajor*sqrt((1-sqr(eccentricity))/
                             (1-sqr(eccentricity*cos(gamma))));
   return r*(cos(gamma)*Ahat+sin(gamma)*Bhat);
}

// ---------------------------------------------------------------------
// Member function target_locations fills an STL vector with locations
// along the current orbit by varying azimuthal angle gamma over
// increments specified by input parameter d_gamma (in radians).  The
// resulting ECI locations within the output STL vector can be plotted
// to illustrate a target's orbit around the earth.

vector<threevector> satelliteorbit::target_locations()
{
   double d_gamma=1.0*PI/180;
   return target_locations(d_gamma);
}

vector<threevector> satelliteorbit::target_locations(double d_gamma)
{
   double gamma_lo=0;
   double gamma_hi=1.05*2*PI;
   int nbins=static_cast<int>((gamma_hi-gamma_lo)/d_gamma+1);
   vector<threevector> orbit_posn_ECI;
   for (int n=0; n<nbins; n++)
   {
      double gamma=gamma_lo+n*d_gamma;
      orbit_posn_ECI.push_back(target_location_given_gamma(gamma)); 
   }
   return orbit_posn_ECI;
}

// ---------------------------------------------------------------------
// Member function target_location returns the approximate target
// position in ECI coordinates corresponding to the time currtime
// measured in seconds since midnight.

threevector satelliteorbit::target_location(double currtime)
{
   return target_location(t_reference,position_reference,currtime);
}

// This overloaded version of member function target_location takes in
// a reference time and position for the satellite.  It returns the
// ECI location of the target at some other input time measured in the
// same way as the reference time.

threevector satelliteorbit::target_location(
   double reference_time,const threevector& reference_satellite_posn,
   double currtime)
{
   double gamma_reference=
      find_azimuthal_orbit_angle_gamma(reference_satellite_posn);
   double gamma=omega_avg.magnitude()*(currtime-reference_time)+
      gamma_reference;
   return target_location_given_gamma(gamma);
}

// ---------------------------------------------------------------------
// Member function target_velocity_given_position returns the target's
// velocity vector in ECI coordinates given its position vector in ECI
// coordinates.  The target's average angular velocity vector along
// with its orbit's Runge-Lenz vector are passed into this method via
// member variables.

threevector satelliteorbit::target_velocity_given_position(
   const threevector& target_position)
{
   threevector crossproduct(omega_avg.cross(target_position));
   double denom=target_position.magnitude()*sqr(crossproduct.magnitude());
   double Vr=-crossproduct.dot(Runge_Lenz_vector)/denom;
   return Vr*target_position.unitvector()+crossproduct;
}

// ---------------------------------------------------------------------
double satelliteorbit::find_azimuthal_orbit_angle_gamma(
   const threevector& input_satellite_posn)
{
   const int iter_max=200;
   const double angle_error_tolerance=0.1*PI/180;

   int iter=0;
   double gamma=0;
   double dgamma;
   do
   {
      threevector R(target_location_given_gamma(gamma));
      threevector V(target_velocity_given_position(R));
      threevector Delta(R-input_satellite_posn);
      double f=0.5*sqr(Delta.magnitude());
      double fprime=V.dot(R-input_satellite_posn)/omega_avg.magnitude();
      dgamma=-f/fprime;
      gamma += dgamma;
      iter++;
//      cout << "iter = " << iter << " gamma = " << gamma*180/PI << endl;
   }
   while (fabs(dgamma) > angle_error_tolerance && iter < iter_max);
   gamma=basic_math::phase_to_canonical_interval(gamma,0,2*PI);
   return gamma;
}

// ---------------------------------------------------------------------
// Member function compute_target_statevector returns the target's
// absolute position and velocity vectors in ECI coordinates given the
// time currtime in seconds since midnight UTC of the pass day.

void satelliteorbit::compute_target_statevector(
   double currtime,statevector& target_statevector)
{
   target_statevector.set_position(target_location(currtime));
   target_statevector.set_velocity(target_velocity_given_position(
      target_statevector.get_position()));
}

statevector satelliteorbit::compute_target_statevector(
   double reference_time,const threevector& reference_satellite_posn,
   double currtime)
{
   threevector target_posn(target_location(
      reference_time,reference_satellite_posn,currtime));
   return statevector(
      currtime,target_posn,target_velocity_given_position(target_posn));
}








