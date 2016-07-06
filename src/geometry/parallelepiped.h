// ==========================================================================
// Header file for parallelepiped class
// ==========================================================================
// Last modified on 8/20/07; 3/17/09; 1/29/12; 4/4/14
// ==========================================================================

#ifndef PARALLELEPIPED_H
#define PARALLELEPIPED_H

#include <string>
#include <sstream>
#include <vector>
#include "geometry/polygon.h"
#include "math/threematrix.h"
#include "math/twovector.h"

class rotation;
class threevector;

class parallelepiped
{

  public:

// Initialization, constructor and destructor functions:

   parallelepiped(void);
   parallelepiped(
      const threevector& U,const threevector& V,const threevector& W,
      const threevector& origin);
   parallelepiped(const polygon& bface,const threevector& hvec);
   parallelepiped(const polygon& bface,const threevector& uhat,double h);
   parallelepiped(
      const std::vector<threevector>& T,const std::vector<threevector>& B);
   parallelepiped(const parallelepiped& p);
   virtual ~parallelepiped();
   parallelepiped operator= (const parallelepiped& f);
   friend std::ostream& operator<< 
      (std::ostream& outstream,parallelepiped& p);
   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line);

// Set and get member functions:

   unsigned int get_n_sidefaces() const;
   double get_width() const;
   double get_length() const;
   double get_height() const;
   const mypoint& get_center() const;
   polygon& get_topface();
   const polygon& get_topface() const;
   polygon& get_bottomface();
   const polygon& get_bottomface() const;
   polygon& get_sideface(int f);
   const polygon& get_sideface(int f) const;

// Parallelepiped properties member functions:

   void calculate_symmetry_vectors_and_lengths();
   void calculate_metric_and_its_inverse();
   void calculate_wlh_coords(
      const threevector& point,double& w,double& h,double& l);

// Interesection member functions:

   void point_location(
      const threevector& point,bool& point_inside,bool& point_outside,
      bool& point_on,double delta_half_fraction=0);
   bool point_inside(const threevector& point);
   void linesegment_intersection(
      const linesegment& l,double& fstart,double& fstop);
   void polygon_edges_intersection(
      const polygon& poly,double fstart[],double fstop[]);
   
   bool ray_pierces_me(
      double tan_elevation,const threevector& ray_basepoint,
      const threevector& ray_hat);
   void wrap_bbox_around_segment(
      const threevector& V1,const threevector& V2,double width_scale);

// Projection methods:

   void XYZ_vertex_positions(std::vector<threevector>& XYZ);
   void XY_vertex_projections(
      std::vector<twovector>& XY,double xscale_factor=1);
   parallelepiped box_approx(polygon& bfp);
   
// Rigid parallelepiped manipulation methods

   void absolute_position(const threevector& rvec);
   void translate(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

  protected:

   static const unsigned int n_sidefaces;
   double width,length,height;
   double half_width,half_length,half_height;
   threevector what,lhat,hhat;
   mypoint center;
   threematrix g,ginv;
   polygon top_face,bottom_face;
   polygon* side_face;

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void generate_top_and_side_faces(const threevector& W);
   void docopy(const parallelepiped& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline unsigned int parallelepiped::get_n_sidefaces() const
{
   return n_sidefaces;
}

inline double parallelepiped::get_width() const
{
   return width;
}

inline double parallelepiped::get_length() const
{
   return length;
}

inline double parallelepiped::get_height() const
{
   return height;
}

inline const mypoint& parallelepiped::get_center() const
{
   return center;
}

inline polygon& parallelepiped::get_topface() 
{
   return top_face;
}

inline const polygon& parallelepiped::get_topface() const
{
   return top_face;
}

inline polygon& parallelepiped::get_bottomface() 
{
   return bottom_face;
}

inline const polygon& parallelepiped::get_bottomface() const
{
   return bottom_face;
}

inline polygon& parallelepiped::get_sideface(int f)
{
   return side_face[f];
}

inline const polygon& parallelepiped::get_sideface(int f) const
{
   return side_face[f];
}

// ---------------------------------------------------------------------
inline std::ostream& parallelepiped::write_to_textstream(
   std::ostream& textstream)
{
   calculate_symmetry_vectors_and_lengths();
   bottom_face.write_to_textstream(textstream);
   textstream << hhat.get(0) << " " << hhat.get(1) << " " << hhat.get(2)
              << std::endl;
   textstream << height << std::endl;
   return textstream;
}

// ---------------------------------------------------------------------
inline void parallelepiped::read_from_text_lines(
   unsigned int& i,std::vector<std::string>& line)
{
   bottom_face.read_from_text_lines(i,line);
   std::vector<double> V=stringfunc::string_to_numbers(line[i++]);
   hhat=threevector(V[0],V[1],V[2]);
   std::istringstream input_string_stream(line[i++]);
   input_string_stream >> height;
   *this=parallelepiped(bottom_face,hhat,height);
}

// ---------------------------------------------------------------------
// Member function calculate_wlh_coords returns the coordinates of an
// input vector relative to the box's center in the
// width-length-height coordinate system

inline void parallelepiped::calculate_wlh_coords(
   const threevector& point,double& w,double& h,double& l)
{
   threevector p=threevector(point)-center.get_center();
//   double v0=p.dot(what);
//   double v1=p.dot(lhat);
//   double v2=p.dot(hhat);
   double v0=p.get(0)*what.get(0)+p.get(1)*what.get(1)+p.get(2)*what.get(2);
   double v1=p.get(0)*lhat.get(0)+p.get(1)*lhat.get(1)+p.get(2)*lhat.get(2);
   double v2=p.get(0)*hhat.get(0)+p.get(1)*hhat.get(1)+p.get(2)*hhat.get(2);
   
   w=ginv.get(0,0)*v0+ginv.get(0,1)*v1+ginv.get(0,2)*v2;
   l=ginv.get(1,0)*v0+ginv.get(1,1)*v1+ginv.get(1,2)*v2;
   h=ginv.get(2,0)*v0+ginv.get(2,1)*v1+ginv.get(2,2)*v2;

//   cout << "ginv = " << ginv << endl;
//   cout << "w = " << w << " wnew = " << wnew << endl;
//   cout << "l = " << l << " lnew = " << lnew << endl;
//   cout << "h = " << h << " hnew = " << hnew << endl;
}

// ---------------------------------------------------------------------
// Boolean member function point_inside is a stripped down version of
// point_location.  

inline bool parallelepiped::point_inside(const threevector& point)
{
   double w,l,h;
   calculate_wlh_coords(point,w,h,l);
   return (w > -half_width && w < half_width &&
           l > -half_length && l < half_length &&
           h > -half_height && h < half_height);
}

#endif  // parallelepiped.h






