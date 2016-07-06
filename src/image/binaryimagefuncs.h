// =========================================================================
// Header file for stand-alone binary image functions.
// =========================================================================
// Last modified on 4/12/14; 4/14/14; 4/17/14; 4/28/14
// =========================================================================

#ifndef BINARYIMAGEFUNCS_H
#define BINARYIMAGEFUNCS_H

#include <map>
#include <set>
#include <vector>

template <class T> class Linkedlist;
class polygon;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace binaryimagefunc
{

// Binary image processing methods:

   void locate_first_nonzero_pixel(
      twoDarray const *zbinary_twoDarray_ptr,int& px_first,int& py_first);
   void binary_image_pixel_bbox(
      twoDarray const *zbinary_twoDarray_ptr,
      int& px_min,int& px_max,int& py_min,int& py_max);

   void binary_threshold(
      double z_threshold,twoDarray* ztwoDarray_ptr,double znull=0,
      double zfill=1);
   void binary_threshold(
      double z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull=0,double zfill=1);
   void binary_threshold(
      double z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull=0,double zfill=1);
   void binary_threshold(
      double z_threshold,int px_min,int px_max,int py_min,int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull,double zfill);

   void binary_threshold_inside_bbox(
      double z_threshold,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull=0);
   void binary_threshold_above_cutoff(
      double z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull=0);
   void binary_threshold_above_cutoff(
      double z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull=0);
   void binary_threshold_for_particular_cutoff(
      double z_threshold,twoDarray const *ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull=0);
   void abs_binary_threshold(
      double abs_z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull=0);
   void abs_binary_threshold(
      double abs_z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull=0);
   twoDarray* binary_dilate(
      int n_size,double znull,twoDarray const *ztwoDarray_ptr);
   twoDarray* binary_dilate(
      double xdilate_dist,double ydilate_dist,double z_threshold,
      twoDarray const *ztwoDarray_ptr,double znull=0);

   void binary_reverse(
      twoDarray *zbinary_twoDarray_ptr,double znull=0,double zfill=1);
   void binary_reverse(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      twoDarray* zbinary_twoDarray_ptr,double znull=0,double zfill=1);

   int binary_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *zbinary_twoDarray_ptr,
      twoDarray *zbinary_twoDarray_filtered_ptr,
      double intensity_frac_threshold);
   twoDarray* binary_density_filter(
      double diameter,double fill_frac_threshold,
      twoDarray const *zbinary_twoDarray_ptr,
      bool copy_all_pixels_in_filter=true);
   Linkedlist<std::pair<int,int> >* binary_image_to_list(
      twoDarray const *zbinary_twoDarray_ptr);
   Linkedlist<std::pair<int,int> >* binary_image_to_list(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      twoDarray const *zbinary_twoDarray_ptr);

// Euler number & perimeter methods

   void pixel_to_quad(
      int px,int py,int pfill,twoDarray const *pbinary_twoDarray_ptr,
      int& tl, int& tr, int& bl, int& br);
   void count_quads(
      int tl,int tr,int bl,int br,int pfill,
      int& n_Q1, int& n_Q2, int& n_Q3, int& n_Q4, int& n_QD);

   double quad_Euler_number_contribution(
      int tl,int tr,int bl,int br,int pfill);
   double quad_perimeter_contribution(
      int tl,int tr,int bl,int br,int pfill);
   double quad_area_contribution(
      int tl,int tr,int bl,int br,int pfill);

   void image_Euler_number_perimeter_area(
      int pfill,twoDarray* pbinary_twoDarray_ptr,double& Euler_number,
      double& perimeter,double& area);

   double quad_Euler_number_contribution(
      int px,int py,int pfill,twoDarray const *pbinary_twoDarray_ptr);
   double Euler_number_contribution_from_single_pixel(
      int px,int py,int pfill,twoDarray* cc_twoDarray_ptr);
   double delta_Euler_number_for_single_pixel(
      int px,int py,int pfill,twoDarray* cc_twoDarray_ptr);

   double Euler_number_contribution_from_single_pixel(
      int px,int py,int pfill,std::vector<int>& neighbor_values);
   double delta_Euler_number_for_single_pixel(
      int px,int py,int pfill,std::vector<int>& neighbor_values);

   double compute_delta_Euler_number(
      std::vector<std::pair<int,int> >& pixel_coords,int pfill,
      twoDarray* cc_twoDarray_ptr,twoDarray* visited_twoDarray_ptr);

// Convex hull methods

   polygon* compute_convex_hull(const twoDarray* zbinary_twoDarray_ptr);
   
}

#endif // binaryimagefuncs.h




