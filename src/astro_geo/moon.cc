// ==========================================================================
// Moon class member function definitions
// ==========================================================================
// Last modified on 11/27/06
// ==========================================================================

#include <iostream>
#include <string>
#include "astro_geo/astrofuncs.h"
#include "astro_geo/Clock.h"
#include "astro_geo/moon.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Moon::allocate_member_objects()
{
}		       

void Moon::initialize_member_objects()
{
   l0=64.975464;	// Moon's mean longitude at 1980 epoch in degs
   P0=349.383063;	// Mean longitude of perigee at 1980 epoch in degs
   N0=151.950429;	// Mean longitude of node at 1980 epoch in degs
   inclination=5.145396*PI/180;	// Moon's orbit inclination in rads
   eccentricity=0.054900;	// Moon's orbit eccentricity in degs
   angular_size=0.5181;	// Moon's angular size at distance a from Earth (degs)
   parallax=0.9507;	// Parallax at distance a from Earth in degs

   semimajor_axis=384401000;	// Semimajor axis of Moon's orbit in meters
   equatorial_radius=1737400;	// meters
}		       

Moon::Moon(Clock* clock_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   Clock_ptr=clock_ptr;
}

// ---------------------------------------------------------------------
Moon::~Moon()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const Moon& M)
{
   outstream << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Member function compute_RA_DEC is based upon section 61
// "Calculating the Moon's position" in "Practical sstronomy with your
// calculator" by Peter Duffett-Smith.

void Moon::compute_RA_DEC()
{
   double epoch_1980_juliandate=astrofunc::julian_day(1980,1,0);
   double elapsed_days=Clock_ptr->get_juliandate()-epoch_1980_juliandate;

   double lambda_solar,M_solar;
   astrofunc::compute_sun_longitude(elapsed_days,M_solar,lambda_solar);

   double l=(13.1763966*elapsed_days+l0)*PI/180;	// rads
   l=basic_math::phase_to_canonical_interval(l,0,2*PI);

   double Mm=l-PI/180*(0.1114041*elapsed_days+P0);	// rads
   Mm=basic_math::phase_to_canonical_interval(Mm,0,2*PI);

   double N=(N0-0.0529539*elapsed_days)*PI/180;		// rads
   N=basic_math::phase_to_canonical_interval(N,0,2*PI);

   double C=l-lambda_solar;		// rads
   double Ev=1.2739*sin(2*C-Mm);	// degs

   double Ae=0.1858*sin(M_solar);	// degs
   double A3=0.37*sin(M_solar);		// degs

// Compute corrected anomaly:
	
   double Mm_prime=Mm+PI/180*(Ev-Ae-A3);	// rads

   double Ec=6.2886*sin(Mm_prime);	// degs

   double A4=0.214*sin(2*Mm_prime);	// degs

   double lprime=l+PI/180*(Ev+Ec-Ae+A4);	// rads

   double V=0.6583*sin(2*(lprime-lambda_solar));	// degs

// Compute true longitude:

   double lprimeprime=lprime+PI/180*V;
   
   double Nprime=N-PI/180*0.16*sin(M_solar);	// rads

   double y=sin(lprimeprime-Nprime)*cos(inclination);
   double x=cos(lprimeprime-Nprime);

   double lambda_m=atan2(y,x)+Nprime;	// rads
   double beta_m=asin(sin(lprimeprime-Nprime)*sin(inclination));

   astrofunc::ecliptic_to_equatorial_coords(
      Clock_ptr->get_juliandate(),lambda_m,beta_m,alpha_m,delta_m);

   cout << "alpha_m = " << alpha_m*180/PI << endl;
   cout << "delta_m = " << delta_m*180/PI << endl;
}

// ---------------------------------------------------------------------
// Member function compute_ECI_direction

threevector Moon::compute_ECI_direction()
{
   compute_RA_DEC();
   double RA=alpha_m*180/PI;
   double DEC=delta_m*180/PI;
   return astrofunc::ECI_vector(RA,DEC);
}

