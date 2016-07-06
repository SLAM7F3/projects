// ==========================================================================
// Header file for connected_components class
// ==========================================================================
// Last modified on 1/24/13; 9/27/13; 4/3/14; 4/18/16
// ==========================================================================

#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H

#include <map>
#include <string>
#include "image/extremal_region.h"
#include "datastructures/Forest.h"
#include "video/texture_rectangle.h"
#include "datastructures/tree.h"
#include "image/TwoDarray.h"
#include "datastructures/vector_union_find.h"

class connected_components
{

  public:

   typedef tree<extremal_region*>* TREE_PTR;
   typedef treenode<extremal_region*>* TREENODE_PTR;

   typedef std::pair<int,int> INT_PAIR;
   typedef std::vector<INT_PAIR> CHANGED_PIXEL_COORDS;
   typedef std::map<int,CHANGED_PIXEL_COORDS> CHANGED_PIXELS_MAP;

   typedef std::map<int,int> LABELS_MAP;
// independent int var: root node ID
// dependent int var: connected component label

   typedef std::map<int,TREENODE_PTR> TREENODES_MAP;
// independent int var: treenode ID
// dependent int var: treenode_ptr

   typedef std::map<int,int> PREV_CURR_PIXELS_MAP;
// independent int var: prev_node_ID
// dependent int var: curr_node_ID

//   const int K_shapes=11;
   typedef dlib::matrix<double, 11, 1> SHAPES_SAMPLE_TYPE;
//   const int K_shapes=10;
//   typedef dlib::matrix<double, 10, 1> SHAPES_SAMPLE_TYPE;
   typedef dlib::radial_basis_kernel<SHAPES_SAMPLE_TYPE> SHAPES_KERNEL_TYPE;
   typedef dlib::probabilistic_decision_function<SHAPES_KERNEL_TYPE> 
      SHAPES_PROBABILISTIC_FUNCT_TYPE;  
   typedef dlib::normalized_function<SHAPES_PROBABILISTIC_FUNCT_TYPE> 
      SHAPES_PFUNCT_TYPE;

   connected_components();
   connected_components(const connected_components& cc);
   ~connected_components();
   connected_components& operator= (const connected_components& cc);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const connected_components& cc);

// Set and get methods:

   TREE_PTR get_tree_ptr()
   {
      return tree_ptr;
   }
   TREENODES_MAP* get_treenodes_map_ptr()
   {
      return prev_treenodes_map_ptr;
   }

   twoDarray* get_ptwoDarray_ptr();
   const twoDarray* get_ptwoDarray_ptr() const;
   twoDarray* get_pbinary_twoDarray_ptr();
   const twoDarray* get_pbinary_twoDarray_ptr() const;
   twoDarray* get_cc_twoDarray_ptr();
   const twoDarray* get_cc_twoDarray_ptr() const;

   void set_min_n_connected_components(int n);
   void set_max_n_connected_components(int n);
   void set_min_fill_frac(double f);
   void set_max_fill_frac(double f);
   std::string get_cc_subdir() const;

   vector_union_find* get_vector_union_find_ptr();
   const vector_union_find* get_vector_union_find_ptr() const;

   int get_n_extremal_regions() const;
   extremal_region* get_extremal_region_ptr(int n);
   const extremal_region* get_extremal_region_ptr(int n) const;

//    void set_shapes_pfunct_ptr(SHAPES_PFUNCT_TYPE* shapes_pfunct_ptr);
   void set_shapes_pfuncts_ptrs(
      std::vector<SHAPES_PFUNCT_TYPE*>* shapes_pfuncts_ptrs);

   void set_cc_subdir(bool invert_binary_values_flag);

// Initialization member functions:

   void reset_image(
      std::string image_filename,int color_channel_ID=-1,int image_counter=-1);
   void delete_texture_rectangle_ptr();
   void reset_texture_rectangle_ptr(texture_rectangle* tr_ptr);

// TwoDarray printing & copying member functions:

   void print_twoDarray(std::string twoDarray_name,twoDarray* ztwoDarray_ptr);
   void print_pbinary_twoDarray();
   void print_prev_cc_twoDarray();
   void print_cc_twoDarray();
   void print_rootnode_twoDarray();
   void print_changed_pixels_map(CHANGED_PIXELS_MAP* pixels_map_ptr);

// Connected component computation member functions:

   int compute_connected_components(int threshold,int min_n_pixels);
   int compute_connected_components(
      int threshold,bool invert_binary_values_flag=false,
      bool export_connected_regions_flag=false);
   int compute_connected_components(
      int index,int threshold,int level,bool RLE_flag,
      bool invert_binary_values_flag=false,
      bool export_connected_regions_flag=false,
      bool retain_only_largest_cc_flag=false);
   int new_compute_connected_components(
      int index,int threshold,int level,bool invert_binary_values_flag=false);
   int ancient_label_ccs();
   int label_ccs();
   void build_extremal_regions_tree(int level);
   void update_treenodes_maps();
   void compute_text_shape_probs(
      int level,double shapes_prob_threshold,
      unsigned int start_object_ID,unsigned int stop_object_ID,
      bool tight_skew_quartic_thresholds_flag=true);
   void RLE_extremal_region(extremal_region* extremal_region_ptr);
   std::vector<extremal_region*> select_extremal_regions(
      int level,double min_aspect_ratio,double max_aspect_ratio,
      double min_compactness,double max_compactness,
      int min_n_holes,int max_n_holes,
      int min_n_crossings,int max_n_crossings);
      
// Connected component export member functions:

   int export_connected_components(
      int index,int threshold,int level,bool invert_binary_values_flag=false);
   void color_connected_components(
      std::string output_image_filename,bool uniform_color_flag=false);
   void export_individual_connected_components(
      std::string cc_subdir,int level);
   unsigned int fill_labels_map();
   int count_all_cc_pixels();
   int purge_small_ccs(int min_n_pixels);
   int retain_only_largest_cc();

// Connected component shape descriptor member functions:

   void compute_shape_descriptors_recursively(int level,bool RLE_flag);
   int compute_median_horiz_crossing(
      int cc_ID,extremal_region* extremal_region_ptr);
   int count_horizontal_crossings(int py,int min_px,int max_px,int cc_label);
   void compute_shape_descriptors(int level);

// Connected component randomness measure member functions:

   bool compute_interior_exterior_std_devs(
      double& sigma_all,double& sigma_interior,double& sigma_exterior);
   bool compute_interior_exterior_entropies(
      double& entropy_all,double& entropy_interior,double& entropy_exterior);

// Extremal region tree traversal member functions:

   std::vector<TREENODE_PTR> identify_stable_extremal_regions(int object_ID);

// Component tree computation member functions:

   void initialize_vector_union_find();
   void mini_recompute_connected_components();
   void recompute_connected_components();
   void update_connected_component_properties();
   int update_connected_component_labels();
   void update_changed_pixels_map();

// Temporary member functions for testing only

   void instantiate_binary_image(int width,int height);
   void set_binary_image_first();
   void set_binary_image_second();
   void set_binary_image_third();
   void set_binary_image_fourth();
   void copy_cc_onto_prev_cc();

  private: 

   unsigned int width,height;
   int cc_label_offset;
   int min_n_connected_components,max_n_connected_components;
   double min_fill_frac,max_fill_frac;

   CHANGED_PIXEL_COORDS curr_changed_pixel_coords;
   INT_PAIR curr_int_pair;
   std::vector<INT_PAIR> four_horiz_neighbors,four_vert_neighbors;
   CHANGED_PIXELS_MAP *changed_pixels_map_ptr,*updated_changed_pixels_map_ptr;
   CHANGED_PIXELS_MAP::iterator changed_pixel_iter;

   std::string cc_subdir;
   texture_rectangle *texture_rectangle_ptr,*cc_texture_rectangle_ptr;
   twoDarray *pbinary_twoDarray_ptr;
   twoDarray *cc_twoDarray_ptr,*prev_cc_twoDarray_ptr,*visited_twoDarray_ptr;
   TREE_PTR tree_ptr;

   vector_union_find* vector_union_find_ptr;
   LABELS_MAP* labels_map_ptr;
   LABELS_MAP::iterator label_iter;

   TREENODES_MAP* curr_treenodes_map_ptr,*prev_treenodes_map_ptr;
   TREENODES_MAP::iterator treenode_iter;

   PREV_CURR_PIXELS_MAP* prev_curr_pixels_map_ptr;
   PREV_CURR_PIXELS_MAP::iterator prev_curr_iter;

   std::vector<extremal_region*> extremal_regions_vector;
   std::vector<int> neighbor_IDs;

//   SHAPES_PFUNCT_TYPE* shapes_pfunct_ptr;
   std::vector<SHAPES_PFUNCT_TYPE*>* shapes_pfuncts_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const connected_components& cc);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline twoDarray* connected_components::get_ptwoDarray_ptr()
{
   return texture_rectangle_ptr->get_ptwoDarray_ptr();
}

inline const twoDarray* connected_components::get_ptwoDarray_ptr() const
{
   return texture_rectangle_ptr->get_ptwoDarray_ptr();
}

inline twoDarray* connected_components::get_pbinary_twoDarray_ptr()
{
   return pbinary_twoDarray_ptr;
}

inline const twoDarray* connected_components::get_pbinary_twoDarray_ptr() const
{
   return pbinary_twoDarray_ptr;
}

inline twoDarray* connected_components::get_cc_twoDarray_ptr()
{
   return cc_twoDarray_ptr;
}

inline const twoDarray* connected_components::get_cc_twoDarray_ptr() const
{
   return cc_twoDarray_ptr;
}

inline void connected_components::set_min_n_connected_components(int n)
{
   min_n_connected_components=n;
}

inline void connected_components::set_max_n_connected_components(int n)
{
   max_n_connected_components=n;
}

inline void connected_components::set_min_fill_frac(double f)
{
   min_fill_frac=f;
}

inline void connected_components::set_max_fill_frac(double f)
{
   max_fill_frac=f;
}

inline std::string connected_components::get_cc_subdir() const
{
   return cc_subdir;
}

inline vector_union_find* connected_components::get_vector_union_find_ptr()
{
   return vector_union_find_ptr;
}

inline const vector_union_find* 
connected_components::get_vector_union_find_ptr() const
{
   return vector_union_find_ptr;
}

/*
inline void connected_components::set_shapes_pfunct_ptr(
   SHAPES_PFUNCT_TYPE* shapes_pfunct_ptr)
{
   this->shapes_pfunct_ptr=shapes_pfunct_ptr;
}
*/

inline void connected_components::set_shapes_pfuncts_ptrs(
   std::vector<SHAPES_PFUNCT_TYPE*>* shapes_pfuncts_ptrs)
{
   this->shapes_pfuncts_ptrs=shapes_pfuncts_ptrs;
}



#endif  // connected_components.h
