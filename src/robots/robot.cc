// ==========================================================================
// Robot class member function definitions
// ==========================================================================
// Last modified on 2/14/08; 2/21/08
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/constant_vectors.h"
#include "math/genmatrix.h"
#include "robots/groundspace.h"
#include "math/mathfuncs.h"
#include "robots/robot.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void robot::allocate_member_objects()
{
   track_ptr=new track(0);
}		       

void robot::initialize_member_objects()
{
   groundspace_ptr=NULL;
   initialize_heading_change_matrices();
   
   curr_statevector.set_time(0);
   curr_statevector.set_position(Zero_vector);
   curr_statevector.set_velocity(Zero_vector);
}

void robot::initialize_heading_change_matrices()
{
   const int n_candidate_headings=7;
//   double max_heading_change=1*PI/180;	// radians
   double max_heading_change=5*PI/180;	// radians
   double d_heading_change=2*max_heading_change/(n_candidate_headings-1);
   genmatrix R(3,3);
   for (int n=0; n<n_candidate_headings; n++)
   {
      double heading_change=-max_heading_change+n*d_heading_change;
      double cos_heading_change=cos(heading_change);
      double sin_heading_change=sin(heading_change);
      R.identity();
      R.put(0,0,cos_heading_change);
      R.put(1,0,sin_heading_change);
      R.put(0,1,-sin_heading_change);
      R.put(1,1,cos_heading_change);
      heading_change_matrix.push_back(R);
   }

//   templatefunc::printVector(heading_change_matrix);
}

robot::robot(int ID)
{
   allocate_member_objects();
   initialize_member_objects();

   this->ID=ID;
}

robot::robot(int ID,groundspace* groundspace_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   this->ID=ID;
   this->groundspace_ptr=groundspace_ptr;
}

// Copy constructor:

robot::robot(const robot& r)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(r);
}

robot::~robot()
{
   delete track_ptr;
}

// ---------------------------------------------------------------------
void robot::docopy(const robot& r)
{
//   cout << "inside robot::docopy()" << endl;

   ID=r.ID;
   n_candidate_headings=r.n_candidate_headings;
   heading_angle=r.heading_angle;
   prev_theta=r.prev_theta;
   curr_statevector=r.curr_statevector;
   groundspace_ptr=r.groundspace_ptr;
   track_ptr=r.track_ptr;

   heading_change_matrix=r.heading_change_matrix;
}

// Overload = operator:

robot& robot::operator= (const robot& r)
{
   if (this==&r) return *this;
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const robot& r)
{
   outstream << endl;

//   cout << "inside robot::operator<<" << endl;
   outstream << "Robot ID = " << r.ID << endl;
   outstream << "Curr statevector = " << r.curr_statevector << endl;

   return(outstream);
}

// =====================================================================
// Member function propagate_statevector

statevector& robot::propagate_statevector(double curr_t)
{
//   cout << "inside robot::propagate_statevector()" << endl;
   double dt=curr_t-curr_statevector.get_time();

// Don't bother to perform expensive cost function evaluations or
// update robot's statevector if dt is less than some minimal value:

   const double min_dt=0.01;	// secs
   if (dt < min_dt) return curr_statevector;

// Loop over possible heading changes, compute possible next positions
// and evaluate cost function to determine which one robot should
// choose:

   threevector curr_position(curr_statevector.get_position());
   threevector curr_velocity(curr_statevector.get_velocity());
   threevector next_position,next_velocity;
   double min_cost=POSITIVEINFINITY;
//    int n_best=-1;
   for (unsigned int n=0; n<heading_change_matrix.size(); n++)
   {
      threevector candidate_next_velocity=
         heading_change_matrix[n]*curr_velocity;
      threevector candidate_next_position=curr_position+
         dt*candidate_next_velocity;
      double curr_cost=cost_function(candidate_next_position,
                                     candidate_next_velocity);
      if (curr_cost < min_cost)
      {
         min_cost=curr_cost;
         next_position=candidate_next_position;
         next_velocity=candidate_next_velocity;
//          n_best=n;
      }
   } // loop over index n labeling candidate heading changes

   curr_statevector.set_time(curr_t);
   curr_statevector.set_position(next_position);
   curr_statevector.set_velocity(next_velocity);

// Compute robot heading based upon its velocity vector:

   double theta=atan2(next_velocity.get(1),next_velocity.get(0));
   theta=basic_math::phase_to_canonical_interval(theta,-PI,PI);
   heading_angle=PI/2-theta;
   prev_theta=theta;

//   cout << "n_best = " << n_best << endl;
//   cout << "curr_statevector = " << curr_statevector << endl;
   return curr_statevector;
}

double robot::cost_function(const threevector& posn,const threevector& vel)
{
//   cout << "inside robot::cost_function()" << endl;
//   double C0=0;

// First compute robot's instantaneous radius from ground space
// origin:

   double radius_from_origin=(posn-groundspace_ptr->get_origin()).magnitude();
   double radius_frac=radius_from_origin/groundspace_ptr->
      get_max_robot_dist_from_origin();

// Penalty for deviating from current velocity direction:

   double alpha_heading=0.01*(0.90-radius_frac);
   
   double E_heading=
      alpha_heading*(vel-curr_statevector.get_velocity()).magnitude();
   
   double E_groundspace=0;
   if (groundspace_ptr != NULL)
   {
      E_groundspace=groundspace_ptr->potential_energy(
         radius_from_origin,posn);
   }

//   cout << "E_heading = " << E_heading << endl;
//   cout << "E_groundspace = " << E_groundspace << endl;

   double Etotal=E_heading+E_groundspace;
   return Etotal;
}
