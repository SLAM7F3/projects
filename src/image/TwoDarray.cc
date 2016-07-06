// ==========================================================================
// TwoDarray class member function definitions
// ==========================================================================
// Last modified on 11/14/11; 1/28/12; 1/23/14; 3/28/14; 4/5/14
// =========================================================================

#include <algorithm>

#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/polygon.h"
#include "math/rotation.h"

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

template <class A> TwoDarray<A>::TwoDarray(int m,int n):
   Genarray<A>(m,n)
{
   if (m <= 0 || n <= 0)
   {
      std::cout << "Error in TwoDarray constructor #1" << std::endl;
      std::cout << "m = " << m << " n = " << n << std::endl;
   }

   xdim=m;
   ydim=n;
   mdim_tmp=this->mdim;
   ndim_tmp=this->ndim;
   xlo=xlo_orig=xhi=xhi_orig=deltax=0;
   ylo=ylo_orig=yhi=yhi_orig=deltay=0;
}

// This constructor takes in a pointer to an already existing
// twoDarray object and builds a new object with the same pixel and
// (x,y) dimensions:

template <class A> TwoDarray<A>::TwoDarray(
   TwoDarray<A> const *ztwoDarray_ptr):
   Genarray<A>(ztwoDarray_ptr->xdim,ztwoDarray_ptr->ydim)
{
   docopy(*ztwoDarray_ptr);
   this->mdim=ztwoDarray_ptr->mdim;
   this->ndim=ztwoDarray_ptr->ndim;
}

// This next constructor takes in a pointer to an already existing
// twoDarray object.  It builds a new object with possibly larger
// pixel (m,n) and (x,y) dimensions.  If input boolean flag
// recenter_flag==true, the new object's center lies at (x,y)=(0,0).
// Otherwise, the new object's (xlo,ylo) and (deltax,deltay) values
// equal those of the input twoDarray.

template <class A> TwoDarray<A>::TwoDarray(
  int m,int n,TwoDarray<A> const *ztwoDarray_ptr,bool recenter_flag):
   Genarray<A>(m,n)
{
   xdim=m;
   ydim=n;
   mdim_tmp=this->mdim;
   ndim_tmp=this->ndim;
   deltax=ztwoDarray_ptr->get_deltax();
   deltay=ztwoDarray_ptr->get_deltay();
   if (recenter_flag)
   {
      init_coord_system();
   }
   else
   {
      xlo=ztwoDarray_ptr->get_xlo();
      ylo=ztwoDarray_ptr->get_ylo();
      xhi=xlo+(this->mdim-1)*deltax;
      yhi=ylo+(this->ndim-1)*deltay;
   }
   xlo_orig=xlo;
   ylo_orig=ylo;
   xhi_orig=xhi;
   yhi_orig=yhi;
}

// Copy constructor:

template <class A> TwoDarray<A>::TwoDarray(TwoDarray<A> const &ztwoDarray):
   Genarray<A>(ztwoDarray)
{
   docopy(ztwoDarray);
}

template <class A> TwoDarray<A>::~TwoDarray()
{
}

// ---------------------------------------------------------------------
template <class A> void TwoDarray<A>::docopy(const TwoDarray<A> & m)
{
   xdim=m.xdim;
   ydim=m.ydim;
   mdim_tmp=m.mdim_tmp;
   ndim_tmp=m.ndim_tmp;
   xlo=m.xlo;
   xlo_orig=m.xlo_orig;
   xhi=m.xhi;
   xhi_orig=m.xhi_orig;
   deltax=m.deltax;
   ylo=m.ylo;
   ylo_orig=m.ylo_orig;
   yhi=m.yhi;
   yhi_orig=m.yhi_orig;
   deltay=m.deltay;
}

// Overload = operator:

template <class A> TwoDarray<A>& TwoDarray<A>::operator= (
   const TwoDarray<A>& m)
{
   if (this==&m) return *this;
   Genarray<A>::operator=(m);
   docopy(m);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class B> std::ostream& operator<< (
   std::ostream& outstream,const TwoDarray<B>& T)
{
   outputfunc::newline();
   outstream << "mdim = " << T.mdim << " ndim = " << T.ndim << std::endl;
   outstream << "xlo = " << T.xlo 
             << " xlo_orig = " << T.xlo_orig 
             << " xhi = " << T.xhi 
             << " xhi_orig = " << T.xhi_orig
             << " deltax = " << T.deltax << std::endl;
   outstream << "ylo = " << T.ylo 
             << " ylo_orig = " << T.ylo_orig 
             << " yhi = " << T.yhi 
             << " yhi_orig = " << T.yhi_orig
             << " deltay = " << T.deltay << std::endl;
   outstream << "xdim = " << T.xdim << " ydim = " << T.ydim << std::endl;
   outstream << "mdim_tmp = " << T.mdim_tmp 
             << " ndim_tmp = " << T.ndim_tmp << std::endl;

/*
   for (int py=0; py<T.ndim; py++)
   {
      for (int px=0; px<T.mdim; px++)
      {
         outstream << "px = " << px << " py = " << py
                   << " T.get(px,py) = " << T.get(px,py) << std::endl;
      }
      outstream << std::endl;
   }
*/
 
//   outstream << (Genarray<B>&)T << std::endl;
   return outstream;
}

// ---------------------------------------------------------------------
// This first version of member function init_coord_system calculates
// the pixel size deltax and deltay if they have not already been set.
// This version should be called for images which do NOT come from
// imagecdf files.

template <class A> void TwoDarray<A>::init_coord_system(
   double max_x,double max_y)
{
   deltax=2.2*max_x/this->mdim;			// m
   deltay=2.2*max_y/this->ndim;			// m
   init_coord_system();
}

template <class A> void TwoDarray<A>::init_coord_system(
   double min_x,double max_x,double min_y,double max_y)
{
   deltax=(max_x-min_x)/this->mdim;			// m
   deltay=(max_y-min_y)/this->ndim;			// m
   xlo=min_x;
   xhi=xlo+(this->mdim-1)*deltax;
   ylo=min_y;
   yhi=ylo+(this->ndim-1)*deltay;
}

template <class A> void TwoDarray<A>::sym_init_coord_system(
   double min_x,double max_x,double min_y,double max_y)
{
   deltax=(max_x-min_x)/this->mdim;			// m
   deltay=(max_y-min_y)/this->ndim;			// m
   xlo=min_x+0.5*deltax;
   xhi=xlo+(this->mdim-1)*deltax;
   ylo=min_y+0.5*deltay;
   yhi=ylo+(this->ndim-1)*deltay;
}

// This second version of member function init_coord_system assumes
// that the pixel size deltax and deltay have already been set.  It
// then computes the extremal pixel location values xlo,xhi,ylo,yhi
// assuming that the origin is located at the center of the ztwoDarray
// objects:

template <class A> void TwoDarray<A>::init_coord_system()
{
   xlo=-(this->mdim-1)/2.0*deltax;	// m 
   xhi=(this->mdim-1)/2.0*deltax;	// m 
   ylo=-(this->ndim-1)/2.0*deltay;	// m 
   yhi=(this->ndim-1)/2.0*deltay;	// m 

//   std::cout << "Inside TwoDarray<A>::init_coord_system()" << std::endl;
//   std::cout << "deltax = " << deltax << " deltay = " << deltay << std::endl;
//   std::cout << "mdim = " << mdim << std::endl;
//   std::cout << "ndim = " << ndim << std::endl;
//   std::cout << "xlo = " << xlo << " xhi = " << xhi 
//        << " deltax = " << deltax << std::endl;
//   std::cout << "ylo = " << ylo << " yhi = " << yhi 
//        << " deltay = " << deltay << std::endl;
//   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function flip_upside_down

template <class A> void TwoDarray<A>::flip_upside_down()
{
   TwoDarray<A> ztmp_twoDarray(this);

   for (unsigned int i=0; i<xdim; i++)
   {
      for (unsigned int j=0; j<ydim; j++)
      {
         ztmp_twoDarray.put(i,j,this->get(i,ydim-1-j));
      }
   }
   for (unsigned int i=0; i<xdim; i++)
   {
      for (unsigned int j=0; j<ydim; j++)
      {
         this->put(i,j,ztmp_twoDarray.get(i,j));
      }
   }
}

// ---------------------------------------------------------------------
// Member function keep_pnt_inside_working_region ensures that
// extremal points lie within the current twoDarray object's "working
// region": 0 <= m <= mdim, 0 <= n <= ndim.

template <class A> void TwoDarray<A>::keep_pnt_inside_working_region(
   unsigned int& min_px,unsigned int& min_py,
   unsigned int& max_px,unsigned int& max_py) const
{
   min_px=basic_math::max(min_px,(unsigned int) 0);
   min_py=basic_math::max(min_py,(unsigned int) 0);
   max_px=basic_math::min(max_px,this->mdim);
   max_py=basic_math::min(max_py,this->ndim);
}

// ---------------------------------------------------------------------
// The next two boolean member functions return true if the current
// pixel lies inside the working image:

template <class A> inline bool TwoDarray<A>::px_inside_working_region(
   unsigned int px) const
{
   return (px < this->mdim);
//   return ((px >= 0) && (px < this->mdim));
}

template <class A> inline bool TwoDarray<A>::py_inside_working_region(
   unsigned int py) const
{
   return (py < this->ndim);
//   return ((py >= 0) && (py < this->ndim));
}

// Explicitly expanded next method for speed purposes:

template <class A> inline bool TwoDarray<A>::pixel_inside_working_region(
   unsigned int px,unsigned int py) const
{
//   return (px_inside_working_region(px) && py_inside_working_region(py));
   return ((px >= 0) && (px < this->mdim) && (py >= 0) && (py < this->ndim));
}

// ---------------------------------------------------------------------
// Given x and y values of a point, member functions x_to_px, y_to_py
// and point_to_pixel calculate the px and py pixel values
// corresponding to this point.  These boolean methods return true if
// the pair (px,py) lies inside the image.

template <class A> bool TwoDarray<A>::x_to_px(double x,unsigned int& px) const
{
   if (this->mdim > mdim_tmp)
   {
      px=basic_math::round((x-xlo)/deltax+0.5*(this->mdim-mdim_tmp));
   }
   else
   {
      px=basic_math::round((x-xlo)/deltax);
   }
   return px_inside_working_region(px);
}

template <class A> inline bool TwoDarray<A>::y_to_py(
   double y,unsigned int& py) const
{
   py=basic_math::round((yhi-y)/deltay);
   return py_inside_working_region(py);
}

template <class A> inline bool TwoDarray<A>::point_to_pixel(
   double x,double y,unsigned int& px,unsigned int& py) const
{
   bool xinside=x_to_px(x,px);
   bool yinside=y_to_py(y,py);
   return (xinside && yinside);
}

template <class A> inline bool TwoDarray<A>::point_to_pixel(
   const threevector& currpoint,unsigned int& px,unsigned int& py) const
{
   return point_to_pixel(currpoint.get(0),currpoint.get(1),px,py);
}

// Member function fast_XY_to_Z() takes in x and y values which are
// assumed to lie inside the current TwoDarray.  It returns the Z
// value corresponding to the input XY coordinates.  In order to
// minimize execution time, this method does not perform any bounds
// checking.

template <class A> A TwoDarray<A>::fast_XY_to_Z(double x,double y) const
{
//   std::cout << "inside TwoDarray::fast_XY_to_Z()" << std::endl;
//   std::cout << "x = " << x << " xlo = " << xlo << std::endl;
//   std::cout << "y = " << y << " yhi = " << yhi << std::endl;
   int px=basic_math::round((x-xlo)/deltax);   
   int py=basic_math::round((yhi-y)/deltay);
//   std::cout << "px = " << px << " py = " << py << std::endl;
//   std::cout << "xdim = " << get_xdim() 
//             << " ydim = " << get_ydim() << std::endl;
   
   return this->get(px,py);
}

template <class A> A TwoDarray<A>::fast_XY_to_Z(
   double x,double y,unsigned int& px,unsigned int& py) const
{
//   std::cout << "inside TwoDarray::fast_XY_to_Z()" << std::endl;
//   std::cout << "x = " << x << " xlo = " << xlo << std::endl;
//   std::cout << "y = " << y << " yhi = " << yhi << std::endl;
   px=basic_math::round((x-xlo)/deltax);   
   py=basic_math::round((yhi-y)/deltay);
//   std::cout << "px = " << px << " py = " << py << std::endl;
//   std::cout << "xdim = " << get_xdim() 
//             << " ydim = " << get_ydim() << std::endl;
   return this->get(px,py);
}

template <class A> inline bool TwoDarray<A>::point_inside_working_region(
   double x,double y) const
{
   unsigned int px,py;
   return point_to_pixel(x,y,px,py);
}

template <class A> inline bool TwoDarray<A>::point_inside_working_region(
   const threevector& currpoint) const
{
   unsigned int px,py;
   return point_to_pixel(currpoint.get(0),currpoint.get(1),px,py);
}

// ---------------------------------------------------------------------
// Member function point_to_interpolated_value first computes indices
// i and j such that the input x and y values are bracketed by
// [xi,xi+deltax] and [yj,yj+deltay].  It next bilinearly interpolates
// in the x and y directions to determine the z value at the point
// (x,y).  If (x,y) does not lie within the current image, this
// boolean method returns false.

// This method is useful for regridding one image before it is
// composited with another image whose size is different.

template <class A> bool TwoDarray<A>::point_to_interpolated_value(
   double x,double y,double& interpolated_value) const
{
//   std::cout << "inside TwoDarray::point_to_interpolated_value()" << std::endl;

// Recall y=ylo <--> py=ndim-1, y=yhi <--> py=0.  Vertical y axis in
// images is flipped so that first row is written out on top of image
// while last row is written out on bottom!

   unsigned int i=basic_math::mytruncate((x-xlo)/deltax);
   unsigned int j=basic_math::mytruncate((yhi-y)/deltay);
   double xi=xlo+i*deltax;
   double yj=yhi-j*deltay;
//   double yj=yhi+j*(-deltay);

   bool point_inside_image=true;
//   double term1,term2,term3,term4;
//   term1=term2=term3=term4=0;

   double z=this->get(i,j);
   double z_right=this->get(i+1,j);
   double z_top=this->get(i,j+1);
   
   if (i>=0 && i<this->mdim-1 && j>=0 && j<this->ndim-1)
   {
//      term1=z;
//      term2=(x-xi)/deltax*(z_right-z);
 //     term3=(y-yj)/(-deltay)*(z_top-z);
//      term4=(x-xi)*(y-yj)/(deltax*(-deltay))
//         *(this->get(i+1,j+1)+z-z_right-z_top);

      interpolated_value=z 
         + (x-xi)/deltax*(z_right-z) 
         - (y-yj)/deltay*(z_top-z) 
         - (x-xi)*(y-yj)/(deltax*deltay)*(this->get(i+1,j+1)+z-z_right-z_top);
   }
   else if (i==this->mdim-1 && j>=0 && j<this->ndim-1)
   {
//      term1=z;
//      term2=0;
//      term3=(y-yj)/(-deltay)*(z_top-z);
//      term4=0;
      interpolated_value=z+(y-yj)/(-deltay)*(z_top-z);
   }
   else if (i>=0 && i<this->mdim-1 && j==this->ndim-1)
   {
//      term1=z;
//      term2=(x-xi)/deltax*(z_right-z);
//      term3=0;
//      term4=0;
      interpolated_value=z+(x-xi)/deltax*(z_right-z);
   }
   else if (i==this->mdim-1 && j==this->ndim-1)
   {
//      term1=z;
//      term2=0;
//      term3=0;
//      term4=0;
      interpolated_value=z;
   }
   else
   {
      point_inside_image=false;
   }
//   interpolated_value=term1+term2+term3+term4;

//   std::cout << "i = " << i << " j = " << j << std::endl;
//   std::cout << "xi = " << xi << " x = " << x << " xi+1 = " << xi+deltax
//        << std::endl;
//   std::cout << "yj = " << yj << " y = " << y << " yj+1 = " << yj+deltay
//        << std::endl;
//   std::cout << "z[i][j] = " << z << std::endl;
//   std::cout << "z[i+1][j] = " << z_right << std::endl;
//   std::cout << "z[i][j+1] = " << z_top << std::endl;
//   std::cout << "z[i+1][j+1] = " << this->get(i+1,j+1) << std::endl;
//   std::cout << "term1 = " << term1 << " term2 = " << term2 << " term3 = " 
//        << term3 << " term4 = " << term4 << std::endl;
//   std::cout << "interp value = " << interpolated_value << std::endl;
//   std::cout << std::endl;
   
   return point_inside_image;
}

/*
template <class A> bool TwoDarray<A>::point_to_interpolated_value(
   double x,double y,double& interpolated_value) const
{
//   std::cout << "inside TwoDarray::point_to_interpolated_value()" << std::endl;

// Recall y=ylo <--> py=ndim-1, y=yhi <--> py=0.  Vertical y axis in
// images is flipped so that first row is written out on top of image
// while last row is written out on bottom!

   int i=basic_math::mytruncate((x-xlo)/deltax);
   int j=basic_math::mytruncate((yhi-y)/deltay);
   double xi=xlo+i*deltax;
   double yj=yhi+j*(-deltay);

   bool point_inside_image=true;
   double term1,term2,term3,term4;
   term1=term2=term3=term4=0;
   if (i>=0 && i<this->mdim-1 && j>=0 && j<this->ndim-1)
   {
      term1=this->get(i,j);
      term2=(x-xi)/deltax*(this->get(i+1,j)-this->get(i,j));
      term3=(y-yj)/(-deltay)*(this->get(i,j+1)-this->get(i,j));
      term4=(x-xi)*(y-yj)/(deltax*(-deltay))
         *(this->get(i+1,j+1)+this->get(i,j)-this->get(i+1,j)-
           this->get(i,j+1));
   }
   else if (i==this->mdim-1 && j>=0 && j<this->ndim-1)
   {
      term1=this->get(i,j);
      term2=0;
      term3=(y-yj)/(-deltay)*(this->get(i,j+1)-this->get(i,j));
      term4=0;
   }
   else if (i>=0 && i<this->mdim-1 && j==this->ndim-1)
   {
      term1=this->get(i,j);
      term2=(x-xi)/deltax*(this->get(i+1,j)-this->get(i,j));
      term3=0;
      term4=0;
   }
   else if (i==this->mdim-1 && j==this->ndim-1)
   {
      term1=this->get(i,j);
      term2=0;
      term3=0;
      term4=0;
   }
   else
   {
      point_inside_image=false;
   }
   interpolated_value=term1+term2+term3+term4;

//   std::cout << "i = " << i << " j = " << j << std::endl;
//   std::cout << "xi = " << xi << " x = " << x << " xi+1 = " << xi+deltax
//        << std::endl;
//   std::cout << "yj = " << yj << " y = " << y << " yj+1 = " << yj+deltay
//        << std::endl;
//   std::cout << "z[i][j] = " << this->get(i,j) << std::endl;
//   std::cout << "z[i+1][j] = " << this->get(i+1,j) << std::endl;
//   std::cout << "z[i][j+1] = " << this->get(i,j+1) << std::endl;
//   std::cout << "z[i+1][j+1] = " << this->get(i+1,j+1) << std::endl;
//   std::cout << "term1 = " << term1 << " term2 = " << term2 << " term3 = " 
//        << term3 << " term4 = " << term4 << std::endl;
//   std::cout << "interp value = " << interpolated_value << std::endl;
//   std::cout << std::endl;
   
   return point_inside_image;
}
*/

// ---------------------------------------------------------------------
// Given a pixel's horizontal location px (vertical location py)
// within an image, member function px_to_x (py_to_y) returns the
// corresponding x (y) value.  If x (y) is well-defined, this boolean
// method returns true.

template <class A> bool TwoDarray<A>::px_to_x(int px,double& x) const
{
   x=xlo+px*deltax;
//   if (!isfinite(x))
//   {
//      std::cout << "Error inside TwoDarray<A>::px_to_x()!" << std::endl;
//      std::cout << "x = " << x << " px = " << px << std::endl;
//      std::cout << "xlo = " << xlo << " xhi = " << xhi << " deltax = " 
//           << deltax << std::endl;
//      return false;
//   }
//   else
//   {
      return true;
//   }
}

template <class A> bool TwoDarray<A>::py_to_y(int py,double& y) const
{
   y=yhi-py*deltay;
//   if (!isfinite(y))
//   {
//      std::cout << "Error inside TwoDarray<A>::py_to_y()!" << std::endl;
//      std::cout << " y = " << y << " py = " << py << std::endl;
//      std::cout << "ylo = " << ylo << " yhi = " << yhi << " deltay = " 
//           << deltay << std::endl;
//      return false;
//   }
//   else
//   {
      return true;
//   }
}

// Member functions fast_px_to_x and fast_py_to_y do not perform any
// in-bounds or finite value checking.  So they should run faster than
// their safer counterparts above...

template <class A> inline double TwoDarray<A>::fast_px_to_x(int px) const
{
  return xlo+px*deltax;
}

template <class A> inline double TwoDarray<A>::fast_py_to_y(int py) const
{
  return yhi-py*deltay;
}

template <class A> inline bool TwoDarray<A>::pixel_to_point(
   int px,int py,double& x,double& y) const
{
   return (px_to_x(px,x) && py_to_y(py,y));
}

// Note added on Fri Nov 26, 2010....
// currpoint should really equal threevector(x,y,get(px,py)) !!!

template <class A> bool TwoDarray<A>::pixel_to_point(
   int px,int py,threevector& currpoint) const
{
   double x,y;
   
   bool xOK=px_to_x(px,x);
   bool yOK=py_to_y(py,y);
   currpoint=threevector(x,y);
   return (xOK && yOK);
}

// Member function pixel_to_threevector() is intended to be a fast
// method for retrieving the (x,y,z) vector corresponding to (px,py)

template <class A> void TwoDarray<A>::pixel_to_threevector(
   int px,int py,threevector& currpoint) const
{
   double x=fast_px_to_x(px);
   double y=fast_py_to_y(py);
   currpoint=threevector(x,y,this->get(px,py));
}

template <class A> void TwoDarray<A>::fast_pixel_to_XYZ(
   int px,int py,double& X,double& Y,double& Z) const
{
   X=fast_px_to_x(px);
   Y=fast_py_to_y(py);
   Z=this->get(px,py);
}

// ---------------------------------------------------------------------
// Member function bbox_corners_to_pixels takes in min/max x and y
// values of a rectangular bounding box.  If the bounding box lies
// outside the range xlo <= x <= xhi & ylo <= y <= yhi, this boolean
// member function returns false.  Otherwise, it returns the
// corresponding min/max px and py pixel values associated with these
// corners.  If the bounding box does not lie entirely within the
// current image, the pixel corner values are cropped so that they do
// not stray beyond their extremal limits.

template <class A> bool TwoDarray<A>::bbox_corners_to_pixels(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py) const
{
   bool nondegenerate_bbox=true;
   if (minimum_x > maximum_x)
   {
      nondegenerate_bbox=false;
      std::cout << "Error inside TwoDarray<A>::bbox_corners_to_pixels()" 
                << std::endl;
      std::cout << "minimum_x = " << minimum_x 
           << " maximum_x = " << maximum_x << std::endl;
   }
   if (minimum_y > maximum_y)
   {
      nondegenerate_bbox=false;
      std::cout << "Error inside TwoDarray<A>::bbox_corners_to_pixels()" 
                << std::endl;
      std::cout << "minimum_y = " << minimum_y 
           << " maximum_y = " << maximum_y << std::endl;
   }
   if (minimum_x > xhi || maximum_x < xlo ||
       minimum_y > yhi || maximum_y < ylo)
   {
      nondegenerate_bbox=false;
   }

   if (nondegenerate_bbox)
   {
      point_to_pixel(minimum_x,maximum_y,min_px,min_py);
      point_to_pixel(maximum_x,minimum_y,max_px,max_py);
      keep_pnt_inside_working_region(min_px,min_py,max_px,max_py);
   }
   return nondegenerate_bbox;
}

// ---------------------------------------------------------------------
// Member function locate_extremal_xy_pixels returns the
// minimum/maximum x and y pixel locations of a bounding box which
// encloses an input polygon.  Recall that min_y <---> max_py and
// max_y <---> min_py:

template <class A> void TwoDarray<A>::locate_extremal_xy_pixels(
   const polygon& poly,unsigned int& min_px,unsigned int& min_py,
   unsigned int& max_px,unsigned int& max_py) const
{
   double min_x,min_y,max_x,max_y;

   poly.locate_extremal_xy_points(min_x,min_y,max_x,max_y);
   point_to_pixel(min_x,max_y,min_px,min_py);
   point_to_pixel(max_x,min_y,max_px,max_py);

// In some rare instances (such as maps where West longitude increases
// from right to left), min_px can actually be larger than max_px.
// This can perhaps also happen for min_py and max_py.  We therefore
// add the following lines just to check for this rare situation:

   if (min_px > max_px) templatefunc::swap(min_px,max_px);
   if (min_py > max_py) templatefunc::swap(min_py,max_py);
   keep_pnt_inside_working_region(min_px,min_py,max_px,max_py);
}

// ---------------------------------------------------------------------
// This overloaded version of locate_extremal_xy_pixels returns the
// minimum/maximum x and y pixel locations of a bounding box which
// encloses either the intersection or union of the npolys polygons
// within input array p_2D.  The input polygons are assumed to lie
// within the xy image plane.

template <class A> void TwoDarray<A>::locate_extremal_xy_pixels(
   bool poly_intersection,bool poly_union,int npolys,
   const polygon p_2D[],
   unsigned int& min_px,unsigned int& min_py,
   unsigned int& max_px,unsigned int& max_py) const
{
   unsigned int curr_min_px,curr_min_py,curr_max_px,curr_max_py;

   if (poly_intersection)
   {
      min_px=min_py=NEGATIVEINFINITY;
      max_px=max_py=POSITIVEINFINITY;
   }
   else if (poly_union)
   {
      min_px=min_py=POSITIVEINFINITY;
      max_px=max_py=NEGATIVEINFINITY;
   }

   for (int n=0; n<npolys; n++)
   {
      locate_extremal_xy_pixels(
         p_2D[n],curr_min_px,curr_min_py,curr_max_px,curr_max_py);

      if (poly_intersection)
      {
         min_px=basic_math::max(min_px,curr_min_px);
         min_py=basic_math::max(min_py,curr_min_py);
         max_px=basic_math::min(max_px,curr_max_px);
         max_py=basic_math::min(max_py,curr_max_py);
      }
      else if (poly_union)
      {
         min_px=basic_math::min(min_px,curr_min_px);
         min_py=basic_math::min(min_py,curr_min_py);
         max_px=basic_math::max(max_px,curr_max_px);
         max_py=basic_math::max(max_py,curr_max_py);
      }
   }
   keep_pnt_inside_working_region(min_px,min_py,max_px,max_py);
}

// ==========================================================================
// Polygon related methods
// ==========================================================================

// Member function pixel_relative_to_poly determines whether a pixel
// (px,py) lies inside, outside or on polygon poly.  We take these
// three boolean options to be mutually exclusive.  So precisely one
// of these is set equal to true, while the other two are returned as
// false by this subroutine:

template <class A> void TwoDarray<A>::pixel_relative_to_poly(
   int px,int py,polygon& poly,
   bool& inside_poly,bool& outside_poly,bool& on_perimeter) const
{
   threevector currpoint;
   pixel_relative_to_poly(
      px,py,currpoint,poly,inside_poly,outside_poly,on_perimeter);
}

template <class A> void TwoDarray<A>::pixel_relative_to_poly(
   int px,int py,threevector& currpoint,polygon& poly,
   bool& inside_poly,bool& outside_poly,bool& on_perimeter) const
{
   pixel_to_point(px,py,currpoint);
//   inside_poly=poly.point_within_polygon(currpoint);
   inside_poly=poly.point_inside_polygon(currpoint);

   double mindist_to_side=poly.point_dist_to_polygon(currpoint);
   double ds_sqrd=sqr(deltax)+sqr(deltay);
   double ratio=sqr(mindist_to_side)/ds_sqrd;
   if (ratio <= 1)
   {
      inside_poly=outside_poly=false;
      on_perimeter=true;
   }
   else if (ratio > 1 && inside_poly)
   {
      inside_poly=true;
      outside_poly=on_perimeter=false;
   }
   else if (ratio > 1 && !inside_poly)
   {
      outside_poly=true;
      inside_poly=on_perimeter=false;
   }
}

// ---------------------------------------------------------------------
// Member function pixel_inside_n_polys takes in a pixel location
// (px,py) which is assumed to lie within the current image along with
// an array p_2D of polygons which are assumed to lie within the image
// plane.  It then scans through all the polygons and determines
// whether the pixel location lies inside all of the polygons and/or
// inside at least one of the polygons.

template <class A> void TwoDarray<A>::pixel_inside_n_polys(
   int px,int py,threevector& currpoint,int npolys,polygon p_2D[],
   bool& pixel_inside_all_polys,bool& pixel_inside_at_least_one_poly) const
{
   bool pixel_inside_poly,pixel_outside_poly,pixel_on_perimeter;

   pixel_inside_all_polys=true;
   pixel_inside_at_least_one_poly=false;
   for (int n=0; n<npolys; n++)
   {
      pixel_relative_to_poly(
         px,py,currpoint,p_2D[n],pixel_inside_poly,pixel_outside_poly,
         pixel_on_perimeter);
      pixel_inside_all_polys=(pixel_inside_all_polys && pixel_inside_poly);
      pixel_inside_at_least_one_poly=
         (pixel_inside_at_least_one_poly || pixel_inside_poly);
   }
}

// ---------------------------------------------------------------------
// Boolean member function pixel_inside_at_least_one_of_n_polys takes
// in a pixel location (px,py) which is assumed to lie within the
// current image along with an array p_2D of polygons which are
// assumed to lie within the image plane.  It then scans through all
// the polygons and determines whether the pixel location lies inside
// at least one of the polygons.  It returns true as soon as it finds
// the pixel lying in some polygon.

template <class A> bool TwoDarray<A>::pixel_inside_at_least_one_of_n_polys(
   unsigned int px,unsigned int py,
   threevector& currpoint,int npolys,polygon p_2D[]) const
{
   bool pixel_inside_at_least_one_poly=false;
   bool pixel_inside_poly,pixel_outside_poly,pixel_on_perimeter;
   int n=0;
   unsigned int min_px,max_px,min_py,max_py;
   
   while (n<npolys && !pixel_inside_at_least_one_poly)
   {

// First place bounding box around current polygon.  If current pixel
// does not lie within bounding box, don't waste time calling the
// relatively expensive function pixel_relative_to_poly:

      locate_extremal_xy_pixels(p_2D[n],min_px,min_py,max_px,max_py);

      if (px >= min_px && px <= max_px && py >= min_py && py <= max_py)
      {
         pixel_relative_to_poly(
            px,py,currpoint,p_2D[n],pixel_inside_poly,pixel_outside_poly,
            pixel_on_perimeter);
         pixel_inside_at_least_one_poly=
            (pixel_inside_at_least_one_poly || pixel_inside_poly);
      }
      n++;
   }
   return pixel_inside_at_least_one_poly;
}

// ---------------------------------------------------------------------
// Member function convert_pixel_to_posn_polygon_vertices takes in a
// polygon whose vertices are assumed to hold PIXEL COORDINATE
// information (e.g. from output of convexhull::convex_hull_poly()).
// This method transforms the polygon's vertices from pixel to
// position space coordinates.

template <class A> void TwoDarray<A>::convert_pixel_to_posn_polygon_vertices(
   polygon* poly_ptr) const
{
   if (poly_ptr != NULL)
   {
      threevector origin=poly_ptr->get_origin();
      threevector transformed_origin;
      pixel_to_point(
         basic_math::round(origin.get(0)),
         basic_math::round(origin.get(1)),
         transformed_origin);
      poly_ptr->set_origin(transformed_origin);

      threevector curr_posn;
      for (unsigned int n=0; n<poly_ptr->get_nvertices(); n++)
      {
         threevector curr_vertex=poly_ptr->get_vertex(n);
         pixel_to_point(basic_math::round(curr_vertex.get(0)),
                        basic_math::round(curr_vertex.get(1)),curr_posn);
         poly_ptr->set_vertex(n,curr_posn);
      }
//      poly_ptr->locate_origin();
   }
}

// ==========================================================================
// TwoDarray copying methods
// ==========================================================================

// Member function copy copies the entire contents of the current
// twoDarray object into *ztwoDarray_copy_ptr:

template <class A> void TwoDarray<A>::copy(
   TwoDarray<A>* ztwoDarray_copy_ptr) const
{
   if (xdim > ztwoDarray_copy_ptr->xdim || ydim > ztwoDarray_copy_ptr->ydim)
   {
      std::cout << "Error in TwoDarray<A>::copy()!" << std::endl;
      std::cout << "xdim = " << xdim << " ydim = " << ydim << std::endl;
      std::cout << "mdim = " << this->mdim << " ndim = " << this->ndim 
                << std::endl;
      std::cout << "ztwoDarray_copy_ptr->xdim = " 
           << ztwoDarray_copy_ptr->xdim << std::endl;
      std::cout << "ztwoDarray_copy_ptr->ydim = " 
           << ztwoDarray_copy_ptr->ydim << std::endl;
      std::cout << "ztwoDarray_copy_ptr->mdim = " 
           << ztwoDarray_copy_ptr->mdim << std::endl;
      std::cout << "ztwoDarray_copy_ptr->ndim = " 
           << ztwoDarray_copy_ptr->ndim << std::endl;
      exit(-1);
   }
   else
   {
      copy(this->mdim,this->ndim,ztwoDarray_copy_ptr);
   }
}

// In this overloaded version of member function copy, the x and y
// ranges of the array to be copied are passed as input arguments:

template <class A> void TwoDarray<A>::copy(
   int nx,int ny,TwoDarray<A>* ztwoDarray_copy_ptr) const
{
   copy_metric_data(ztwoDarray_copy_ptr);
   memcpy(ztwoDarray_copy_ptr->e,this->e,nx*ny*sizeof(this->e[0]));
}

template <class A> void TwoDarray<A>::copy_entries(
   TwoDarray<A>* ztwoDarray_newcopy_ptr,
   bool move_image_to_lower_left_corner) const
{
   int py_offset;
   if (move_image_to_lower_left_corner)
   {
      py_offset=ztwoDarray_newcopy_ptr->get_ndim()-this->ndim;
   }
   else
   {
      py_offset=0;
   }

   for (unsigned int i=0; i<this->mdim; i++)
   {
      for (unsigned int j=0; j<this->ndim; j++)
      {
         ztwoDarray_newcopy_ptr->put(i,j+py_offset,this->get(i,j));
      } // loop over j index
   } // loop over i index
}

template <class A> template <class B> void TwoDarray<A>::copy_metric_data(
   TwoDarray<B>* ztwoDarray_copy_ptr) const
{
//   std::cout << "inside TwoDarray<A>::copy_metric_data" << std::endl;
//   std::cout << "xdim = " << xdim << " ydim = " << ydim << std::endl;
//   std::cout << "mdim = " << this->mdim << " ndim = " << this->ndim << std::endl;
//   std::cout << "xlo = " << xlo << " xhi = " << xhi << std::endl;
//   std::cout << "xlo_orig = " << xlo_orig << " xhi_orig = " << xhi_orig << std::endl;
//   std::cout << "ylo = " << ylo << " yhi = " << yhi << std::endl;
//   std::cout << "deltax = " << deltax << " deltay = " << deltay << std::endl;
   
   ztwoDarray_copy_ptr->set_mdim(this->mdim);
   ztwoDarray_copy_ptr->set_ndim(this->ndim);
   ztwoDarray_copy_ptr->set_mdim_tmp(mdim_tmp);
   ztwoDarray_copy_ptr->set_ndim_tmp(ndim_tmp);
   ztwoDarray_copy_ptr->set_xlo(xlo);
   ztwoDarray_copy_ptr->set_xlo_orig(xlo_orig);
   ztwoDarray_copy_ptr->set_xhi(xhi);
   ztwoDarray_copy_ptr->set_xhi_orig(xhi_orig);
   ztwoDarray_copy_ptr->set_deltax(deltax);
   ztwoDarray_copy_ptr->set_ylo(ylo);
   ztwoDarray_copy_ptr->set_yhi(yhi);
   ztwoDarray_copy_ptr->set_deltay(deltay);
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and
// not within class member function definition file.  Friendly
// functions should not be declared as member functions of a class.
// So their definition syntax is not

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list)
// ==========================================================================

// Overload + operator:

template <class B> TwoDarray<B> operator+ (
   const TwoDarray<B>& X,const TwoDarray<B>& Y)
{
   if (X.xdim==Y.xdim && X.ydim==Y.ydim)
   {
      TwoDarray<B> Z(X.xdim,X.ydim);

      for (unsigned int i=0; i<X.xdim; i++)
      {
         for (unsigned int j=0; j<X.ydim; j++)
         {
            Z.put(i,j,X.get(i,j)+Y.get(i,j));
         }
      }
      return Z;
   }
   else
   {
      std::cout << "Error inside operator+ in twoDarray.cc class!" 
                << std::endl;
      std::cout << "X.xdim = " << X.xdim << " X.ydim = " << X.ydim 
                << std::endl;
      std::cout << "Y.xdim = " << Y.xdim << " Y.ydim = " << Y.ydim 
                << std::endl;
      std::cout << "Cannot add together two twoDarrays of different dimensions!"
           << std::endl;
      exit(-1);
   }
}

// Overload - operator:

template <class B> TwoDarray<B> operator- (
   const TwoDarray<B>& X,const TwoDarray<B>& Y)
{
   if (X.xdim==Y.xdim && X.ydim==Y.ydim)
   {
      TwoDarray<B> Z(X.xdim,X.ydim);

      for (unsigned int i=0; i<X.xdim; i++)
      {
         for (unsigned int j=0; j<X.ydim; j++)
         {
            Z.put(i,j,X.get(i,j)-Y.get(i,j));
         }
      }
      return Z;
   }
   else
   {
      std::cout << "Error inside operator- in twoDarray.cc class!" << std::endl;
      std::cout << "X.xdim = " << X.xdim << " X.ydim = " << X.ydim << std::endl;
      std::cout << "Y.xdim = " << Y.xdim << " Y.ydim = " << Y.ydim << std::endl;
      std::cout << "Cannot subtract two twoDarrays of different dimensions!"
           << std::endl;
      exit(-1);
   }
}

// Overload - operator:

template <class B> TwoDarray<B> operator- (const TwoDarray<B>& X)
{
   TwoDarray<B> Y(X.xdim,X.ydim);
	 
   for (unsigned int i=0; i<X.xdim; i++)
   {
      for (unsigned int j=0; j<X.ydim; j++)
      {
         Y.put(i,j,-X.get(i,j));
      }
   }
   return Y;
}

// Overload * operator for multiplying a twoDarray by a scalar

template <class A,class B> TwoDarray<B> operator* (A a,const TwoDarray<B>& X)
{
   TwoDarray<B> Y(X.xdim,X.ydim);

   for (unsigned int i=0; i<X.xdim; i++)
   {
      for (unsigned int j=0; j<X.ydim; j++)
      {
         Y.put(i,j,a*X.get(i,j));
      }
   }
   return Y;
}

template <class A,class B> TwoDarray<B> operator* (const TwoDarray<B>& X,A a)
{
   return a*X;
}

// ==========================================================================
// Moving around and manipulating pixel values:
// ==========================================================================

// Member function translate moves all the pixels within the current
// twoDarray object by the specified displacement vector rvec:

template <class A> void TwoDarray<A>::translate(
   const threevector& rvec,double znull)
{
   TwoDarray<A>* ztransformed_twoDarray_ptr=new TwoDarray(this);
   ztransformed_twoDarray_ptr->initialize_values(znull);

   threevector currpoint;
   for (unsigned int px=0; px<this->mdim; px++)
   {
      for (unsigned int py=0; py<this->ndim; py++)
      {
         pixel_to_point(px,py,currpoint);
         threevector translated_point(currpoint+rvec);

         unsigned int px_trans,py_trans;
         if (point_to_pixel(
            translated_point.get(0),translated_point.get(1),
            px_trans,py_trans))
         {
            ztransformed_twoDarray_ptr->put(
               px_trans,py_trans,this->get(px,py));
         }
      } // py loop
   } // px loop
   ztransformed_twoDarray_ptr->copy(this);
   delete ztransformed_twoDarray_ptr;
}

// This overloaded version of member function translate moves all the
// pixels within twoDarray *ztwoDarray_ptr by input parameters
// delta_px and delta_py:

template <class A> void TwoDarray<A>::translate(
   int delta_px,int delta_py,double znull)
{
   TwoDarray<A>* ztransformed_twoDarray_ptr=new TwoDarray(this);
   ztransformed_twoDarray_ptr->initialize_values(znull);

   for (unsigned int px=0; px<this->mdim; px++)
   {
      for (unsigned int py=0; py<this->ndim; py++)
      {
         int px_new=px+delta_px;
         int py_new=py+delta_py;
         if (px_new >= 0 && px_new < int(this->mdim) && 
             py_new >= 0 && py_new < int(this->ndim) )
         {
            ztransformed_twoDarray_ptr->put(px_new,py_new,this->get(px,py));
         }
      } // py loop
   } // px loop
   ztransformed_twoDarray_ptr->copy(this);
   delete ztransformed_twoDarray_ptr;
}

// ---------------------------------------------------------------------
template <class A> void TwoDarray<A>::rotate(const rotation& R,double znull)
{
   rotate(Zero_vector,R,znull);
}

// ---------------------------------------------------------------------
template <class A> void TwoDarray<A>::rotate(double theta,double znull)
{
   rotation R(0,0,theta);
   rotate(Zero_vector,R,znull);
}

// ---------------------------------------------------------------------
template <class A> void TwoDarray<A>::rotate(
   const threevector& rotation_origin,double theta,double znull)
{
   rotation R(0,0,theta);
   rotate(rotation_origin,R,znull);
}

// ---------------------------------------------------------------------
template <class A> void TwoDarray<A>::rotate(
   const threevector& rotation_origin,const rotation& R,double znull)
{
   unsigned int px_orig,py_orig;
   threevector orig_point,rotated_point,dv;
   rotation Rinv(R.transpose());
   TwoDarray<A>* ztransformed_twoDarray_ptr=new TwoDarray(this);
   ztransformed_twoDarray_ptr->initialize_values(znull);
   
// Map each pixel in rotated array onto some corresponding pixel in
// input array:

   for (unsigned int px_rot=0; px_rot<this->mdim; px_rot++)
   {
      for (unsigned int py_rot=0; py_rot<this->ndim; py_rot++)
      {
         pixel_to_point(px_rot,py_rot,rotated_point);
         dv=Rinv*(rotated_point-rotation_origin);
         orig_point=rotation_origin+dv;
         if (point_to_pixel(orig_point.get(0),orig_point.get(1),
                            px_orig,py_orig))
         {
            ztransformed_twoDarray_ptr->put(
               px_rot,py_rot,this->get(px_orig,py_orig));
         }
      } // py_rot loop
   } // px_rot loop

   ztransformed_twoDarray_ptr->copy(this);
   delete ztransformed_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method expand_pixel_extents takes in pixel sizes m_new and n_new of
// an enlarged TwoDarray output.  It then instantiates a new
// TwoDarray, nulls its entries, and copies the current TwoDarray into
// the new object's lower left corner.

template <class A> TwoDarray<A>* TwoDarray<A>::expand_pixel_extents(
   int m_new,int n_new,double znull,bool recenter_flag) const
{
   TwoDarray<A>* zexpand_twoDarray_ptr=new TwoDarray(
      m_new,m_new,this,recenter_flag);
   zexpand_twoDarray_ptr->initialize_values(znull);
   copy_entries(zexpand_twoDarray_ptr);
   int delta_m=(zexpand_twoDarray_ptr->get_mdim()-this->mdim)/2;
   int delta_n=-(zexpand_twoDarray_ptr->get_ndim()-this->ndim)/2;
   zexpand_twoDarray_ptr->translate(delta_m,delta_n,znull);
   return zexpand_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Method expand_for_rotation returns a new, enlarged TwoDarray which
// can be rotated about its center without any loss of information.

template <class A> TwoDarray<A>* TwoDarray<A>::expand_for_rotation(
   double znull,bool recenter_flag) const
{
   double x_extent=xhi-xlo;
   double y_extent=yhi-ylo;
   double new_extent=sqrt(sqr(x_extent)+sqr(y_extent));
   int m_new=basic_math::round(new_extent/deltax)+1;
   int n_new=basic_math::round(new_extent/deltay)+1;
   return expand_pixel_extents(m_new,n_new,znull,recenter_flag);
}

// ---------------------------------------------------------------------
// Method rotate_about_center is assumed to be called on an enlarged
// TwoDarray.  It computes the TwoDarray's center pixel location and
// returns a new TwoDarray in which the image is rotated about the
// center by angle theta (in radians).

template <class A> void TwoDarray<A>::rotate_about_center(
   double theta,TwoDarray<A>* zrot_twoDarray_ptr,double znull,
   bool recenter_flag) const
{
   double x_extent=xhi-xlo;
   double y_extent=yhi-ylo;
   copy(zrot_twoDarray_ptr);
   threevector rotation_origin(Zero_vector);
   if (!recenter_flag)
   {
      rotation_origin=threevector(0.5*x_extent,0.5*y_extent,0);
   }
   zrot_twoDarray_ptr->rotate(rotation_origin,theta,znull);
}

// ---------------------------------------------------------------------
// Member function expanded_pixel_coords takes in the pixel
// coordinates of some point in the current TwoDarray object as well
// as a TwoDarray *zexpand_twoDarray_ptr whose horizontal and vertical
// extents are assumed to be enlarged.  It returns the pixel
// coordinates for the corresponding point within the enlarged
// TwoDarray.

template <class A> void TwoDarray<A>::expanded_pixel_coords(
   unsigned int px,unsigned int py,unsigned int& px_new,unsigned int& py_new,
   TwoDarray<A> const *zexpand_twoDarray_ptr,
   bool move_image_to_center,bool move_image_to_lower_left_corner) const
{
   int px_offset,py_offset;
   if (move_image_to_center)
   {
      px_offset=(zexpand_twoDarray_ptr->get_mdim()-this->mdim)/2;
      py_offset=(zexpand_twoDarray_ptr->get_ndim()-this->ndim)/2;
   }
   else if (move_image_to_lower_left_corner)
   {
      px_offset=0;
      py_offset=zexpand_twoDarray_ptr->get_ndim()-this->ndim;
   }
   else
   {
      px_offset=py_offset=0;
   }
   px_new=px+px_offset;
   py_new=py+py_offset;
}

// ---------------------------------------------------------------------
// Method rotated_pixel_coords takes in pixel coords (px,py) of a
// point within the current twoDarray object.  It also takes in
// TwoDarray *zexpand_twoDarray_ptr whose pixel extents are assumed to
// be enlarged compared to the current object, and the current
// twoDarray information is assumed to be centered within this new,
// enlarged array.  Given a theta angle in radians, this method
// returns the pixel location within the expanded TwoDarray of the
// input point.

template <class A> void TwoDarray<A>::rotated_pixel_coords(
   unsigned int px,unsigned int py,double theta,
   unsigned int& px_rot,unsigned int& py_rot,
   TwoDarray<A> const *zexpand_twoDarray_ptr) const
{
   unsigned int px_trans,py_trans;
   expanded_pixel_coords(px,py,px_trans,py_trans,zexpand_twoDarray_ptr,
                         true,false);
   threevector r_expand;
   zexpand_twoDarray_ptr->pixel_to_point(px_trans,py_trans,r_expand);
   double x_extent=zexpand_twoDarray_ptr->get_xhi()-
      zexpand_twoDarray_ptr->get_xlo();
   double y_extent=zexpand_twoDarray_ptr->get_yhi()-
      zexpand_twoDarray_ptr->get_ylo();
   threevector expanded_array_center(0.5*x_extent,0.5*y_extent,0);   
   threevector r_expand_rel_to_center(r_expand-expanded_array_center);

   double cos_theta=cos(theta);
   double sin_theta=sin(theta);
   double x_rot=cos_theta*r_expand_rel_to_center.get(0)-
      sin_theta*r_expand_rel_to_center.get(1);
   double y_rot=sin_theta*r_expand_rel_to_center.get(0)+
      cos_theta*r_expand_rel_to_center.get(1);
   threevector r_rot(threevector(x_rot,y_rot,0)+expanded_array_center);
   zexpand_twoDarray_ptr->point_to_pixel(r_rot,px_rot,py_rot);
}



