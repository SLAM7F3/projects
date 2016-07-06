// ==========================================================================
// COMPOSITEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 12/4/10; 8/5/12; 8/8/12; 4/5/14
// ==========================================================================

#include <vector>
#include "image/compositefuncs.h"
#include "math/mathfuncs.h"
#include "image/myimage.h"
#include "math/Tensor.h"
#include "image/TwoDarray.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ostream;
using std::ofstream;
using std::vector;

namespace compositefunc
{

// ==========================================================================
// Subsampling and compositing methods
// ==========================================================================

// Member function regrid_twoDarray takes in intensity values within
// input twoDarray *ztwoDarray_ptr and maps their
// average[minimal]{median}(maximal) values onto the output twoDarray
// *znew_twoDarray_ptr if input percentile_sentinel=-1[0]{1}(2).  The
// new lo and hi values of x and y are also specified as input
// parameters. The new delta_x and delta_y step sizes are returned as
// member variables of the regridded twoDarray *znew_twoDarray_ptr.

   void regrid_twoDarray(
      double new_xhi,double new_xlo,double new_yhi,double new_ylo,
      twoDarray const *ztwoDarray_ptr,twoDarray* znew_twoDarray_ptr,
      int percentile_sentinel)
      {
//         cout << "inside compositefunc::regrid_twoDarray()" << endl;
//         cout << "znew_twoDarray_ptr->get_mdim() = " 
//              << znew_twoDarray_ptr->get_mdim() 
//              << " znew_twoDarray_ptr->get_ndim() = " 
//              << znew_twoDarray_ptr->get_ndim() << endl;
//         cout << "new_xhi = " << new_xhi << " new_xlo = " << new_xlo << endl;
//         cout << "new_yhi = " << new_yhi << " new_ylo = " << new_ylo << endl;
//         cout << "ztwoDarray_ptr->get_mdim() = " 
//              << ztwoDarray_ptr->get_mdim()
//              << " ztwoDarray_ptr->get_ndim() = " 
//              << ztwoDarray_ptr->get_ndim() << endl;
         
         znew_twoDarray_ptr->set_xhi(new_xhi);
         znew_twoDarray_ptr->set_xlo(new_xlo);
         znew_twoDarray_ptr->set_yhi(new_yhi);
         znew_twoDarray_ptr->set_ylo(new_ylo);

         if (znew_twoDarray_ptr->get_mdim() <= 1)
         {
            znew_twoDarray_ptr->set_deltax(0);
         }
         else
         {
            znew_twoDarray_ptr->set_deltax((new_xhi-new_xlo)/
            (znew_twoDarray_ptr->get_mdim()-1));
         }
         if (znew_twoDarray_ptr->get_ndim() <= 1)
         {
            znew_twoDarray_ptr->set_deltay(0);
         }
         else
         {
            znew_twoDarray_ptr->set_deltay((new_yhi-new_ylo)/
            (znew_twoDarray_ptr->get_ndim()-1));
         }
         
         znew_twoDarray_ptr->clear_values();
//         cout << "*znew_twoDarray_ptr = " << *znew_twoDarray_ptr << endl;
         
         for (unsigned int j=0; j<znew_twoDarray_ptr->get_ndim(); j++)
         {
            double y=new_yhi-j*znew_twoDarray_ptr->get_deltay();
            for (unsigned int i=0; i<znew_twoDarray_ptr->get_mdim(); i++)
            {
               double x=new_xlo+i*znew_twoDarray_ptr->get_deltax();

               if (percentile_sentinel==1)
               {
                  znew_twoDarray_ptr->put(i,j,percentile_pixel_value(
                     x,y,znew_twoDarray_ptr->get_deltax(),
                     znew_twoDarray_ptr->get_deltay(),ztwoDarray_ptr,
                     percentile_sentinel));
               }
               else if (percentile_sentinel==0 || percentile_sentinel==2)
               {
                  int m=basic_math::round(
                     znew_twoDarray_ptr->get_deltax()/
                     ztwoDarray_ptr->get_deltax());
                  int n=basic_math::round(
                     znew_twoDarray_ptr->get_deltay()/
                     ztwoDarray_ptr->get_deltay());
//                  cout << "m = " << m << " n = " << n << endl;
                  
                  int px=m/2+i*m;
                  int py=n/2+j*n;
                  znew_twoDarray_ptr->put(
                     i,j,extremal_pixel_value(
                        px,py,m,n,ztwoDarray_ptr,percentile_sentinel));
               }
               else
               {
                  znew_twoDarray_ptr->put(i,j,average_pixels(
                     x,y,znew_twoDarray_ptr->get_deltax(),
                     znew_twoDarray_ptr->get_deltay(),ztwoDarray_ptr));
               }
               
//               cout << "Inside regrid, i = " << i << " j = " << j
//                    << " x = " << x << " y = " << y << " znew = " 
//                    << znew_twoDarray_ptr->get(i,j) << endl;
            } // i loop
         } // j loop
         
//         cout << "at end of compositefunc::regrid_twoDarray()" << endl;
      }

// ---------------------------------------------------------------------
// Method average_pixels takes in an (x,y) location within the
// intensity twoDarray *ztwoDarray_ptr as well as bounding box x and y
// extents.  This method evaluates the intensities within an mxn
// lattice centered on (x,y).  It then integrates the intensities and
// divides by the lattice's area in order to compute the average
// intensity within the bounding box.

   double average_pixels(
      double x,double y,double x_extent,double y_extent,
      twoDarray const *ztwoDarray_ptr)
      {
//         cout << "inside compositefunc::average_pixels()" << endl;
//         cout << "x = " << x << " y = " << y << " x_extent = " << x_extent
//              << " y_extent = " << y_extent << endl;

         int m=basic_math::mytruncate(
            x_extent/ztwoDarray_ptr->get_deltax())+1;
         if (is_even(m)) m++;
         m=basic_math::max(3,m);

         int n=basic_math::mytruncate(
            y_extent/ztwoDarray_ptr->get_deltay())+1;
         if (is_even(n)) n++;
         n=basic_math::max(3,n);

         double xlow=basic_math::max(ztwoDarray_ptr->get_xlo(),x-x_extent/2);
         double xhigh=basic_math::min(ztwoDarray_ptr->get_xhi(),x+x_extent/2);
         double dx=(xhigh-xlow)/(m-1);
   
         double ylow=basic_math::max(ztwoDarray_ptr->get_ylo(),y-y_extent/2);
         double yhigh=basic_math::min(ztwoDarray_ptr->get_yhi(),y+y_extent/2);
         double dy=(yhigh-ylow)/(n-1);

         double dA=dx*dy;
         double finteg=0;
         double area=0;

//         cout << "x = " << x << " y = " << y << endl;
//         cout << "x_extent = " << x_extent << " y_extent = " << y_extent 
//              << endl;
//         cout << "ztwoDarray_ptr->get_mdim() = " << ztwoDarray_ptr->get_mdim() 
//              << " ztwoDarray_ptr->get_ndim() = " 
//              << ztwoDarray_ptr->get_ndim() << endl;
//         cout << "ztwoDarray_ptr->xlo = " << ztwoDarray_ptr->get_xlo() 
//              << " ztwoDarray_ptr->xhi = " << ztwoDarray_ptr->get_xhi() 
//              << endl;
//         cout << "ztwoDarray_ptr->deltax = " << ztwoDarray_ptr->get_deltax()
//              << " ztwoDarray_ptr->deltay = " << ztwoDarray_ptr->get_deltay()
//              << endl;
//         cout << "xlow = " << xlow << " xhigh = " << xhigh << endl;
//         cout << "ylow = " << ylow << " yhigh = " << yhigh << endl;
//         outputfunc::newline();

         double zinterp;
         for (int j=0; j<n; j++)
         {
            y=ylow+j*dy;
            for (int i=0; i<m; i++)
            {
               x=xlow+i*dx;
//               cout << "x = " << x << " y = " << y << endl;
               if (ztwoDarray_ptr->point_to_interpolated_value(x,y,zinterp))
               {
                  area += dA;
                  finteg += zinterp*dA;
               }
            } // i loop
         } // j loop

         double favg;
         if (area==0)
         {
            favg=0;
         }
         else
         {
            favg=finteg/area;
         }
         return favg;
      }

// ---------------------------------------------------------------------
// Method percentile_pixel_value() takes in an (x,y) location within
// the intensity twoDarray *ztwoDarray_ptr as well as bounding box x
// and y extents.  If input percentile_sentinel==0[1]{2}, this method
// returns the minimum[median]{maximum} of the intensities within an
// mxn lattice centered on (x,y).

   double percentile_pixel_value(
      double x,double y,double x_extent,double y_extent,
      twoDarray const *ztwoDarray_ptr,int percentile_sentinel)
      {
//         cout << "inside compositefunc::percentile_pixel_value()" << endl;
//         cout << "x = " << x << " y = " << y
//              << " x_extent = " << x_extent << " y_extent = " << y_extent
//              << endl;

// On 8/6/03, we realized (the hard & difficult way!) that the
// built-in floor function does NOT always return the same results on
// different platforms!  In particular, floor(3.000000000) may equal
// either 3 or 2.  So to avoid this particular difficulty, we add a
// TINY constant to a value prior to calling the floor function:

         const double TINY=1E-10;
         int m=basic_math::mytruncate(
            x_extent/ztwoDarray_ptr->get_deltax())+1;
         if (is_even(m)) m++;
         m=basic_math::max(3,m);
//         cout << "m = " << m << endl;

         int n=basic_math::mytruncate(
            y_extent/ztwoDarray_ptr->get_deltay())+1;
         if (is_even(n)) n++;
         n=basic_math::max(3,n);
//         cout << "n = " << n << endl;
         
         double xlow=x-x_extent/2;
         double xhigh=x+x_extent/2;
         double dx=(xhigh-xlow)/(m-1);
//         cout << "xlow = " << xlow << " xhigh = " << xhigh << " dx = " << dx
//              << endl;
   
         double ylow=y-y_extent/2;
         double yhigh=y+y_extent/2;
         double dy=(yhigh-ylow)/(n-1);
//         cout << "ylow = " << ylow << " yhigh = " << yhigh << " dy = " << dy
//              << endl;

         double zinterp;
         vector<double> intensity;
         for (int j=0; j<n; j++)
         {
            y=ylow+j*dy;
            for (int i=0; i<m; i++)
            {
               x=xlow+i*dx;
               if (ztwoDarray_ptr->point_to_interpolated_value(x,y,zinterp))
               {
//                  cout << "i = " << i << " j = " << j 
//                       << " x = " << x << " y = " << y 
//                       << " zinterp = " << zinterp
//                       << endl;
                  intensity.push_back(zinterp);
               }
            } // i loop
         } // j loop

         double value=0;
         if (intensity.size() > 0)
         {
            if (percentile_sentinel==0)		// minimal value
            {
               value=mathfunc::minimal_value(intensity);
            }
            else if (percentile_sentinel==1)	// median value
            {
               value=mathfunc::median_value(intensity);
               if (fabs(value) < TINY) value=0;
            }
            else if (percentile_sentinel==2)	// maximal value
            {
               value=mathfunc::maximal_value(intensity);
            }
         } // intensity.size() > 0 conditional

//         cout << "value = " << value << endl;
         return value;
      }

// ---------------------------------------------------------------------
// Method extremal_subsample_twoDarray() takes in *ztwoDarray_ptr and
// *znew_twoDarray_ptr.  The latter is assumed to be smaller in size
// than the former.  This method first sets xlo, xhi, deltax, ylo,
// yhi, deltay such that the extreme x and y edge values of the 2
// twoDarrays match (i.e. xmin, xmax, ymin, ymax which differ from
// xlo, xhi, ylo and yhi by 0.5*deltax and 0.5*deltay).  This method
// next runs an mxn block across *ztwoDarray_ptr where m,n are set by
// the ratio of the smaller to larger twoDarrays' delta_x and delta_y
// values.  The maximal [minimal] pixel values within the blocks are
// stored within *znew_twoDarray_ptr if input extremal_sentinel=2 [0].

   void extremal_subsample_twoDarray(
      twoDarray const *ztwoDarray_ptr,twoDarray* znew_twoDarray_ptr,
      int extremal_sentinel)
      {
//         cout << "inside compositefunc::extremal_subsample_twoDarray()" 
//              << endl;

         double xmin=ztwoDarray_ptr->get_xlo()
            -0.5*ztwoDarray_ptr->get_deltax();
         double xmax=ztwoDarray_ptr->get_xhi()
            +0.5*ztwoDarray_ptr->get_deltax();         
         double dx=(xmax-xmin)/znew_twoDarray_ptr->get_mdim();
         znew_twoDarray_ptr->set_xlo(xmin+0.5*dx);
         znew_twoDarray_ptr->set_xhi(xmax-0.5*dx);
         znew_twoDarray_ptr->set_deltax(dx);

         double ymin=ztwoDarray_ptr->get_ylo()
            -0.5*ztwoDarray_ptr->get_deltay();
         double ymax=ztwoDarray_ptr->get_yhi()
            +0.5*ztwoDarray_ptr->get_deltay();         
         double dy=(ymax-ymin)/znew_twoDarray_ptr->get_ndim();
         znew_twoDarray_ptr->set_ylo(ymin+0.5*dy);
         znew_twoDarray_ptr->set_yhi(ymax-0.5*dy);
         znew_twoDarray_ptr->set_deltay(dy);
         
         znew_twoDarray_ptr->clear_values();

//         cout << "*znew_twoDarray_ptr = " << *znew_twoDarray_ptr << endl;
         for (unsigned int j=0; j<znew_twoDarray_ptr->get_ndim(); j++)
         {
            for (unsigned int i=0; i<znew_twoDarray_ptr->get_mdim(); i++)
            {
               int m=basic_math::round(znew_twoDarray_ptr->get_deltax()/
                                       ztwoDarray_ptr->get_deltax());
               int n=basic_math::round(znew_twoDarray_ptr->get_deltay()/
                                       ztwoDarray_ptr->get_deltay());
//               cout << "m = " << m << " n = " << n << endl;
                  
               int px=m/2+i*m;
               int py=n/2+j*n;
               znew_twoDarray_ptr->put(
                  i,j,extremal_pixel_value(
                     px,py,m,n,ztwoDarray_ptr,extremal_sentinel));
            } // i loop
         } // j loop
      }

// ---------------------------------------------------------------------
// Method extremal_pixel_value() takes in coordinates (px,py) for some
// pixel within input *ztwoDarray_ptr.  It also takes in an m x n
// block (where m and n are assumed to be odd).  This method performs
// a brute force search of all pixels of *ztwoDarray_ptr within the m
// x n neighborhood of (px,py).  It returns the Z of the
// maximal[minimal]-valued pixel in the neighborhood.

   double extremal_pixel_value(
      int px,int py,int m,int n,
      twoDarray const *ztwoDarray_ptr,int extremal_sentinel)
      {
//         cout << "inside compositefunc::extremal_pixel_value()" << endl;

         vector<double> Z;
         for (int i=-m/2; i<=m/2; i++)
         {
            for (int j=-n/2; j<=n/2; j++)
            {
               unsigned int new_px=px+i;
               new_px=basic_math::max(new_px,Unsigned_Zero);
               new_px=basic_math::min(new_px,ztwoDarray_ptr->get_mdim()-1);
               
               unsigned int new_py=py+j;
               new_py=basic_math::max(new_py,Unsigned_Zero);
               new_py=basic_math::min(new_py,ztwoDarray_ptr->get_ndim()-1);
               Z.push_back(ztwoDarray_ptr->get(new_px,new_py));
            }
         }

         double value=0;
         if (extremal_sentinel==0)		// minimal value
         {
            value=mathfunc::minimal_value(Z);
         }
         else if (extremal_sentinel==2)	// maximal value
         {
            value=mathfunc::maximal_value(Z);
         }
//         cout << "value = " << value << endl;
         return value;
      }

// ---------------------------------------------------------------------
// Method downsample dynamically generates a twoDarray whose numbers
// of rows and columns are set by input parameters nxbins_regrid and
// nybins_regrid.  It then interpolates the contents of input
// twoDarray *ztwoDarray_ptr onto the new twoDarray.

   twoDarray* downsample(
      int nxbins_regrid,int nybins_regrid,twoDarray const *ztwoDarray_ptr,
      int percentile_sentinel)
      {
//         cout << "inside compositefunc::downsample()" << endl;
//         cout << "nxbins_regrid = " << nxbins_regrid
//              << " nybins_regrid = " << nybins_regrid << endl;
         
         twoDarray* zsmall_twoDarray_ptr=new twoDarray(
            nxbins_regrid,nybins_regrid);
         zsmall_twoDarray_ptr->set_xlo(ztwoDarray_ptr->get_xlo());
         zsmall_twoDarray_ptr->set_xhi(ztwoDarray_ptr->get_xhi());
         zsmall_twoDarray_ptr->set_ylo(ztwoDarray_ptr->get_ylo());
         zsmall_twoDarray_ptr->set_yhi(ztwoDarray_ptr->get_yhi());
   
//   outputfunc::newline();
//   cout << "nxbins_regrid = " << nxbins_regrid << " nybins_regrid = "
//        << nybins_regrid << endl;
//   outputfunc::newline();

         regrid_twoDarray(
            ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_xlo(),
            ztwoDarray_ptr->get_yhi(),ztwoDarray_ptr->get_ylo(),
            ztwoDarray_ptr,zsmall_twoDarray_ptr,percentile_sentinel);

//   cout << "zsmall_twoDarray = " << *zsmall_twoDarray_ptr << endl;
         return zsmall_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// On 6/30/01, we empirically discovered that cross range resolution
// in a handful of imagecdf files is rarely smaller than 15
// centimeters.  So we can speed up coarse smear detection by
// regridding raw images onto a smaller array with pixel size equal to
// 0.15 m x 0.15 m.  

   void downsampled_pixel_numbers(
      double min_pixel_length,int& nxbins_regrid,int& nybins_regrid,
      twoDarray const *ztwoDarray_ptr)
      {
         nxbins_regrid=basic_math::min(basic_math::round(
            (ztwoDarray_ptr->get_xhi()-ztwoDarray_ptr->get_xlo())/
            min_pixel_length),(int) ztwoDarray_ptr->get_mdim());

         nybins_regrid=basic_math::min(
            (unsigned int) nxbins_regrid,
            ztwoDarray_ptr->get_ndim());
      }

// ---------------------------------------------------------------------
// Method downsample_to_specified_resolution dynamically generates a
// twoDarray whose pixel size roughly equals min_pixel_length X
// min_pixel_length.  It then interpolates the contents of input
// twoDarray *ztwoDarray_ptr onto the new twoDarray.

   twoDarray* downsample_to_specified_resolution(
      double min_pixel_length,twoDarray const *ztwoDarray_ptr,
      int percentile_sentinel)
      {
         int nxbins_regrid,nybins_regrid;
         downsampled_pixel_numbers(
            min_pixel_length,nxbins_regrid,nybins_regrid,ztwoDarray_ptr);
//   cout << "inside  compositefunc::downsample()" << endl;
//   cout << "imagenumber = " << imagenumber << endl;
//   cout << "orig nxbins = " << ztwoDarray_ptr->get_mdim() 
//        << " subsampled nxbins = " << nxbins_regrid << endl;
//   cout << "orig nybins = " << ztwoDarray_ptr->get_ndim() 
//        << " subsampled nybins = " << nybins_regrid << endl;
//   outputfunc::newline();
         return downsample(nxbins_regrid,nybins_regrid,ztwoDarray_ptr,
                           percentile_sentinel);
      }

// ---------------------------------------------------------------------
// Method high_pass_composite scans through each pixel within two
// input arrays and keeps the maximum intensity value.  This nonlinear
// operation essentially acts like a high pass spatial filter.  The
// returned array zmax has the same pixel size and x/y extents as
// zarray2.

   void high_pass_composite(
      twoDarray *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      twoDarray* zmax_twoDarray_ptr,int percentile_sentinel)
      {
         twoDarray* zregrid1_twoDarray_ptr=NULL;

// Regrid ztwoDarray1, if necessary, so that it has exactly the same
// "working" pixel dimensions as ztwoDarray2:

         if (ztwoDarray1_ptr->get_mdim() != ztwoDarray2_ptr->get_mdim() || 
             ztwoDarray1_ptr->get_ndim() != ztwoDarray2_ptr->get_ndim() ||
             ztwoDarray1_ptr->get_xlo() != ztwoDarray2_ptr->get_xlo() ||
             ztwoDarray1_ptr->get_xhi() != ztwoDarray2_ptr->get_xhi() ||
             ztwoDarray1_ptr->get_ylo() != ztwoDarray2_ptr->get_ylo() ||
             ztwoDarray1_ptr->get_yhi() != ztwoDarray2_ptr->get_yhi())
         {
            zregrid1_twoDarray_ptr=new twoDarray(ztwoDarray2_ptr);
            regrid_twoDarray(
               ztwoDarray2_ptr->get_xhi(),ztwoDarray2_ptr->get_xlo(),
               ztwoDarray2_ptr->get_yhi(),ztwoDarray2_ptr->get_ylo(),
               ztwoDarray1_ptr,zregrid1_twoDarray_ptr,percentile_sentinel);
         }
         else
         {
            zregrid1_twoDarray_ptr=ztwoDarray1_ptr;
         }

         zmax_twoDarray_ptr->clear_values();
         for (unsigned int i=0; i<ztwoDarray2_ptr->get_dimproduct(); i++)
         {
            zmax_twoDarray_ptr->put(
               i,basic_math::max(zregrid1_twoDarray_ptr->get(i),
                     ztwoDarray2_ptr->get(i)));
         }
         delete zregrid1_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method low_pass_composite scans through each pixel within two input
// arrays and computes their weighted average.  This operation
// essentially acts like a low pass spatial filter.  Input parameters
// w1 and w2 are ABSOLUTE weight values (which do not necessarily sum
// to unity!)  The returned twoarray *zavg_twoDarray_ptr has the same
// pixel size and x/y extents as *ztwoDarray2_ptr.

// Note: Since *ztwoDarray2_ptr and *zavg_twoDarray_ptr may sometimes
// be equal, we do not make the former const.

   void low_pass_composite(
      double w1,twoDarray const *ztwoDarray1_ptr,
      double w2,twoDarray *ztwoDarray2_ptr,
      twoDarray* zavg_twoDarray_ptr,int percentile_sentinel)
      {
//   outputfunc::newline();
//   cout << "Inside myimage::low_pass_composite()" << endl;
//   cout << "ztwoDarray1 = " << *ztwoDarray1_ptr << endl;
//   cout << "ztwoDarray2 = " << *ztwoDarray2_ptr << endl;
//   cout << "zavg_twoDarray_ptr->get_mdim() = " << zavg_twoDarray_ptr->get_mdim() 
//        << " zavg_twoDarray_ptr->get_ndim() = " << zavg_twoDarray_ptr->get_ndim() 
//        << endl;

// Regrid ztwoDarray1, if necessary, so that it has exactly the same
// pixel dimensions as ztwoDarray2:

         twoDarray const* zregrid1_twoDarray_ptr=NULL;
         if (ztwoDarray1_ptr->get_mdim() != ztwoDarray2_ptr->get_mdim() || 
             ztwoDarray1_ptr->get_ndim() != ztwoDarray2_ptr->get_ndim() ||
             ztwoDarray1_ptr->get_xlo() != ztwoDarray2_ptr->get_xlo() ||
             ztwoDarray1_ptr->get_xhi() != ztwoDarray2_ptr->get_xhi() ||
             ztwoDarray1_ptr->get_ylo() != ztwoDarray2_ptr->get_ylo() ||
             ztwoDarray1_ptr->get_yhi() != ztwoDarray2_ptr->get_yhi())
         {
            twoDarray* zregrid1_twoDarray_tmp_ptr=
               new twoDarray(ztwoDarray2_ptr);
            regrid_twoDarray(
               ztwoDarray2_ptr->get_xhi(),ztwoDarray2_ptr->get_xlo(),
               ztwoDarray2_ptr->get_yhi(),ztwoDarray2_ptr->get_ylo(),
               ztwoDarray1_ptr,zregrid1_twoDarray_tmp_ptr,
               percentile_sentinel);
            zregrid1_twoDarray_ptr=zregrid1_twoDarray_tmp_ptr;
         }
         else
         {
            zregrid1_twoDarray_ptr=ztwoDarray1_ptr;
         }

//   outputfunc::newline();
//   cout << "Before actual compositing takes place:" << endl;
//   cout << "zregrid1_twoDarray_ptr->get_dimproduct() = " 
//        << zregrid1_twoDarray_ptr->get_dimproduct() << endl;
//   cout << "ztwoDarray2_ptr->get_dimproduct() = " 
//        << ztwoDarray2_ptr->get_dimproduct() << endl;
//   cout << "zavg_twoDarray_ptr->get_dimproduct() = " 
//        << zavg_twoDarray_ptr->get_dimproduct() << endl;
//   outputfunc::newline();

//   dynamic_colortable=true;
//   writeimage("ztwoDarray1","ztwoDarray1",ztwoDarray1_ptr);
//   writeimage("zregrid1","zregrid1",zregrid1_twoDarray_ptr);
//   writeimage("ztwoDarray2","ztwoDarray2",ztwoDarray2_ptr);
//   dynamic_colortable=false;   

         for (unsigned int i=0; i<ztwoDarray2_ptr->get_dimproduct(); i++)
         {
            zavg_twoDarray_ptr->put(
               i,w1*zregrid1_twoDarray_ptr->get(i)+
               w2*ztwoDarray2_ptr->get(i));
         }

//   dynamic_colortable=true;
//   writeimage("zavg","zavg",zavg_twoDarray_ptr);
//   dynamic_colortable=false;   

         if (zregrid1_twoDarray_ptr != ztwoDarray1_ptr)
         {
            delete zregrid1_twoDarray_ptr;
         }
      }

// ---------------------------------------------------------------------
// Method restore_supersampled_from_subsampled_twoDarray takes in a
// subsampled image along with an original "supersampled" raw image.
// It scans through each in the supersampled image and determines the
// enlarged pixel within the subsampled image to which it corresponds.
// If the enlarged pixel's intensity is nonzero, this method copies
// the original raw image's intensity into output twoDarray
// *ztwoDarray_ptr.  

   void restore_supersampled_from_subsampled_twoDarray(
      double zmin,twoDarray const *zsubsampled_twoDarray_ptr,
      twoDarray *ztwoDarray_orig_ptr,twoDarray *ztwoDarray_ptr)
      {
         unsigned int px_sub,py_sub;
         double x,y;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,x,y);
               zsubsampled_twoDarray_ptr->point_to_pixel(x,y,px_sub,py_sub);
               double zrestored=ztwoDarray_orig_ptr->get(px,py);
               if (zsubsampled_twoDarray_ptr->get(px_sub,py_sub) > zmin &&
                   zrestored > zmin)
               {
                  ztwoDarray_ptr->put(px,py,zrestored);
               }
               else
               {
                  ztwoDarray_ptr->put(px,py,0);
               }
            } // loop over py
         } // loop over px
      }

// ---------------------------------------------------------------------
// Method average_identically_sized_twoDarrays takes in 2 twoDarrays
// along with weighting factors w1 & w2.  The dimensions and pixel
// sizes of the input twoDarrays are assumed to be identical.  For all
// pixels whose values are NOT equal to input parameter null_value,
// this method returns the average intensity.  Otherwise, it sets the
// output pixel's intensity equal to null_value.

   void average_identically_sized_twoDarrays(
      double w1,twoDarray const *ztwoDarray1_ptr,
      double w2,twoDarray const *ztwoDarray2_ptr,
      twoDarray* zavg_twoDarray_ptr,double null_value)
      {
         for (unsigned int i=0; i<ztwoDarray2_ptr->get_dimproduct(); i++)
         {
            double currz1=ztwoDarray1_ptr->get(i);
            double currz2=ztwoDarray2_ptr->get(i);
            
            double zavg;
            if (currz1==null_value || currz2==null_value)
            {
               zavg=currz2;
            }
            else
            {
               zavg=w1*currz1+w2*currz2;
            }
            zavg_twoDarray_ptr->put(i,zavg);
         } // loop over index i
      }

// ---------------------------------------------------------------------
// Method combine_identically_sized_twoDarrays_in_quadrature takes in
// 2 twoDarrays.  The dimensions and pixel sizes of the input
// twoDarrays are assumed to be identical.  For each pixel, this
// method returns sqrt(v1**2+v2**2) where v1 and v2 are intensity
// values in the first and second twoDarrays.

   void combine_identically_sized_twoDarrays_in_quadrature(
      twoDarray const *ztwoDarray1_ptr,
      twoDarray const *ztwoDarray2_ptr,twoDarray* zquadrature_twoDarray_ptr,
      double null_value)
      {
         for (unsigned int i=0; i<ztwoDarray2_ptr->get_dimproduct(); i++)
         {
            double currz1=ztwoDarray1_ptr->get(i);
            double currz2=ztwoDarray2_ptr->get(i);
            double z_quadrature;

// Do not combine currz1 with currz2 if either one equals
// "NEGATIVEINFINITY" which we assume implies data absence:

            if (null_value < 0) 
            {
               if (currz1==null_value || currz2==null_value)
               {
                  z_quadrature=null_value;
               }
               else
               {
                  z_quadrature=sqrt(sqr(currz1)+sqr(currz2));
               }
            }
            else
            {
               z_quadrature=sqrt(sqr(currz1)+sqr(currz2));
            }
            zquadrature_twoDarray_ptr->put(i,z_quadrature);
         } // loop over index i
      }
      
} // compositefunc namespace




