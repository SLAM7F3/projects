// ==========================================================================
// Header file for piecewise_linear class
// ==========================================================================
// Last updated on 9/12/07; 5/14/13; 4/4/14
// ==========================================================================

#ifndef PIECEWISE_LINEAR_H
#define PIECEWISE_LINEAR_H

#include <vector>
#include "math/twovector.h"

class piecewise_linear
{
  public:

// Initialization, constructor and destructor functions:

   piecewise_linear(
      const std::vector<double>& Xinput,const std::vector<double>& Yinput);
   piecewise_linear(const std::vector<twovector>& V);
   piecewise_linear(const piecewise_linear& p);
   ~piecewise_linear();
   piecewise_linear& operator= (const piecewise_linear& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const piecewise_linear& p);
 
// Set and get methods:

   double get_X_filtered(int n);
   double get_filtered_value(int n);

// Polynomial evaluation methods:

   double value(double x) const;
   unsigned int closest_vertex_ID(double x) const;

// Filtering methods:

   void filter_values(double dx,double sigma);

  private: 

   unsigned int n_vertices;
   std::vector<double> X,Y;
   std::vector<double> X_filtered,Y_filtered;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const piecewise_linear& p);

   unsigned int get_segment_number(double x) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline double piecewise_linear::get_X_filtered(int n)
{
   return X_filtered[n];
}

inline double piecewise_linear::get_filtered_value(int n)
{
   return Y_filtered[n];
}

# endif // piecewise_linear.h











