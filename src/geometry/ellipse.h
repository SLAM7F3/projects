// ==========================================================================
// Header file for ellipse class
// ==========================================================================
// Last modified on 2/19/07; 2/21/07
// ==========================================================================

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <iostream>
#include <vector>
#include "math/threevector.h"

class ellipse
{

  public:

// Initialization, constructor and destructor functions:

   ellipse();
   ellipse(double a,double b,double theta);
   ellipse(const threevector& XY_center,double a,double b,double theta);
   ellipse(const ellipse& e);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~ellipse();
   ellipse& operator= (const ellipse& p);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const ellipse& e);

// Set and get member functions:

   std::vector<threevector>& get_vertices() ;
   const std::vector<threevector>& get_vertices() const;
   threevector& get_vertex(int n);
   const threevector& get_vertex(int n) const;

// Ellipse vertex generation member functions:

   std::vector<threevector>& generate_vertices(
      int n_vertices,double phase_offset=0);

  private: 

   double a,b; // semi major & minor axes lengths
   double theta; // orientation angle of ellipse's major axis with +x_hat
   threevector XY_center;
   std::vector<threevector> vertex;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ellipse& e);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::vector<threevector>& ellipse::get_vertices() 
{
   return vertex;
}

inline const std::vector<threevector>& ellipse::get_vertices() const
{
   return vertex;
}

inline threevector& ellipse::get_vertex(int n) 
{
   return vertex[n];
}

inline const threevector& ellipse::get_vertex(int n) const
{
   return vertex[n];
}

#endif  // ellipse.h



