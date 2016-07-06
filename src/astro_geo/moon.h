// ==========================================================================
// Header file for Moon class 
// ==========================================================================
// Last modified on 11/27/06
// ==========================================================================

#ifndef MOON_H
#define MOON_H

#include "math/threevector.h"

class Clock;

class Moon
{
   
  public:

// Initialization, constructor and destructor functions:

   Moon(Clock* clock_ptr);
   ~Moon();
   Moon operator= (const Moon& M);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Moon& M);

// Set and get methods:

   double get_semimajor_axis() const;
   double get_equatorial_radius() const;
   double get_RA() const;
   double get_DEC() const;

   void compute_RA_DEC();
   threevector compute_ECI_direction();

  private: 

   double l0,P0,N0;
   double inclination,eccentricity,angular_size;
   double semimajor_axis,equatorial_radius,parallax;
   double alpha_m,delta_m;
   Clock* Clock_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Moon& C);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline double Moon::get_semimajor_axis() const
{
   return semimajor_axis;
}

inline double Moon::get_equatorial_radius() const
{
   return equatorial_radius;
}

inline double Moon::get_RA() const
{
   return alpha_m;
}

inline double Moon::get_DEC() const
{
   return delta_m;
}

#endif  // Moon.h


