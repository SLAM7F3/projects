// ==========================================================================
// Header file for robot class
// ==========================================================================
// Last modified on 2/14/08; 2/21/08
// ==========================================================================

#ifndef ROBOT_H
#define ROBOT_H

#include "math/statevector.h"
#include "track/track.h"

class groundspace;

class robot
{

  public:

// Initialization, constructor and destructor functions:

   robot(int ID);
   robot(int ID,groundspace* groundspace_ptr);
   robot(const robot& r);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~robot();
   robot& operator= (const robot& r);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const robot& r);

// Set and get member functions:

   int get_ID() const;
   statevector& get_statevector();
   const statevector& get_statevector() const;
   double get_heading_angle() const;

   track* get_track_ptr();
   const track* get_track_ptr() const;

// Propagation member functions:

   statevector& propagate_statevector(double curr_t);
   double cost_function(const threevector& posn,const threevector& vel);

  private: 

   int ID;
   int n_candidate_headings;
   double heading_angle,prev_theta;
   statevector curr_statevector;
   groundspace* groundspace_ptr;
   track* track_ptr;

   std::vector<genmatrix> heading_change_matrix;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const robot& r);

   void initialize_heading_change_matrices();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int robot::get_ID() const
{
   return ID;
}

inline statevector& robot::get_statevector()
{
   return curr_statevector;
}

inline const statevector& robot::get_statevector() const
{
   return curr_statevector;
}

inline double robot::get_heading_angle() const
{
   return heading_angle;
}

inline track* robot::get_track_ptr()
{
   return track_ptr;
}

inline const track* robot::get_track_ptr() const
{
   return track_ptr;
}

#endif  // robot.h



