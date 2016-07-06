// =========================================================================
// Nister_ccs class member function definitions
// =========================================================================
// Last modified on 6/27/14; 6/28/14; 6/29/14; 6/30/14
// =========================================================================

#include <iostream>

#include "datastructures/disjoint_set.h"
#include "math/mathfuncs.h"
#include "video/nister_ccs.h"
#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void nister_ccs::allocate_member_objects()
{
//   cout << "inside nister_ccs::allocate_member_objects()" 
//        << endl;
}

void nister_ccs::initialize_member_objects()
{

}

// ---------------------------------------------------------------------
nister_ccs::nister_ccs()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

nister_ccs::nister_ccs(const nister_ccs& cc)
{
   docopy(cc);
}

nister_ccs::~nister_ccs()
{
//   cout << "inside nister_ccs destructor" << endl;

}

// ---------------------------------------------------------------------
void nister_ccs::docopy(const nister_ccs& cc)
{
}

// Overload = operator:

nister_ccs& nister_ccs::operator= (const nister_ccs& cc)
{
   if (this==&cc) return *this;
   docopy(cc);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

   ostream& operator<< (ostream& outstream,const nister_ccs& cc)
   {
      outstream << endl;
      return outstream;
   }

// =========================================================================
// Set & get member functions
// =========================================================================


// =========================================================================
// Nister-Stewenius linear time MSER methods
// =========================================================================

// Member function calculate_extremal_regions() implements speeded up
// version of connected components tree and MSER extraction algorithm
// described within "Linear Time Maximally Stable Extremal Regions" by
// David Nister and Henrik Stewenius, ECCV 2008.

void nister_ccs::calculate_extremal_regions(
//   p_image *im, connect_comp_list_t *cc_list, 
   ER_model_t *ER_model, 
   int bright_foreground_flag)
{
   int visited_pixel_value = 1;
   bool test_image_flag = true;
// bool test_image_flag = false;

   int x, y, nx, ny, i;
   unsigned int width = get_ptwoDarray_ptr()->get_xdim();
   unsigned int height = get_ptwoDarray_ptr()->get_ydim();

   int restart_counter = 0;
   int n_pixel_neighbors = 8;	// 8-connectivity
//  int n_pixel_neighbors = 4;	// 4-connectivity

  feat_point_t source, neigh_p;
  int heap_not_empty = 1; 

  unsigned int grey_count[256]; 
  for (unsigned int i=0; i<256; i++)
  {
     grey_count[i]=0;
  }

//   Initialize all entries in grey-scale histogram to 0
  int neigh[16] = {-1, 0, 1, 0, 0, -1, 0, 1, -1, -1, 1, -1, -1, 1, 1, 1};

  twoDarray* binary_mask_twoDarray_ptr=new twoDarray(width,height);
//   Binary mask of visited pixels ( = pixels to which water has already
//   had access

  twoDarray* cc_twoDarray_ptr=new twoDarray(width,height);

// Instantiate stack to hold connected component information.  Maximum
// number of entries on the stack equals number of grey-levels:

  connect_comp_t *cmp = static_cast<connect_comp_t*>(
     calloc(257, sizeof(connect_comp_t)) );

  feat_point_t** boundary_heap = (feat_point_t **) malloc(
     256*sizeof(feat_point_t *) + 
     (width*height+256)*sizeof(feat_point_t));  // priority queue of
					 	// boundary pixels

  int* boundary_marker = static_cast<int*> (
     calloc(256 / (sizeof(int) * 8), sizeof(int))); // marker for boundary heap

  linked_point_t* pts = static_cast<linked_point_t*>(
     calloc(width*height+10, sizeof(linked_point_t)));
  linked_point_t* pts_start = pts;

  disjoint_set union_find(width*height);

  if (test_image_flag)
  {
     string label="Original image";

// Work with small test image for extremal region development purposes:

     instantiate_test_image();

     width = get_ptwoDarray_ptr()->get_xdim();
     height = get_ptwoDarray_ptr()->get_ydim();
     cc_twoDarray_ptr=new twoDarray(width,height);
     cc_twoDarray_ptr->clear_values();

     print_array_values(get_ptwoDarray_ptr(), NULL, label);
  }

  histogram_greyscale_values(grey_count);
  initialize_boundary_heap(grey_count,boundary_heap);

// Push dummy-component onto CC stack with grey-level higher than any
// allowed in image:

  pre_initialize_cmp(cmp);
  cmp->grey_level = 999;

  while (find_source(binary_mask_twoDarray_ptr, &source))
  {
     heap_not_empty = 1;
     restart_counter++;
     cout << "iteration " << restart_counter 
          << " : Found source point at pixel location (" 
          << source.x << " , " << source.y << " ) with value = " 
          << source.val << endl; 
  } // find_source conditional

  free(boundary_heap);
  free(boundary_marker);
  free(pts_start);

  delete binary_mask_twoDarray_ptr;
  delete cc_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function histogram_greyscale_values() generates a histogram
// for grey-scales values from an input grey-level image.

void nister_ccs::histogram_greyscale_values(unsigned int *grey_count)
{
   unsigned int width = get_ptwoDarray_ptr()->get_xdim();
   unsigned int height = get_ptwoDarray_ptr()->get_ydim();

   for (unsigned py=0; py<height; py++)
   {
      for (unsigned px=0; px<width; px++)
      {
         unsigned int curr_val=get_ptwoDarray_ptr()->get(px,py);
         (grey_count)[curr_val]++;
      }
   }
}

// ---------------------------------------------------------------------
// Member function initialize_boundary_heap().  A boundary heap's
// independent variable ranges from 0 - 255.  Its dependent 1D stacks
// initially have sizes equal to one plus the number of grey scale
// values at each threshold setting.  The 0th entry in each 1D stack
// holds the current size of the 1D array.  So this method assigns
// boundary_heap memory space according to the input grey_count
// histogram.
 
void nister_ccs::initialize_boundary_heap(
   unsigned int grey_count[], feat_point_t **boundary_heap)
{
   feat_point_t *buf = (feat_point_t *)(boundary_heap + 256);
   int accum = 0;

   for (int i = 0; i < 256; i++)
   {
      boundary_heap[i] = buf + accum;
      boundary_heap[i][0].x = 1;
      boundary_heap[i][0].y = 0;
      boundary_heap[i][0].val = 0;

// Recall each 1D stack within the boundary heap contains its length
// within its zeroth element.  So we need to increment accum by
// grey[count] + 1 and not just by grey[count].  See figure 8 in Nister paper.

      accum += grey_count[i] + 1;
   }
}

// ---------------------------------------------------------------------
void nister_ccs::pre_initialize_cmp(connect_comp_t *cmp)
{
   cmp->id = -1;
   cmp->class_id = -1;
   cmp->active_flag = 1;
   cmp->stuff_flag = 0;	// Initially ignorant whether component corresponds 
			// to "stuff" or not

   cmp->head = NULL;
   cmp->tail = NULL;
   cmp->size = 0;
   cmp->prev_size = 0;
   cmp->pixel_perim = 0;
//   cmp->Euler_number = 0;	// n_8_ccs - n_4_holes = 0 - 0 = 0

   cmp->prev_bbox_min_px = -1;
   cmp->prev_bbox_max_px = -1;
   cmp->prev_bbox_min_py = -1;
   cmp->prev_bbox_max_py = -1;
   cmp->bbox_mu_intensity = 0;
   cmp->bbox_sigma_intensity = 0;
   cmp->bbox_entropy = -1;

   cmp->quantized_color_hist = NULL;
   cmp->hog_descrip = NULL;
}

// ---------------------------------------------------------------------
void nister_ccs::initialize_cmp(
   connect_comp_t *cmp, feat_point_t *source, 
   int bright_foreground_flag,
   unsigned int xsize, unsigned int ysize)
{
   pre_initialize_cmp(cmp);

   cmp->bright_foreground_flag = bright_foreground_flag;
   cmp->grey_level = source->val;
   cmp->max_val = source->val;
   cmp->min_val = source->val;
   cmp->foreground_mu_intensity = source->val;
   cmp->foreground_sigma_intensity = 0;
   cmp->bbox_min_px = basic_math::max(Unsigned_Zero, source->x);
   cmp->bbox_max_px = basic_math::min(xsize - 1, source->x + 1);
   cmp->bbox_min_py = basic_math::max(Unsigned_Zero, source->y);
   cmp->bbox_max_py = basic_math::min(ysize - 1, source->y + 1);
}

// ---------------------------------------------------------------------
// Member function find_source() scans over *binary_mask_twoDarray_ptr
// for its first non-zero entry.  It returns that entry as a "source"
// point.

bool nister_ccs::find_source(
   twoDarray* binary_mask_twoDarray_ptr, feat_point_t *source)
{
   for (unsigned int j = 0; j < binary_mask_twoDarray_ptr->get_ydim(); j++ )
   {
      for (unsigned int i = 0; i < binary_mask_twoDarray_ptr->get_xdim(); i++ )
      {
         int value = binary_mask_twoDarray_ptr->get(i,j);
         if (value != 0)
         {
            source->x = i;
            source->y = j;
            source->val = get_ptwoDarray_ptr()->get(i,j);
            return true;
         }
      } // loop over index i 
   } // loop over index j 
   return false;
}


// ---------------------------------------------------------------------
// Member function append_pixel_to_cmp() adds input linked-point to
// top component of CC stack.  Also updates several incremental
// parameters for top CC.

// Input  : cmp	    		Top connected component on CC stack
//          pts     		Current linked point to be appended to 
// 				   CC's doubly linked list
// 	    union_find
//	    image_width		Measured in pixels

void nister_ccs::append_pixel_to_cmp(
   connect_comp_t *cmp, linked_point_t *pts, disjoint_set& union_find,
   int xsize, int ysize)
{
   int neighbor_values[9];
   for (int i=0; i<9; i++)
   {
      neighbor_values[i]=0;
   }
   

			// initialize all neighbor values to zero
// Don't waste time appending input pixel onto input CC if latter is inactive:

   if (cmp->active_flag == 0) return;

// First ensure new point *pts and connect component *cmp belong to
// same union_find equivalence class:

   if (cmp->id < 0){
      cmp->id = pts->id;
   }
   else{
      union_find.link(cmp->id, pts->id);
   }

//  log_verbose("inside append_pixel_to_cmp() \n");
//  log_verbose("cmp->id = %d \n", cmp->id);

   if (cmp->size == 0){
      cmp->head = cmp->tail = pts;
      pts->prev = NULL;
      pts->next = NULL;
   }
   else {
      cmp->tail->next = pts;
      pts->prev = cmp->tail;
      pts->next = NULL;
      cmp->tail = pts;
   }

   unsigned int px = pts->p.x;
   unsigned int py = pts->p.y;
   unsigned int val = pts->p.val;

// Update connected component's metadata fields following addition of pts:

   cmp->max_val = basic_math::max(cmp->max_val, val);
   cmp->min_val = basic_math::min(cmp->min_val, val);
   cmp->foreground_sigma_intensity = 
      mathfunc::incremental_std_dev(
         cmp->size, (float) val, cmp->foreground_mu_intensity, 
         cmp->foreground_sigma_intensity);
   cmp->foreground_mu_intensity = mathfunc::incremental_mean(
      cmp->size, (float) val, cmp->foreground_mu_intensity);

// Count input pixel's neighbors which also belong to *cmp:
  
//  log_verbose(" px = %d  py = %d  pixel_val = %d  pts->id = %d \n", px, py, pts->p.val, pts->id);

   int root_cmp = union_find.find(cmp->id);

   int qx = px - 1;
   int qy = py - 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[0] = 1;
   }

   qx = px;
   qy = py - 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[1] = 1;
   }

   qx = px + 1;
   qy = py - 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[2] = 1;
   }

   qx = px - 1;
   qy = py;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[3] = 1;
   }
  
   qx = px + 1;
   qy = py;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[5] = 1;
   }

   qx = px - 1;
   qy = py + 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[6] = 1;
   }

   qx = px;
   qy = py + 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[7] = 1;
   }

   qx = px + 1;
   qy = py + 1;
   if (qx >= 0 && qx < xsize && qy >= 0 && qy < ysize)
   {
      int neighbor_id = qx + xsize * qy;
      int root_neighbor = union_find.find(neighbor_id);
      if (root_neighbor == root_cmp) neighbor_values[8] = 1;
   }

   int n_pixel_four_neighbors = neighbor_values[1] + neighbor_values[3] + 
      neighbor_values[5] + neighbor_values[7];
   cmp->pixel_perim += 4 - 2 * n_pixel_four_neighbors;

//   int pfill = 1;
//   cmp->Euler_number += p_delta_Euler_for_pixel(px, py, pfill, neighbor_values);

   cmp->bbox_min_px = basic_math::min(cmp->bbox_min_px, px);
   cmp->bbox_max_px = basic_math::max(cmp->bbox_max_px, px);
   cmp->bbox_min_py = basic_math::min(cmp->bbox_min_py, py);
   cmp->bbox_max_py = basic_math::max(cmp->bbox_max_py, py);

   cmp->bbox_sigma_intensity = mathfunc::incremental_std_dev(
      cmp->size, val, cmp->bbox_mu_intensity, cmp->bbox_sigma_intensity);
   cmp->bbox_mu_intensity = mathfunc::incremental_mean(
      cmp->size, val, cmp->bbox_mu_intensity);

   cmp->size++;
}

// ---------------------------------------------------------------------
// Member function process_cmp() implements the "ProcessStack" subroutine in 
// paper "Linear time maximally stable extremal regions" by Nister and
// Stewenius, ECCV 2008.  If input ER_model pointer is non-null,
// Neumann-Matas incremental tests for extremal regions are performed.

// Input  : cmp	    		   Pointer to CC stack's top element
//          source		   Latest point popped from boundary heap
//          im			   Original input image
//	    test_image_flag   	   1 if working with small test image, 
// 				      0 otherwise
//	    ER_model		   Structure holding extremal region data 
// 				      arrays and parameters
//
// Output:  cc_list	   	   Dynamic array holding extremal regions 
//	    return val		   If -1, extremal region & all its subsequent
//				      descendants can be ignored 

int nister_ccs::process_cmp(
   connect_comp_t **cmp,feat_point_t source,int test_image_flag, 
   vector<connect_comp_t*>& cc_list, disjoint_set& union_find,
   ER_model_t *ER_model)
{
   int extremal_region_flag = 0;

   do
   {

// First process component on top of CC stack:

     if (test_image_flag) print_CC(*cmp);

      if (ER_model == NULL)
      {
         extremal_region_flag = 1;
      }
      else
      {
	extremal_region_flag = incrementally_test_ER(*cmp,ER_model);
      }
    
      if (extremal_region_flag == 1) cc_list.push_back(*cmp);

// Check source point's grey value with 2nd CC's on stack. Recall
// CC stack always has a "dummy" component at its bottom.
// Even if CC stack currently contains just one genuine
// component as its "top" element, cmp should always contain at least
// 2 elements.  So (*cmp)[-1] should always refer to the connected
// component below (*cmp)[0] = *cmp :

      if (source.val < (*cmp)[-1].grey_level)
      {

         // Update current component's grey level and then return from 
	 // subroutine:
         (*cmp)->grey_level = source.val;
         break;
      }
      else 
      {
         // MERGE top two components on CC stack:
         merge_regions( (*cmp), (*cmp)-1, (*cmp)-1, union_find);
         (*cmp)--;
      }
   }
   while(source.val > (*cmp)->grey_level);

   return extremal_region_flag;
}

// ---------------------------------------------------------------------
// Member function merge_regions() combines together double-linked
// lists of points within two iput connected components.  It also
// updates combined component's metadata (e.g. size, perimeter,
// bounding box, etc).

 // Inputs: comp1    	Top of CC stack
 //	    comp2     	Next-to-top of CC stack
 //	    union_find	Disjoint set that keeps track of relationships between
 //			pixels & connected components

 // Output: comp	comp1 and comp2 combined together

void nister_ccs::merge_regions(
   connect_comp_t *comp1, connect_comp_t *comp2, connect_comp_t *comp,
   disjoint_set& union_find)
{
   int root_comp;

// Don't waste time merging comp1 with comp2 if both are empty or
// either is inactive:

   if (comp1->id < 0 && comp2->id < 0) return;
   if (comp1->active_flag == 0 || comp2->active_flag == 0) return;

   if (comp1->id < 0)
   {
      root_comp = comp2->id;
   }
   else if (comp2->id < 0)
   {
      root_comp = comp1->id;
   }
   else
   {
      root_comp = union_find.link(comp1->id, comp2->id);
   }

// Select the winner by size

   linked_point_t* head;
   linked_point_t* tail;
   if ( comp1->size >= comp2->size)
   {
      if ( comp1->size > 0 && comp2->size > 0)
      {
         comp1->tail->next = comp2->head;
         comp2->head->prev = comp1->tail;
      }
      head = ( comp1->size > 0 ) ? comp1->head : comp2->head;
      tail = ( comp2->size > 0 ) ? comp2->tail : comp1->tail;
   } 
   else 
   {
      if ( comp1->size > 0 && comp2->size > 0 )
      {
         comp2->tail->next = comp1->head;
         comp1->head->prev = comp2->tail;
      }
      head = ( comp2->size > 0 ) ? comp2->head : comp1->head;
      tail = ( comp1->size > 0 ) ? comp1->tail : comp2->tail;
   }
   comp->head = head;
   comp->tail = tail;

   comp->id = root_comp;
   comp->grey_level = comp2->grey_level;
   comp->max_val = basic_math::max(comp1->max_val, comp2->max_val);
   comp->min_val = basic_math::min(comp1->min_val, comp2->min_val);
   comp->size = comp1->size + comp2->size;
   comp->prev_size = basic_math::max(comp1->size, comp2->size);

   comp->bbox_min_px = basic_math::min(comp1->bbox_min_px, comp2->bbox_min_px);
   comp->bbox_max_px = basic_math::max(comp1->bbox_max_px, comp2->bbox_max_px);
   comp->bbox_min_py = basic_math::min(comp1->bbox_min_py, comp2->bbox_min_py);
   comp->bbox_max_py = basic_math::max(comp1->bbox_max_py, comp2->bbox_max_py);

// We have empirically observed that comp1 and comp2 have no intersection
// in terms of vertices, edges and pixels.  So the merged connected
// component's pixel, Euler number and area simply equals the sum
// of those for components 1 and 2:

   comp->pixel_perim = comp1->pixel_perim + comp2->pixel_perim;
//   comp->Euler_number = comp1->Euler_number + comp2->Euler_number;

   comp->foreground_mu_intensity = mathfunc::combined_mean(
      comp1->size, comp2->size, comp1->foreground_mu_intensity, 
      comp2->foreground_mu_intensity);
   comp->foreground_sigma_intensity = mathfunc::combined_std_dev(
      comp1->size, comp2->size, comp1->foreground_mu_intensity, 
      comp2->foreground_mu_intensity,
      comp1->foreground_sigma_intensity, 
      comp2->foreground_sigma_intensity);
}

// ---------------------------------------------------------------------
// Member function print_CC () prints greyscale values within input
// image in array format.  It also prints current connected component
// in array format.  This method is useful for debugging Nister's
// algorithm.

void nister_ccs::print_CC(connect_comp_t* cc)
{
   unsigned int width = get_ptwoDarray_ptr()->get_xdim();
   unsigned int height = get_ptwoDarray_ptr()->get_ydim();
   int pixel_counter = 1;
   linked_point_t *curr_linked_pt = cc->head;
   twoDarray* curr_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* cc_twoDarray_ptr=new twoDarray(width,height);
   
   curr_twoDarray_ptr->initialize_values(255);	// dummy value

   while (curr_linked_pt != NULL)
   {
//      cout << "pixel_counter = " << pixel_counter
//           << "  px = " << curr_linked_pt->p.x
//           << "  py = " << curr_linked_pt->p.y
//           << " grey_value = " << curr_linked_pt->p.val
//           << endl;

      curr_twoDarray_ptr->put(curr_linked_pt->p.x,curr_linked_pt->p.y,
                              curr_linked_pt->p.val);
      cc_twoDarray_ptr->put(curr_linked_pt->p.x,curr_linked_pt->p.y,0);
      pixel_counter++;
      curr_linked_pt = curr_linked_pt->next;
   }

   string label="Original image";
   print_array_values(get_ptwoDarray_ptr(), NULL, label);

   cout << "CC ID = " <<  cc->id << endl;
   cout << "CC grey_level = " << cc->grey_level << endl;
   cout << "CC size = " << cc->size << endl;
   cout << "CC perim = " << cc->pixel_perim << endl;
//   cout << "CC Euler number = " << cc->Euler_number << endl;
   label = "Current connected component:";
   print_array_values(curr_twoDarray_ptr, cc_twoDarray_ptr, label);
   cout << "---------------------------------------- " << endl;

   delete curr_twoDarray_ptr;
   delete cc_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function print_array_values() takes in a greyscale image
// with possible "dummy" entries equaling 255.  Dummy entries are
// rendered as periods.  Non-dummy entries are colored according to
// their corresponding values within input cc_array.  If cc_array =
// NULL, the text output from this method is simply black.

void nister_ccs::print_array_values(
   twoDarray* ptwoDarray_ptr, twoDarray* cc_twoDarray_ptr, string image_label)
{
   unsigned int width = ptwoDarray_ptr->get_xdim();
   unsigned int height = ptwoDarray_ptr->get_ydim();

   vector<Color::Modifier> text_colors;
   set_text_colors(text_colors);

   cout << image_label << endl;
   for (unsigned int py = 0; py < height; py++)
   {
      for (unsigned int px = 0; px < width; px++)
      {
         unsigned int curr_p = ptwoDarray_ptr->get(px,py);
         int curr_color = -1;
         if (cc_twoDarray_ptr != NULL)
         {
            int curr_cc=cc_twoDarray_ptr->get(px,py);
            curr_color=curr_cc % text_colors.size();
         }
         
         if (curr_p == 255)
         {
            cout << ". ";
         }
         else 
         {
            if (curr_color < 0)
            {
               cout << curr_p << " ";
            }
            else 
            {
               cout << text_colors[curr_color];
               cout << curr_p;
               cout << Color::FG_DEFAULT;
            }
         }
      } // loop over px
      cout << endl;
   } // loop over py
   cout << endl;
}

// =========================================================================
// Neumann-Matas incremental ER methods
// =========================================================================

// Member function incrementally_test_ER() performs several tests of
// an extremal region which can be performed incrementally as the
// greyscale level is raised.  See "Real-time scene text localization and
// recognition" by Neumann and Matas, CVPR 12.

bool nister_ccs::incrementally_test_ER(connect_comp_t* cc,ER_model_t *ER_model)
{
  unsigned int horiz_pixel_size, vert_pixel_size;
  double aspect_ratio, sqr_inverse_compactness;

// We want to minimize the number of candidate extremal regions.  So
// we impose a few basic criteria which extremal regions corresponding
// to objects of interest are very likely to satisfy:

// 1.  Ignore any extremal region containing too few or too many
// pixels to be of interest:

  if (cc->size < ER_model->ER_incremental_params.min_cc_pixel_size)
  {
    update_CC_metadata(cc);
    return false;
  }

  if (cc->size > ER_model->ER_incremental_params.max_cc_pixel_size)
  {
    cc->active_flag = 0;
    return false;
  }

// 2. Ignore any extremal regions whose bbox horiz/vert pixel sizes
// are too small or large:
  
  horiz_pixel_size = cc->bbox_max_px - cc->bbox_min_px;
  vert_pixel_size = cc->bbox_max_py - cc->bbox_min_py;
  
//  log_verbose("horiz_pixel_size = %d  vert_pixel_size = %d \n",
//	      horiz_pixel_size, vert_pixel_size);

  if (horiz_pixel_size < ER_model->ER_incremental_params.min_horiz_pixel_size)
  {
    update_CC_metadata(cc);
    return false;
  }

  if (horiz_pixel_size > ER_model->ER_incremental_params.max_horiz_pixel_size)
  {
    cc->active_flag = 0;
    return false;
  }

  if (vert_pixel_size < ER_model->ER_incremental_params.min_vert_pixel_size)
  {
    update_CC_metadata(cc);
    return false;
  }

  if (vert_pixel_size > ER_model->ER_incremental_params.max_vert_pixel_size)
  {
    cc->active_flag = 0;
    return false;
  }

// 3.  Ignore any extremal regions whose aspect ratios differ too much
// from unity:

  aspect_ratio = (double) horiz_pixel_size / (double) vert_pixel_size;
  if (aspect_ratio < ER_model->ER_incremental_params.min_aspect_ratio || 
      aspect_ratio > ER_model->ER_incremental_params.max_aspect_ratio) 
  {
    update_CC_metadata(cc);
    return false;
  }

// 4.  Ignore any extremal region if its bbox has not significantly changed
// from previously updated bbox:

  if (abs(cc->bbox_min_px  - cc->prev_bbox_min_px) >  
      ER_model->ER_incremental_params.min_bbox_delta ||
      abs(cc->bbox_max_px  - cc->prev_bbox_max_px) >  
      ER_model->ER_incremental_params.min_bbox_delta ||
      abs(cc->bbox_min_py  - cc->prev_bbox_min_py) >  
      ER_model->ER_incremental_params.min_bbox_delta ||
      abs(cc->bbox_max_py  - cc->prev_bbox_max_py) >  
      ER_model->ER_incremental_params.min_bbox_delta) 
  {
    cc->prev_bbox_min_px = cc->bbox_min_px;
    cc->prev_bbox_max_px = cc->bbox_max_px;
    cc->prev_bbox_min_py = cc->bbox_min_py;
    cc->prev_bbox_max_py = cc->bbox_max_py;
  }
  else
  {
    update_CC_metadata(cc);
    return false;
  }

// 5a.  Ignore any extremal region whose brightness variation exceeds
// some threshold fraction of the maximum possible brightness
// difference:

  if (cc->max_val - cc->min_val > ER_model->ER_incremental_params.max_value_spread) 
  {
    cc->active_flag = 0;
    return false;
  }

// 5b.  Ignore any extremal region whose brightness standard deviation
// is too large:

  if (cc->foreground_sigma_intensity > ER_model->ER_incremental_params.max_value_std_dev)
  {
    cc->active_flag = 0;
    return false;
  }

// 6.  Ignore any extremal regions whose "sqrd inverse compactness" =
// sqr(perimeter) / area exceeds some large threshold:

  sqr_inverse_compactness=(double) sqr(cc->pixel_perim) / (double) cc->size;
  if (sqr_inverse_compactness > ER_model->ER_incremental_params.max_sqr_inverse_compactness) return false;

  return true;
}

// ---------------------------------------------------------------------
void nister_ccs::update_CC_metadata(connect_comp_t* cc)
{
  cc->prev_size = cc->size;
}


// =========================================================================
// Test image and debugging tool methods
// =========================================================================

// Member function instantiate_test_image() fills array with random
// integers ranging from 0 - 9.  Such simple greyscale test images are
// useful for testing and debugging purposes.

void nister_ccs::instantiate_test_image()
{
   unsigned int width = 8;
   unsigned int height = 8;
//  unsigned int width = 13;
//  unsigned int height = 9;
   long seed;

  cout << "Enter random number generator seed: " << endl;
  cin >> seed;
  nrfunc::init_default_seed(seed);

// Destroy existing grey-scale image and instantiate new one:

  twoDarray* ptwoDarray_ptr=get_ptwoDarray_ptr();
  delete ptwoDarray_ptr;
  ptwoDarray_ptr=new twoDarray(width,height);
  ptwoDarray_ptr->clear_values();

  for(unsigned r = 0; r <height; r++)
  {
    for(unsigned c = 0; c <width; c++)
    {
//      unsigned int curr_value=4 * RANDDUNIT_R(&seed);
       unsigned int curr_value=10 * nrfunc::ran1();
       ptwoDarray_ptr->put(c,r,curr_value);
    }
  }
}

// ---------------------------------------------------------------------
// Member function set_text_colors() fills input STL vector
// text_colors() with different text colors that are readily visible
// against a white background terminal window.

void nister_ccs::set_text_colors(vector<Color::Modifier>& text_colors)
{
   text_colors.push_back(Color::FG_RED);
   text_colors.push_back(Color::FG_ORANGE);
   text_colors.push_back(Color::FG_GREEN);
   text_colors.push_back(Color::FG_CYAN);
   text_colors.push_back(Color::FG_BLUE);
   text_colors.push_back(Color::FG_PURPLE);
   text_colors.push_back(Color::FG_GREY);
}

