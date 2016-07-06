// ==========================================================================
// Header file for parallelogram class
// ==========================================================================
// Last modified on 1/28/12; 1/29/12; 2/29/12
// ==========================================================================

#ifndef PARALLELOGRAM_H
#define PARALLELOGRAM_H

#include "math/genmatrix.h"
#include "geometry/polygon.h"
#include "math/threematrix.h"
#include "math/threevector.h"

class contour;
class rotation;

class parallelogram:public polygon
{

  public:

// Initialization, constructor and destructor functions:

   parallelogram(void);
   parallelogram(double width,double length);
   parallelogram(const std::vector<threevector>& V);
   parallelogram(const polygon& p);
   parallelogram(contour const *c_ptr);
   parallelogram(const parallelogram& p);
   ~parallelogram();
   parallelogram& operator= (const parallelogram& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const parallelogram& p);

// Set and get member functions:
   
   double get_width() const;
   double get_length() const;
   double get_area() const;
   const threevector& get_what() const;
   const threevector& get_lhat() const;

// Intrinsic parallelogram properties:

   bool point_inside(const threevector& point) const;

// Moving around parallelogram methods:

// Note added on 11/6/03: Vadim demands that we incorporate the
// following "using" statement which enables the compiler to handle
// overloaded member functions of the base polygon class which have
// not been overridden in this derived parallelogram class:

//   using polygon::scale;
   
   void scale(double scalefactor);
   void scale(const threevector& scale_origin,double scalefactor);
   void scale_width(double scalefactor);
   void scale_length(double scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

// Rectangle member functions:

   void rectangle_within_quadrilateral(
      const std::vector<threevector>& corner);
   parallelogram rectangle_approx();

  private: 

   double width,length,area;
   threevector what,lhat;
   threematrix g,ginv;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const parallelogram& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline double parallelogram::get_width() const
{
   return width;
}

inline double parallelogram::get_length() const
{
   return length;
}

inline double parallelogram::get_area() const
{
   return area;
}

inline const threevector& parallelogram::get_what() const
{
   return what;
}

inline const threevector& parallelogram::get_lhat() const
{
   return lhat;
}

// ---------------------------------------------------------------------
// Member function point_inside first computes the 2-vector
// corresponding to some input point relative to the current
// parallelogram's origin (which we take to be its 0th vertex).  It
// then computes coefficients cw and cl of the 2-vector relative to
// the w_hat and l_hat basis.  If 0 <= cw/width, cl/length < =1, the
// point lies inside the parallelogram, and this boolean method
// returns true.

// This method has intentionally been optimized for speed.

inline bool parallelogram::point_inside(const threevector& point) const
{
   double w_dotproduct=what.dot(point-get_vertex(0));
   double l_dotproduct=lhat.dot(point-get_vertex(0));

   double cw=ginv.get(0,0)*w_dotproduct+ginv.get(0,1)*l_dotproduct;
   double cl=ginv.get(1,0)*w_dotproduct+ginv.get(1,1)*l_dotproduct;
   return (cw >= 0 && cw <= width && cl >= 0 && cl <= length);
}

#endif  // parallelogram.h
