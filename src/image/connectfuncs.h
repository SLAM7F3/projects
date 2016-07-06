// =========================================================================
// Header file for stand-alone connected components functions.
// =========================================================================
// Last modified on 8/5/06; 11/24/10; 12/1/10; 7/28/12
// =========================================================================

#ifndef CONNECTFUNCS_H
#define CONNECTFUNCS_H

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "math/constants.h"
#include "datastructures/datapoint.h"
#include "math/threevector.h"

template <class T> class Hashtable;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
template <class T> class Mynode;
typedef Mynode<datapoint> mynode;
class threevector;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace connectfunc
{

// Run length encoding/decoding & connected components methods:

   linkedlist* run_length_encode(const twoDarray* zbinary_twoDarray_ptr);
   void run_length_decode(
      const linkedlist* run_list_ptr,twoDarray* ztwoDarray_ptr);
   linkedlist* convert_run_to_pixel_list(
      const mynode* run_node_ptr,const twoDarray* ztwoDarray_ptr);

   void run_length_encode(
      const twoDarray* zbinary_twoDarray_ptr,linkedlist* RLE_list_ptr[]);
   int connect_runs(
      linkedlist* run_list_ptr,const twoDarray* zbinary_twoDarray_ptr,
      bool eight_connected=true);
   void count_runs(unsigned int ndim,linkedlist* RLE_list_ptr[],int& nruns);
   void connect_top_down_runs(
      int nruns,int row,int id[],int size[],
      linkedlist* RLE_list_ptr[],bool eight_connected);
   void connect_bottom_up_runs(
      int nruns,int row,int id[],int size[],
      linkedlist* RLE_list_ptr[],bool eight_connected);
   int relabel_runs(unsigned int nruns,int id[],linkedlist* run_list_ptr);
   Hashtable<linkedlist*>* generate_connected_hashtable(
      int n_connected_runs,unsigned int min_component_pixels,
      linkedlist* run_list_ptr,twoDarray const *ztwoDarray_ptr);
   Hashtable<linkedlist*>* generate_connected_hashtable(
      double zthreshold,int min_component_pixels,
      twoDarray const *ztwoDarray_ptr,bool threshold_below_cutoff=true);
   void decode_connected_hashtable(
      Hashtable<linkedlist*>* connected_hashtable_ptr,
      twoDarray* ptwoDarray_ptr,
      bool set_component_intensities_less_than_unity=false,
      double null_value=0);
   void delete_connected_hashtable(
      Hashtable<linkedlist*>* connected_hashtable_ptr);

// Largest connected component methods

   linkedlist* largest_connected_component(
      Hashtable<linkedlist*>* connected_hashtable_ptr);
   bool pixel_connected_to_component(
      unsigned int px,unsigned int py,twoDarray* zconnected_twoDarray_ptr);
   int aggregate_pixels_into_connected_component(
      const twoDarray* zorig_twoDarray_ptr,
      twoDarray* zconnected_twoDarray_ptr);
   void fill_connected_component_from_original_image(
      const twoDarray* zorig_twoDarray_ptr,
      twoDarray* zconnected_twoDarray_ptr);

// Quasi derivative methods:

   bool compute_local_pixel_intensity_variation(
      const linkedlist* pixel_list_ptr,
      const twoDarray* ztwoDarray_ptr,const twoDarray* zbinary_twoDarray_ptr,
      double& avg_global_delta_f);

// Pixel list properties methods:

   threevector pixel_list_COM(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      bool compute_binary_COM=true,bool use_pixel_coords=true);
   bool pixel_list_moi(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      const threevector& origin,double& Imin,double& Imax,
      threevector& Imin_hat,threevector& Imax_hat,
      bool compute_binary_moi=true);
   linkedlist* pixels_close_to_point(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      threevector& ref_point,double radius);
   void similar_height_pixels(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      const double ref_height,double height_tolerance,
      linkedlist* close_pixels_list_ptr);
   double pixel_list_height_percentile(
      double height_frac,Linkedlist<std::pair<int,int> > const 
      *pixel_list_ptr,twoDarray const *ztwoDarray_ptr);

// Pixel list display methods:

   void convert_pixel_list_to_twoDarray(
      const linkedlist* pixel_list_ptr,twoDarray* ftwoDarray_ptr);
   linkedlist* convert_twoDarray_to_pixel_list(
      double mask_value,twoDarray const *fmask_twoDarray_ptr);
   bool convert_pixel_list_to_binary_image(
      const linkedlist* pixel_list_ptr,twoDarray* zbinary_twoDarray_ptr);
   bool convert_pixel_list_to_image(
      const linkedlist* pixel_list_ptr,twoDarray* ztwoDarray_ptr);
   bool convert_pixel_list_to_image(
      const linkedlist* pixel_list_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr);

// Extremal Region pooled memory methods:

   void create_extremal_region_pooled_memory();
   void delete_extremal_region_pooled_memory();

}

#endif // connectfuncs.h




