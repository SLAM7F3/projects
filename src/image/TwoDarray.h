// Note: Rename locate_extremal_xy_pixels() method as something like
// polygon_bbox_pixels().  Then search through all classes and make
// sure that we have not needlessly repeated this method's code in
// lots of locations!

// ==========================================================================
// Header file for templatized twoDarray class 
// ==========================================================================
// Last modified on 9/30/09; 11/14/11; 3/28/14; 4/2/14; 4/5/14
// ==========================================================================

#ifndef T_TWODARRAY_H
#define T_TWODARRAY_H

#include "math/Genarray.h"
#include "math/threevector.h"
class polygon;
class rotation;

template <class A>
class TwoDarray:public Genarray<A>
{

  public:

// Initialization, constructor and destructor functions:

   TwoDarray(int m,int n);
   TwoDarray(TwoDarray<A> const *zTwoDarray_ptr);
   TwoDarray(int m,int n,TwoDarray<A> const *ztwoDarray_ptr,
             bool recenter_flag=false);
   TwoDarray(TwoDarray<A> const& zTwoDarray);
   virtual ~TwoDarray();
   TwoDarray<A>& operator= (const TwoDarray<A>& m);

   template <class B>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const TwoDarray<B>& T);

// Set and get member functions:

   void set_xdim(unsigned int x);
   void set_ydim(unsigned int y);
   void set_mdim_tmp(int m);
   void set_ndim_tmp(int n);
   void set_xlo_orig(double xloorig);
   void set_xhi_orig(double xhiorig);
   void set_xlo(double x_lo);
   void set_xhi(double x_hi);
   void set_deltax(double dx);
   void set_ylo_orig(double y_lo);
   void set_yhi_orig(double y_hi);
   void set_ylo(double y_lo);
   void set_yhi(double y_hi);
   void set_deltay(double dy);
   
   unsigned int get_xdim() const;
   unsigned int get_ydim() const;
   unsigned int get_mdim_tmp() const;
   unsigned int get_ndim_tmp() const;
   double get_xlo_orig() const;
   double get_xhi_orig() const;
   double get_xlo() const;
   double get_xhi() const;
   double get_deltax() const;
   double get_ylo_orig() const;
   double get_yhi_orig() const;
   double get_ylo() const;
   double get_yhi() const;
   double get_deltay() const;
   
   void init_coord_system(
      double min_x,double max_x,double min_y,double max_y);
   void sym_init_coord_system(
      double min_x,double max_x,double min_y,double max_y);
   void init_coord_system(double max_x,double max_y);
   void init_coord_system();
   threevector center_point() const;
   void flip_upside_down();

   void keep_pnt_inside_working_region(
      unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py) const;
   bool px_inside_working_region(unsigned int px) const;
   bool py_inside_working_region(unsigned int py) const;
   bool pixel_inside_working_region(unsigned int px,unsigned int py) const;
   bool x_to_px(double x,unsigned int& px) const;
   bool y_to_py(double y,unsigned int& py) const;
   bool point_to_pixel(
      double x,double y,unsigned int& px,unsigned int& py) const;
   bool point_to_pixel(const threevector& currpoint,
                       unsigned int& px,unsigned int& py) const;
   A fast_XY_to_Z(double x,double y) const;
   A fast_XY_to_Z(double x,double y,unsigned int& px,unsigned int& py) const;

   bool point_inside_working_region(double x,double y) const;
   bool point_inside_working_region(const threevector& currpoint) const;
   bool point_to_interpolated_value(
      double x,double y,double& interpolated_value) const;
   bool px_to_x(int px,double& x) const;
   bool py_to_y(int py,double& y) const;
   double fast_px_to_x(int px) const;
   double fast_py_to_y(int py) const;
   bool pixel_to_point(int px,int py,double& x,double& y) const;
   bool pixel_to_point(int px,int py,threevector& currpoint) const;
   void pixel_to_threevector(int px,int py,threevector& currpoint) const;
   void fast_pixel_to_XYZ(int px,int py,double& X,double& Y,double& Z) const;

   bool bbox_corners_to_pixels(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py) const;
   void locate_extremal_xy_pixels(
      const polygon& poly,unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py) const;
   void locate_extremal_xy_pixels(
      bool poly_intersection,bool poly_union,int npolys,
      const polygon p_2D[],unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py) const;

// Polygon related methods:

   void pixel_relative_to_poly(
      int px,int py,polygon& poly,
      bool& inside_poly,bool& outside_poly,bool& on_perimeter) const;

// Note: currpoint argument should be removed from following argument
// list...

   void pixel_relative_to_poly(
      int px,int py,threevector& currpoint,polygon& poly,
      bool& inside_poly,bool& outside_poly,bool& on_perimeter) const;
   void pixel_inside_n_polys(
      int px,int py,threevector& currpoint,int npolys,polygon p_2D[],
      bool& pixel_inside_all_polys,
      bool& pixel_inside_at_least_one_poly) const;
   bool pixel_inside_at_least_one_of_n_polys(
      unsigned int px,unsigned int py,threevector& currpoint,int npolys,
      polygon p_2D[]) const;
   void convert_pixel_to_posn_polygon_vertices(polygon* poly_ptr) const;

// Copying methods:

   void copy(TwoDarray<A>* zTwoDarray_copy_ptr) const;
   void copy(int nx,int ny,TwoDarray<A>* zTwoDarray_copy_ptr) const;
   void copy_entries(TwoDarray<A>* zTwoDarray_newcopy_ptr,
                     bool move_image_to_lower_left_corner=true) const;
   template <class B> void copy_metric_data(
      TwoDarray<B>* zTwoDarray_copy_ptr) const;
   
// Moving around and manipulating pixel values:

   void translate(const threevector& rvec,double znull=0);
   void translate(int delta_px,int delta_py,double znull=0);
   void rotate(const rotation& R,double znull=0);
   void rotate(double theta,double znull=0);
   void rotate(const threevector& rotation_origin,
               double theta,double znull=0);
   void rotate(const threevector& rotation_origin,const rotation& R,
               double znull=0);


   TwoDarray<A>* expand_pixel_extents(
      int m_new,int n_new,double znull=0,
      bool recenter_flag=false) const;
   TwoDarray<A>* expand_for_rotation(double znull=0,bool recenter_flag=false)
      const;
   void rotate_about_center(double theta,TwoDarray<A>* zrot_twoDarray_ptr,
                            double znull=0,bool recenter_flag=false) const;
   void expanded_pixel_coords(
      unsigned int px,unsigned int py,
      unsigned int& px_new,unsigned int& py_new,
      TwoDarray<A> const *zexpand_twoDarray_ptr,
      bool move_image_to_center=true,
      bool move_image_to_lower_left_corner=false) const;
   void rotated_pixel_coords(
      unsigned int px,unsigned int py,double theta,
      unsigned int& px_rot,unsigned int& py_rot,
      TwoDarray<A> const *zexpand_twoDarray_ptr) const;

// Friend functions:

   template <class B>
   friend TwoDarray<B> operator+ (
      const TwoDarray<B>& X,const TwoDarray<B>& Y);
   template <class B>
   friend TwoDarray<B> operator- (
      const TwoDarray<B>& X,const TwoDarray<B>& Y);
   template <class B>
   friend TwoDarray<B> operator- (const TwoDarray<B>& X);
   template <class B>
   friend TwoDarray<B> operator* (A a,const TwoDarray<B>& X);
   template <class B>
   friend TwoDarray<B> operator* (const TwoDarray<B>& X,A a);

// In June 2005, we learned that as of version 3.4 of GCC, unqualified
// names in a template definition will no longer find members of a
// dependent base.  According to
// http://gcc.gnu.org/gcc-3.4/changes.html, it is strongly recommended
// that such names be made dependent by prefixing them with "this->".
// But to avoid lots of dumb this->get statments, we simply invoke the
// following using statement:

//   using TwoDarray<A>::get;

// We found in Aug 2007 that GCC 4.1.2 no longer permits the preceding
// hack.  So we explicitly write this->get() within TwoDarray.cc
// methods...

  private: 

// Integers xdim and ydim represent the "physical" or "absolute true"
// dimensions of the TwoDarray object:

   unsigned int xdim,ydim;	

// Integers mdim and ndim represent the "relevant working" dimensions
// of the TwoDarray object.  In general mdim <= xdim and ndim <=ydim.
// But for image display purposes, we sometimes set mdim > ydim and
// ndim > ydim !  In this case, the image display methods will stuff
// various output files with zeros to make up for the differences
// mdim-xdim and ndim-ydim.

// As of 6/28/02, we believe it will be very helpful to incorporate
// these additional member variables in order to build in extra
// flexibility when working with our TwoDarray objects.  For example,
// images from imagecdf files generally have all different sizes.  Yet
// it is inefficient and expensive to dynamically allocate TEMPORARY
// TwoDarray objects inside loops over all the images within a pass.
// So instead, we will simply allocate a single TwoDarray object once
// outside the loop whose xdim and ydim values are large enough to
// accomodate all of the individual images.  But when we use the
// temporary TwoDarray object inside the loop, we can adjust mdim and
// ndim so as to take into account individual image size differences.

// For a few image display tasks (such as cropping and equating of
// horizontal and vertical ranges), it is necessary to store
// temporary, altered values for the numbers of horizontal and
// vertical pixels as well as the horizontal spatial bounds into the
// following member variables:

   unsigned int mdim_tmp,ndim_tmp;
   double xlo_orig,xhi_orig;
   double ylo_orig,yhi_orig;

// TwoDarray objects may or may not be endowed with a natural metric.
// For those which are, we store pixel extremal locations as well as
// pixel bin sizes (measured in meters):

   double xlo,xhi,deltax;
   double ylo,yhi,deltay;

   void docopy(const TwoDarray<A>& m);
};

typedef TwoDarray<double> twoDarray;

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

template <class A> inline void TwoDarray<A>::set_xdim(unsigned int x)
{
   xdim=x;
}

template <class A> inline void TwoDarray<A>::set_ydim(unsigned int y)
{
   ydim=y;
}

template <class A> inline void TwoDarray<A>::set_mdim_tmp(int m)
{
   mdim_tmp=m;
}

template <class A> inline void TwoDarray<A>::set_ndim_tmp(int n)
{
   ndim_tmp=n;
}

template <class A> inline void TwoDarray<A>::set_xlo_orig(double xloorig)
{
   xlo_orig=xloorig;
}

template <class A> inline void TwoDarray<A>::set_xhi_orig(double xhiorig)
{
   xhi_orig=xhiorig;
}

template <class A> inline void TwoDarray<A>::set_xlo(double x_lo)
{
   xlo=x_lo;
}

template <class A> inline void TwoDarray<A>::set_xhi(double x_hi)
{
   xhi=x_hi;
}

template <class A> inline void TwoDarray<A>::set_deltax(double dx)
{
   deltax=dx;
}

template <class A> inline void TwoDarray<A>::set_ylo_orig(double y_lo_orig)
{
   ylo_orig=y_lo_orig;
}

template <class A> inline void TwoDarray<A>::set_yhi_orig(double y_hi_orig)
{
   yhi_orig=y_hi_orig;
}

template <class A> inline void TwoDarray<A>::set_ylo(double y_lo)
{
   ylo=y_lo;
}

template <class A> inline void TwoDarray<A>::set_yhi(double y_hi)
{
   yhi=y_hi;
}

template <class A> inline void TwoDarray<A>::set_deltay(double dy)
{
   deltay=dy;
}

template <class A> inline unsigned int TwoDarray<A>::get_xdim() const
{
   return xdim;
}

template <class A> inline unsigned int TwoDarray<A>::get_ydim() const
{
   return ydim;
}

template <class A> inline unsigned int TwoDarray<A>::get_mdim_tmp() const
{
   return mdim_tmp;
}

template <class A> inline unsigned int TwoDarray<A>::get_ndim_tmp() const
{
   return ndim_tmp;
}

template <class A> inline double TwoDarray<A>::get_xlo_orig() const
{
   return xlo_orig;
}

template <class A> inline double TwoDarray<A>::get_xhi_orig() const
{
   return xhi_orig;
}

template <class A> inline double TwoDarray<A>::get_xlo() const
{
   return xlo;
}

template <class A> inline double TwoDarray<A>::get_xhi() const
{
   return xhi;
}

template <class A> inline double TwoDarray<A>::get_deltax() const
{
   return deltax;
}

template <class A> inline double TwoDarray<A>::get_ylo_orig() const
{
   return ylo_orig;
}

template <class A> inline double TwoDarray<A>::get_yhi_orig() const
{
   return yhi_orig;
}

template <class A> inline double TwoDarray<A>::get_ylo() const
{
   return ylo;
}

template <class A> inline double TwoDarray<A>::get_yhi() const
{
   return yhi;
}

template <class A> inline double TwoDarray<A>::get_deltay() const
{
   return deltay;
}

template <class A> inline threevector TwoDarray<A>::center_point() const
{
   return threevector(0.5*(xlo+xhi),0.5*(ylo+yhi));
}

#include "TwoDarray.cc" 

#endif  // datastructures/TwoDarray.h




