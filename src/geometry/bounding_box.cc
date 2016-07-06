// =========================================================================
// Bounding_Box class member function definitions
// =========================================================================
// Last modified on 6/25/16; 6/26/16; 7/3/16; 7/5/16
// =========================================================================

#include <iostream>
#include "geometry/bounding_box.h"
#include "math/constant_vectors.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "geometry/polyline.h"
#include "geometry/polygon.h"
#include "math/rotation.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void bounding_box::allocate_member_objects()
{
}

void bounding_box::initialize_member_objects()
{
   ID = -1;
   xmin=xmax=ymin=ymax=zmin=zmax=0;
   Umin=Umax=Vmin=Vmax=0;
   physical_deltaX=physical_deltaY=-1;
   bbox_color=colorfunc::white;
   wmin=wmax=lmin=lmax=0;
   what=lhat=COM=Zero_vector;
   label="";
}		 

// ---------------------------------------------------------------------
bounding_box::bounding_box()
{
   allocate_member_objects();
   initialize_member_objects();
}

bounding_box::bounding_box(double xmin,double xmax,double ymin,double ymax)
{
   allocate_member_objects();
   initialize_member_objects();

   this->xmin=xmin;
   this->xmax=xmax;
   this->ymin=ymin;
   this->ymax=ymax;
}

bounding_box::bounding_box(double xmin,double xmax,double ymin,double ymax,
                           double zmin,double zmax)
{
   allocate_member_objects();
   initialize_member_objects();

   this->xmin=xmin;
   this->xmax=xmax;
   this->ymin=ymin;
   this->ymax=ymax;
   this->zmin=zmin;
   this->zmax=zmax;
}

bounding_box::bounding_box(const polyline* polyline_ptr)
{
//   cout << "inside bbox constructor taking polyline as arg" << endl;
   allocate_member_objects();
   initialize_member_objects();

   xmin=ymin=zmin=1000.0*POSITIVEINFINITY;
   xmax=ymax=zmax=1000.0*NEGATIVEINFINITY;
   for (unsigned int i=0; i<polyline_ptr->get_n_vertices(); i++)
   {
      threevector curr_vertex(polyline_ptr->get_vertex(i));
      xmin=basic_math::min(xmin,curr_vertex.get(0));
      xmax=basic_math::max(xmax,curr_vertex.get(0));
      ymin=basic_math::min(ymin,curr_vertex.get(1));
      ymax=basic_math::max(ymax,curr_vertex.get(1));
      zmin=basic_math::min(zmin,curr_vertex.get(2));
      zmax=basic_math::max(zmax,curr_vertex.get(2));
   }
}

// ---------------------------------------------------------------------
// Copy constructor:

bounding_box::bounding_box(const bounding_box& b)
{
   docopy(b);
}

bounding_box::~bounding_box()
{
}

// ---------------------------------------------------------------------
void bounding_box::docopy(const bounding_box& b)
{
//   cout << "inside bounding_box::docopy()" << endl;

   ID = b.ID;
   xmin=b.xmin;
   xmax=b.xmax;
   ymin=b.ymin;
   ymax=b.ymax;
   zmin=b.zmin;
   zmax=b.zmax;

   Umin=b.Umin;
   Umax=b.Umax;
   Vmin=b.Vmin;
   Vmax=b.Vmax;

   physical_deltaX=b.physical_deltaX;
   physical_deltaY=b.physical_deltaY;

   wmin=b.wmin;
   wmax=b.wmax;
   lmin=b.lmin;
   lmax=b.lmax;
   
   what=b.what;
   lhat=b.lhat;
   COM=b.COM;

   set_label(b.get_label());
   set_color(b.get_color());

   bounding_box::ATTRIBUTES_MAP::const_iterator iter;
   for(iter = b.get_attributes_map().begin(); 
       iter != b.get_attributes_map().end(); iter++)
   {
      attributes_map[iter->first] = iter->second;
   }
}

// Overload = operator:

bounding_box& bounding_box::operator= (const bounding_box& b)
{
   if (this==&b) return *this;
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream, const bounding_box& b)
{
//    outstream << endl;
   outstream << "Bounding_Box: ID = " << b.get_ID() << endl;
   
   outstream << " xmin = " << b.get_xmin() 
             << " xmax = " << b.get_xmax()
             << " ymin = " << b.get_ymin()
             << " ymax = " << b.get_ymax() 
             << " zmin = " << b.get_zmin()
             << " zmax = " << b.get_zmax() 
             << endl;
   outstream << "Color = " << colorfunc::get_colorstr(b.get_color()) 
             << endl;
   outstream << "label = " << b.get_label() << endl;

   bounding_box::ATTRIBUTES_MAP::const_iterator iter;
   for(iter = b.get_attributes_map().begin(); 
       iter != b.get_attributes_map().end(); iter++)
   {
      outstream << " Attribute key = " << iter->first
                << " Attribute value = " << iter->second
                << endl;

   }

   return outstream;
}

// =========================================================================
// Bounding box properties member functions
// =========================================================================

// Member function recompute_bounding_box() generates a 3D bounding
// box which encloses the input set of threevectors.

void bounding_box::recompute_bounding_box(const std::vector<threevector>& V)
{
//   cout << "inside bounding_box::recompute_bounding_box()" << endl;
   xmin=ymin=zmin=1000.0*POSITIVEINFINITY;
   xmax=ymax=zmax=1000.0*NEGATIVEINFINITY;

   unsigned int n_points=V.size();
   for (unsigned int i=0; i<n_points; i++)
   {
      xmin=basic_math::min(xmin,V[i].get(0));
      xmax=basic_math::max(xmax,V[i].get(0));
      ymin=basic_math::min(ymin,V[i].get(1));
      ymax=basic_math::max(ymax,V[i].get(1));
      zmin=basic_math::min(zmin,V[i].get(2));
      zmax=basic_math::max(zmax,V[i].get(2));
   }
//   cout << *this << endl;
}

// ---------------------------------------------------------------------

double bounding_box::get_area() const
{
//   cout << "inside bounding_box::get_area()" << endl;
//   cout << "xmax = " << xmax << " xmin = " << xmin 
//        << " ymax = " << ymax << " ymin = " << ymin 
//        << endl;
   return (xmax-xmin)*(ymax-ymin);
}

threevector bounding_box::get_midpoint() const
{
   threevector midpoint( 0.5*(xmin+xmax) , 0.5*(ymin+ymax), 0.5*(zmin+zmax) );
   return midpoint;
}

// ---------------------------------------------------------------------
// Member function get_bbox_corners() fills and returns an STL
// vector with the 3D corners of the current bounding box.

vector<threevector> bounding_box::get_bbox_corners() const
{
   vector<threevector> bbox_corners;
   bbox_corners.push_back(threevector(xmin,ymin,zmin));
   bbox_corners.push_back(threevector(xmax,ymin,zmin));
   bbox_corners.push_back(threevector(xmax,ymax,zmin));
   bbox_corners.push_back(threevector(xmin,ymax,zmin));

   bbox_corners.push_back(threevector(xmin,ymin,zmax));
   bbox_corners.push_back(threevector(xmax,ymin,zmax));
   bbox_corners.push_back(threevector(xmax,ymax,zmax));
   bbox_corners.push_back(threevector(xmin,ymax,zmax));

   return bbox_corners;
}

// ---------------------------------------------------------------------
// Member function get_bbox_diagonals() fills and returns an STL
// vector with linesegments which connect opposite corners on the
// bottom and top faces of the current bounding box.

vector<linesegment> bounding_box::get_bbox_diagonals() const
{
   vector<threevector> bbox_corners=get_bbox_corners();

   vector<linesegment> bbox_diagonals;
   bbox_diagonals.push_back(linesegment(bbox_corners[0],bbox_corners[6]));
   bbox_diagonals.push_back(linesegment(bbox_corners[1],bbox_corners[7]));
   bbox_diagonals.push_back(linesegment(bbox_corners[2],bbox_corners[4]));
   bbox_diagonals.push_back(linesegment(bbox_corners[3],bbox_corners[5]));
   
   return bbox_diagonals;
}

// =========================================================================
// Fractional coordinate member functions
// =========================================================================

// Member function frac_XY[UV]_coords takes in point (x,y) [(U,V)] and
// returns its coordinates as fractions ranging from 0 to 1.

void bounding_box::frac_XY_coords(
   double x,double y,double& xfrac,double& yfrac)
{
   xfrac=(x-xmin)/(xmax-xmin);
   yfrac=(y-ymin)/(ymax-ymin);
}

void bounding_box::frac_UV_coords(
   double U,double V,double& Ufrac,double& Vfrac)
{
   Ufrac=(U-Umin)/(Umax-Umin);
   Vfrac=(V-Vmin)/(Vmax-Vmin);
}

// ---------------------------------------------------------------------
void bounding_box::XY_frac_coords(
   double xfrac,double yfrac,double& x,double& y)
{
   x=xmin+xfrac*(xmax-xmin);
   y=ymin+yfrac*(ymax-ymin);
}

void bounding_box::UV_frac_coords(
   double ufrac,double vfrac,double& u,double& v)
{
   u=Umin+ufrac*(Umax-Umin);
   v=Vmin+vfrac*(Vmax-Vmin);
}

// ---------------------------------------------------------------------
// Member function XY_to_UV_coords takes in point (x,y) and returns its
// corresponding (U,V) coordinates.

twovector bounding_box::XY_to_UV_coords(double x,double y)
{
   double xfrac,yfrac;
   frac_XY_coords(x,y,xfrac,yfrac);
   
   double U=Umin+xfrac*(Umax-Umin);
   double V=Vmin+yfrac*(Vmax-Vmin);
   return twovector(U,V);
}

twovector bounding_box::UV_to_XY_coords(double U,double V)
{
   double Ufrac,Vfrac;
   frac_UV_coords(U,V,Ufrac,Vfrac);

   double x=xmin+Ufrac*(xmax-xmin);
   double y=ymin+Vfrac*(ymax-ymin);
   return twovector(x,y);
}

void bounding_box::update_bounds(const bounding_box* bbox_ptr)
{
   xmin=basic_math::min(xmin,bbox_ptr->get_xmin());
   xmax=basic_math::max(xmax,bbox_ptr->get_xmax());
   ymin=basic_math::min(ymin,bbox_ptr->get_ymin());
   ymax=basic_math::max(ymax,bbox_ptr->get_ymax());
   zmin=basic_math::min(zmin,bbox_ptr->get_zmin());
   zmax=basic_math::max(zmax,bbox_ptr->get_zmax());
}

void bounding_box::update_bounds(double x,double y)
{
   xmin=basic_math::min(xmin,x);
   xmax=basic_math::max(xmax,x);
   ymin=basic_math::min(ymin,y);
   ymax=basic_math::max(ymax,y);
}

void bounding_box::update_bounds(const threevector& curr_XYZ)
{
   xmin=basic_math::min(xmin,curr_XYZ.get(0));
   xmax=basic_math::max(xmax,curr_XYZ.get(0));
   ymin=basic_math::min(ymin,curr_XYZ.get(1));
   ymax=basic_math::max(ymax,curr_XYZ.get(1));
   zmin=basic_math::min(zmin,curr_XYZ.get(2));
   zmax=basic_math::max(zmax,curr_XYZ.get(2));
}

void bounding_box::update_bounds(const vector<threevector>& XYZ)
{
   for (unsigned int n=0; n<XYZ.size(); n++)
   {
      update_bounds(XYZ[n]);
   }
}

void bounding_box::reset_bounds(const vector<threevector>& XYZ)
{
   xmin=xmax=XYZ[0].get(0);
   ymin=ymax=XYZ[0].get(1);
   zmin=zmax=XYZ[0].get(2);
   update_bounds(XYZ);
}

// =========================================================================
// Rotated bounding box member functions
// =========================================================================

// Member function dilate() multiplies all bbox coordinates by input
// factor alpha

void bounding_box::dilate(double alpha)
{
   xmin *= alpha;
   xmax *= alpha;
   ymin *= alpha;
   ymax *= alpha;
   zmin *= alpha;
   zmax *= alpha;

   Umin *= alpha;
   Umax *= alpha;
   Vmin *= alpha;
   Vmax *= alpha;
}

// ---------------------------------------------------------------------
// Member function inflate() expands all bbox coordinates relative to
// its center by input factor alpha

void bounding_box::inflate(double alpha)
{
   double width = get_xextent();
   xmin = get_xcenter() - 0.5 * alpha * width;
   xmax = get_xcenter() + 0.5 * alpha * width;

   double height = get_yextent();
   ymin = get_ycenter() - 0.5 * alpha * height;
   ymax = get_ycenter() + 0.5 * alpha * height;
}

// ---------------------------------------------------------------------
// Member function translate() adds a 2D displacement vector to the X
// and Y bounds.

void bounding_box::translate(double dx, double dy)
{
   xmin += dx;
   xmax += dx;
   ymin += dy;
   ymax += dy;
}

// ---------------------------------------------------------------------
// Member function recenter() 

void bounding_box::recenter(double x, double y)
{
   double xextent = xmax - xmin;
   double yextent = ymax - ymin;
   xmin = x - 0.5 * xextent;
   xmax = x + 0.5 * xextent;
   ymin = y - 0.5 * yextent;
   ymax = y + 0.5 * yextent;
}

// ---------------------------------------------------------------------
// Member function inscribed_bbox() takes in a 4 2D corners which
// approximately describe a rectangular bounding box that generally is
// NOT aligned with the X & Y axes.  (For instance, the corners may be
// manually selected.)  This method computes orthogonal direction
// vectors what & lhat which as well as rotated rectangular bbox
// coordinates wmin, wmax, lmin and lmax.  The bounding box is
// guaranteed to fit inside the quadrilateral defined by the input
// corners.  

// We wrote this method in Nov 2010 in order to eliminate noise from
// ALIRT maps that generally are not aligned with X and Y directions
// but which are well-approximated by rotated rectangles.

void bounding_box::inscribed_bbox(const vector<threevector>& corner)
{
   what=(corner[1]-corner[0])+(corner[2]-corner[3]);
   what.put(2,0);
   what=what.unitvector();

   lhat=(corner[3]-corner[0])+(corner[2]-corner[1]);
   lhat.put(2,0);
   lhat=lhat.unitvector();
   
   double theta_init=acos(what.dot(lhat));
   double dtheta=PI/2 - theta_init;
   rotation R(0,0,-0.5*dtheta);
   what = R*what;
   
   rotation Rinv(0,0,0.5*dtheta);
   lhat = Rinv*lhat;
   
   cout << "what = " << what << endl;
   cout << "lhat = " << lhat << endl;
   cout << "what.lhat = " << what.dot(lhat) << endl;
   
   COM=0.25*(corner[0]+corner[1]+corner[2]+corner[3]);
   COM.put(2,0);

   vector<double> w_coeffs,l_coeffs;
   for (unsigned int c=0; c<4; c++)
   {
      threevector curr_rel_corner=corner[c]-COM;
      double curr_w=curr_rel_corner.dot(what);
      double curr_l=curr_rel_corner.dot(lhat);
      w_coeffs.push_back(curr_w);
      l_coeffs.push_back(curr_l);
//      cout << "c = " << c << " w = " << curr_w << " l = " << curr_l << endl;
   } // loop over index c labeling input corners

   std::sort(w_coeffs.begin(),w_coeffs.end());
   std::sort(l_coeffs.begin(),l_coeffs.end());
   
   wmin=w_coeffs[1];
   wmax=w_coeffs[2];
   lmin=l_coeffs[1];
   lmax=l_coeffs[2];
//   cout << "wmin = " << wmin << " wmax = " << wmax << endl;
//   cout << "lmin = " << lmin << " lmax = " << lmax << endl;

   vector<threevector> bbox_corners;
   bbox_corners.push_back(COM+wmin*what+lmin*lhat);
   bbox_corners.push_back(COM+wmax*what+lmin*lhat);
   bbox_corners.push_back(COM+wmax*what+lmax*lhat);
   bbox_corners.push_back(COM+wmax*what+lmin*lhat);

// Explicitly check that bbox corners lie inside of input quadrilateral:

   vector<threevector> projected_corner;
   for (unsigned int c=0; c<4; c++)
   {
      threevector curr_projected_corner(corner[c]);
      curr_projected_corner.put(2,0);
      projected_corner.push_back(curr_projected_corner);
   }
   polygon quad(projected_corner);   
   quad.set_origin(quad.vertex_average());
//   cout << "quad poly = " << quad << endl;

   for (unsigned int c=0; c<4; c++)
   {
      cout << "c = " << c 
           << " bbox corner = " << bbox_corners[c] << endl;

      bool inside_flag=quad.point_inside_polygon(bbox_corners[c]);
      cout << "bbox corner inside quad = " << inside_flag << endl;
   } // loop over index c labeling input corners
}

// ---------------------------------------------------------------------
// Member function XY_inside_WL_bbox() assumes that members
// wmin, wmax, lmin, lmax, what, lhat and COM have all been previously
// calculated via a call to inscribed_bbox().  It tests whether input
// 2D point (x,y) lies the rotated rectangular bounding box defined
// along the what and lhat directions.  If so, this boolean function
// returns true.

bool bounding_box::XY_inside_WL_bbox(double x,double y)
{
   threevector rel_XY=threevector(x,y)-COM;
   double curr_w=rel_XY.dot(what);
   double curr_l=rel_XY.dot(lhat);

   bool WL_inside_flag=true;
   if (curr_w < wmin || curr_w > wmax) WL_inside_flag=false;
   if (curr_l < lmin || curr_l > lmax) WL_inside_flag=false;

   return WL_inside_flag;
}

// =========================================================================
// Intersection & union member functions
// =========================================================================

// Member function linesegment_inside_2D_bbox() converts input
// linesegment l into a very thin polygon.  It then computes the
// intersection of this "sliver" with the bounding box polygon.  If
// the intersection is empty, this boolean method returns false.
// Otherwise, the points p1 and p2 which define the segment's
// intersection with the bounding box are returned.  

bool bounding_box::linesegment_inside_2D_bbox(
   const linesegment& l,threevector& p1,threevector& p2)
{
//   cout << "inside bounding_box::linesegment_inside_bbox()" << endl;

   threevector v1=l.get_v1();
   threevector v2=l.get_v2();
   threevector e_hat=l.get_ehat();
   threevector f_hat=z_hat.cross(e_hat);

   double width=0.0001*l.get_length();
   threevector v3=v2+width*f_hat;
   threevector v4=v1+width*f_hat;
   threevector origin=0.25*(v1+v2+v3+v4);

   vector<threevector> sliver_vertices;
   sliver_vertices.push_back(v1);
   sliver_vertices.push_back(v2);
   sliver_vertices.push_back(v3);
   sliver_vertices.push_back(v4);
   
   polygon sliver(origin,sliver_vertices);
//   cout << "sliver = " << sliver << endl;
   
   polygon bbox_poly(get_xmin(),get_ymin(), get_xmax(), get_ymax());
//   cout << "bbox_poly = " << bbox_poly << endl;

   vector<polygon> bbox_sliver_intersection=
      geometry_func::polygon_intersection(bbox_poly,sliver);

   if (bbox_sliver_intersection.size()==0) return false;

   polygon intersection_poly=bbox_sliver_intersection.front();

   vector<double> distances;
   vector<threevector> point_projections;
   for (unsigned int c=0; c<4; c++)
   {
      threevector closest_point_on_segment;
      distances.push_back(l.point_to_line_segment_distance(
         intersection_poly.get_vertex(c),closest_point_on_segment));
      point_projections.push_back(closest_point_on_segment);
   }
   templatefunc::Quicksort(distances,point_projections);
   
   p1=point_projections[0];
   p2=point_projections[1];
   distances.clear();
   distances.push_back( (p1-l.get_v1()).magnitude() );
   distances.push_back( (p2-l.get_v1()).magnitude() );

   if (distances[0] > distances[1])
   {
      templatefunc::swap(p1,p2);
   }
   return true;
}

// ---------------------------------------------------------------------
// Member function overlap() takes in bounding box bbox.  If bbox
// overlaps any part of *this, this boolean method returns true.

bool bounding_box::overlap(const bounding_box& bbox) const
{
   bool overlap_flag=true;
   if (bbox.get_xmin() > xmax) overlap_flag=false;
   if (bbox.get_xmax() < xmin) overlap_flag=false;
   if (bbox.get_ymin() > ymax) overlap_flag=false;
   if (bbox.get_ymax() < ymin) overlap_flag=false;

   return overlap_flag;
}

// ---------------------------------------------------------------------
// Member function bbox_intersection() takes in bounding box bbox.  If
// bbox does not overlap any part of *this, this boolean method
// returns false.  Otherwise, it computes the intersection between
// input bbox and *this and returns the result within
// intersection_bbox.

bool bounding_box::bbox_intersection(
   const bounding_box& bbox,bounding_box& intersection_bbox) const
{
   if (!overlap(bbox)) return false;

   double X_min=basic_math::max(xmin,bbox.get_xmin());
   double X_max=basic_math::min(xmax,bbox.get_xmax());
   double Y_min=basic_math::max(ymin,bbox.get_ymin());
   double Y_max=basic_math::min(ymax,bbox.get_ymax());
   intersection_bbox=bounding_box(X_min,X_max,Y_min,Y_max);

   return true;
}

// ---------------------------------------------------------------------
// Member function bbox_union() takes in bounding box bbox.  It computes
// the union between the input bbox and *this and returns the result
// within union_bbox.

void bounding_box::bbox_union(
   const bounding_box& bbox,bounding_box& union_bbox) const
{
   double X_min=basic_math::min(xmin,bbox.get_xmin());
   double X_max=basic_math::max(xmax,bbox.get_xmax());
   double Y_min=basic_math::min(ymin,bbox.get_ymin());
   double Y_max=basic_math::max(ymax,bbox.get_ymax());
   union_bbox=bounding_box(X_min,X_max,Y_min,Y_max);
}

// ---------------------------------------------------------------------
// Member function intersection_over_union() takes in bounding box
// bbox.  It computes the intersection and union between input bbox
// and *this.  The ratio of the intersection to union bboxes' areas is
// returned.

double bounding_box::intersection_over_union(const bounding_box &bbox) const
{
   bounding_box intersection_bbox, union_bbox;
   if(!bbox_intersection(bbox, intersection_bbox)) return 0;
   bbox_union(bbox, union_bbox);
   
   double intersection_area = intersection_bbox.get_xextent() * 
      intersection_bbox.get_yextent();
   double union_area = union_bbox.get_xextent() * 
      union_bbox.get_yextent();
   return intersection_area / union_area;
}

// ---------------------------------------------------------------------
// Member function encloses() takes in bounding box bbox.  If *this
// contains bbox, this boolean method returns true.

bool bounding_box::encloses(const bounding_box& bbox) const
{
   return(bbox.get_xmin() >= xmin &&
          bbox.get_xmax() <= xmax &&
          bbox.get_ymin() >= ymin &&
          bbox.get_ymax() <= ymax);
}

// ---------------------------------------------------------------------
// Boolean member function nearly_equal() computes the intersection
// and union of an input bounding box with *this.  It returns true if
// the area ratio of the intersection to union bboxes exceeds
// an empirically determined (and reasonable) threshold value.

bool bounding_box::nearly_equal(const bounding_box& bbox2) const
{
   bounding_box intersection_bbox,union_bbox;
   if (!bbox_intersection(bbox2,intersection_bbox))
   {
      return false;
   }

   bbox_union(bbox2,union_bbox);
   double bbox_overlap = intersection_bbox.get_area()/
      union_bbox.get_area();

   const double bbox_overlap_threshold = 0.35;
   return (bbox_overlap > bbox_overlap_threshold);
}

