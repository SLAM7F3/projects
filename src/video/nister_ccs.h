// ==========================================================================
// Header file for computing connected components tree via Nister's method
// ==========================================================================
// Last modified on 6/27/14; 6/28/14; 6/29/14; 6/30/14
// ==========================================================================

#ifndef NISTER_CCS_H
#define NISER_CCS_H

#include <string>
#include <vector>
#include "color/colortext.h"
#include "video/texture_rectangle.h"

class disjoint_set;

typedef struct{
  unsigned int x;
  unsigned int y;
  unsigned short val;
} feat_point_t;


typedef struct linked_point_t{
  int id;                      // id = x + y * image_width
  feat_point_t p;              // point info
  struct linked_point_t *prev; // link to previous point
  struct linked_point_t *next; // link to next point
} linked_point_t;

typedef struct{
  int id;			// unique integer index
  int class_id;			// Equivalence class ID (e.g. string IDs for 
				//    candidate text character ERs)
  int active_flag;
  int bright_foreground_flag;	// Boolean indicates bright extremal region 
				//    against dark background
  int stuff_flag;		// Boolean indicates if bbox contents has 
				//    been classified as "stuff"
  linked_point_t *head;       	// head of doubly-linked list of points in 
				//    the connected component (CC)
  linked_point_t *tail;       	// end of doubly-linked list of points in 
				//    the CC
  unsigned int grey_level;    	// the grey level of this merge activity
  unsigned int max_val;		// Maximum grey-scale value within CC
  unsigned int min_val;		// Minimum grey-scale value within CC
  unsigned int size;  		// size of this CC at this merge activity
  unsigned int prev_size;	// size of CC at previous merge activity
  unsigned int pixel_perim;	// One pixel has perim = 4; 2 adjacent pixels 
				//    have perim = 6; etc
  unsigned int bbox_min_px, bbox_max_px;	// Pixel coords for current 
						//    bounding box that snuggly
  unsigned int bbox_min_py, bbox_max_py;	//    wraps around CC
  unsigned int prev_bbox_min_px, prev_bbox_max_px;  // Pixel coords for prev 
						    // bbox that snuggly 
  unsigned int prev_bbox_min_py, prev_bbox_max_py;  // around previous CC

  double Euler_number;			// 1 - n_holes
  double foreground_mu_intensity;      	// Mean greyscale intensity within CC 
					//    (incrementally calculated)
  double foreground_sigma_intensity;	// Greyscale intensity std dev within 
					//    CC (incrementally calculated)
  double bbox_mu_intensity;		// Mean greyscale intensity for all 
					//    bbox pixels
  double bbox_sigma_intensity;		// Std dev of  greyscale intensities 
					//    for all bbox pixels
  double background_mu_intensity;      	// Mean greyscale intensity for all 
					// pixels inside bbox but not inside CC
  double background_sigma_intensity;	// Std dev of greyscale intensities 
					//    for pixels inside bbox
					//     but not inside CC
  double bbox_entropy;			// Image entropy for pixels contained 
					//    inside bbox
  float *quantized_color_hist;		// Histogram for RGB pixels inside 
					//    bbox over 33 quantized colors
  float *hog_descrip;			// HOG descriptor 

} connect_comp_t;


typedef struct {
  unsigned int min_cc_pixel_size;	// Ignore any extremal region with 
					//    n_pixels below this size
  unsigned int max_cc_pixel_size;	// Ignore any extremal region with 
					//    n_pixels above this size
  unsigned int min_horiz_pixel_size;	// Ignore any ER with horizontal bbox 
					//    dimension less than this size
  unsigned int max_horiz_pixel_size;	// Ignore any ER with horizontal bbox 
 					//    dimension greater than this size
  unsigned int min_vert_pixel_size;	// Ignore any ER with vertical bbox 
					//    dimension less than this size
  unsigned int max_vert_pixel_size;	// Ignore any ER with vertical bbox 
					//    dimension greater than this size
  double min_aspect_ratio;		// Ignore any ER whose horiz/vert bbox
					//   dimensions is less than this ratio
  double max_aspect_ratio;		// Ignore any ER whose horiz/vert bbox
					//  dimensions > this ratio
  unsigned int max_value_spread;	// Ignore any ER whose max - min 
					//  intensity vals exceeds this thresh
  float max_value_std_dev;		// Ignore any ER whose intensity std 
					//   dev exceeds this threshold
  double max_sqr_inverse_compactness;	// Ignore any ER whose sqr(pixel_perim)
					//   area exceeds this threshold
  unsigned int min_bbox_delta;		// Ignore ER if its bbox horiz or vert
					//   dim has not changed from its 
					//   progenitor bbox dims by at least 
					//   this number of pixels
  unsigned int n_params;		// Number of variable parameters 
					//   within this structure
} ER_incremental_param_t;


typedef struct {
  double min_bbox_sigma_intensity;	// Ignore any ER bbox whose intensity 
					//  std dev is less than this threshold
  double max_bbox_sigma_intensity;	// Ignore any ER bbox whose intensity 
					// std dev is greater than this thresh
  double min_bbox_area_ratio;		// Min threshold area ratio for nested
					//    bounding boxes
  double max_bbox_area_ratio;		// Max threshold area ratio for nested
					//    bounding boxes
  double min_entropy;			// Ignore any ER bbox whose intensity 
					//   entropy is less than this thresh
  double max_entropy;			// Ignore any ER bbox whose intensity
					// entropy is greater than this thresh
} ER_bbox_param_t;



typedef struct {
  int min_n_CCs;			// Min num of CCs for which "stuff vs 
					//   thing" classification should be 
					//   performed
  double image_height_search_frac;	// Fraction of image height used to 
					//   set height of query bbox in which
					//  search is perform for "stuff"
  double min_bbox_area_ratio;		// Min ratio of bbox areas for 
					//   candidate neighbor to query CC 
  double max_bbox_area_ratio;		// Max ratio of bbox areas for 
					//   candidate neighbor to query CC
  double max_delta_entropy;		// Max image entropy difference 
					//   between candidate neighbor and 
					//   query CC
  double max_sqrd_Mahalanobis_color_dist;  // Max sqrd Mahalanobis distance 
					// between neighbor & query CC color 
					// hists for both to be considered as 
					// belonging to same "stuff" 
					// equivalence class
  double sigma_color;  			// Width for sigmoid color function 
  double max_sqrd_Mahalanobis_hog_dist;  // Max sqrd Mahalanobis distance 
					//  between neighbor & query CC HOG 
					//  descrips for both to be considered
					//  belonging to same "stuff" 
					//   equivalence class
  double sigma_hog;  			// Width for sigmoid HOG function 

  double rel_color_weight;		// Relative weight of similar color 
					//   hists score in final stuff 
					//   computation
  double rel_hog_weight;		// Relative weight of similar HOG 
					//   descrips score in final stuff 
					//   computation
  int stuff_neighbors_threshold;	// Classify query bbox as "stuff" if 
					//   it has at least this many neighbor
					//    bboxes w similar descrip content
} ER_stuff_param_t;

typedef struct
{
  unsigned short *quantized_RGB_indices;
  unsigned int *index2RGB;
  float **sqrt_inv_bright_color_covar, **sqrt_inv_dark_color_covar;
  float **sqrt_inv_bright_HOG_covar, **sqrt_inv_dark_HOG_covar;
} ER_data_t;

typedef struct
{
  ER_incremental_param_t ER_incremental_params;
  ER_bbox_param_t ER_bbox_params;
  ER_stuff_param_t ER_stuff_params;
  ER_data_t ER_data;
} ER_model_t;

class nister_ccs
{

  public:

   nister_ccs();
   nister_ccs(const nister_ccs& cc);
   ~nister_ccs();
   nister_ccs& operator= (const nister_ccs& cc);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const nister_ccs& cc);

// Set and get methods:

   twoDarray* get_ptwoDarray_ptr();
   const twoDarray* get_ptwoDarray_ptr() const;

// Initialization member functions:

// Nister-Stewenius linear time MSER methods:

   void calculate_extremal_regions(
//   p_image *im, connect_comp_list_t *cc_list, 
     ER_model_t *ER_model, 
      int bright_foreground_flag);

// Neumann-Matas incremental ER methods:

   bool incrementally_test_ER(connect_comp_t* cc,ER_model_t *ER_model);

// Test image and debugging tool methods:

   void instantiate_test_image();
   void set_text_colors(std::vector<Color::Modifier>& text_colors);

  private: 

   texture_rectangle *texture_rectangle_ptr;
   twoDarray* binary_mask_twoDarray_ptr;
   twoDarray* cc_twoDarray_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const nister_ccs& cc);


   void histogram_greyscale_values(unsigned int *grey_count);
   void initialize_boundary_heap(
      unsigned int grey_count[], feat_point_t **boundary_heap);
   void pre_initialize_cmp(connect_comp_t *cmp);
   void initialize_cmp(
      connect_comp_t *cmp, feat_point_t *source, 
      int bright_foreground_flag,
      unsigned int xsize, unsigned int ysize);

   bool find_source(
     twoDarray* binary_mask_twoDarray_ptr, feat_point_t *source);

   void append_pixel_to_cmp(
     connect_comp_t *cmp, linked_point_t *pts, disjoint_set& union_find,
     int xsize, int ysize);
   int process_cmp(
      connect_comp_t **cmp,feat_point_t source,int test_image_flag, 
      std::vector<connect_comp_t*>& cc_list, disjoint_set& union_find,
      ER_model_t *ER_model);
   void merge_regions(
      connect_comp_t *comp1, connect_comp_t *comp2, connect_comp_t *comp,
      disjoint_set& union_find);
   void print_CC(connect_comp_t* cc);
   void print_array_values(
      twoDarray* ptwoDarray_ptr,twoDarray* cc_twoDarray_ptr,
      std::string image_label);



   void update_CC_metadata(connect_comp_t* cc);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline twoDarray* nister_ccs::get_ptwoDarray_ptr()
{
   return texture_rectangle_ptr->get_ptwoDarray_ptr();
}

inline const twoDarray* nister_ccs::get_ptwoDarray_ptr() const
{
   return texture_rectangle_ptr->get_ptwoDarray_ptr();
}


#endif  // nister_ccs.h
