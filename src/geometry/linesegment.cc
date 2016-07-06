// =========================================================================
// Linesegment class member function definitions
// =========================================================================
// Last modified on 1/28/12; 1/29/12; 5/13/12
// =========================================================================

#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "geometry/linesegment.h"
#include "templates/mytemplates.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void linesegment::allocate_member_objects()
{
}		       

void linesegment::initialize_member_objects()
{
}

linesegment::linesegment(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

void linesegment::compute_length_and_direction()
{
   length=(v2-v1).magnitude();
//   if (length==0 || !isfinite(length))
   if (length==0)
   {
      cout << "Trouble inside linesegment::compute_length_and_direction()" 
           << endl;
      cout << "v1 = " << v1 << endl;
      cout << "v2 = " << v2 << endl;
      cout << "v2-v1 = " << v2-v1 << endl;
      cout << "Direction unitvector e_hat ill-defined!" << endl;
   }
   else
   {
      e_hat=(v2-v1).unitvector();
   }
}

linesegment::linesegment(const threevector& V1,const threevector& V2)
{
   allocate_member_objects();
   initialize_member_objects();

   v1=V1;
   v2=V2;
   compute_length_and_direction();
}

linesegment::linesegment(
   const threevector& V1,double l,double direction_angle)
{
   allocate_member_objects();
   initialize_member_objects();

   v1=V1;
   length=l;
   e_hat=threevector(cos(direction_angle),sin(direction_angle),0);
   v2=v1+length*e_hat;
}

// Copy constructor:

linesegment::linesegment(const linesegment& l)
{
   docopy(l);
}

linesegment::~linesegment()
{
}

// ---------------------------------------------------------------------
void linesegment::docopy(const linesegment& l)
{
//   cout << "inside linesegment::docopy()" << endl;
//   cout << "l = " << l << endl;
//   cout << "l.length = " << l.length << endl;
//   cout << "this = " << this << endl;
   length=l.get_length();
//   cout << "length = " << length << endl;
//   cout << "l.v1 = " << l.v1 << endl;
   v1=l.get_v1();
//   cout << "l.v2 = " << l.v2 << endl;
   v2=l.get_v2();
//   cout << "l.e_hat = " << l.e_hat << endl;
   e_hat=l.get_ehat();
//   cout << "at end of linesegment::docopy()" << endl;
}

// Overload = operator:

linesegment& linesegment::operator= (const linesegment& l)
{
   if (this==&l) return *this;
   docopy(l);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const linesegment& l)
{
   outstream << endl;
   outstream << "vertex[0] = " << l.get_v1() << endl;
   outstream << "vertex[1] = " << l.get_v2() << endl;
//   outstream << "vertex[0]=threevector(" 
//             << l.v1.get(0) << "," << l.v1.get(1) << ","
//             << l.v1.get(2) << ");" << endl;
//   outstream << "vertex[1]=threevector(" 
//             << l.v2.get(0) << "," << l.v2.get(1) << ","
//             << l.v2.get(2) << ");" << endl;
//   outstream << "v1 = " << l.v1 << endl;
//   outstream << "v2 = " << l.v2 << endl;
   outstream << "length = " << l.get_length() << endl;
   outstream << "ehat = " << l.get_ehat() << endl;
   return(outstream);
}

// =====================================================================
// Intrinsic linesegment properties:
// =====================================================================
   
// Member function point_on_segment returns true if the input vector
// lies (approximately) on the current linesegment object between
// endpoints v1 and v2:

bool linesegment::point_on_segment(
   const threevector& currpoint,double endpoint_distance_threshold) const
{
   bool pnt_on_segment=false;
   threevector s1(currpoint-threevector(v1));
   threevector s2(currpoint-threevector(v2));

//   const double TINY=1E-8;
   if (nearly_equal(s1.magnitude(),0,endpoint_distance_threshold) || 
       nearly_equal(s2.magnitude(),0,endpoint_distance_threshold))
   {
      pnt_on_segment=true;
   }
   else
   {
      threevector s1_hat(s1.unitvector());
      double dotproduct=s1_hat.dot(e_hat);
      if (fabs(dotproduct) > 0.99999)
      {
         if (s1.dot(s2) < 0) pnt_on_segment=true;
      }
   }
   return pnt_on_segment;
}

// ---------------------------------------------------------------------
// Member function get_frac_posn() returns the threevector
// corresponding to the input fraction between the current
// linesegment's endpoints.

threevector linesegment::get_frac_posn(double frac) const
{
   threevector start_posn=get_v1();
   threevector stop_posn=get_v2();
   threevector frac_posn=start_posn+frac*(stop_posn-start_posn);
   return frac_posn;
}

// ---------------------------------------------------------------------
// Member function frac_distance_along_segment returns -1 if the input
// point does not lie on the current line segment.  Otherwise, it
// returns the point's fractional distance from endpoint v1.

double linesegment::frac_distance_along_segment(
   const threevector& currpoint) const
{
   if (!point_on_segment(currpoint))
   {
      return -1;
   }
   else
   {
      return fraction_distance_along_segment(currpoint);
   }
}

double linesegment::fraction_distance_along_segment(
   const threevector& currpoint) const
{
   return (currpoint-v1).magnitude()/length;
}

// ---------------------------------------------------------------------
// Member function frac_distance_along_infinite_line() returns a
// positive value if input currpoint lies "upstream" of v1 in the
// +e_hat direction.  It returns a negative value if currpoint lies
// "downstream" of v1 in the -e_hat direction.  And it returns
// NEGATIVEINFINITY if currpoint doesn't lie along the infinite line
// defined by the current line segment.

double linesegment::frac_distance_along_infinite_line(
   const threevector& currpoint) const
{
   if (currpoint.nearly_equal(v1)) return 0;

   double frac=(currpoint-v1).magnitude()/length;

   threevector d_hat=(currpoint-v1).unitvector();
   double dotproduct=e_hat.dot(d_hat);
   if (nearly_equal(dotproduct,1))
   {
      frac *= 1;
   }
   else if (nearly_equal(dotproduct,-1))
   {
      frac *= -1;
   }
   else
   {
      frac=NEGATIVEINFINITY;
   }
   return frac;
}

// ---------------------------------------------------------------------
// Member function point_to_line_distance calculates the
// minimum perpendicular distance between the input point currpoint
// and the infinite line running through the current line segment
// object:

double linesegment::point_to_line_distance(const threevector& currpoint) const
{
   threevector delta(currpoint-v1);
   double term1=delta.sqrd_magnitude();
   double term2=sqr(delta.dot(e_hat));

   double bperp=0;
   if (term1 > term2)
   {
      bperp=sqrt(term1-term2);
   }
   return bperp;
}

// ---------------------------------------------------------------------
// Member function point_to_line_vector returns the vector pointing
// from the input currpoint to the location on the infinite line which
// lies closest to currpoint:

threevector linesegment::point_to_line_vector(const threevector& currpoint) 
   const
{
   threevector delta(currpoint-v1);
   double term1=sqr(delta.magnitude());
   double term2=sqr(delta.dot(e_hat));

//   double bperp;
   if (term1 < term2)
   {
//      bperp=0;
      return Zero_vector;
   }
   else
   {
//      bperp=sqrt(term1-term2);
      return - ( delta-(delta.dot(e_hat))*e_hat ) ;
   }
}

// ---------------------------------------------------------------------
// Member function closest_pnt_on_sphere returns the location on the
// sphere with the input origin and radius which lies closest to the
// infinite line passing through the current linesegment.

threevector linesegment::closest_pnt_on_sphere(
   const threevector& sphere_origin,double sphere_radius) const
{
   threevector V=point_to_line_vector(sphere_origin);
   threevector closest_pnt=sphere_origin+sphere_radius*V.unitvector();
   return closest_pnt;
}

// ---------------------------------------------------------------------
// Member function point_to_line_segment_distance computes the minimum
// distance to a FINITE line segment.  If input point currpoint lies
// within the infinite strip which passes orthogonally to the current
// linesegment object, this method returns its perpendicular distance
// to the linesegment.  But if it lies outside of this strip, this
// method returns the smaller of the two distances to the
// linesegment's endpoints.

// On 2/6/05, we fixed up this method so that neither currpoint nor
// the current linesegment object need lie within the XY plane.

double linesegment::point_to_line_segment_distance(
   const threevector& currpoint) const
{
   threevector closest_point_on_segment;
   return point_to_line_segment_distance(currpoint,closest_point_on_segment);
}

double linesegment::point_to_line_segment_distance(
   const threevector& currpoint,threevector& closest_point_on_segment) const
{
   threevector R(currpoint-v1);
   threevector Q(currpoint-v2);
   double R_parallel=R.dot(e_hat);
   double Q_parallel=Q.dot(e_hat);
   
   if (R_parallel < 0)
   {
      closest_point_on_segment=v1;
   }
   else if (Q_parallel > 0)
   {
      closest_point_on_segment=v2;
   }
   else
   {
      closest_point_on_segment=v1+R_parallel*e_hat;
   }
   return (currpoint-closest_point_on_segment).magnitude();
}

// ---------------------------------------------------------------------
// Boolean method direction_vector_intersects_segment returns false if
// a ray starting from basepoint and propagated forward in the v_hat
// direction does not intersect the current linesegment object.  If it
// does, this method returns the intersection point within
// intersection_pnt.

bool linesegment::direction_vector_intersects_segment(
   const threevector& basepoint,const threevector& v_hat) const
{
   threevector intersection_pnt;
   return direction_vector_intersects_segment(
      basepoint,v_hat,intersection_pnt);
}

bool linesegment::direction_vector_intersects_segment(
   const threevector& basepoint,const threevector& v_hat,
   threevector& intersection_pnt) const
{
   if (v_hat.magnitude() < 0.9 || segment_parallel_to_vector(v_hat))
   {
      return false;
   }
   else
   {
      threevector vperp_hat(-v_hat.get(1),v_hat.get(0),0);
      double numer=(basepoint-v1).dot(vperp_hat);
      double denom=e_hat.dot(vperp_hat);
      double lambda=numer/denom;
      intersection_pnt=v1+lambda*e_hat;

// intersection_pnt = v1+lambda*e_hat = basepoint+alpha*v_hat is
// genuine only if alpha > 0

      double alpha=(threevector(intersection_pnt)-basepoint).dot(v_hat);
      if (alpha < 0)
      {
         return false;
      }
      else
      {
/*
         if (point_on_segment(intersection_pnt))
         {
            cout << "inside linesegment::direction_vector_intersects_segment()"
                 << endl;
            cout << "lambda = " << lambda << endl;         
            cout << "Basepoint = " << basepoint << endl;
            cout << "v_hat = " << v_hat << endl;
            cout << "intersection_pnt = " << intersection_pnt << endl;
            cout << "alpha = " << alpha << endl;
         }
*/
      }
      return point_on_segment(intersection_pnt);
   }
}

// ---------------------------------------------------------------------
// Member function segment_intersects_infinite_line determines whether
// the current linesegment object is parallel to input linesegment l.
// If not, it computes the point at which the two segments would meet
// if l were an infinite line.  If the intersection point lies on the
// current linesegment object, this boolean method returns true.

bool linesegment::segment_intersects_infinite_line(
   const linesegment& l,threevector& intersection_pnt) const
{
   threevector l_eperp_hat(-l.get_ehat().get(1),l.get_ehat().get(0),0);
   double numer=(l.get_v1()-threevector(v1)).dot(l_eperp_hat);
   double denom=e_hat.dot(l_eperp_hat);

// linesegment l and current line segment object are parallel if denom==0:

   if (denom==0)
   {
      return false;
   }
   else
   {
      double lambda=numer/denom;
      intersection_pnt=v1+lambda*e_hat;
      return point_on_segment(intersection_pnt);
   }
}

// ---------------------------------------------------------------------
// Member function point_of_intersection determines whether the
// current linesegment object is parallel to another input linesegment
// l.  If so, it calls member function parallel_segment_intersection.
// If not, it computes the point at which the two segments would meet
// if they were infinite lines.  If the intersection point lies on the
// current linesegment object, boolean variable
// point_on_curr_linesegment is set to true.  If the intersection
// point lies on linesegment l, the boolean variable point_on_l is set
// to true.

void linesegment::point_of_intersection(
   const linesegment& l,threevector& intersection_pnt,
   bool& intersection_pnt_on_curr_linesegment,
   bool& intersection_pnt_on_l,double endpoint_distance_threshold) const
{
   threevector l_eperp_hat(-l.e_hat.get(1),l.e_hat.get(0),0);
   threevector diff_vec(l.v1-v1);
   double numer=diff_vec.dot(l_eperp_hat);
   double denom=e_hat.dot(l_eperp_hat);

// linesegment l and current line segment object are parallel if denom==0:

   if (denom==0)
   {
      parallel_segment_intersection(
         l,intersection_pnt,intersection_pnt_on_curr_linesegment,
         intersection_pnt_on_l);
   }
   else
   {
      double lambda=numer/denom;
      intersection_pnt=v1+lambda*e_hat;
      intersection_pnt_on_curr_linesegment=
         point_on_segment(intersection_pnt,endpoint_distance_threshold);
      intersection_pnt_on_l=l.point_on_segment(
         intersection_pnt,endpoint_distance_threshold);
   }
}

// ---------------------------------------------------------------------
// Member function parallel_segment_intersection computes the
// transverse distance between linesegment l and the current line
// segment object which are assumed to be parallel.  If the distance
// vanishes, it next determines whether the two parallel segments
// overlap.  If so, it sets intersection_pnt to either the v1 point of
// l or of the current line segment object:

void linesegment::parallel_segment_intersection(
   const linesegment& l,threevector& intersection_pnt,
   bool& intersection_pnt_on_curr_linesegment,
   bool& intersection_pnt_on_l) const
{
   const double TINY=1E-10;
   intersection_pnt_on_curr_linesegment=intersection_pnt_on_l=false;

// First calculate transverse distance between the two parallel line
// segments when both are extended to infinite lines:

   threevector deltavec(l.v1-v1);
   double term1=sqr(deltavec.magnitude());
   double term2=sqr(deltavec.dot(e_hat));
   double dist_between_parallel_lines=sqrt(term1-term2);

   if (dist_between_parallel_lines < TINY)
   {
      bool lv1_on_segment=point_on_segment(l.v1);
      bool lv2_on_segment=point_on_segment(l.v2);
      bool v1_on_l=l.point_on_segment(v1);
      bool v2_on_l=l.point_on_segment(v2);

      if (lv1_on_segment || lv2_on_segment)
         intersection_pnt_on_curr_linesegment=true;
      if (v1_on_l || v2_on_l)
         intersection_pnt_on_l=true;
      
      if (lv1_on_segment) intersection_pnt=l.v1;
      if (v1_on_l) intersection_pnt=v1;
   }
}

// ---------------------------------------------------------------------
// Boolean member function segment_parallel_to_vector determines whether the
// current linesegment object is parallel to some specified unit vector:

bool linesegment::segment_parallel_to_vector(const threevector& v) const
{
   const double TINY=1E-8;
   
   threevector eperp_hat(-e_hat.get(1),e_hat.get(0),0);
   double dotproduct=eperp_hat.dot(v);

// Vector v and current line segment object are parallel if
// |dotproduct| < TINY:

   return (fabs(dotproduct) < TINY);
}

// ---------------------------------------------------------------------
// Member function xy_projection returns a linesegment whose vertices
// have had their z components set to zero:

linesegment linesegment::xy_projection() const
{
   threevector v1_proj(v1.get(0),v1.get(1),0);
   threevector v2_proj(v2.get(0),v2.get(1),0);
   return linesegment(v1_proj,v2_proj);
}

// ---------------------------------------------------------------------
// Member function max_direction_overlap takes in two direction
// vectors w_hat and l_hat.  This method computes the dotproduct of
// the current linesegment's direction vector e_hat with w_hat, l_hat,
// -w_hat and -l_hat.  It returns the direction vector which has the
// maximum overlap with e_hat.

threevector linesegment::max_direction_overlap(
   const threevector& w_hat,const threevector& l_hat) const
{
   const int ndirs=4;

   threevector direction_vector[ndirs];
   direction_vector[0]=w_hat;
   direction_vector[1]=l_hat;
   direction_vector[2]=-direction_vector[0];
   direction_vector[3]=-direction_vector[1];

   double dotproduct[ndirs];
   for (int n=0; n<ndirs; n++)
   {
      dotproduct[n]=e_hat.dot(direction_vector[n]);
   }
   int max_index;
   max_array_value(ndirs,max_index,dotproduct);
   return direction_vector[max_index];
}

// ---------------------------------------------------------------------
// This overloaded version of member function max_direction_overlap
// takes in starting symmetry direction vector w_hat and angle
// delta_theta measured in radians.  It generates 2*PI/delta_theta
// bins corresponding to angles w_hat, w_hat+delta_theta,
// w_hat+2*delta_theta, etc.  This method computes the dotproduct of
// each angular bin with the current linesegment's direction vector.
// It returns the direction vector fmax_hat which has the largest
// overlap with the linesegment's direction vector e_hat.

threevector linesegment::max_direction_overlap(
   const threevector& w_hat,double delta_theta) const
{
//   cout << "inside linesegment::max_direction_overlap()" << endl;
   int n_dirs=basic_math::round(2*PI/delta_theta);
//   cout << "n_dirs = " << n_dirs << endl;

   double max_dotproduct=NEGATIVEINFINITY;
   double theta_start=mathfunc::angle_between_unitvectors(x_hat,w_hat);
   threevector fmax_hat;
   for (int n=0; n<n_dirs; n++)
   {
      double theta=theta_start+n*delta_theta;
      threevector f_hat(cos(theta),sin(theta),0);
      double curr_dotproduct=f_hat.dot(e_hat);
      if (curr_dotproduct > max_dotproduct)
      {
         max_dotproduct=curr_dotproduct;
         fmax_hat=f_hat;
      }
//      cout << "n = " << n << " theta = " << theta*180/PI 
//           << " emax_hat = " << emax_hat << endl;
   } // loop over index n labeling direction vectors

   return fmax_hat;
}

// ---------------------------------------------------------------------
// Member function total_segment_overlap takes in linesegment l which
// is assumed to be aligned or anti-aligned with e_hat.  This method
// computes the ehat coordinates of the current linesegment's
// endpoints as well as those of l.  If the former completely enclose
// the latter, this boolean method returns true.

bool linesegment::total_segment_overlap(const linesegment& l) const
{
   
// First compute f_hat = z_hat x e_hat and check whether input
// linesegment l and current segment have very similar f coordinate
// values:

   threevector f_hat(z_hat.cross(e_hat));
//   double f1=v1.dot(f_hat);
//   double f2=v2.dot(f_hat);

//   if (!nearly_equal(f1,f2))
//   {
//      cout << "Trouble in linesegment::total_segment_overlap()" << endl;
//      cout << "f1 = " << f1 << " f2 = " << f2 << " f_hat = " << f_hat 
//           << endl;
//   }

   double E1=v1.dot(e_hat);
   double E2=v2.dot(e_hat);
   double min_E=basic_math::min(E1,E2);
   double max_E=basic_math::max(E1,E2);
   double eps1=l.v1.dot(e_hat);
   double eps2=l.v2.dot(e_hat);
//   double phi1=l.v1.dot(f_hat);
//   double phi2=l.v2.dot(f_hat);

//   if (!nearly_equal(phi1,phi2))
//   {
//      cout << "Trouble in linesegment::total_segment_overlap()" << endl;
//      cout << "phi1 = " << phi1 << " phi2 = " << phi2 << " f_hat = " << f_hat 
//           << endl;
//   }

//   cout << "eps1 = " << eps1 << " eps2 = " << eps2 << endl;
//   cout << "min_E = " << min_E << " max_E = " << max_E << endl;

   bool total_overlap=(
      (eps1 > min_E || nearly_equal(eps1,min_E)) &&
      (eps1 < max_E || nearly_equal(eps1,max_E)) &&
      (eps2 > min_E || nearly_equal(eps2,min_E)) &&
      (eps2 < max_E || nearly_equal(eps2,max_E)));

//   cout << "total_overlap = " << total_overlap << endl;
   return total_overlap;
}

// ---------------------------------------------------------------------
// Member function segment_overlap takes in linesegment l and returns
// a value between 0 and 1 indicating how much it overlaps with the
// current linesegment *this.

double linesegment::segment_overlap(const linesegment& l) const
{

// First check whether directions of input linesegment l and current
// linesegment *this are nearly aligned:

   double overlap=0;
   double dotproduct=e_hat.dot(l.e_hat);
   
   if (fabs(dotproduct) > 0.999)	// cos(2 degs)=0.9993908
   {

// Introduce "long" and "short" linesegments.  Translate both so that
// the "long" linesegment starts at (0,0,0).  If these segments are
// anti-aligned, rotate the shorter one by 180 degrees:
      
      linesegment l_long,l_short;
      if (length >= l.length)
      {
         l_long=linesegment(v1-v1,v2-v1);
         l_short=linesegment(l.v1-v1,l.v2-v1);
      }
      else
      {
         l_long=linesegment(l.v1-l.v1,l.v2-l.v1);
         l_short=linesegment(v1-l.v1,v2-l.v1);
      }
      if (dotproduct < 0) l_short=linesegment(l_short.v2,l_short.v1);

// Next check whether l and *this lie nearly on the same infinite
// line:

      double transverse_separation=l_long.point_to_line_distance(
         l_short.get_midpoint());
      double transverse_separation_ratio=transverse_separation/l_short.length;
//      cout << "trans sep ratio = " << transverse_separation_ratio << endl;

      const double SMALL=0.03;
      if (transverse_separation_ratio < SMALL)
      {
         double short_v1=l_short.v1.dot(l_long.e_hat);
         double short_v2=l_short.v2.dot(l_long.e_hat);
         double long_v1=l_long.v1.dot(l_long.e_hat);
         double long_v2=l_long.v2.dot(l_long.e_hat);
//         cout << "short_v1 = " << short_v1 
//              << " short_v2 = " << short_v2 << endl;
//         cout << "long_v1 = " << long_v1 
//              << " long_v2 = " << long_v2 << endl;

         if (short_v1 > long_v2)
         {
            overlap=0;
         }
         else if (short_v2 < long_v1)
         {
            overlap=0;
         }
         else if (short_v1 >= long_v1 && short_v2 <= long_v2)
         {
            overlap=1.0;
         }
         else if (short_v1 >= long_v1 && short_v2 >= long_v2)
         {
            overlap=(long_v2-short_v1)/l_short.length;
         }
         else if (short_v1 <= long_v1 && short_v2 <= long_v2)
         {
            overlap=(short_v2-long_v1)/l_short.length;
         }
      } // l & *this on same infinite line conditional
   } // l & *this directions aligned conditional
   return overlap;
}

// =====================================================================
// Moving around linesegments
// =====================================================================

void linesegment::translate(const threevector& rvec)
{
   v1 += rvec;
   v2 += rvec;
}

// ---------------------------------------------------------------------
// Member function absolute_position translates the line segment so
// that its midpoint coincides with input threevector rvec:

void linesegment::absolute_position(const threevector& rvec)
{
   threevector origin(get_midpoint());
   v1 += rvec-origin;
   v2 += rvec-origin;
}

// ---------------------------------------------------------------------
void linesegment::scale(const threevector& scale_origin,
                        const threevector& scalefactor)
{
   threevector dv1(v1-scale_origin);
   threevector dv2(v2-scale_origin);
   dv1.scale(scalefactor);
   dv2.scale(scalefactor);
   v1=scale_origin+dv1;
   v2=scale_origin+dv2;
   compute_length_and_direction();
}

// ---------------------------------------------------------------------
void linesegment::rotate(const rotation& R)
{
   const threevector rotation_origin(Zero_vector);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void linesegment::rotate(const threevector& rotation_origin,const rotation& R)
{
   threevector dv1(v1-rotation_origin);
   threevector dv2(v2-rotation_origin);
   dv1=R*dv1;
   dv2=R*dv2;
   v1=rotation_origin+dv1;
   v2=rotation_origin+dv2;
   e_hat=R*e_hat;
}

// ---------------------------------------------------------------------
void linesegment::rotate(const threevector& rotation_origin,
                         double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
// Member function rotate_about_midpoint takes in a new direction
// vector f_hat.  It replaces the current linesegment with a new one
// having the same midpoint but with the new direction vector.

void linesegment::rotate_about_midpoint(const threevector& f_hat)
{
   linesegment new_l(get_midpoint()-0.5*length*f_hat,
                     get_midpoint()+0.5*length*f_hat);
   *this=new_l;
}

// =========================================================================
// 2D line property methods
// =========================================================================

// Member function compute_2D_line_coeffs() calculates infinite 2D
// line coefficients a,b,c in the equation a * X + b * Y + c = 0.
// These coefficients are needed for geometric camera calibration via
// tie-line matching.  It stores them within member STL vector
// twoD_line_coeffs.

// Note added on 12/20/11: Much simpler version of this member
// function would simply set (a,b,c) = get_v1() x get_v2() !

vector<double>& linesegment::compute_2D_line_coeffs()
{
//   cout << "inside linsegment::compute_2D_line_coeffs()" << endl;
   
   double u_one=get_v1().get(0);
   double v_one=get_v1().get(1);
   double u_two=get_v2().get(0);
   double v_two=get_v2().get(1);
   
   double det=u_one*v_two-u_two*v_one;
   const double TINY=1.0E-8;
   
   double a,b,c;
   if (nearly_equal(u_one,u_two,TINY))
   {
      a=1;
      b=0;
      c=-u_one;
   }
   else if (nearly_equal(v_one,v_two,TINY))
   {
      a=0;
      b=1;
      c=-v_one;
   }
   else if (nearly_equal(det,0,TINY))
   {
      a=v_two-v_one;
      b=u_one-u_two;
      c=0;
   }
   else
   {
      a=(v_two-v_one)/det;
      b=(u_one-u_two)/det;
      c=-1;
   }

   twoD_line_coeffs.push_back(a);
   twoD_line_coeffs.push_back(b);
   twoD_line_coeffs.push_back(c);

//   cout << "a = " << a << " b = " << b 
//        << " c = " << c << " sgn(det) = " << sgn(det) << endl;

/*
   threevector v_1=get_v1();
   threevector v_2=get_v2();
   v_1.put(2,1);
   v_2.put(2,1);

   threevector abc=v_1.cross(v_2);
   abc /= abc.get(2);
   cout << "abc = " << abc << endl;
*/

   return twoD_line_coeffs;
}

// ---------------------------------------------------------------------
// Member function sqrd_twoD_distance_to_point() takes in a 2D point (U,V).
// It returns the squared 2D distance between the input point and an
// infinite 2D line specified by a * X + b * Y + c = 0.

double linesegment::sqrd_twoD_distance_to_point(
   double a,double b, double c,double U,double V)
{
   double numer=sqr(a*U+b*V+c);
   double denom=a*a+b*b;
   return numer/denom;
}


/*
// ---------------------------------------------------------------------
// Member function compute_3D_line_coeffs() takes in 3-vectors V1
// and V2. It returns coefficients (alpha,beta,gamma,delta) within an
// output fourvector which satisfy alpha x + beta y + gamma z + delta
// = 0.

fourvector linesegment::compute_3D_line_coeffs()
{
   cout << "inside linesegment::compute_3D_line_coeffs()" << endl;
   cout << "V1 = " << v1 << " V2 = " << v2 << endl;

   double x1=v1.get(0);
   double x2=v2.get(0);
   double y1=v1.get(1);
   double y2=v2.get(1);
   double z1=v1.get(2);
   double z2=v2.get(2);
         
   double dx=x2-x1;
   double dy=y2-y1;
   double dz=z2-z1;

   double det_xy=x1*y2-y1*x2;
   double det_yz=y1*z2-z1*y2;
   double det_zx=z1*x2-x1*z2;

   double alpha=dy-dz;
   double beta=dz-dx;
   double gamma=dx-dy;
   double delta=-(det_xy+det_yz+det_zx);

   cout << "a = " << alpha << " b = " << beta << " c= " << gamma
        << " d = " << delta << endl;

   double result1=alpha*x1+beta*y1+gamma*z1+delta;
   double result2=alpha*x2+beta*y2+gamma*z2+delta;
      
   cout << "result1 = " << result1
        << " result2 = " << result2 << endl;

   fourvector line_coeffs(alpha,beta,gamma,delta);
   cout << "line coeffs = " << line_coeffs << endl;
   return line_coeffs;
}
*/
      



