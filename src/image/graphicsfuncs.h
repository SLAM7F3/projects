// Note added on 12/27/12: We should move get_pixel_ID() and
// get_pixel_px_py() out of graphicsfuncs namespace into arrayfuncs

// =========================================================================
// Header file for stand-alone graphics functions.
// =========================================================================
// Last modified on 3/28/14; 4/5/14; 6/16/16; 6/20/16
// =========================================================================

#ifndef GRAPHICSFUNCS_H
#define GRAPHICSFUNCS_H

#include <map>
#include <set>
#include <vector>
#include "math/threevector.h"
class contour;
class polygon;
template <class T> class Linkedlist;
template <class T> class Stack;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace graphicsfunc
{
   int* poor_mans_circle_array(unsigned int n);
   std::vector<double> compute_delta_s_values(double dx,double dy);
   bool point_near_hot_pixels(
      int min_hot_pixels,double znull,
      const threevector& curr_pnt,twoDarray const *zbinary_twoDarray_ptr,
      int n_window_size=3);
   double height_percentile(
      double znull,const threevector& curr_pnt,
      twoDarray const *zbinary_twoDarray_ptr,twoDarray const *ztwoDarray_ptr);

// Contour methods:

   polygon* connected_region_convex_hull(twoDarray const *ztwoDarray_ptr);
   polygon* connected_region_convex_hull(
      twoDarray const *ztwoDarray_ptr,
      int min_px,int max_px,int min_py,int max_py);
   void shrink_wrap_regularized_contour(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr,
      double delta_r,unsigned int n_max_iters=100);
   int dilate_regularized_contour(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr);
   void avg_heights_at_contour_vertices(
      double znull,contour& c,twoDarray const *zbinary_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr);
   double edge_gradient_contour_integral(
      int n,double correlation_length,const contour& c,
      twoDarray const *ztwoDarray_ptr);

// Turtle perimeter finding methods:

   std::vector<std::pair<int, int> > new_turtle_boundary(
     twoDarray* zorig_binary_twoDarray_ptr);
   Linkedlist<threevector>* turtle_boundary(
      twoDarray* zorig_binary_twoDarray_ptr);
   contour* convert_turtle_boundary_to_contour(
      double delta_s,Linkedlist<threevector> const *turtle_list_ptr);
   contour* contour_surrounding_enclosed_region(
      double z_null,double z_boundary,double delta_s,
      const threevector& origin,twoDarray const *ztwoDarray_ptr);
   void turtle_erode_binary_region(
      unsigned int n_iters,int nsize,twoDarray* zbinary_twoDarray_ptr,
      double znull=0);

// Scan filling algorithms:

   twoDarray* mask_boundaryFill(
      double zfill,double znull,const threevector& origin,
      twoDarray const *ztwoDarray_ptr,int& npixels_filled);
   void mask_boundaryFill(
      double zfill,double znull,const threevector& origin,
      twoDarray const *ztwoDarray_ptr,twoDarray* zmask_twoDarray_ptr,
      int& npixels_filled);

   int basicfill(int seed_x,int seed_y,double zfill,
                 twoDarray* ztwoDarray_ptr);
   int basicfill(
      int seed_x,int seed_y,double zfill,twoDarray* ztwoDarray_ptr,
      int& min_px,int& max_px,int& min_py,int& max_py);
      
   void fill_line(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& lx,int& rx,
      int& npixels_filled);
   int fill_right(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& npixels_filled);
   int fill_left(
      unsigned int qx,unsigned int qy,
      const double old_value,const double new_value,
      twoDarray* ztwoDarray_ptr,int& npixels_filled);
   void scanhi(
      unsigned int qy,unsigned int lx,unsigned int rx,
      const double old_value,twoDarray* ztwoDarray_ptr,
      Stack<std::pair<int,int> >& pixel_stack);
   void scanlo(
      unsigned int qy,unsigned int lx,unsigned int rx,
      const double old_value,twoDarray* ztwoDarray_ptr,
      Stack<std::pair<int,int> >& pixel_stack);
   
// Connected components labeling methods

   typedef std::map<int,std::pair<int, int> > LABELS_MAP;

// Independent int:  CC ID

// Dependent int #1: Class ID
// Dependent int #2: Number of pixels with CC 

   void label_connected_components(
      int n_neighbors, int label_offset,const twoDarray* ptwoDarray_ptr,
      int curr_class_ID, LABELS_MAP& labels_map, 
      twoDarray* cc_labels_twoDarray_ptr);

   int Label_Connected_Components(
      int n_neighbors,int label_offset,double z_null,
      const twoDarray* pbinary_twoDarray_ptr,
      twoDarray* cc_labels_twoDarray_ptr);
   void lexicographical_four_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      std::vector<std::pair<unsigned int,unsigned int> >& 
      four_neighbor_pixels);
   void compute_four_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      std::vector<std::pair<int,int> >& four_neighbor_pixels);
   void compute_four_neighbor_IDs(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      std::vector<int>& four_neighbor_IDs);
   void lexicographical_eight_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      std::vector<std::pair<unsigned int,unsigned int> >& 
      eight_neighbor_pixels);
   void compute_eight_neighbor_pixels(
      unsigned int px,unsigned int py,unsigned int xdim,unsigned int ydim,
      std::vector<std::pair<unsigned int,unsigned int> >& 
      eight_neighbor_pixels);
   void compute_eight_neighbor_IDs(
      int px,int py,int xdim,int ydim,
      std::vector<int>& eight_neighbor_IDs);
   std::vector<int> run_length_encode(
      int cc_label,unsigned int px_start,unsigned int px_stop,
      unsigned int py_start,unsigned int py_stop,
      double threshold,const twoDarray* cc_twoDarray_ptr);

   int twopass_cc_labeling(
      int n_neighbors,int label_offset,double z_null,
      const twoDarray* pbinary_twoDarray_ptr,
      twoDarray* cc_labels_twoDarray_ptr);

// ==========================================================================
// Inlined methods:
// ==========================================================================

   inline int get_pixel_ID(unsigned int px,unsigned int py,int image_width)
   {
      return py*image_width+px;
   }

   inline void get_pixel_px_py(int ID,int image_width,
                               unsigned int& px,unsigned int& py)
   {
      py=ID/image_width;
      px=ID%image_width;
   }

}


#endif // graphicsfuncs.h




