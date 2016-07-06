// ==========================================================================
// Header file for euler class which decomposes an arbitrary rotation
// matrix into Euler (quaternion) parameters e0, e1, e2 and e3.  The
// rotation is uniquely described by an axis n_hat and an angle chi
// about n_hat.  e0 = cos(chi/2) while (e1,e2,e3) = n_hat * sin(chi/2)
// and e0**2+e1**2+e2**2+e3**2=1.  See discussion in
// http://mathworld.wolfram.com/EulerAngles.html.
// ==========================================================================
// Last modified on 2/7/06; 10/27/08; 1/29/12
// ==========================================================================

#ifndef EULER_H
#define EULER_H

#include <vector>
#include "math/threevector.h"

class rotation;

class euler
{

  public:

// Initialization, constructor and destructor functions:

   euler();
   ~euler();

// Set & get member functions:

   genvector* get_Eptr();
   genvector* get_best_Eptr();

// Initialization member functions:

   void fill_data_matrix();
   void initialize_E();
   void compute_Jacobian();

// Iterative refinement member functions:

   double compute_F();
   void refine_E();
   bool solve_for_E(const rotation& R);
   void rotation_axis_and_angle(threevector& n_hat,double& phi);
   
  private: 

   genmatrix* M_ptr;
   genvector* N_ptr;
   genvector *E_ptr,*best_Eptr;
   genvector* F_ptr;
   genmatrix* J_ptr;   

   void allocate_member_objects();
   void initialize_member_objects();

   void fill_N_vector(const rotation& R);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline genvector* euler::get_Eptr()
{
   return E_ptr;
}

inline genvector* euler::get_best_Eptr()
{
   return best_Eptr;
}

#endif  // numerical/euler.h





