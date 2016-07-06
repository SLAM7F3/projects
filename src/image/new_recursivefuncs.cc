// ==========================================================================
// RECURSIVEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 8/3/06; 8/5/06; 10/28/07; 2/25/14; 4/5/14
// ==========================================================================

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "math/constant_vectors.h"
#include "image/drawfuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "datastructures/Linkedlist.h"
#include "image/myimage.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace recursivefunc
{

// Method pseudo_random_sequence returns an array of integers whose
// values range from 0 to 7.  Its output is a quasi-random yet
// DETERMINISTIC function of input integer n.  We cooked up this
// little utility function in Aug 03 (with lots of pointer help from
// Vadim!) to take the place of mathfunc::random_sequence().  We hope
// that calling this method rather than mathfunc::random_sequence()
// should help from a speed, error sensitivity and threading
// standpoint.

   void pseudo_random_sequence(int n,int *a_ptr_ptr[])
      {
         const int n_arrays=17;
         const int n_neighbors=8;
         
         static int b[n_arrays][n_neighbors]=
         {
            {0,3,2,6,1,6,5,7},
            {3,5,6,4,1,0,2,7},
            {6,3,4,5,0,2,7,1},
            {5,4,2,6,1,7,0,3},
            {2,0,6,3,4,7,1,5},
            {4,7,5,1,0,6,3,2},
            {7,2,6,1,3,0,4,5},
            {1,4,7,3,2,6,5,0},
            {5,6,3,4,2,1,7,0},
            {4,3,5,0,1,2,7,6},
            {2,1,3,5,4,6,7,0},
            {6,5,4,3,7,1,2,0},
            {0,3,6,2,5,4,1,7},
            {7,6,5,3,4,2,1,0},
            {4,2,0,1,6,7,3,5},
            {3,5,1,6,2,4,0,7},
            {1,4,7,2,3,5,6,0}
         };
         *a_ptr_ptr=b[modulo(n,n_arrays)];
      }

// ---------------------------------------------------------------------
// Method recursive_empty takes in twoDarray *ztwoDarray_ptr and
// recursively empties it of small noisy "islands".

   void recursive_empty(
      int nrecursion_max,twoDarray *ztwoDarray_ptr,
      bool find_pixel_borders_before_filling,double znull)
      {
         recursive_empty(nrecursion_max,0,ztwoDarray_ptr,
                         find_pixel_borders_before_filling,znull);
      }

   void recursive_empty(
      int nrecursion_max,double zmin,twoDarray *ztwoDarray_ptr,
      bool find_pixel_borders_before_filling,double znull)
      {
//	cout << "inside recursivefunc::recursive_empty()" << endl;
	
         unsigned int px_min,px_max,py_min,py_max;
         if (find_pixel_borders_before_filling)
         {
            imagefunc::compute_pixel_borders(
               ztwoDarray_ptr,px_min,px_max,py_min,py_max);
         }
         else
         {
            px_min=py_min=0;

// On 2/25/14, we changed the following defns for px_max and py_max:

            px_max=ztwoDarray_ptr->get_mdim();
            py_max=ztwoDarray_ptr->get_ndim();
//            px_max=ztwoDarray_ptr->get_mdim()-1;
//            py_max=ztwoDarray_ptr->get_ndim()-1;
         }
         
         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

         binaryimagefunc::binary_threshold(
            zmin,px_min,px_max,py_min,py_max,
            ztwoDarray_ptr,zbinary_twoDarray_ptr);
         binary_fill(nrecursion_max,px_min,px_max,py_min,py_max,
                     1,0,zbinary_twoDarray_ptr);
         binary_null(ztwoDarray_ptr,zbinary_twoDarray_ptr,
                     px_min,px_max,py_min,py_max,znull);

         delete zbinary_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method recursive_fill takes in twoDarray *ztwoDarray_ptr and
// recursively fills in small holes with values taken from
// *zorig_twoDarray_ptr.

   void recursive_fill(
      int nrecursion_max,twoDarray *ztwoDarray_ptr,
      twoDarray const *zorig_twoDarray_ptr,
      bool find_pixel_borders_before_filling)
      {
         unsigned int px_min,px_max,py_min,py_max;
         if (find_pixel_borders_before_filling)
         {
            imagefunc::compute_pixel_borders(
               ztwoDarray_ptr,px_min,px_max,py_min,py_max);
         }
         else
         {
            px_min=py_min=0;
            px_max=ztwoDarray_ptr->get_mdim()-1;
            py_max=ztwoDarray_ptr->get_ndim()-1;
         }

         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binaryimagefunc::binary_threshold(
            Unsigned_Zero,px_min,px_max,py_min,py_max,
            ztwoDarray_ptr,zbinary_twoDarray_ptr);
         binary_fill(nrecursion_max,px_min,px_max,py_min,py_max,
                     0,1,zbinary_twoDarray_ptr);
         binary_restore(
            ztwoDarray_ptr,zorig_twoDarray_ptr,zbinary_twoDarray_ptr,
            px_min,px_max,py_min,py_max);
         delete zbinary_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method binary_null nulls elements inside input twoDarray
// *ztwoDarray_ptr within a bounding box defined by pixel limits pxlo
// < px < pxhi and pylo < py < pyhi if the corresponding zbinary
// values are zero:

   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      double znull)
      {
         binary_null(ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_ylo(),
                     ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_yhi(),
                     ztwoDarray_ptr,zbinary_twoDarray_ptr,znull);
      }
   
   void binary_null(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      double znull)
      {
         unsigned int min_px,max_px,min_py,max_py;
         ztwoDarray_ptr->point_to_pixel(minimum_x,maximum_y,min_px,min_py);
         ztwoDarray_ptr->point_to_pixel(maximum_x,minimum_y,max_px,max_py);
         ztwoDarray_ptr->keep_pnt_inside_working_region(
            min_px,min_py,max_px,max_py);
         binary_null(ztwoDarray_ptr,zbinary_twoDarray_ptr,
                     min_px,max_px,min_py,max_py,znull);
      }

   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi,
      double znull)
      {
//	cout << "inside binary_null()" << endl;
//	cout << "pxlo = " << pxlo << " pxhi = " << pxhi
//	     << " pylo = " << pylo << " pyhi = " << pyhi << endl;
//	cout << "znull = " << znull << endl;

         for (unsigned int i=pxlo; i<pxhi; i++)
         {
            for (unsigned int j=pylo; j<pyhi; j++)
            {
               if (nearly_equal(zbinary_twoDarray_ptr->get(i,j),0))
                  ztwoDarray_ptr->put(i,j,znull);
            }
         }
      }

// This next overloaded version of the binary_null method takes in
// threshold value z_threshold.  Depending upon the value of boolean
// input parameter nearly_equal_flag, this method nulls all entries
// within *ztwoDarray_ptr corresponding to those locations within the
// mask image *zmask_twoDarray_ptr whose values are less than or equal
// to z_threshold.

   void binary_null(
      double z_threshold,twoDarray* ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,double znull,
      bool nearly_equal_flag,bool greater_than_flag)
      {
//         cout << "inside recursivefunc::binary_null()" << endl;
//         cout << "nearly_equal_flag = " << nearly_equal_flag
//              << " greater_than_flag = " << greater_than_flag << endl;

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (nearly_equal_flag)
               {
                  if (nearly_equal(zmask_twoDarray_ptr->get(px,py),
                                   z_threshold))
                     ztwoDarray_ptr->put(px,py,znull);
               }
               else if (greater_than_flag)
               {
                  if (zmask_twoDarray_ptr->get(px,py) > z_threshold)
                     ztwoDarray_ptr->put(px,py,znull);
               }
               else
               {
                  if (zmask_twoDarray_ptr->get(px,py) < z_threshold)
                     ztwoDarray_ptr->put(px,py,znull);
               }
            } // loop over py index
         } // loop over px index
      }

   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      polygon& bbox,double znull)
      {
         unsigned int pxlo,pxhi,pylo,pyhi;
         ztwoDarray_ptr->locate_extremal_xy_pixels(bbox,pxlo,pylo,pxhi,pyhi);
         binary_null(ztwoDarray_ptr,zbinary_twoDarray_ptr,
                     pxlo,pxhi,pylo,pyhi,znull);
      }

// ---------------------------------------------------------------------
// Method binary_restore sets elements inside input twoDarray
// ztwoDarray_ptr within a bounding box defined by pixel limits pxlo <
// px < pxhi and pylo < py < pyhi to their counterpart values in
// *rawz_twoDarray_ptr if the corresponding zbinary values equal
// unity.  This particular method is essentially the inverse of
// binary_null(twoDarray* ztwoDarray_ptr,int pxlo,int pxhi,int
// pylo,int pyhi).

   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         binary_restore(
            ztwoDarray_ptr,rawz_twoDarray_ptr,zbinary_twoDarray_ptr,
            0,ztwoDarray_ptr->get_mdim()-1,0,ztwoDarray_ptr->get_ndim()-1);
      }

   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi)
      {
         for (unsigned int i=pxlo; i<pxhi; i++)
         {
            for (unsigned int j=pylo; j<pyhi; j++)
            {
               if (zbinary_twoDarray_ptr->get(i,j) > 0.999)
               {
                  ztwoDarray_ptr->put(i,j,rawz_twoDarray_ptr->get(i,j));
               }
            }
         }
      }

// ---------------------------------------------------------------------
// This overloaded version of method binary_restore sets elements
// inside input twoDarray *ztwoDarray_ptr within a bounding box
// defined by minimum_x < x < maximum_x and minimum_y < y < maximum_y
// to their counterpart values in twoDarray *raw_ztwoDarray_pt if the
// corresponding zbinary_twoDarray values equal unity.  This
// particular method is essentially the inverse of binary_null(double
// minimum_x,double minimum_y,double maximum_x,double maximum_y,
// twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr).

   void binary_restore(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         unsigned int pxlo,pylo,pxhi,pyhi;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,pxlo,pxhi,pylo,pyhi))
         {
            binary_restore(
               ztwoDarray_ptr,rawz_twoDarray_ptr,zbinary_twoDarray_ptr,
               pxlo,pylo,pxhi,pyhi);
         }
   
      }

   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr,polygon& bbox)
      {
         unsigned int pxlo,pxhi,pylo,pyhi;
         ztwoDarray_ptr->locate_extremal_xy_pixels(bbox,pxlo,pylo,pxhi,pyhi);
         binary_restore(ztwoDarray_ptr,rawz_twoDarray_ptr,
                        zbinary_twoDarray_ptr,pxlo,pxhi,pylo,pyhi);
      }
// ---------------------------------------------------------------------
// Method binary_filter takes in twoDarray *ztwoDarray_ptr along with
// binary-valued twoDarray *zbinary_filter_twoDarray_ptr.  It nulls
// all pixels whose corresponding binary filter values equal unity.
// It also fills in any null-valued hole within *ztwoDarray_ptr if its
// corresponding binary filter value equals zero.  

   void binary_filter(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_filter_twoDarray_ptr,
      double znull,double zfill_value)
      {
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<ztwoDarray_ptr->get_ndim(); j++)
            {
               double curr_zbinary=zbinary_filter_twoDarray_ptr->get(i,j);
               if (curr_zbinary > 0.999)
               {
                  ztwoDarray_ptr->put(i,j,znull);
               }
               else if (curr_zbinary < 0.001 && 
                        ztwoDarray_ptr->get(i,j)==znull)
               {
                  ztwoDarray_ptr->put(i,j,zfill_value);
               }
            } // loop over j index
         } // loop over i index
      }

// ---------------------------------------------------------------------
// Method binary_fill scans through the twoDarray
// *zbinary_twoDarray_ptr after it has been filled with binary
// thresholded data.  The scan is limited to the bounding box defined
// by pixel limits pxlo < px < pxhi and pylo < py < pyhi.  In
// conjunction with method check_nearest_neighbors,
// binary_fill recursively searches for islands of zempty_value valued
// pixels which are completely surrounded by oceans of zfill_value
// valued pixels.  It sets the values of all such island pixels equal
// to zfill_value.

   void binary_fill(
      int max_recursion_levels,
      unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,twoDarray* zbinary_twoDarray_ptr)
      {
//	cout << "inside recursivefunc::binary_fill()" << endl;
//	cout << "pxlo = " << pxlo << " pxhi = " << pxhi
//	     << " pylo = " << pylo << " pyhi = " << pyhi << endl;
//	cout << "zempty_value = " << zempty_value << " zfill_value = " << zfill_value
//	     << endl;

         const int max_iterations=4;
         const int n_neighbors=8;

         bool bit_changed;
         int loop=0;
         int* neighbor_order=NULL;

         int row[n_neighbors];
         int column[n_neighbors];

         do
         {
            bit_changed=false;
            loop++;

// Note added on 6/6/02: Recall that the results of recursive
// filling/emptying DO depend upon the order and direction in which
// the recursion takes place.  So to maintain consistency with
// 2000-2002 results, we do NOT swap the order of the following loops
// over the vertical and horizontal pixel indices...
      
            int counter=0;
            for (unsigned int j=pylo+1; j<pyhi-1; j++)
            {
               for (unsigned int i=pxlo+1; i<pxhi-1; i++)
               {
                  double zbinary_orig=zbinary_twoDarray_ptr->get(i,j);
                  if (zbinary_orig==zempty_value) 
                  {
                     int curr_recursive_level=0;
                     bool deadend=false;
                     pseudo_random_sequence(counter++,&neighbor_order);
		     
                     check_nearest_neighbors(
                        i,j,max_recursion_levels,
                        pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                        row,column,curr_recursive_level,deadend,
                        neighbor_order,zbinary_twoDarray_ptr);
                  }
                  if (zbinary_twoDarray_ptr->get(i,j) != zbinary_orig)
                  {
                     bit_changed=true;
                  }
               }  // loop over index i
            }	// loop over index j
         }
         while(bit_changed && loop < max_iterations);
      }

// ---------------------------------------------------------------------
// Method binary_fill scans through the zbinary array after it has
// been filled with binary thresholded data.  The scan is limited to
// the bounding box defined by minimum_x < x < maximum_x and minimum_y
// < y < maximum_y.  In conjunction with method
// check_nearest_neighbors, binary_fill recursively
// searches for islands of zempty_value valued pixels which are
// completely surrounded by oceans of zfill_value valued pixels.  It
// sets the values of all such island pixels equal to zfill_value.

   void binary_fill(
      int max_recursion_levels,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zempty_value,double zfill_value,twoDarray* zbinary_twoDarray_ptr)
      {
         unsigned int pxlo,pylo,pxhi,pyhi;
         if (zbinary_twoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,pxlo,pylo,pxhi,pyhi))
         {
            binary_fill(max_recursion_levels,pxlo,pxhi,pylo,pyhi,
                        zempty_value,zfill_value,zbinary_twoDarray_ptr);
         }
      }

// ---------------------------------------------------------------------
// Method binary_fill scans through the *zbinary_twoDarray_ptr
// twoDarray after it has been filled with binary thresholded data.
// The scan is limited to the bounding box defined by minimum_x < x <
// maximum_x and minimum_y < y < maximum_y.  In conjunction with
// method check_nearest_neighbors, binary_fill
// recursively searches for islands of zempty_value valued pixels
// which are completely surrounded by oceans of zfill_value valued
// pixels.  It sets the values of all such island pixels equal to
// zfill_value.

   void binary_fill(
      bool remove_xstreaks,bool remove_ystreaks,
      int max_recursion_xlevels,int max_recursion_ylevels,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zempty_value,double zfill_value,
      twoDarray* zbinary_twoDarray_ptr,twoDarray* zbinaryorig_twoDarray_ptr)
      {
         const unsigned int max_iterations=3;
         const unsigned int n_neighbors=4;

         zbinary_twoDarray_ptr->copy_metric_data(zbinaryorig_twoDarray_ptr);
         unsigned int min_px,max_px,min_py,max_py;
         if (zbinary_twoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            bool bit_changed;
            unsigned int loop=0;
            int neighbor_order[n_neighbors];
            int row[n_neighbors];
            int column[n_neighbors];
            do
            {
               bit_changed=false;
               loop++;
//               cout << "binary_fill loop = " << loop << flush;

// Before commencing current iteration loop's recursive
// filling/emptying, copy contents of zbinary_twoDarray within
// bounding box onto zbinaryorig_twoDarray_ptr:

               for (unsigned int i=min_px; i<max_px; i++)
               {
                  for (unsigned int j=min_py; j<max_py; j++)
                  {
                     zbinaryorig_twoDarray_ptr->put(
                        i,j,zbinary_twoDarray_ptr->get(i,j));
                  }
               }

               for (unsigned int i=min_px+1; i<max_px-1; i++)
               {
//                  cout << i << " " << flush;
                  for (unsigned int j=min_py+1; j<max_py-1; j++)
                  {
                     if (zbinaryorig_twoDarray_ptr->get(i,j)==zempty_value)
                     {
                        int curr_recursive_xlevel=0;
                        int curr_recursive_ylevel=0;
                        bool deadend=false;

// Since we look for streaks with definite horizontal or vertical
// orientations, we do NOT perform a RANDOM search over pixel neighbor
// directions.  This leads to a random walk with bad, slow sqrt(t)
// behavior.

                        for (unsigned int k=0; k<n_neighbors; k++) 
                           neighbor_order[k]=k;

                        check_nearest_neighbors(
                           remove_xstreaks,remove_ystreaks,
                           i,j,max_recursion_xlevels,max_recursion_ylevels,
                           min_px,max_px,min_py,max_py,
                           zempty_value,zfill_value,row,column,
                           curr_recursive_xlevel,curr_recursive_ylevel,
                           deadend,neighbor_order,zbinary_twoDarray_ptr);

                        if (zbinary_twoDarray_ptr->get(i,j) != 
                            zbinaryorig_twoDarray_ptr->get(i,j))
                        {
                           zbinaryorig_twoDarray_ptr->put(
                              i,j,zbinary_twoDarray_ptr->get(i,j));
                           bit_changed=true;
                        }

                        for (unsigned int k=min_px; k<max_px; k++)
                        {
                           for (unsigned int l=min_py; l<max_py; l++)
                           {
                              zbinary_twoDarray_ptr->put(
                                 k,l,zbinaryorig_twoDarray_ptr->get(k,l));
                           }
                        }

                     }  // zbinaryorig_twoDarray_ptr->get(i,j)==zempty_value 
                  }  // loop over index j
               }  // loop over index i
            }
            while(bit_changed && loop < max_iterations);
         } // bbox_corners_to_pixels conditional
      }

// Note: The following method can probably be eliminated in
// favor of binary_fill(bool remove_xstreaks,bool remove_ystreaks, int
// max_recursion_xlevels,int max_recursion_ylevels, double
// minimum_x,double minimum_y,double maximum_x,double maximum_y,
// double zempty_value,double zfill_value,twoDarray*
// zbinary_twoDarray_ptr) which we added on 12/27/01:

   void binary_fill(
      int max_recursion_levels,polygon& bbox,
      double zempty_value,double zfill_value,twoDarray* zbinary_twoDarray_ptr)
      {
         unsigned int pxlo,pxhi,pylo,pyhi;
         zbinary_twoDarray_ptr->locate_extremal_xy_pixels(
            bbox,pxlo,pylo,pxhi,pyhi);
         binary_fill(max_recursion_levels,pxlo,pxhi,pylo,pyhi,
                     zempty_value,zfill_value,zbinary_twoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method check_nearest_neighbors examines the 8 pixel values
// immediately surrounding some zero valued pixel specified by input
// row and column values i and j.  If all surrounding pixels have unit
// value, check_nearest_neighbors sets the value of the (i,j) pixel
// also equal to unity.  If one or more of the pixels surrounding
// (i,j) is also zero valued, this function temporarily sets pixel
// (i,j)'s value to unity and then recursively calls itself.  Upon
// returning, pixel (i,j)'s value is reset back to zero, and this
// function checks again to see whether all surrounding pixels are
// equal to unity after the recursive call has been executed.  In
// order to prevent runaway recursion within regions of an image that
// correspond to completely empty space, the recursion loop is
// terminated if the number of recursive calls exceeds
// max_recursion_levels or if the island of zero valued pixels bumps
// up against a border of the image.

// We have intentionally tried to optimize this method for speed.

   void check_nearest_neighbors(
      unsigned int i,unsigned int j,int max_recursion_levels,
      unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,int row[],int column[],
      int& curr_recursive_level,bool& deadend,int neighbor_order[],
      twoDarray* zbinary_twoDarray_ptr)
      {
//	cout << "inside recursivefunc::check_nearest_neighbors()" << endl;
	
         bool all_neighbors_filled=false;
         bool recheck_original_neighbors=false;
         const int n_neighbors=8;
         int n_eff=zbinary_twoDarray_ptr->get_ndim()*i+j;
         unsigned int inew=0;
         unsigned int jnew=0;
         do
         {
            compute_neighboring_pixel_column_row_values(
               i,j,n_neighbors,neighbor_order,row,column);
            if (!is_some_neighboring_pixel_empty(
               n_neighbors,row,column,zempty_value,zbinary_twoDarray_ptr,
               inew,jnew))
            {
               all_neighbors_filled=true;
               zbinary_twoDarray_ptr->put(n_eff,zfill_value);
            }

            if (!all_neighbors_filled)
            {
               if (inew > pxlo && jnew > pylo  &&
                   inew < pxhi-1 && jnew < pyhi-1 &&
                   curr_recursive_level < max_recursion_levels)
               {
                  curr_recursive_level++;
                  zbinary_twoDarray_ptr->put(n_eff,zfill_value);
                  check_nearest_neighbors(
                     inew,jnew,max_recursion_levels,
                     pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                     row,column,curr_recursive_level,
                     deadend,neighbor_order,zbinary_twoDarray_ptr);
                  curr_recursive_level--;
                  zbinary_twoDarray_ptr->put(n_eff,zempty_value);
                  recheck_original_neighbors=true;
               }
               else
               {
                  deadend=true;
               }
            }
         }
         while (recheck_original_neighbors && !deadend && 
                !all_neighbors_filled);
      }

// ---------------------------------------------------------------------
// Method check_nearest_neighbors examines the 4 or 8 pixel values
// immediately surrounding some zero valued pixel specified by input
// row and column values i and j.  If all surrounding pixels have unit
// value, check_nearest_neighbors sets the value of the (i,j) pixel
// also equal to unity.  If one or more of the pixels surrounding
// (i,j) is also zero valued, this function temporarily sets pixel
// (i,j)'s value to unity and then recursively calls itself.  Upon
// returning, pixel (i,j)'s value is reset back to zero, and this
// function checks again to see whether all surrounding pixels are
// equal to unity after the recursive call has been executed.  In
// order to prevent runaway recursivion within regions of an image
// that correspond to completely empty space, the recursion loop is
// terminated if the number of recursive calls exceeds
// max_recursion_levels or if the island of zero valued pixels bumps
// up against a border of the image.

   void check_nearest_neighbors(
      bool remove_xstreaks,bool remove_ystreaks,
      unsigned int i,unsigned int j,
      int max_recursion_xlevels,int max_recursion_ylevels,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,int row[],int column[],
      int& curr_recursive_xlevel,int& curr_recursive_ylevel,
      bool& deadend,int neighbor_order[],twoDarray* zbinary_twoDarray_ptr)
      {
         bool all_neighbors_filled=false;
         bool recheck_original_neighbors=false;

         const unsigned int n_neighbors=4;
         unsigned int inew=0;
         unsigned int jnew=0;
         do
         {
            compute_neighboring_pixel_column_row_values(
               i,j,n_neighbors,neighbor_order,row,column);

            if (!is_some_neighboring_pixel_empty(
               n_neighbors,row,column,zempty_value,zbinary_twoDarray_ptr,
               inew,jnew))
            {
               all_neighbors_filled=true;
               zbinary_twoDarray_ptr->put(i,j,zfill_value);
            }

//         zbinary_twoDarray_ptr->pixel_to_point(inew,jnew,currx,curry);
//         cout << "inew = " << inew << " jnew = " << jnew 
//              << " xnew = " << currx << " ynew = " << curry << endl;
//         cout << "all_neighbors_filled = "
//              << all_neighbors_filled << endl;

            if (remove_xstreaks)
            {
               if (!all_neighbors_filled && (inew <= pxlo || inew >= pxhi))
               {
                  all_neighbors_filled=true;
                  zbinary_twoDarray_ptr->put(i,j,zfill_value);
//            cout << "inew = " << inew << " pxlo = " << pxlo
//                 << " pxhi = " << pxhi << endl;
               }
            }
            if (remove_ystreaks)
            {
               if (!all_neighbors_filled && (jnew <= pylo || jnew >= pyhi))
               {
                  all_neighbors_filled=true;
                  zbinary_twoDarray_ptr->put(i,j,zfill_value);
               }
            }

            if (!all_neighbors_filled)
            {
               if (inew > pxlo && jnew > pylo  &&
                   inew < pxhi-1 && jnew < pyhi-1 &&
                   abs(curr_recursive_xlevel) < max_recursion_xlevels &&
                   abs(curr_recursive_ylevel) < max_recursion_ylevels)
               {
                  if (inew==i-1)
                  {
                     curr_recursive_xlevel--;
                  }
                  else if (inew==i+1)
                  {
                     curr_recursive_xlevel++;
                  }
                  if (jnew==j-1)
                  {
                     curr_recursive_ylevel--;
                  }
                  else if (jnew==j+1)
                  {
                     curr_recursive_ylevel++;
                  }

                  zbinary_twoDarray_ptr->put(i,j,zfill_value);
                  check_nearest_neighbors(
                     remove_xstreaks,remove_ystreaks,
                     inew,jnew,max_recursion_xlevels,max_recursion_ylevels,
                     pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                     row,column,curr_recursive_xlevel,curr_recursive_ylevel,
                     deadend,neighbor_order,zbinary_twoDarray_ptr);

                  if (inew==i-1)
                  {
                     curr_recursive_xlevel++;
                  }
                  else if (inew==i+1)
                  {
                     curr_recursive_xlevel--;
                  }
                  if (jnew==j-1)
                  {
                     curr_recursive_ylevel++;
                  }
                  else if (jnew==j+1)
                  {
                     curr_recursive_ylevel--;
                  }
                  zbinary_twoDarray_ptr->put(i,j,zempty_value);
                  recheck_original_neighbors=true;
               }
               else
               {
                  deadend=true;
               }
            } // !all_neighbors_filled conditional
         }
         while (recheck_original_neighbors && !deadend
                && !all_neighbors_filled);
      }

// =====================================================================
// Boundary filling methods
// =====================================================================

// In Nov 2004, we discovered the hard way that the following simple
// recursive approach to contour filling can fail for large regions
// due to memory limitations.  This problem is widely recognized in
// the computer graphics literature.  So the following method is NOT
// robust!

// The following approach to filling polygons is used frequently in
// paint programs.  The idea is to select a point inside the region to
// be filled, set its color to the desired color and then apply the
// same procedure recursively to all its neighbors.  Of course, one
// must take care to not exceed the boundaries of the area to be
// filled, nor to recolor pixels which have already been colored.

//  The following method was adapted from
//  http://occs.cs.oberlin.edu/faculty/jdonalds/357/lecture05.html

   void boundaryFill(
      bool& recursion_limit_exceeded,int& npixels_filled,int& n_recursion,
      const int px,const int py,const double zfill,const double zboundary,
      int& max_empty_neighbors,int& new_px,int& new_py,
      twoDarray* ztwoDarray_ptr)
      {
         const int recursion_limit=5000;
//         const int recursion_limit=25000;
//         const int recursion_limit=50000;
         n_recursion++;

         if (n_recursion < recursion_limit && !recursion_limit_exceeded)
         {
            if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (!nearly_equal(curr_z,zfill) && 
                   !nearly_equal(curr_z,zboundary))
               {

//               cout << "nfilled = " << npixels_filled 
//                    << " px = " << px << " py = " << py 
//                    << " mdim = " << ztwoDarray_ptr->get_mdim()
//                    << " ndim = " << ztwoDarray_ptr->get_ndim() << endl;
//               cout << "curr_z = " << curr_z << " zfill = " << zfill
//                    << " zboundary = " << zboundary << endl;

                  ztwoDarray_ptr->put(px,py,zfill);
                  npixels_filled++;

                  boundaryFill(
                     recursion_limit_exceeded,npixels_filled,n_recursion,
                     px+1,py,zfill,zboundary,max_empty_neighbors,
                     new_px,new_py,ztwoDarray_ptr);
                  boundaryFill(
                     recursion_limit_exceeded,npixels_filled,n_recursion,
                     px,py+1,zfill,zboundary,max_empty_neighbors,
                     new_px,new_py,ztwoDarray_ptr);
                  boundaryFill(
                     recursion_limit_exceeded,npixels_filled,n_recursion,
                     px-1,py,zfill,zboundary,max_empty_neighbors,
                     new_px,new_py,ztwoDarray_ptr);
                  boundaryFill(
                     recursion_limit_exceeded,npixels_filled,n_recursion,
                     px,py-1,zfill,zboundary,max_empty_neighbors,
                     new_px,new_py,ztwoDarray_ptr);
               } // curr_z != zfill && curr_z != z_boundary conditional
            } // pixel inside working region conditional
         }
         else
         {
            recursion_limit_exceeded=true;
            double z_east=NEGATIVEINFINITY;
            double z_south=NEGATIVEINFINITY;
            double z_west=NEGATIVEINFINITY;
            double z_north=NEGATIVEINFINITY;
            if (ztwoDarray_ptr->pixel_inside_working_region(px+1,py))
            {
               z_east=ztwoDarray_ptr->get(px+1,py);
            }
            if (ztwoDarray_ptr->pixel_inside_working_region(px,py+1))
            {
               z_south=ztwoDarray_ptr->get(px,py+1);
            }
            if (ztwoDarray_ptr->pixel_inside_working_region(px-1,py))
            {
               z_west=ztwoDarray_ptr->get(px-1,py);
            }
            if (ztwoDarray_ptr->pixel_inside_working_region(px,py-1))
            {
               z_north=ztwoDarray_ptr->get(px,py-1);
            }

            int n_empty_neighbors=0;
            if (!nearly_equal(z_east,zfill) && 
                !nearly_equal(z_east,zboundary)) n_empty_neighbors++;
            if (!nearly_equal(z_south,zfill) && 
                !nearly_equal(z_south,zboundary)) n_empty_neighbors++;
            if (!nearly_equal(z_west,zfill) && 
                !nearly_equal(z_west,zboundary)) n_empty_neighbors++;
            if (!nearly_equal(z_north,zfill) && 
                !nearly_equal(z_north,zboundary)) n_empty_neighbors++;
            if (n_empty_neighbors >= max_empty_neighbors)
            {
               max_empty_neighbors=n_empty_neighbors;
               new_px=px;
               new_py=py;
            }
         } // n_recursion < recursion_limit 
         n_recursion--;
      }

   void boundaryFill(
      int& npixels_filled,const unsigned int px,const unsigned int py,
      const double zfill,const double zboundary,
      twoDarray* ztwoDarray_ptr)
      {
         if (px < 0 || px >= ztwoDarray_ptr->get_mdim() ||
             py < 0 || py >= ztwoDarray_ptr->get_ndim())
         {
            cout << "Error in recursivefunc::boundaryFill()" << endl;
            cout << "px = " << px << " py = " << py << endl;
         }
         
         if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
         {
            double curr_z=ztwoDarray_ptr->get(px,py);
            if (!nearly_equal(curr_z,zfill) && 
                !nearly_equal(curr_z,zboundary))
            {
//               cout << "nfilled = " << npixels_filled 
//                    << " px = " << px << " py = " << py 
//                    << " mdim = " << ztwoDarray_ptr->get_mdim()
//                    << " ndim = " << ztwoDarray_ptr->get_ndim() << endl;
//               cout << "curr_z = " << curr_z << " zfill = " << zfill
//                    << " zboundary = " << zboundary << endl;

//               cout << "prior to put(px,py) px = " << px 
//                    << " py = " << py <<  " zfill = " << zfill << endl;
               ztwoDarray_ptr->put(px,py,zfill);
               npixels_filled++;
               boundaryFill(npixels_filled,px+1,py,zfill,zboundary,
                            ztwoDarray_ptr);
               boundaryFill(npixels_filled,px,py+1,zfill,zboundary,
                            ztwoDarray_ptr);
               boundaryFill(npixels_filled,px-1,py,zfill,zboundary,
                            ztwoDarray_ptr);
               boundaryFill(npixels_filled,px,py-1,zfill,zboundary,
                            ztwoDarray_ptr);
            }
         } // pixel inside working region conditional
      }
   
// =====================================================================
// Pixel cluster detection methods
// =====================================================================

// Method binary_cluster recursively searches for pixel "lumps".  It
// takes in twoDarray *ztwoDarray_ptr and creates a binary thresholded
// version *zbinary_twoDarray_ptr of its contents.  It then scans
// through zbinary_twoDarray_ptr within the bounding box defined by
// pxlo < px < pxhi and pylo < py < pyhi.  It looks for zempty_value
// valued pixels.  When it finds one such pixel, it recursively calls
// method find_colored_neighbors.  After the recursion is complete,
// linkedlist *lumplist_ptr[nlumps] contains nodes whose independent
// variables correspond to the pixel locations of the colored pixels
// and whose dependent variables contain corresponding pixel intensity
// information.  Within *zbinary_twoDarray_ptr, the initial seed pixel
// along with all of its neighboring friends are destructively set
// equal to zfill_value.  After the first lump is completely found,
// the method continues to scan over the remaining
// *zbinary_twoDarray_ptr pixels inside the bounding box.  By the end
// of this method, array lumplist_ptr[] contains nlumps linkedlist
// pointers which point to individual lump pixel information.

   void binary_cluster(
      int max_recursion_levels,unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      const twoDarray* ztwoDarray_ptr,unsigned int& nlumps,
      linkedlist* lumplist_ptr[])
      {
// First binary threshold input ztwoDarray:

         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binaryimagefunc::binary_threshold(
            1,ztwoDarray_ptr,zbinary_twoDarray_ptr);

         nlumps=0;
         int* neighbor_order=NULL;
         int counter=0;
         for (unsigned int i=pxlo+1; i<pxhi-1; i++)
         {
            for (unsigned int j=pylo+1; j<pyhi-1; j++)
            {
               if (zbinary_twoDarray_ptr->get(i,j)==zempty_value)
               {
                  int curr_recursive_level=0;
                  bool deadend=false;
                  pseudo_random_sequence(counter++,&neighbor_order);
                  lumplist_ptr[nlumps]=new linkedlist;
                  find_colored_neighbors(
                     i,j,max_recursion_levels,
                     pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                     curr_recursive_level,deadend,
                     neighbor_order,zbinary_twoDarray_ptr,
                     lumplist_ptr[nlumps],ztwoDarray_ptr);
                  nlumps++;
               }
            }  // loop over index j
         }  // loop over index i
         delete zbinary_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
   void binary_cluster(
      int max_recursion_levels,unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      const twoDarray* ztwoDarray_ptr,unsigned int& nlumps,
      vector<linkedlist*>& lumplist_ptr)
      {
// First binary threshold input ztwoDarray:

         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binaryimagefunc::binary_threshold(
            1,ztwoDarray_ptr,zbinary_twoDarray_ptr);

         nlumps=0;
         int* neighbor_order=NULL;
         int counter=0;
         for (unsigned int i=pxlo+1; i<pxhi-1; i++)
         {
            for (unsigned int j=pylo+1; j<pyhi-1; j++)
            {
               if (zbinary_twoDarray_ptr->get(i,j)==zempty_value)
               {
                  int curr_recursive_level=0;
                  bool deadend=false;
                  pseudo_random_sequence(counter++,&neighbor_order);
                  lumplist_ptr[nlumps]=new linkedlist;
                  find_colored_neighbors(
                     i,j,max_recursion_levels,
                     pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                     curr_recursive_level,deadend,
                     neighbor_order,zbinary_twoDarray_ptr,
                     lumplist_ptr[nlumps],ztwoDarray_ptr);
                  nlumps++;
               }
            }  // loop over index j
         }  // loop over index i
         delete zbinary_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method find_colored_neighbors recursively searches for all
// neighbors of pixel (i,j) whose binary thresholded
// *zbinary_twoDarray_ptr values equal zempty_value.  It conducts its
// search within the bounding box defined by pxlo < px < pxhi and pylo
// < py < pyhi.  The pixel location along with intensity value of any
// colored neighbor which this method finds is placed within
// linkedlist lumplist.  At the locations of the colored pixels, the
// entries within the binary thresholded *zbinary_twoDarray_ptr are
// set equal to zfill_value.  However, the contents of ztwoDarray_ptr
// are left untouched by this method.

   void find_colored_neighbors(
      int i,int j,int max_recursion_levels,
      unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      int& curr_recursive_level,bool& deadend,
      int neighbor_order[],twoDarray* zbinary_twoDarray_ptr,
      linkedlist* lumplist_ptr,const twoDarray* ztwoDarray_ptr)
      {
         bool all_neighbors_filled=false;
         bool recheck_original_neighbors=false;

         const unsigned int n_neighbors=8;
         const unsigned int n_indep_vars=2;
         double var[n_indep_vars];

         unsigned int inew=0;
         unsigned int jnew=0;
         int row[n_neighbors];
         int column[n_neighbors];
         do
         {
            compute_neighboring_pixel_column_row_values(
               i,j,n_neighbors,neighbor_order,row,column);


            if (!is_some_neighboring_pixel_empty(
               n_neighbors,row,column,zempty_value,zbinary_twoDarray_ptr,
               inew,jnew))
            {
               all_neighbors_filled=true;
               zbinary_twoDarray_ptr->put(i,j,zfill_value);
               var[0]=i;
               var[1]=j;
               lumplist_ptr->append_node(datapoint(n_indep_vars,var,
                                         ztwoDarray_ptr->get(i,j)));
            }

            if (!all_neighbors_filled)
            {
               if (inew > pxlo && jnew > pylo  &&
                   inew < pxhi-1 && jnew < pyhi-1 &&
                   curr_recursive_level < max_recursion_levels)
               {
                  curr_recursive_level++;
                  zbinary_twoDarray_ptr->put(i,j,zfill_value);
                  find_colored_neighbors(
                     inew,jnew,max_recursion_levels,
                     pxlo,pxhi,pylo,pyhi,zempty_value,zfill_value,
                     curr_recursive_level,deadend,
                     neighbor_order,zbinary_twoDarray_ptr,
                     lumplist_ptr,ztwoDarray_ptr);

                  curr_recursive_level--;
                  zbinary_twoDarray_ptr->put(i,j,zempty_value);
                  recheck_original_neighbors=true;
               }
               else
               {
                  deadend=true;
               }
            }
         }
         while (recheck_original_neighbors && !deadend 
                && !all_neighbors_filled);
      }

// ---------------------------------------------------------------------
// Method compute_cluster_COMs takes in an array of linked lists
// containing pixel "lump" location and intensity information.  It
// computes the COM of each lump as well as its integrated intensity.

   void compute_cluster_COMs(
      unsigned int nlumps,vector<linkedlist*>& lumplist_ptr,
      const twoDarray* ztwoDarray_ptr,
      vector<int>& npixels_in_lump,vector<double>& lump_intensity_sum,
      vector<threevector>& lumpCOM) 
      {
         for (unsigned int l=0; l<nlumps; l++)
         {
            double weighted_xsum=0;
            double weighted_ysum=0;
            npixels_in_lump[l]=0;
            lump_intensity_sum[l]=0;

            threevector currpoint;
            mynode* currnode_ptr=lumplist_ptr[l]->get_start_ptr();
            while (currnode_ptr != NULL)
            {
               unsigned int px=
                  basic_math::round(currnode_ptr->get_data().get_var(0));
               unsigned int py=
                  basic_math::round(currnode_ptr->get_data().get_var(1));
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               weighted_xsum += currnode_ptr->get_data().get_func(0)*
                  currpoint.get(0);
               weighted_ysum += currnode_ptr->get_data().get_func(0)*
                  currpoint.get(1);
               npixels_in_lump[l]++;
               lump_intensity_sum[l] += currnode_ptr->get_data().get_func(0);
               currnode_ptr=currnode_ptr->get_nextptr();
            }

            if (lump_intensity_sum[l] <= 0)
            {
               cout << "Error in recursivefunc::compute_cluster_COMs()" 
                    << endl;
               cout << "l = " << l << " lump_intensity_sum[l] = " 
                    << lump_intensity_sum[l] << endl;
               cout << "npixels_in_lump[l] = " << npixels_in_lump[l]
                    << endl;
               cout << "lumplist_ptr[l]->size() = " 
                    << lumplist_ptr[l]->size() << endl;
               cout << "nlumps = " << nlumps << endl;
               lumpCOM[l]=Zero_vector;
            }
            else
            {
               lumpCOM[l]=threevector(
                  weighted_xsum/lump_intensity_sum[l],
                  weighted_ysum/lump_intensity_sum[l],0);
            }
         }
      }

// ---------------------------------------------------------------------
// Method compute_cluster_thresholds takes in an array of linked lists
// containing pixel "lump" location and intensity information.  It
// computes the intensity distribution for each lump in the array and
// returns the threshold intensity corresponding to the input
// intensity_frac without output array lump_threshold[].

   void compute_cluster_thresholds(
      unsigned int nlumps,linkedlist* lumplist_ptr[],
      const twoDarray* ztwoDarray_ptr,
      double intensity_frac,double lump_threshold[]) 
      {
         for (unsigned int l=0; l<nlumps; l++)
         {
            int npixels=lumplist_ptr[l]->size();
            double* lump_pixel_intensity;
            new_clear_array(lump_pixel_intensity,npixels);

            int n=0;
            mynode* currnode_ptr=lumplist_ptr[l]->get_start_ptr();
            while (currnode_ptr != NULL)
            {
               lump_pixel_intensity[n++]=currnode_ptr->get_data().get_func(0);
               currnode_ptr=currnode_ptr->get_nextptr();
            }
            prob_distribution prob(npixels,lump_pixel_intensity,30);
            lump_threshold[l]=prob.find_x_corresponding_to_pcum(
               intensity_frac);
            delete lump_pixel_intensity;
         }
      }

// ---------------------------------------------------------------------
// Method consolidate_clusters takes in linkedlist lumplist[] which
// contains lumps of hot pixels.  Input parameters min_xseparation and
// min_yseparation (in meters) specify the distances in the x and y
// directions by which two lumps' center-of-masses must be apart in
// order for both lumps to be regarded as distinct.  Otherwise, this
// method merges the two lumps together.  The consolidated list of
// lumps is returned within a new version of lumplist[].

   bool consolidate_clusters(
      unsigned int max_nlumps,unsigned int& nlumps,
      double min_xseparation,double min_yseparation,
      vector<threevector>& lumpCOM,vector<linkedlist*>& lumplist_ptr) 
      {
         bool* distinct_lump=new bool[max_nlumps];
         for (unsigned int i=0; i<max_nlumps; i++) distinct_lump[i]=true;
         
         bool some_clusters_consolidated=false;
         for (unsigned int l=0; l<nlumps; l++)
         {
            if (distinct_lump[l])
            {
               for (unsigned int k=l+1; k<nlumps; k++)
               {
                  if (distinct_lump[k])
                  {
                     threevector lump_separation(lumpCOM[l]-lumpCOM[k]);
                     if (fabs(lump_separation.get(0)) < min_xseparation &&
                         fabs(lump_separation.get(1)) < min_yseparation)
                     {
                        lumplist_ptr[l]->concatenate(lumplist_ptr[k]);
                        distinct_lump[k]=false;
                        some_clusters_consolidated=true;
                     }
                  } // distinct_lump[k] conditional
               } // loop over index k
            } // distinct_lump[l] conditional
         } // loop over index l

// Create temporary array consolidated_lumplist_ptr[] of linkedlist
// pointers.  Fill contents of this array with just those linkedlist
// pointers corresponding to distinct lumps.  Delete linked lists
// which do not correspond to distinct lumps. 

         unsigned int new_nlumps=0;
         linkedlist* consolidated_lumplist_ptr[nlumps];
         for (unsigned int l=0; l<nlumps; l++)
         {
            if (distinct_lump[l])
            {
               consolidated_lumplist_ptr[new_nlumps]=lumplist_ptr[l];
               new_nlumps++;
            }
            else
            {
               delete lumplist_ptr[l];
            }
         }
         delete distinct_lump;

// Copy contents of consolidated_lumplist_ptr[] back into first
// new_nlumps elements of lumplist_ptr[]:

         for (unsigned int l=0; l<new_nlumps; l++)
         {
            lumplist_ptr[l]=consolidated_lumplist_ptr[l];
         }
         nlumps=new_nlumps;

         return some_clusters_consolidated;
      }

// ---------------------------------------------------------------------
// Method function locate_hot_pixel_clusters takes in twoDarray
// *ztwoDarray_ptr which is assumed to have already been thresholded.
// It recursively empties and fills the image which contains a
// relatively few number of hot pixels.  It then recursively
// identifies clusters of hot pixels.  After consolidating together
// clusters which are located close together, this method computes the
// COM locations of the remaining clusters.  This method returns the
// number of distinct lumps as well as their COM locations.

   void locate_hot_pixel_clusters(
      unsigned int max_nlumps,unsigned int& nlumps,
      vector<threevector>& lumpCOM,
      vector<linkedlist*>& lumplist_ptr,twoDarray* ztwoDarray_ptr,
      double min_lump_xseparation,double min_lump_yseparation,
      bool merge_close_lumps_together,double null_zvalue)
      {
         vector<int> npixels_in_lump(max_nlumps);
         vector<double> lump_intensity_sum(max_nlumps);
   
         int max_recursion_levels=10000;
         binary_cluster(
            max_recursion_levels,0,ztwoDarray_ptr->get_mdim(),
            0,ztwoDarray_ptr->get_ndim(),
            1.0,0.0,ztwoDarray_ptr,nlumps,lumplist_ptr);
         if (merge_close_lumps_together)
         {
            bool some_clusters_consolidated;
            do
            {
               compute_cluster_COMs(
                  nlumps,lumplist_ptr,ztwoDarray_ptr,
                  npixels_in_lump,lump_intensity_sum,lumpCOM);
               some_clusters_consolidated=
                  consolidate_clusters(
                     max_nlumps,nlumps,min_lump_xseparation,
                     min_lump_yseparation,lumpCOM,lumplist_ptr);
            }
            while (some_clusters_consolidated);
         }
         else
         {
            compute_cluster_COMs(
               nlumps,lumplist_ptr,ztwoDarray_ptr,npixels_in_lump,
               lump_intensity_sum,lumpCOM);
         } // merge_close_lumps_together conditional

         if (!merge_close_lumps_together)
         {

// Eliminate lumps which have very few nonzero pixels:

            const int npixels_min=3;
            int n_biglumps=0;
            templatefunc::Quicksort_descending(
               npixels_in_lump,lumplist_ptr,lump_intensity_sum,lumpCOM);
            for (unsigned int l=0; l<nlumps; l++)
            {
               if (npixels_in_lump[l] < npixels_min)
               {
                  mynode* currnode_ptr=lumplist_ptr[l]->get_start_ptr();
                  while (currnode_ptr != NULL)
                  {
                     unsigned int px=basic_math::round(
                        currnode_ptr->get_data().get_var(0));
                     unsigned int py=basic_math::round(
                        currnode_ptr->get_data().get_var(1));
                     ztwoDarray_ptr->put(px,py,null_zvalue);
                     currnode_ptr=currnode_ptr->get_nextptr();
                  }
               }
               else
               {
                  n_biglumps++;
               }
            } // loop over index l labeling lump number

            for (unsigned int l=n_biglumps; l<nlumps; l++)
            {
               delete lumplist_ptr[l];
            }
            nlumps=n_biglumps;
            
// Based upon their integrated intensities, we next remove some weak
// fraction of the remaining lumps:

            templatefunc::Quicksort_descending(
               lump_intensity_sum,lumplist_ptr,npixels_in_lump,lumpCOM);
            const double frac_to_retain=0.5;
            int n_hotlumps=basic_math::mytruncate(frac_to_retain*nlumps);
            for (unsigned int l=n_hotlumps; l<nlumps; l++)
            {
               mynode* currnode_ptr=lumplist_ptr[l]->get_start_ptr();
               while (currnode_ptr != NULL)
               {
                  unsigned int px=basic_math::round(
                     currnode_ptr->get_data().get_var(0));
                  unsigned int py=basic_math::round(
                     currnode_ptr->get_data().get_var(1));
                  ztwoDarray_ptr->put(px,py,null_zvalue);
                  currnode_ptr=currnode_ptr->get_nextptr();
               }
            }
            for (unsigned int l=n_hotlumps; l<nlumps; l++)
            {
               delete lumplist_ptr[l];
            }
            nlumps=n_hotlumps;
         } // !merge_close_lumps_together conditional

//         if (!merge_close_lumps_together && nlumps <= 3)
//         {
//            cout << "nlumps = " << nlumps << endl;
//            for (unsigned int l=0; l<nlumps; l++)
//            {
//               cout << "lump " << l << endl;
//               cout << "lump COM.x = " << lumpCOM[l].get(0)
//                    << " lump COM.y = " << lumpCOM[l].get(1) << endl;
//               cout << "lumplist[l]->size() = " 
//                    << lumplist_ptr[l]->size() 
//                    << " lump intensity sum = " << lump_intensity_sum[l] 
//                    << endl;
//               cout << "npixels_in_lump = " << npixels_in_lump[l] << endl;
//               outputfunc::newline();
//               drawfunc::draw_hugepoint(
//                  lumpCOM[l],0.2,colorfunc::white,ztwoDarray_ptr);
//            }
//         } // !merge_close_lumps_together conditional
      }

// ---------------------------------------------------------------------
// Method wrap_polygon_around_cluster generates a series of rays of
// length max_cluster_radius which emanate outwards from the
// cluster_center location.  This method then integrates the intensity
// within input twoDarray *ztwoDarray_ptr along the ray.  It
// determines the point along the ray for which the intensity integral
// equals some significant fraction intensity_integral_frac (e.g. 95%)
// of its total value.  The method uses that point as a vertex on the
// polygon which is "shrink-wrapped" around the cluster.

   void wrap_polygon_around_cluster(
      double max_cluster_radius,double intensity_integral_frac,
      const threevector& cluster_center,twoDarray* ztwoDarray_ptr,
      polygon& poly)
      {
         const unsigned int nvertices=36;
         vector<threevector> vertex(nvertices);
   
         double dtheta=2*PI/double(nvertices);
         for (unsigned int n=0; n<nvertices; n++)
         {
            double theta=n*dtheta;
            threevector rhat(cos(theta),sin(theta),0);
            linesegment l(cluster_center,cluster_center+
                          max_cluster_radius*rhat);
            imagefunc::find_null_border_along_ray(
               intensity_integral_frac,ztwoDarray_ptr,l,vertex[n],true);
         }
         poly=polygon(vertex);
         poly.compute_area();
//   imagefunc::draw_polygon(poly,colorfunc::white,ztwoDarray_ptr);
      }
   
} // recursivefunc namespace




