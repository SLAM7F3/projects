// =========================================================================
// Header file for stand-alone image recursive image processing functions.
// =========================================================================
// Last modified on 8/13/04; 8/3/06; 8/5/06; 10/28/07; 4/5/14
// =========================================================================

#ifndef RECURSIVEFUNCS_H
#define RECURSIVEFUNCS_H

#include <vector>
#include "datastructures/datapoint.h"
class polygon;
class threevector;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;
#include "image/TwoDarray.h"

namespace recursivefunc
{
   void pseudo_random_sequence(int n,int* a_ptr_ptr[]);
   void recursive_empty(
      int nrecursion_max,twoDarray *ztwoDarray_ptr,
      bool find_pixel_borders_before_filling,double znull=0);
   void recursive_empty(
      int nrecursion_max,double zmin,twoDarray *ztwoDarray_ptr,
      bool find_pixel_borders_before_filling,double znull=0);
   void recursive_fill(
      int nrecursion_max,twoDarray *ztwoDarray_ptr,
      twoDarray const *zorig_twoDarray_ptr,
      bool find_pixel_borders_before_filling);

   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      double znull=0);
   void binary_null(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      double znull=0);
   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi,
      double znull=0);
   void binary_null(
      double z_threshold,twoDarray* ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,double znull=0,
      bool nearly_equal_flag=false,bool greater_than_flag=false);
   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      polygon& bbox,double znull=0);

   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr);
   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi);
   void binary_restore(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr);
   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr,polygon& bbox);

   void binary_filter(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_filter_twoDarray_ptr,
      double znull,double zfill_value);
   
   void binary_fill(
      int max_recursion_levels,unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      twoDarray* zbinary_twoDarray_ptr);
   void binary_fill(
      int max_recursion_levels,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zempty_value,double zfill_value,
      twoDarray* zbinary_twoDarray_ptr);
   void binary_fill(
      bool remove_xstreaks,bool remove_ystreaks,
      int max_recursion_xlevels,int max_recursion_ylevels,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zempty_value,double zfill_value,
      twoDarray* zbinary_twoDarray_ptr,twoDarray* zbinaryorig_twoDarray_ptr);
   void binary_fill(
      int max_recursion_levels,polygon& bbox,
      double zempty_value,double zfill_value,
      twoDarray* zbinary_twoDarray_ptr);
   void compute_neighboring_pixel_column_row_values(
      int i,int j,int n_neighbors,int neighbor_order[],
      int row[],int column[]);
   bool is_some_neighboring_pixel_empty(
      unsigned int n_neighbors,int row[],int column[],
      double zempty_value,const twoDarray* zbinary_twoDarray_ptr,
      unsigned int& inew,unsigned int& jnew);
   void check_nearest_neighbors(
      unsigned int i,unsigned int j,int max_recursion_levels,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,int row[],int column[],
      int& curr_recursive_level,bool& deadend,int neighbor_order[],
      twoDarray* zbinary_twoDarray_ptr);
   void check_nearest_neighbors(
      bool remove_xstreaks,bool remove_ystreaks,
      unsigned int i,unsigned int j,
      int max_recursion_xlevels,int max_recursion_ylevels,
      unsigned int pxlo,unsigned int pxhi,unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,int row[],int column[],
      int& curr_recursive_xlevel,int& curr_recursive_ylevel,
      bool& deadend,int neighbor_order[],twoDarray* zbinary_twoDarray_ptr);

// Boundary filling methods:
   
   void boundaryFill(
      bool& recursion_limit_exceeded,int& npixels_filled,int& n_recursion,
      const int px,const int py,const double zfill,const double zboundary,
      int& max_empty_neighbors,int& new_px,int& new_py,
      twoDarray* ztwoDarray_ptr);
   void boundaryFill(
      int& npixels_filled,const unsigned int px,const unsigned int py,
      const double zfill,const double zboundary,twoDarray* ztwoDarray_ptr);

// Pixel cluster detection methods:

   void binary_cluster(
      int max_recursion_levels,unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      const twoDarray* ztwoDarray_ptr,unsigned int& nlumps,
      linkedlist* lumplist_ptr[]);
   void binary_cluster(
      int max_recursion_levels,unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      const twoDarray* ztwoDarray_ptr,unsigned int& nlumps,
      std::vector<linkedlist*>& lumplist_ptr);
   void find_colored_neighbors(
      int i,int j,int max_recursion_levels,
      unsigned int pxlo,unsigned int pxhi,
      unsigned int pylo,unsigned int pyhi,
      double zempty_value,double zfill_value,
      int& curr_recursive_level,bool& deadend,
      int neighbor_order[],twoDarray* zbinary_twoDarray_ptr,
      linkedlist* lumplist_ptr,const twoDarray* ztwoDarray_ptr);
   void compute_cluster_COMs(
      unsigned int nlumps,std::vector<linkedlist*>& lumplist_ptr,
      const twoDarray* ztwoDarray_ptr,std::vector<int>& npixels_in_lump,
      std::vector<double>& lump_intensity_sum,
      std::vector<threevector>& lumpCOM);
   void compute_cluster_thresholds(
      unsigned int nlumps,linkedlist* lumplist_ptr[],
      const twoDarray* ztwoDarray_ptr,
      double intensity_frac,double lump_threshold[]);
   bool consolidate_clusters(
      unsigned int max_nlumps,unsigned int& nlumps,
      double min_xseparation,double min_yseparation,
      std::vector<threevector>& lumpCOM,
      std::vector<linkedlist*>& lumplist_ptr);
   void locate_hot_pixel_clusters(
      unsigned int max_nlumps,unsigned int& nlumps,
      std::vector<threevector>& lumpCOM,
      std::vector<linkedlist*>& lumplist_ptr,twoDarray* ztwoDarray_ptr,
      double min_lump_xseparation,double min_lump_yseparation,
      bool merge_close_lumps_together=true,double null_zvalue=0);
   void wrap_polygon_around_cluster(
      double max_cluster_radius,double intensity_integral_frac,
      const threevector& cluster_center,twoDarray* ztwoDarray_ptr,
      polygon& poly);

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Method compute_neighboring_pixel_column_row_values is a small
// utility method which sets the upper left neighbor's row and column
// values equal to (i-1,j-1), the upper neighbor's values to
// (i,j-1),... the lower right neighbor's values to (i+1,j+1):

   inline void compute_neighboring_pixel_column_row_values(
      int i,int j,int n_neighbors,int neighbor_order[],int row[],int column[])
      {
         for (register int k=0; k<n_neighbors; k++)
         {
            int norder;
            if (neighbor_order[k] <= 3)
            {
               norder=neighbor_order[k];
            }
            else
            {
               norder=neighbor_order[k]+1;
            }
            column[k]=i+norder%3-1;
            row[k]=j+norder/3-1;
         } // loop over index k
      }

// ---------------------------------------------------------------------
// Method is_some_neighboring_pixel_empty scans through the
// neighboring pixel information encoded within integer arrays row[]
// and column[].  It checks whether any of the neighbors have a
// *zbinary_twoDarray_ptr value equal to zempty_value.  If so, this
// boolean method returns true as well as the inew and jnew locations
// of the "empty" pixel.

   inline bool is_some_neighboring_pixel_empty(
      unsigned int n_neighbors,int row[],int column[],
      double zempty_value,const twoDarray* zbinary_twoDarray_ptr,
      unsigned int& inew,unsigned int& jnew)
      {
         bool some_neighboring_pixel_empty=false;
         for (register unsigned int k=0; k<n_neighbors; k++)
         {
            if (zbinary_twoDarray_ptr->get(column[k],row[k])==zempty_value) 
            {
               inew=column[k];
               jnew=row[k];
               some_neighboring_pixel_empty=true;
               break;
            }
         }
         return some_neighboring_pixel_empty;
      }

} // recursivefunc namespace

#endif // recursivefuncs.h



