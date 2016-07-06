// =========================================================================
// Header file for stand-alone image recursive image processing functions.
// =========================================================================
// Last modified on 2/1/04
// =========================================================================

#ifndef RECURSIVEFUNCS_H
#define RECURSIVEFUNCS_H

#include "datastructures/datapoint.h"
class myvector;
class polygon;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

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
      int pxlo,int pxhi,int pylo,int pyhi,double znull=0); 
   void binary_null(
      double zmin,twoDarray* ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,double znull=0);
   void binary_null(
      twoDarray* ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      polygon& bbox,double znull=0);
   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr);
   void binary_restore(
      twoDarray* ztwoDarray_ptr,twoDarray const *rawz_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr,
      int pxlo,int pxhi,int pylo,int pyhi);
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
      int max_recursion_levels,int pxlo,int pxhi,int pylo,int pyhi,
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
      int n_neighbors,int row[],int column[],
      double zempty_value,const twoDarray* zbinary_twoDarray_ptr,
      int& inew,int& jnew);
   void check_nearest_neighbors(
      int i,int j,int max_recursion_levels,
      int pxlo,int pxhi,int pylo,int pyhi,
      double zempty_value,double zfill_value,
      int& curr_recursive_level,bool& deadend,int neighbor_order[],
      twoDarray* zbinary_twoDarray_ptr);
   void check_nearest_neighbors(
      bool remove_xstreaks,bool remove_ystreaks,
      int i,int j,int max_recursion_xlevels,int max_recursion_ylevels,
      int pxlo,int pxhi,int pylo,int pyhi,
      double zempty_value,double zfill_value,int row[],int column[],
      int& curr_recursive_xlevel,int& curr_recursive_ylevel,
      bool& deadend,int neighbor_order[],twoDarray* zbinary_twoDarray_ptr);

// Pixel clustering methods:

   void binary_cluster(
      int max_recursion_levels,int pxlo,int pxhi,int pylo,int pyhi,
      double zempty_value,double zfill_value,
      const twoDarray* ztwoDarray_ptr,int& nlumps,
      linkedlist* lumplist_ptr[]);
   void find_colored_neighbors(
      int i,int j,int max_recursion_levels,
      int pxlo,int pxhi,int pylo,int pyhi,
      double zempty_value,double zfill_value,
      int& curr_recursive_level,bool& deadend,
      int neighbor_order[],twoDarray* zbinary_twoDarray_ptr,
      linkedlist* lumplist_ptr,const twoDarray* ztwoDarray_ptr);
   void compute_cluster_COMs(
      int nlumps,linkedlist* lumplist_ptr[],
      const twoDarray* ztwoDarray_ptr,
      int npixels_in_lump[],double lump_intensity_sum[],myvector lumpCOM[]);
   void compute_cluster_thresholds(
      int nlumps,linkedlist* lumplist_ptr[],
      const twoDarray* ztwoDarray_ptr,
      double intensity_frac,double lump_threshold[]);
   bool consolidate_clusters(
      int max_nlumps,int& nlumps,
      double min_xseparation,double min_yseparation,myvector lumpCOM[],
      linkedlist* lumplist_ptr[]);
   void locate_hot_pixel_clusters(
      int max_nlumps,int& nlumps,myvector lumpCOM[],
      linkedlist* lumplist_ptr[],twoDarray* ztwoDarray_ptr,
      double min_lump_xseparation,double min_lump_yseparation,
      bool merge_close_lumps_together=true,double null_zvalue=0);
   void wrap_polygon_around_cluster(
      double max_cluster_radius,double intensity_integral_frac,
      const myvector& cluster_center,twoDarray* ztwoDarray_ptr,
      polygon& poly);
}

#endif // recursivefuncs.h



