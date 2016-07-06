// ==========================================================================
// Header file for INSTANEOUS_COORDS class
// ==========================================================================
// Last modified on 7/18/05; 4/27/06
// ==========================================================================

#ifndef INSTANTANEOUS_COORDS_H
#define INSTANTANEOUS_COORDS_H

#include <vector>
#include "osg/osgGraphicals/instantaneous_obs.h"
#include "math/statevector.h"

class instantaneous_coords: public instantaneous_obs
{

  public:

// Initialization, constructor and destructor functions:

   instantaneous_coords();
   instantaneous_coords(double curr_t);
   ~instantaneous_coords();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const instantaneous_coords& C);

// Set & get member functions:

   void set_S(const statevector& s_vector);
   statevector& get_S();
   const statevector& get_S() const;

  private:

// Statevector representing a feature's true (filtered) position and
// velocity in world-space at time t:

   statevector S;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const instantaneous_coords& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void instantaneous_coords::set_S(const statevector& s_vector)
{
   S=s_vector;
}

inline statevector& instantaneous_coords::get_S() 
{
   return S;
}

inline const statevector& instantaneous_coords::get_S() const
{
   return S;
}

#endif // instantaneous_coords.h



