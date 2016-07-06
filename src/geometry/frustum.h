// ==========================================================================
// Header file for frustum class
// ==========================================================================
// Last modified on 4/21/06; 6/10/06; 1/29/12; 4/5/14
// ==========================================================================

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "mypoint.h"
#include "regular_polygon.h"

class rotation;

class frustum
{

  public:

// Initialization, constructor and destructor functions:

   frustum(void);
   frustum(int num_of_sidefaces);
   frustum(int num_of_sidefaces,double top_radius,double bottom_radius,
           double h);
   frustum(const frustum& f);
   ~frustum();
   frustum& operator= (const frustum& f);

// Set and get member functions:

   void set_n_sidefaces(int n);
   void set_height(double h);
   void set_center(const mypoint& c);
   void set_top_face(const regular_polygon& face);
   void set_bottom_face(const regular_polygon& face);
   unsigned int get_n_sidefaces() const;
   double get_height() const;
   mypoint& get_center();
   regular_polygon& get_top_face();
   const regular_polygon& get_top_face() const;
   regular_polygon& get_bottom_face();
   const regular_polygon& get_bottom_face() const;
   polygon& get_sideface(int f);
   const polygon& get_sideface(int f) const;

   void orient_normals_outward();
   void print_frustum();
   void translate(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   bool point_inside_xyprojected_frustum(const threevector& point) const;

  private: 

   static const int nmax_sidefaces;
   unsigned int n_sidefaces;
   double height;
   mypoint center;
   regular_polygon top_face;
   regular_polygon bottom_face;
   polygon* side_face;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const frustum& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void frustum::set_n_sidefaces(int n)
{
   n_sidefaces=n;
}

inline void frustum::set_height(double h)
{
   height=h;
}

inline void frustum::set_center(const mypoint& c)
{
   center=c;
}

inline void frustum::set_top_face(const regular_polygon& face)
{
   top_face=face;
}

inline void frustum::set_bottom_face(const regular_polygon& face)
{
   bottom_face=face;
}

inline unsigned int frustum::get_n_sidefaces() const
{
   return n_sidefaces;
}

inline double frustum::get_height() const
{
   return height;
}

inline mypoint& frustum::get_center() 
{
   return center;
}

inline regular_polygon& frustum::get_top_face() 
{
   return top_face;
}

inline const regular_polygon& frustum::get_top_face() const
{
   return top_face;
}

inline regular_polygon& frustum::get_bottom_face()
{
   return bottom_face;
}

inline const regular_polygon& frustum::get_bottom_face() const
{
   return bottom_face;
}

inline polygon& frustum::get_sideface(int f) 
{
   return side_face[f];
}

inline const polygon& frustum::get_sideface(int f) const
{
   return side_face[f];
}

#endif  // frustum.h






