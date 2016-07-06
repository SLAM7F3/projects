// ==========================================================================
// Parallelogram class member function definitions
// ==========================================================================
// Last modified on 1/28/12; 1/29/12; 2/29/12
// ==========================================================================

#include <vector>
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "templates/mytemplates.h"
#include "geometry/parallelogram.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ==========================================================================
// Initialization, constructor and destructor functions:
// ==========================================================================

void parallelogram::allocate_member_objects()
{
}		       

void parallelogram::initialize_member_objects()
{
   polygon::initialize_edge_segments();
   lhat=edge[0].get_ehat();
   what=edge[1].get_ehat();
   width=edge[1].get_length();
   length=edge[0].get_length();
   area=width*length*(what.cross(lhat)).magnitude();

   g.put(0,0,what.dot(what));
   g.put(0,1,what.dot(lhat));
   g.put(1,0,g.get(0,1));
   g.put(1,1,lhat.dot(lhat));
   
   double detg=g.get(0,0)*g.get(1,1)-sqr(g.get(0,1));
   ginv.put(0,0,g.get(1,1)/detg);
   ginv.put(0,1,-g.get(0,1)/detg);
   ginv.put(1,0,ginv.get(0,1));
   ginv.put(1,1,g.get(0,0)/detg);
}

parallelogram::parallelogram(void):
   polygon(4)
{
}

// Rectangle constructor:

parallelogram::parallelogram(double w,double l):
   polygon(4)
{
   width=w;
   length=l;
   what=x_hat;
   lhat=y_hat;

   set_vertex(0,Zero_vector);
   set_vertex(1,threevector(width,0,0));
   set_vertex(2,threevector(width,length,0));
   set_vertex(3,threevector(0,length,0));
   absolute_position(Zero_vector);
   normal=z_hat;
   polygon::initialize_edge_segments();
   
   g.put(0,0,1);
   g.put(1,1,1);
   g.put(0,1,0);
   g.put(1,0,0);

   ginv.put(0,0,1);
   ginv.put(1,1,1);
   ginv.put(0,1,0);
   ginv.put(1,0,0);
}

// This next constructor takes in 4 threevectors which are assumed to
// obey a right-hand rule ordering.  It forms a parallelogram which
// represents a "best fit" to these input points:

parallelogram::parallelogram(const vector<threevector>& V):
   polygon(4)
{
   cout << "inside parallelogram constructor" << endl;
   if (V.size() != 4)
   {
      cout << "Error in parallelogram(vector<threevector>) constructor"
           << endl;
      cout << "V.size() = " << V.size() << endl;
      exit(-1);
   }
   threevector center=0.25*(V[0]+V[1]+V[2]+V[3]);
   cout << "center = " << center << endl;

   threevector l=0.5*(V[3]-V[2]+V[0]-V[1]);
   threevector w=0.5*(V[0]-V[3]+V[1]-V[2]);

   vector<threevector> vertices;
   vertices.push_back(center+0.5*l+0.5*w);
   vertices.push_back(center-0.5*l+0.5*w);
   vertices.push_back(center-0.5*l-0.5*w);
   vertices.push_back(center+0.5*l-0.5*w);

   polygon p(vertices);
   *this=parallelogram(p);
//   *this=parallelogram(vertices);

   allocate_member_objects();
   initialize_member_objects();
}

parallelogram::parallelogram(const polygon& p):
   polygon(4)
{
   for (int i=0; i<4; i++)
   {
      set_vertex(i,p.get_vertex(i));
   }
   compute_normal();

   allocate_member_objects();
   initialize_member_objects();
}

parallelogram::parallelogram(contour const *c_ptr):
   polygon(4)
{
   for (int i=0; i<4; i++)
   {
      set_vertex(i,c_ptr->get_vertex(i).first);
   }
   compute_normal();
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

parallelogram::parallelogram(const parallelogram& p):
   polygon(p)
{
   docopy(p);
}

parallelogram::~parallelogram()
{
}

// ---------------------------------------------------------------------
void parallelogram::docopy(const parallelogram& p)
{
   width=p.width;
   length=p.length;
   what=p.what;
   lhat=p.lhat;
   g=p.g;
   ginv=p.ginv;
}	

// Overload = operator:

parallelogram& parallelogram::operator= (const parallelogram& p)
{
   if (this==&p) return *this;
   polygon::operator=(p);
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const parallelogram& p)
{
   outstream << (polygon&)p << endl;
   outstream << "width = " << p.width << " length = " << p.length << endl;
   outstream << "what = " << p.what << " lhat = " << p.lhat << endl;
   outstream << "g = " << p.g << endl;
   return(outstream);
}

// ==========================================================================
// Moving around parallelogram methods
// ==========================================================================

void parallelogram::scale(double scalefactor)
{
   scale(vertex_average(),scalefactor);
}

// ---------------------------------------------------------------------
void parallelogram::scale(const threevector& scale_origin,double scalefactor)
{
   polygon::scale(scale_origin,scalefactor);
   width *= scalefactor;
   length *= scalefactor;
}

// ---------------------------------------------------------------------
// Member function scale_width [scale_length] dynamically generates
// and returns a new parallelogram whose center and orientation is the
// same as the current object.  But its width [length] is
// multiplicatively enlarged by scalefactor.

void parallelogram::scale_width(double scalefactor)
{
   threevector origin(vertex_average());

   vector<threevector> vertex(4);
   set_vertex(0,origin-0.5*scalefactor*width*what+0.5*length*lhat);
   set_vertex(1,origin-0.5*scalefactor*width*what-0.5*length*lhat);
   set_vertex(2,origin+0.5*scalefactor*width*what-0.5*length*lhat);
   set_vertex(3,origin+0.5*scalefactor*width*what+0.5*length*lhat);
   *this=parallelogram(vertex);
}

void parallelogram::scale_length(double scalefactor)
{
   threevector origin(vertex_average());

   vector<threevector> vertex(4);
   set_vertex(0,origin+0.5*width*what-0.5*scalefactor*length*lhat);
   set_vertex(1,origin+0.5*width*what+0.5*scalefactor*length*lhat);
   set_vertex(2,origin-0.5*width*what+0.5*scalefactor*length*lhat);
   set_vertex(3,origin-0.5*width*what-0.5*scalefactor*length*lhat);
   *this=parallelogram(vertex);
}

// ---------------------------------------------------------------------
void parallelogram::rotate(const rotation& R)
{
   threevector rotation_origin(Zero_vector);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void parallelogram::rotate(const threevector& rotation_origin,
                           const rotation& R)
{
   polygon::rotate(rotation_origin,R);
   what=R*what;
   lhat=R*lhat;
}

// ---------------------------------------------------------------------
void parallelogram::rotate(const threevector& rotation_origin,
                           double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ==========================================================================
// Rectangle member functions:
// ==========================================================================

// Member function rectangle_within_quadrilateral takes in 4 corner
// vertices which are assumed to define a non-self-intersecting
// quadrilateral that is approximately trapezoidal in shape.  It
// generates from this input quadrilateral an output rectangle which
// is guaranteed to lie entirely inside the quadrilateral.  Moreover,
// the rectangle is always oriented along the quadrilateral's longest
// edge.  We devised this algorithm for orthorectifying airborne video
// images projected onto a planar ground surface.

void parallelogram::rectangle_within_quadrilateral(
   const vector<threevector>& corner)
{
   cout << "inside parallelogram::rectangle_within_quadrilateral()" << endl;
   
   polygon input_quad(corner);
   input_quad.initialize_edge_segments();

// Find longest side of input quadrilateral:

   int longest_edge=-1;
   double max_length=-1;
   for (int c=0; c<4; c++)
   {
      linesegment curr_edge(input_quad.get_edge(c));
      linesegment next_edge(input_quad.get_edge(modulo(c+1,4)));
      if (curr_edge.get_length() > max_length)
      {
         longest_edge=c;
         max_length=curr_edge.get_length();
      }
   }

// Generate new quadrilateral whose zeroth edge is its longest side:

   vector<threevector> corners;
   for (int c=0; c<4; c++)
   {
      corners.push_back(input_quad.get_vertex(modulo(c+longest_edge,4)));
   }
   polygon quad(corners);
   quad.initialize_edge_segments();
//   cout << "quad = " << quad << endl;

   double theta_sum=0;
   for (int c=0; c<4; c++)
   {
      linesegment curr_edge(quad.get_edge(c));
      linesegment next_edge(quad.get_edge(modulo(c+1,4)));
      double dotproduct=curr_edge.get_ehat().dot(next_edge.get_ehat());
      double theta=PI-acos(dotproduct);
      theta_sum += theta;
//      cout << "c = " << c << " theta = " << theta*180/PI << endl;
   }
//   cout << "theta_sum = " << theta_sum*180/PI << endl;

// Define l as quadrilateral's longest vector and w[0],w[1] as its
// adjacent edge vectors:

   threevector l(corners[1]-corners[0]);
   threevector l_hat(quad.get_edge(0).get_ehat());

// Compute components of w[0] and w[1] orthogonal to l_hat.  Then
// choose the smaller of these two components:

   vector<threevector> w(2),wperp(2);
   w[0]=corners[3]-corners[0];
   w[1]=corners[2]-corners[1];
   for (int i=0; i<2; i++)
   {
      wperp[i]=w[i]-(l_hat.dot(w[i]))*l_hat;
   }
   threevector W(wperp[0]);
   if (wperp[1].magnitude() < wperp[0].magnitude()) W=wperp[1];

//   cout << "W = " << W << endl;

// Compute rectangles vertices:

   vector<threevector> new_vertex(4);
   new_vertex[0]=corners[0];
   if (w[0].dot(l_hat) > 0)
   {
      new_vertex[0] += w[0].dot(l_hat)*l_hat;
   }
   new_vertex[1]=corners[1];
   if (w[1].dot(-l_hat) > 0)
   {
      new_vertex[1] += w[1].dot(l_hat)*l_hat;
   }
   threevector L(new_vertex[1]-new_vertex[0]);

//   cout << "L = " << L << endl;
//   cout << "L.W = " << L.dot(W) << endl;

   new_vertex[2]=new_vertex[1]+W;
   new_vertex[3]=new_vertex[2]-L;

   *this=parallelogram(new_vertex);
//   cout << "rectangle = " << *this << endl;
}

// ---------------------------------------------------------------------
// Member function rectangle_approx aligns a rectangle with the
// longest sides of the current parallelogram object.  The rectangle's
// short sides bisect the short sides of the parallelogram.

parallelogram parallelogram::rectangle_approx()
{
   threevector nhat((lhat.cross(what)).unitvector());
   threevector lnew_hat,wnew_hat;
   if (length > width)
   {
      lnew_hat=lhat;
      wnew_hat=nhat.cross(lnew_hat);
   }
   else
   {
      wnew_hat=what;
      lnew_hat=-nhat.cross(wnew_hat);
   }
 
//   cout << "lnew_hat = " << lnew_hat << endl;
//   cout << "wnew_hat = " << wnew_hat << endl;
//   cout << "lnew.wnew = " << lnew_hat.dot(wnew_hat) << endl;

   threevector center=vertex_average();
   absolute_position(threevector(0,0,0));

// Set the rectangles new length and width equal to the average of the
// absolute values of the parallelogram's vertex projections onto
// lnew_hat and wnew_hat:

   double mid_lnew=0;
   double mid_wnew=0;
   for (int j=0; j<4; j++)
   {
      mid_lnew += fabs(lnew_hat.dot(get_vertex(j)));
      mid_wnew += fabs(wnew_hat.dot(get_vertex(j)));
   }
   mid_lnew *= 0.25;
   mid_wnew *= 0.25;
   
//   cout << "mid_lnew = " << mid_lnew << " mid_wnew = " << mid_wnew << endl;
   vector<threevector> rect_vertex(4);
   rect_vertex[0]=mid_lnew*lnew_hat+mid_wnew*wnew_hat;
   rect_vertex[1]=-mid_lnew*lnew_hat+mid_wnew*wnew_hat;
   rect_vertex[2]=-mid_lnew*lnew_hat-mid_wnew*wnew_hat;
   rect_vertex[3]=mid_lnew*lnew_hat-mid_wnew*wnew_hat;
   
   parallelogram rectangle(rect_vertex);
   rectangle.absolute_position(center);
   absolute_position(center);

   return rectangle;
}
