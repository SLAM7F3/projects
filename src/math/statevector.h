// ==========================================================================
// Header file for instantaneous STATEVECTOR class
// ==========================================================================
// Last modified on 4/27/06
// ==========================================================================

#ifndef STATEVECTOR_H
#define STATEVECTOR_H

#include <iostream>
#include "math/threevector.h"

class statevector
{

  public:

// Constructor functions:

   statevector();
   statevector(double t,const threevector& posn,const threevector& vel);
   statevector(const statevector& s);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   ~statevector();

   void docopy(const statevector& s);
   statevector& operator= (const statevector& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const statevector& s);

// Set and get methods:

   void set_time(double t);
   void set_position(const threevector& p);
   void set_velocity(const threevector& v);
   double get_time() const;
   const threevector& get_position() const;
   const threevector& get_velocity() const;

  private:

   double time;			// sec
   double heading;		// rads measured relative to north
   threevector position;  	// ECI (meters)
   threevector velocity; 	// ECI (meters/sec)

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void statevector::set_time(double t)
{
   time=t;
}

inline void statevector::set_position(const threevector& p)
{
   position=p;
}

inline void statevector::set_velocity(const threevector& v)
{
   velocity=v;
}

inline double statevector::get_time() const
{
   return time;
}

inline const threevector& statevector::get_position() const
{
   return position;
}

inline const threevector& statevector::get_velocity() const
{
   return velocity;
}


#endif // statevector.h

