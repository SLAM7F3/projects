// ==========================================================================
// Header file for piecewise_linear_vector class
// ==========================================================================
// Last updated on 5/14/08; 4/4/14
// ==========================================================================

#ifndef PIECEWISE_LINEAR_VECTOR_H
#define PIECEWISE_LINEAR_VECTOR_H

#include <vector>
#include "math/rpy.h"
#include "math/threevector.h"

class piecewise_linear_vector
{
  public:

// Initialization, constructor and destructor functions:

   piecewise_linear_vector(
      const std::vector<double>& T_input,
      const std::vector<threevector>& R_input);
   piecewise_linear_vector(
      const std::vector<double>& T_input,
      const std::vector<rpy>& RPY_input);
   piecewise_linear_vector(const piecewise_linear_vector& p);
   ~piecewise_linear_vector();
   piecewise_linear_vector& operator= (const piecewise_linear_vector& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const piecewise_linear_vector& p);
 
// Set and get methods:

// Polynomial evaluation methods:

   threevector value(double t) const;
   int closest_vertex_ID(double t) const;

  private: 

   unsigned int n_vertices;
   std::vector<double> T;
   std::vector<threevector> R;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const piecewise_linear_vector& p);

   int get_segment_number(double t) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

# endif // piecewise_linear_vector.h











