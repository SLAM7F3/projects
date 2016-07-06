// ==========================================================================
// Ground_Radar class member function definitions
// ==========================================================================
// Last modified on 4/28/06; 6/8/06; 12/21/06; 10/8/11
// ==========================================================================

#include <iostream>
#include "astro_geo/geofuncs.h"
#include "astro_geo/ground_radar.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;


// ==========================================================================
// Initialization, constructor and destructor functions:
// ==========================================================================

void ground_radar::allocate_member_objects() 
{
}

void ground_radar::initialize_member_objects() 
{
   max_slew_rate=0;
}

ground_radar::ground_radar()
{
   initialize_member_objects();
}

ground_radar::ground_radar(string ground_radar_name,double ground_radar_bandwidth,
               const geopoint& ground_radar_posn)
{
   initialize_member_objects();
   name=ground_radar_name;
   bandwidth=ground_radar_bandwidth;
   geolocation=ground_radar_posn;
}

// Copy constructor:

ground_radar::ground_radar(const ground_radar& g)
{
   initialize_member_objects();
   docopy(g);
}

ground_radar::~ground_radar()
{
}

// ---------------------------------------------------------------------
void ground_radar::docopy(const ground_radar& s)
{
   name=s.name;
   bandwidth=s.bandwidth;
   max_slew_rate=s.max_slew_rate;
   geolocation=s.geolocation;
   geolocation=s.geolocation;
}	

// Overload = operator:

ground_radar ground_radar::operator= (const ground_radar& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,ground_radar& s)
{
   outstream << "name = " << s.name << endl;
   outstream << "bandwidth = " << s.bandwidth << endl;
   outstream << "max slew rate = " << s.max_slew_rate << endl;
   outstream << "geolocation = " << s.geolocation << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Member set_max_slew_rate hardwires in reasonable estimates
// (provided by J. Eshbaugh and K. Fields in Aug 03) for maximum
// azimuth and elevation rates which various ground_radars can undergo.

void ground_radar::set_max_slew_rate()
{
   if (name=="Haystack LRIR")
   {
      max_slew_rate=2;	// degs/sec
   }
   else if (name=="Haystack Auxiliary")
   {
      max_slew_rate=8;	// degs/sec
   }
   else if (name=="Millimeter Wave")
   {
      max_slew_rate=20;	// guesstimate
   }
}

// ---------------------------------------------------------------------
// Member function compute_location takes in an initial ground_radar
// position in ECI coordinates along with a time duration t (measured
// in secs).  This method rotates the earth about its axis and then
// returns the new ground_radar postion in ECI coordinates.

threevector ground_radar::compute_position(
   double t,const threevector& init_ground_radar_posn)
{
   double phi_t=geofunc::omega_earth*t;
   threevector curr_ground_radar_posn(init_ground_radar_posn);
   curr_ground_radar_posn.put(0,cos(phi_t)*init_ground_radar_posn.get(0)-
                        sin(phi_t)*init_ground_radar_posn.get(1));
   curr_ground_radar_posn.put(1,sin(phi_t)*init_ground_radar_posn.get(0)+
                        cos(phi_t)*init_ground_radar_posn.get(1));
   return curr_ground_radar_posn;
}

// ---------------------------------------------------------------------
// Member function compute_statevector takes in an initial ground_radar
// position in ECI coordinates along with a time duration t (measured
// in secs).  It returns the ground_radar's statevector in ECI coordinates.

statevector& ground_radar::compute_statevector(
   double t,const threevector& init_ground_radar_posn)
{
   posn_vel.set_position(compute_position(t,init_ground_radar_posn));

   const threevector zhat(0,0,1);
   posn_vel.set_velocity(geofunc::omega_earth*zhat.cross(
      posn_vel.get_position()));
   return posn_vel;
}


