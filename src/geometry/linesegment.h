// ==========================================================================
// Header file for linesegment class
// ==========================================================================
// Last modified on 1/27/12; 1/29/12; 5/13/12
// ==========================================================================

#ifndef LINESEGMENT_H
#define LINESEGMENT_H

#include <vector>
#include "general/stringfuncs.h"
#include "math/threevector.h"
#include "math/fourvector.h"
class rotation;

class linesegment
{

  public:

// Initialization, constructor and destructor functions:

   linesegment(void);
   linesegment(const threevector& V1,const threevector& V2);
   linesegment(const threevector& V1,double length,double direction_angle);
   linesegment(const linesegment& l);
   ~linesegment();
   linesegment& operator= (const linesegment& l);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const linesegment& l);
   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line);

// Set and get methods:

   double get_length() const;
   std::vector<double>& get_2D_line_coeffs();
   const std::vector<double>& get_2D_line_coeffs() const;
   threevector& get_v1();
   const threevector& get_v1() const;
   threevector& get_v2();
   const threevector& get_v2() const;
   threevector& get_ehat();
   const threevector& get_ehat() const;
   threevector get_midpoint(double f=0.5) const;

   void set_v1(const threevector& V);
   void set_v2(const threevector& V);

// Intrinsic linesegment properties:
   
   bool point_on_segment(
      const threevector& currpoint,
      double endpoint_distance_threshold=1E-8) const;
   threevector get_frac_posn(double frac) const;
   double frac_distance_along_segment(
      const threevector& currpoint) const;
   double fraction_distance_along_segment(
      const threevector& currpoint) const;
   double frac_distance_along_infinite_line(
      const threevector& currpoint) const;

   double point_to_line_distance(const threevector& currpoint) const;
   threevector point_to_line_vector(const threevector& currpoint) const;
   threevector closest_pnt_on_sphere(
      const threevector& sphere_origin,double sphere_radius) const;

   double point_to_line_segment_distance(const threevector& currpoint) const;
   double point_to_line_segment_distance(
      const threevector& currpoint,threevector& closest_point_on_segment) 
      const;

   bool direction_vector_intersects_segment(
      const threevector& basepoint,const threevector& v_hat) const;
   bool direction_vector_intersects_segment(
      const threevector& basepoint,const threevector& v_hat,
      threevector& intersection_pnt) const;

   bool segment_intersects_infinite_line(
      const linesegment& l,threevector& intersection_pnt) const;
   void point_of_intersection(
      const linesegment& l,threevector& intersection_pnt,
      bool& intersection_pnt_on_curr_linesegment,
      bool& intersection_pnt_on_l,double endpoint_distance_threshold=1E-8) 
      const;
   void parallel_segment_intersection(
      const linesegment& l,threevector& intersection_pnt,
      bool& intersection_pnt_on_curr_linesegment,
      bool& intersection_pnt_on_l) const;
   bool segment_parallel_to_vector(const threevector& v) const;
   linesegment xy_projection() const;
   threevector max_direction_overlap(
      const threevector& w_hat,const threevector& l_hat) const;
   threevector max_direction_overlap(
      const threevector& w_hat,double delta_theta) const;
   bool total_segment_overlap(const linesegment& l) const;
   double segment_overlap(const linesegment& l) const;

// Moving around linesegment methods

   void translate(const threevector& r_vec);
   void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   void rotate_about_midpoint(const threevector& f_hat);

// 2D line property methods

   std::vector<double>& compute_2D_line_coeffs();
   double sqrd_twoD_distance_to_point(
      double a,double b, double c,double U,double V);

  private: 

   double length;
   std::vector<double> twoD_line_coeffs;
   threevector v1,v2,e_hat;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const linesegment& l);

   void compute_length_and_direction();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline double linesegment::get_length() const
{
   return length;
}

inline std::vector<double>& linesegment::get_2D_line_coeffs()
{
   return twoD_line_coeffs;
}

inline const std::vector<double>& linesegment::get_2D_line_coeffs() const
{
   return twoD_line_coeffs;
}

inline threevector& linesegment::get_v1() 
{
   return v1;
}

inline const threevector& linesegment::get_v1() const
{
   return v1;
}

inline threevector& linesegment::get_v2() 
{
   return v2;
}

inline const threevector& linesegment::get_v2() const 
{
   return v2;
}

inline threevector& linesegment::get_ehat() 
{
   return e_hat;
}

inline const threevector& linesegment::get_ehat() const
{
   return e_hat;
}

inline threevector linesegment::get_midpoint(double f) const
{
   return v1+f*(v2-v1);
}

inline void linesegment::set_v1(const threevector& V)
{
   v1=V;
}

inline void linesegment::set_v2(const threevector& V)
{
   v2=V;
}

inline std::ostream& linesegment::write_to_textstream(
   std::ostream& textstream)
{
   v1.write_to_textstream(textstream);
   v2.write_to_textstream(textstream);
   return textstream;
}

inline void linesegment::read_from_text_lines(
   unsigned int& i,std::vector<std::string>& line)
{
   std::vector<double> V1=stringfunc::string_to_numbers(line[i++]);
   v1=threevector(V1[0],V1[1],V1[2]);
   std::vector<double> V2=stringfunc::string_to_numbers(line[i++]);
   v2=threevector(V2[0],V2[1],V2[2]);
   *this=linesegment(v1,v2);
//   std::cout << "*this = " << *this << std::endl;
}

#endif  // linesegment.h



