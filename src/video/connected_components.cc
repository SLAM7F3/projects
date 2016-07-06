// =========================================================================
// Connected_Components class member function definitions
// =========================================================================
// Last modified on 1/24/13; 9/27/13; 4/3/14; 4/18/16
// =========================================================================

#include <iostream>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "math/prob_distribution.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void connected_components::allocate_member_objects()
{
//   cout << "inside connected_components::allocate_member_objects()" 
//        << endl;
   
   texture_rectangle_ptr=new texture_rectangle();
   changed_pixels_map_ptr=new CHANGED_PIXELS_MAP;
   updated_changed_pixels_map_ptr=new CHANGED_PIXELS_MAP;
   vector_union_find_ptr=new vector_union_find();
   labels_map_ptr=new LABELS_MAP;
   curr_treenodes_map_ptr=new TREENODES_MAP;
   prev_treenodes_map_ptr=new TREENODES_MAP;
   prev_curr_pixels_map_ptr=new PREV_CURR_PIXELS_MAP;
}

void connected_components::initialize_member_objects()
{
   tree_ptr=NULL;
   cc_texture_rectangle_ptr=NULL;
   pbinary_twoDarray_ptr=NULL;
   cc_twoDarray_ptr=NULL;
   prev_cc_twoDarray_ptr=NULL;
   visited_twoDarray_ptr=NULL;
//    shapes_pfunct_ptr=NULL;
   shapes_pfuncts_ptrs=NULL;

   min_n_connected_components=-1;
   max_n_connected_components=100000;
   min_fill_frac=-1;
   max_fill_frac=2;
}

// ---------------------------------------------------------------------
connected_components::connected_components()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

connected_components::connected_components(const connected_components& cc)
{
   docopy(cc);
}

connected_components::~connected_components()
{
//   cout << "inside connected_components destructor" << endl;

   delete texture_rectangle_ptr;
   delete cc_texture_rectangle_ptr;
   delete tree_ptr;
   delete pbinary_twoDarray_ptr;
   delete cc_twoDarray_ptr;
   delete prev_cc_twoDarray_ptr;
   delete visited_twoDarray_ptr;

   delete changed_pixels_map_ptr;
   delete updated_changed_pixels_map_ptr;
   delete vector_union_find_ptr;
   delete labels_map_ptr;
   delete curr_treenodes_map_ptr;
   delete prev_treenodes_map_ptr;
   delete prev_curr_pixels_map_ptr;
}

// ---------------------------------------------------------------------
void connected_components::docopy(const connected_components& cc)
{
}

// Overload = operator:

connected_components& connected_components::operator= (const connected_components& cc)
{
   if (this==&cc) return *this;
   docopy(cc);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const connected_components& cc)
{
   outstream << endl;
   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================

int connected_components::get_n_extremal_regions() const
{
   return vector_union_find_ptr->get_n_nodes();
}

extremal_region* connected_components::get_extremal_region_ptr(int n)
{
   return static_cast<extremal_region*>(
      vector_union_find_ptr->get_data_ptr(n));
}

const extremal_region* 
connected_components::get_extremal_region_ptr(int n) const
{
   return static_cast<extremal_region*>(
      vector_union_find_ptr->get_data_ptr(n));
}

// ---------------------------------------------------------------------
// Member function set_cc_subdir()

void connected_components::set_cc_subdir(bool invert_binary_values_flag)
{
//   cout << "inside connected_components::set_cc_subdir()" << endl;

   if (invert_binary_values_flag)
   {
      cc_subdir="./dark_connected_components/";
   }
   else
   {
      cc_subdir="./bright_connected_components/";
   }
   filefunc::dircreate(cc_subdir);
}

// =========================================================================
// Initialization member functions
// =========================================================================

void connected_components::reset_image(
   string image_filename,int color_channel_ID,int image_counter)
{
//   cout << "inside connected_components::reset_image()" << endl;
//   cout << "color_channel_ID = " << color_channel_ID << endl;
//   cout << "image_filename = " << image_filename << endl;
//   cout << "image_counter = " << image_counter << endl;

   texture_rectangle_ptr->import_photo_from_file(image_filename);
   if (color_channel_ID==-2) // luminosity value
   {
      texture_rectangle_ptr->convert_color_image_to_luminosity();
   }
   else if (color_channel_ID==-1) // greyscale value
   {
      texture_rectangle_ptr->convert_color_image_to_h_s_or_v(2);
   }
   else if (color_channel_ID==0) // saturation
   {
      texture_rectangle_ptr->convert_color_image_to_h_s_or_v(1);
   }
   else
   {
      bool generate_greyscale_image_flag=true;
      texture_rectangle_ptr->convert_color_image_to_single_color_channel(
         color_channel_ID,generate_greyscale_image_flag);
      texture_rectangle_ptr->refresh_ptwoDarray_ptr();
   }

   string output_filename="color_channel";
   if (image_counter >= 0)
   {
      output_filename += "_"+stringfunc::integer_to_string(image_counter,3);
   }
   if (color_channel_ID==-2)
   {
      output_filename="luminosity";
   }
   if (texture_rectangle_ptr->getNchannels()==4)
   {
      output_filename=output_filename+".png";
   }
   else
   {
      output_filename=output_filename+".jpg";
   }
//   texture_rectangle_ptr->write_curr_frame(output_filename);
//   cout << "Exported "+output_filename << endl;
//   outputfunc::enter_continue_char();

   reset_texture_rectangle_ptr(texture_rectangle_ptr);
}

void connected_components::delete_texture_rectangle_ptr()
{
   delete texture_rectangle_ptr;
}

void connected_components::reset_texture_rectangle_ptr(
   texture_rectangle* tr_ptr)
{
//   cout << "inside connected_components::reset_texture_rectangle_ptr()" << endl;

   texture_rectangle_ptr = tr_ptr;

   delete pbinary_twoDarray_ptr;
   pbinary_twoDarray_ptr=new twoDarray(get_ptwoDarray_ptr());
   width=pbinary_twoDarray_ptr->get_xdim();
   height=pbinary_twoDarray_ptr->get_ydim();

   delete cc_twoDarray_ptr;
   cc_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);
   cc_twoDarray_ptr->clear_values();

   delete prev_cc_twoDarray_ptr;
   prev_cc_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);
   prev_cc_twoDarray_ptr->clear_values();

   delete visited_twoDarray_ptr;
   visited_twoDarray_ptr=new twoDarray(width+1,height+1);
   visited_twoDarray_ptr->clear_values();

   delete tree_ptr;
   bool generate_treenode_maps_flag=true;
   tree_ptr=new tree<extremal_region*>(generate_treenode_maps_flag);

   delete cc_texture_rectangle_ptr;
   cc_texture_rectangle_ptr=new texture_rectangle(width,height,1,3,NULL);

   initialize_vector_union_find();

   cc_label_offset=0;
}

// ========================================================================
// TwoDarray printing & copying member functions
// =========================================================================

// Member function print_twoDarray() writes the contents of
// *twoDarray_ptr as an ascii matrix to stdout.

void connected_components::print_twoDarray(
   string twoDarray_name,twoDarray* ztwoDarray_ptr)
{
   cout << endl;
   cout << twoDarray_name << endl;

   for (unsigned int py=0; py<height; py++)
   {
      for (unsigned int px=0; px<width; px++)
      {
         cout << ztwoDarray_ptr->get(px,py) << " " << flush;
      }
      cout << endl;
   }
   cout << endl;
}

void connected_components::print_pbinary_twoDarray()
{
   print_twoDarray("*pbinary_twoDarray_ptr:",pbinary_twoDarray_ptr);
}

void connected_components::print_prev_cc_twoDarray()
{
   print_twoDarray("*prev_cc_twoDarray_ptr:",prev_cc_twoDarray_ptr);
}

void connected_components::print_cc_twoDarray()
{
   print_twoDarray("*cc_twoDarray_ptr:",cc_twoDarray_ptr);
}

void connected_components::print_rootnode_twoDarray()
{
   cout << "Root node array:" << endl;
   for (unsigned int py=0; py<height; py++)
   {
      for (unsigned int px=0; px<width; px++)
      {
         int node_ID=graphicsfunc::get_pixel_ID(px,py,width);
         int root_ID=vector_union_find_ptr->Find(node_ID);
         cout << root_ID << " " << flush;
      }
      cout << endl;
   }
   cout << endl;
}

void connected_components::print_changed_pixels_map(
   CHANGED_PIXELS_MAP* pixels_map_ptr)
{
   for (changed_pixel_iter=pixels_map_ptr->begin();
        changed_pixel_iter != pixels_map_ptr->end();
        changed_pixel_iter++)
   {
      cout << "cc_label = " << changed_pixel_iter->first
           << endl;
      vector<INT_PAIR> V=changed_pixel_iter->second;
      for (unsigned int p=0; p<V.size(); p++)
      {
         cout << "  px = " << V[p].first << " py = " << V[p].second
              << endl;
      }
   }
}

// ========================================================================
// Connected component computation member functions
// =========================================================================

// This first version of compute_connected_components() takes in
// a threshold setting which ranges from 0 to 255 along with a minimal
// number of pixels that each connected component should possess.  It
// returns the number of connected components which it finds above
// (below) the threshold in the binary version of the current image.

int connected_components::compute_connected_components(
   int threshold,int min_n_pixels)
{
//   cout << "inside connected_components::compute_connected_components()"
//        << endl;
//   timefunc::initialize_timeofday_clock();

   double znull=0;
   double zfill=1;
   binaryimagefunc::binary_threshold(
      threshold,get_ptwoDarray_ptr(),pbinary_twoDarray_ptr,znull,zfill);

//   cout << "connected_components::compute_connected_components() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;

   int n_connected_components=label_ccs();
   if (n_connected_components==0) return 0;

   count_all_cc_pixels();
   n_connected_components=purge_small_ccs(min_n_pixels);

// Convert purged version of *cc_twoDarray_ptr into binary image:

   double thresh=0.5;
   binaryimagefunc::binary_threshold(
      thresh,cc_twoDarray_ptr,pbinary_twoDarray_ptr,znull,zfill);

   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function compute_connected_components() takes in a threshold
// setting which ranges from 0 to 255.  It returns the number of
// connected components which it finds above (below) the threshold in
// the binary version of the current image.

int connected_components::compute_connected_components(
   int threshold,
   bool invert_binary_values_flag,bool export_connected_regions_flag)
{
//   cout << "inside connected_components::compute_connected_components(), level = " << level << endl;
//   cout << "invert_binary_values_flag = " << invert_binary_values_flag
//        << endl;
//   timefunc::initialize_timeofday_clock();

   double znull=0;
   double zfill=1;
   if (invert_binary_values_flag)
   {
      znull=1;
      zfill=0;
   }
   binaryimagefunc::binary_threshold(
      threshold,get_ptwoDarray_ptr(),pbinary_twoDarray_ptr,znull,zfill);

//   cout << "connected_components::compute_connected_components() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;

   int n_connected_components=label_ccs();
   if (n_connected_components==0) return 0;

   if (export_connected_regions_flag)
   {
      set_cc_subdir(invert_binary_values_flag);
      string connected_regions_filename=cc_subdir+"connected_regions_"+
         stringfunc::integer_to_string(threshold,3)+".png";
      color_connected_components(connected_regions_filename);
      cout << "connected_regions_filename = " << connected_regions_filename
           << endl;
   }

   cc_twoDarray_ptr->copy(prev_cc_twoDarray_ptr);

//   cout << "threshold = " << threshold 
//        << " level = " << level
//        << endl;
//   cout << "n_connected_components = " << n_connected_components << endl;

   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function compute_connected_components() takes in a threshold
// setting which ranges from 0 to 255.  It returns the number of
// connected components which it finds above (below) the threshold in
// the binary version of the current image.

int connected_components::compute_connected_components(
   int index,int threshold,int level,bool RLE_flag,
   bool invert_binary_values_flag,bool export_connected_regions_flag,
   bool retain_only_largest_cc_flag)
{
//   cout << "inside connected_components::compute_connected_components(), level = " << level << endl;
//   cout << "invert_binary_values_flag = " << invert_binary_values_flag
//        << endl;
//   timefunc::initialize_timeofday_clock();

   double znull=0;
   double zfill=1;
   if (invert_binary_values_flag)
   {
      znull=1;
      zfill=0;
   }
   binaryimagefunc::binary_threshold(
      threshold,get_ptwoDarray_ptr(),pbinary_twoDarray_ptr,znull,zfill);

//   cout << "connected_components::compute_connected_components() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;

   int n_connected_components=label_ccs();
   if (n_connected_components==0) return 0;

/*
// When running COMPUTE_SHAPE_DESCRIPTORS for TOC12, we want to work
// with just the largest connected component for each symbol letter.
// So delete all entries in *cc_twoDarray_ptr which do not correspond
// to largest cc:

   if (retain_only_largest_cc_flag)
   {
      n_connected_components=retain_only_largest_cc();
   }
*/

   if (export_connected_regions_flag)
   {
      set_cc_subdir(invert_binary_values_flag);
      string connected_regions_filename=cc_subdir+"connected_regions_"+
         stringfunc::integer_to_string(index,5)+"_"+
         stringfunc::integer_to_string(level,3)+".png";
      color_connected_components(connected_regions_filename);
      cout << "connected_regions_filename = " << connected_regions_filename
           << endl;
   }


//   cout << "min_n_connected_components = " << min_n_connected_components
//        << endl;
//   cout << "max_n_connected_components = " << max_n_connected_components
//        << endl;
   if (n_connected_components < min_n_connected_components ||
       n_connected_components > max_n_connected_components)
   {
      return n_connected_components;
   }

/*
// FAKE FAKE: Tues Aug 7, 2012 at 8:22 am.  
// TOC12 symbol recognition only...

//   if (level==64)
   if (level==128)
//   if (level==192)
   {
      set_cc_subdir(invert_binary_values_flag);
      export_individual_connected_components(cc_subdir,level);      
   }
*/

// Note #2: Sat Jul 28 at 2:40 pm
// Very slow memory leak found in update_extremal_regions_tree() call !!!

   build_extremal_regions_tree(level);

   compute_shape_descriptors_recursively(level,RLE_flag);

   update_treenodes_maps();

   cc_twoDarray_ptr->copy(prev_cc_twoDarray_ptr);

//   cout << "threshold = " << threshold 
//        << " level = " << level
//        << endl;
//   cout << "n_connected_components = " << n_connected_components << endl;

   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function label_ccs() works with *pbinary_twoDarray_ptr and
// assumes that null-valued entries equal zero.  It fills
// *cc_twoDarray_ptr with connected component integer labels.  The
// total number of connected components within *pbinary_twoDarray_ptr
// is returned.

int connected_components::ancient_label_ccs()
{
//   cout << "inside connected_components::ancient_label_ccs()" << endl;

   double pbinary_twoDarray_integral=0;
   for (unsigned int py=0; py<pbinary_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<pbinary_twoDarray_ptr->get_xdim(); px++)
      {
         pbinary_twoDarray_integral += pbinary_twoDarray_ptr->get(px,py);
      }
   }
//   cout << "pbinary_twoDarray_integral = " << pbinary_twoDarray_integral
//        << endl;
   
   if (nearly_equal(pbinary_twoDarray_integral,0)) return 0;

   int n_neighbors=4;

/*
   for (unsigned int py=0; py<pbinary_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<pbinary_twoDarray_ptr->get_xdim(); px++)
      {
         cout << pbinary_twoDarray_ptr->get(px,py) << " ";
      }
      cout << endl;
   }
   cout << endl;
*/

/*
   graphicsfunc::label_connected_components(
      n_neighbors,cc_label_offset,pbinary_twoDarray_ptr,cc_twoDarray_ptr);
 
   int n_connected_components=0;
   for (unsigned int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
      {
         int label=cc_twoDarray_ptr->get(px,py);
         n_connected_components=basic_math::max(label,n_connected_components);
//         cout << "py = " << py << " px = " << px  
//              << " cc_label = " << label << endl;
      } // loop over py index
   } // loop over px index

   n_connected_components -= cc_label_offset;
*/

   double z_null=0;
   int n_connected_components=graphicsfunc::Label_Connected_Components(
      n_neighbors,cc_label_offset,z_null,
      pbinary_twoDarray_ptr,cc_twoDarray_ptr);


   cc_label_offset += n_connected_components;
//   cout << "n_connected_components = " << n_connected_components << endl;

/*
   for (unsigned int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
      {
         cout << cc_twoDarray_ptr->get(px,py) << " ";
      } // loop over py index
      cout << endl;
   } // loop over px index
   outputfunc::enter_continue_char();
*/
 
   return n_connected_components;
}

// ---------------------------------------------------------------------
// This version of connected_components::label_ccs() should be called
// by program LOCATE_CHARS which works with an extremal region tree.

// As of 4:30 pm on Mon, Aug 6, 2012, label_ccs() is 2-4 times more
// expensive than build_extremal_regions_tree() and
// compute_shape_descriptors_recursively().

int connected_components::label_ccs()
{
//   cout << "inside connected_components::label_ccs()" << endl;
//     timefunc::initialize_timeofday_clock();

   mini_recompute_connected_components();
   int n_connected_components=update_connected_component_labels();

//   cout << "connected_components::label_ccs() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function build_extremal_regions_tree() first instantiates
// treenodes for each entry within *labels_map_ptr corresponding to
// the specified input level.  It next loops over all pixels in
// *cc_twoDarray_ptr and *prev_cc_twoDarray_ptr.  After comparing cc
// labels, this method establishes parent-child relationships between
// treenodes at current and previous levels.

// On 8/5/12, we introduced member STL maps *curr_treenodes_map_ptr
// and *prev_treenodes_map_ptr.  This method uses these temporary STL
// maps instead of calling tree_ptr->get_treenode_ptr() which is VERY
// expensive.

void connected_components::build_extremal_regions_tree(int level)
{
//   cout << "inside connected_components::build_extremal_regions_tree()" 
//        << endl;
//   cout << "level = " << level << endl;
//   timefunc::initialize_timeofday_clock();

   TREENODES_MAP* treenodes_map_ptr=tree_ptr->get_or_create_treenodes_map_ptr(
      level);

   for (label_iter=labels_map_ptr->begin(); 
        label_iter != labels_map_ptr->end(); label_iter++)
   {
      int curr_node_ID=label_iter->second;
//      cout << "curr_node_ID = " << curr_node_ID << endl;

      TREENODE_PTR treenode_ptr=
         tree_ptr->generate_new_treenode(curr_node_ID,level,treenodes_map_ptr);
      extremal_region* extremal_region_ptr=new extremal_region(curr_node_ID);
      extremal_region_ptr->set_image_height(height);

      treenode_ptr->set_data_ptr(extremal_region_ptr);
      (*curr_treenodes_map_ptr)[curr_node_ID]=treenode_ptr;
   } // label_iter loop

// Store parent-child relationships between current and previous
// level extremal regions within *prev_curr_pixels_map_ptr:

   prev_curr_pixels_map_ptr->clear();
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int curr_node_ID=cc_twoDarray_ptr->get(px,py);
         if (curr_node_ID <= 0) continue;
         int prev_node_ID=prev_cc_twoDarray_ptr->get(px,py);
         if (prev_node_ID <=0) continue;

         prev_curr_iter=prev_curr_pixels_map_ptr->find(prev_node_ID);
         if (prev_curr_iter==prev_curr_pixels_map_ptr->end())
         {
            (*prev_curr_pixels_map_ptr)[prev_node_ID]=curr_node_ID;
         }
      } // loop over py index
   } // loop over px index

// Encode parent-child relationships into current and previous
// level treenodes:

   for (prev_curr_iter=prev_curr_pixels_map_ptr->begin(); 
        prev_curr_iter != prev_curr_pixels_map_ptr->end(); prev_curr_iter++)
   {
      int prev_node_ID=prev_curr_iter->first;
      treenode_iter=prev_treenodes_map_ptr->find(prev_node_ID);
      TREENODE_PTR prev_treenode_ptr=treenode_iter->second;
      if (prev_treenode_ptr != NULL)
      {
         if (prev_treenode_ptr->get_parent_node_ptr()==NULL)
         {
            int curr_node_ID=prev_curr_iter->second;
            treenode_iter=curr_treenodes_map_ptr->find(curr_node_ID);
            TREENODE_PTR treenode_ptr=treenode_iter->second;
            prev_treenode_ptr->set_parent_node_ptr(treenode_ptr);
         }
      }
   }

   for (treenode_iter=curr_treenodes_map_ptr->begin(); 
        treenode_iter != curr_treenodes_map_ptr->end(); 
        treenode_iter++)
   {
      TREENODE_PTR curr_treenode_ptr=treenode_iter->second;
      if (curr_treenode_ptr->is_leaf_flag())
      {
         tree_ptr->pushback_leaf_node(curr_treenode_ptr);
      }
   } // treenode_iter loop

//   cout << "connected_components::build_extremal_regions_tree() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function update_treenodes_maps() sets
// prev_treenodes_map_ptr=curr_treenodes_map_ptr.

void connected_components::update_treenodes_maps()
{
//   cout << "inside connected_components::update_treenodes_maps()" << endl;
//   timefunc::initialize_timeofday_clock();

   delete prev_treenodes_map_ptr;
   prev_treenodes_map_ptr=curr_treenodes_map_ptr;
   curr_treenodes_map_ptr=new TREENODES_MAP;
//   cout << "connected_components::update_treenodes_maps() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function compute_text_shape_probs() loops over all treenodes
// within *tree_ptr at the specified level.  It ignores any extremal
// region whose pixel size is too small.  It also rejects any extremal
// region whose skew or dimensionless quartic z values are too large.
// For all other extremal regions, this method computes the
// probabilities they correspond to text characters based upon just
// their shape features.

void connected_components::compute_text_shape_probs(
   int level,double shapes_prob_threshold,
   unsigned int start_object_ID,unsigned int stop_object_ID,
   bool tight_skew_quartic_thresholds_flag)
{
//   cout << "inside connected_components::compute_text_shape_probs()" << endl;
//   cout << "level = " << level << endl;
//   timefunc::initialize_timeofday_clock();

   double max_skew_z_threshold=1.35;
   double max_dimensionless_quartic_z_threshold=4;

   if (!tight_skew_quartic_thresholds_flag)
   {
      max_skew_z_threshold *= 2;
      max_dimensionless_quartic_z_threshold *= 2;
   }

   TREENODE_PTR treenode_ptr=tree_ptr->reset_curr_treenodes_map_ptr(level);
   while (treenode_ptr != NULL)
   {
      extremal_region* extremal_region_ptr=treenode_ptr->get_data_ptr();
      treenode_ptr=tree_ptr->get_next_treenode_ptr();

/*
      int ID=extremal_region_ptr->get_ID();
      int pixel_area=extremal_region_ptr->get_pixel_area();
      int pixel_perimeter=extremal_region_ptr->get_pixel_perim();
      int euler_number=extremal_region_ptr->get_Euler_number();

      unsigned int left_pu,bottom_pv,right_pu,top_pv;
      extremal_region_ptr->get_bbox(left_pu,top_pv,right_pu,bottom_pv);
      int pixel_width=right_pu-left_pu;
      int pixel_height=top_pv-bottom_pv;

      double aspect_ratio=extremal_region_ptr->get_aspect_ratio();
      double compactness=extremal_region_ptr->get_compactness();
      int n_holes=extremal_region_ptr->get_n_holes();
      int median_horiz_crossings=extremal_region_ptr->
         get_n_horiz_crossings();
*/

// Median horiz crossings are only calculated for extremal regions
// which have changed since the previous level.  Otherwise, they have
// default -1 values.  So checking for median horiz crossing > 0 is
// tantamount to determining which extremal regions have changed since
// the previous level.  Ignore any extremal region which has NOT
// changed:

      if (extremal_region_ptr->get_n_horiz_crossings() < 0) 
         continue;

// Reject candidate extremal regions whose pixel height or width are
// less or more than reasonable threshold values:
         
      if (extremal_region_ptr->region_too_small_or_too_big(
         width-2,height-2)) continue;

//      cout << "  n = " << n << " node_ID = " << ID << endl;
//      cout << "  aspect_ratio = " << aspect_ratio << endl;
//      cout << "  compactness = " << compactness << endl;
//      cout << "  n_holes = " << n_holes << endl;
//      cout << "  median_horiz_crossings = " << median_horiz_crossings 
//           << endl;
      
/*
      if (print_flag)
      {
         cout << "node: ID = " << ID
              << " area = " << pixel_area
              << " perim = " << pixel_perimeter
              << " euler = " << euler_number
              << " horiz_crossings = " << median_horiz_crossings << endl;

         cout << "  left_pu = " << left_pu
              << " right_pu = " << right_pu
              << " bottom_pv = " << bottom_pv
              << " top_pv = " << top_pv << endl;

         cout << "  px_sum = " << extremal_region_ptr->get_px_sum()
              << " py_sum = " << extremal_region_ptr->get_py_sum() 
              << " z_sum = " << extremal_region_ptr->get_z_sum()
              << endl;
         cout << "  sqr_px_sum = " 
              << extremal_region_ptr->get_sqr_px_sum()
              << " sqr_py_sum = " 
              << extremal_region_ptr->get_sqr_py_sum() 
              << " px_py_sum = " << extremal_region_ptr->get_px_py_sum() 
              << " sqr_z_sum = " << extremal_region_ptr->get_sqr_z_sum()
              << endl;
         cout << "  cube_px_sum = " 
              << extremal_region_ptr->get_cube_px_sum()
              << " sqr_px_py_sum = " 
              << extremal_region_ptr->get_sqr_px_py_sum() 
              << " sqr_py_px_sum = " 
              << extremal_region_ptr->get_sqr_py_px_sum() 
              << " cube_py_sum = " 
              << extremal_region_ptr->get_cube_py_sum() 
              << " cube_z_sum = " 
              << extremal_region_ptr->get_cube_z_sum()
              << endl;
         cout << "  prob = " << extremal_region_ptr->get_object_prob()
              << endl;
      } // print_flag conditional
*/

            
// As of Fri July 13, 2012, we no longer believe trying to use
// sigma_z as a text char feature helps much at all.  On the other
// hand, skew_z and dimensionless_quartic_z are features which yield
// modest rejection of non-text extremal regions:


//      double sigma_z=extremal_region_ptr->get_sigma_z();
//      cout << "sigma_z = " << sigma_z << endl;
//      if (extremal_region_ptr->get_sigma_z() > max_sigma_z_threshold)
//      {
//         extremal_region_ptr->set_object_prob(-1);
//         n_sigmaz_rejections++;
//      }

//      double skew_z=extremal_region_ptr->get_skew_z();
//            cout << "skew_z = " << skew_z << endl;
      if (fabs(extremal_region_ptr->get_skew_z()) > max_skew_z_threshold)
         continue;

//      double dimensionless_quartic_z=extremal_region_ptr->
//         get_dimensionless_quartic_z();
//            cout << "dimensionless quartic z = " 
//                 << dimensionless_quartic_z << endl;
      if (extremal_region_ptr->get_dimensionless_quartic_z() > 
          max_dimensionless_quartic_z_threshold) 
         continue;

      bool print_flag=false;
      for (unsigned int object_ID=start_object_ID; object_ID < stop_object_ID;
           object_ID++)
      {
         extremal_region_ptr->compute_shape_text_prob(
            shapes_pfuncts_ptrs->at(object_ID),shapes_prob_threshold,
            object_ID,print_flag);
      }
      
   } // loop over index n labeling current treenodes

//   cout << "connected_components::compute_text_shape_probs() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function RLE_extremal_region() run length encodes the input
// extremal region's pixels.

void connected_components::RLE_extremal_region(
   extremal_region* extremal_region_ptr)
{
//   cout << "inside connected_components::RLE_extremal_region()" << endl;

//   print_cc_twoDarray();
   extremal_region_ptr->run_length_encode(cc_twoDarray_ptr);
}

// ---------------------------------------------------------------------
// Member function select_extremal_regions() imports one particular
// level within the *tree_ptr and loops over all its extremal regions.
// It culls out and returns those regions whose aspect_ratio,
// compactness, number of holes and number of horizontal crossings lie
// within specified bounds.  

vector<extremal_region*> connected_components::select_extremal_regions(
   int level,double min_aspect_ratio,double max_aspect_ratio,
   double min_compactness,double max_compactness,
   int min_n_holes,int max_n_holes,
   int min_n_crossings,int max_n_crossings)
{
//   cout << "inside connected_components::select_extremal_regions()" << endl;
//   cout << "level = " << level << endl;
//   timefunc::initialize_timeofday_clock();

   vector<extremal_region*> extremal_region_ptrs;

   TREENODE_PTR treenode_ptr=tree_ptr->reset_curr_treenodes_map_ptr(level);
   while (treenode_ptr != NULL)
   {
      extremal_region* extremal_region_ptr=treenode_ptr->get_data_ptr();
      treenode_ptr=tree_ptr->get_next_treenode_ptr();

//      int ID=extremal_region_ptr->get_ID();
//      int pixel_area=extremal_region_ptr->get_pixel_area();
//      int pixel_perimeter=extremal_region_ptr->get_pixel_perim();
//      int euler_number=extremal_region_ptr->get_Euler_number();

      unsigned int left_pu,bottom_pv,right_pu,top_pv;
      extremal_region_ptr->get_bbox(left_pu,top_pv,right_pu,bottom_pv);
//      int pixel_width=right_pu-left_pu;
//      int pixel_height=top_pv-bottom_pv;

      double aspect_ratio=extremal_region_ptr->get_aspect_ratio();
      double compactness=extremal_region_ptr->get_compactness();
      int n_holes=extremal_region_ptr->get_n_holes();
      int median_horiz_crossings=extremal_region_ptr->
         get_n_horiz_crossings();

// Reject candidate extremal regions whose pixel height or width are
// less or more than reasonable threshold values:
         
      if (extremal_region_ptr->region_too_small_or_too_big(
         width-2,height-2)) continue;

/*
      if (n_holes > 0)
      {
         cout << "   region_ID = " << ID << endl;
         cout << "   pixel area = " << pixel_area << endl;
         cout << "   left_pu = " << left_pu << " right_pu = " << right_pu
              << endl;
         cout << "   top_pv = " << top_pv << " bottom_pv = " << bottom_pv
              << endl;
         cout << "   aspect_ratio = " << aspect_ratio << endl;
         cout << "   compactness = " << compactness << endl;
         cout << "   n_holes = " << n_holes << endl;
         cout << "   median_horiz_crossings = " << median_horiz_crossings 
              << endl << endl;
      }
*/

      if (aspect_ratio < min_aspect_ratio) 
      {
//         cout << "aspect_ratio too small" << endl;
         continue;
      }

      if (aspect_ratio > max_aspect_ratio) 
      {
//         cout << "aspect_ratio too large" << endl;
         continue;
      }

      if (compactness < min_compactness) 
      {
//         cout << "compactness too small" << endl;
         continue;
      }

      if (compactness > max_compactness)
      {
//         cout << "compactness too large" << endl;
         continue;
      }

      if (n_holes < min_n_holes) 
      {
//         cout << "Too few holes" << endl;
         continue;
      }

      if (n_holes > max_n_holes)
      {
//         cout << "Too many holes" << endl;
         continue;
      }

      if (median_horiz_crossings < min_n_crossings)
      {
//         cout << "Too few horiz crossings" << endl;
         continue;
      }
      
      if (median_horiz_crossings > max_n_crossings)
      {
//         cout << "Too many horiz crossings" << endl;
         continue;
      }
//      cout << "region PASSES" << endl << endl;
      
      extremal_region_ptrs.push_back(extremal_region_ptr);
      
   } // loop over index n labeling current treenodes

   return extremal_region_ptrs;
}

// =========================================================================
// Connected component export member functions
// =========================================================================

// Member function export_connected_components() takes in a threshold
// setting which ranges from 0 to 255.  It returns the number of
// connected components which it finds above (below) the threshold in
// the binary version of the current image.

int connected_components::export_connected_components(
   int index,int threshold,int level,bool invert_binary_values_flag)
{
//   cout << "inside connected_components::export_connected_components(), level = " << level << endl;

   double znull=0;
   double zfill=1;
   binaryimagefunc::binary_threshold(
      threshold,get_ptwoDarray_ptr(),pbinary_twoDarray_ptr,znull,zfill);
   
   if (invert_binary_values_flag)
   {
      binaryimagefunc::binary_reverse(pbinary_twoDarray_ptr,znull,zfill);
   }

//   print_pbinary_twoDarray();

   int n_connected_components=label_ccs();
//   cout << "n_connected_components = " << n_connected_components << endl;

   if (n_connected_components==0) return 0;

   if (n_connected_components < min_n_connected_components ||
       n_connected_components > max_n_connected_components)
   {
      return n_connected_components;
   }

   set_cc_subdir(invert_binary_values_flag);
   string connected_regions_filename=cc_subdir+"connected_regions_"+
      stringfunc::integer_to_string(index,5)+"_"+
      stringfunc::integer_to_string(level,3)+".png";
//      cout << "connected_regions_filename = " << connected_regions_filename
//           << endl;
   color_connected_components(connected_regions_filename);

   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function color_connected_components() takes in twoDarray
// *cc_twoDarray_ptr which is assumed to be filled with integer values.
// Any pixels corresponding to non-positive values are colored black.
// Otherwise, pixels with positive integer values are assigned
// quantized colors.  The colored visualization of *cc_twoDarray_ptr is
// exported to output_image_filename.

void connected_components::color_connected_components(
   string output_image_filename,bool uniform_color_flag)
{
//   cout << "inside connected_components::color_connected_components()" << endl;
//   cout << "output_image_filename = " << output_image_filename << endl;
   cc_texture_rectangle_ptr->initialize_twoDarray_image(cc_twoDarray_ptr,3);

   int n_nonzero_pixels=0;
   
   int R,G,B;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int color_index=cc_twoDarray_ptr->get(px,py);
         if (color_index <= 0) continue;

         if (uniform_color_flag)
         {
            R=G=B=255;
         }
         else
         {
            colorfunc::Color curr_color=colorfunc::get_color(color_index%12);
            colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(curr_color);
                     
            R=255*curr_RGB.first;
            G=255*curr_RGB.second;
            B=255*curr_RGB.third;
         }
         cc_texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);

         n_nonzero_pixels++;
      } // loop over py index
   } // loop over px index

// For non-text extremal region generation, we want to limit the fill
// fraction to some reasonable interval within [0,1]:

   double fill_frac=double(n_nonzero_pixels)/(width*height);
   if (fill_frac > min_fill_frac && fill_frac < max_fill_frac)
   {
      cc_texture_rectangle_ptr->write_curr_frame(output_image_filename);
   }
}

// ---------------------------------------------------------------------
// Member function export_individual_connected_components() works with 
// twoDarray *cc_twoDarray_ptr which is assumed to be have previously
// been filled with integer valued connected component labels.  
// Looping over all connected components, this method exports each one
// as a binary image to a separate PNG file.  We wrote this method in
// July 2012 in order to break apart complex symbols (e.g. biohazard
// signs) into separate extremal regions for "text location" purposes.

void connected_components::export_individual_connected_components(
   string cc_subdir,int level)
{
   cout << "inside connected_components::export_individual_connected_components()" << endl;

   cout << "level = " << level << endl;
   unsigned int n_connected_components=fill_labels_map();
   cout << "n_connected_components = " << n_connected_components
        << endl;

   twoDarray* blank_twoDarray_ptr=new twoDarray(cc_twoDarray_ptr);
   blank_twoDarray_ptr->clear_values();

   for (unsigned int component_counter=0; 
        component_counter < n_connected_components; 
        component_counter++)
   {
      cc_texture_rectangle_ptr->initialize_twoDarray_image(
         blank_twoDarray_ptr,3);

      label_iter=labels_map_ptr->find(component_counter);
      int cc_label=label_iter->second;
      
      cout << "component_counter = " << component_counter
           << " cc_label = " << cc_label
           << endl;

      int n_nonzero_pixels=0;
      for (unsigned int px=0; px<width; px++)
      {
         for (unsigned int py=0; py<height; py++)
         {
            int cc_label=cc_twoDarray_ptr->get(px,py);
            if (cc_label < 0) continue;
            label_iter=labels_map_ptr->find(cc_label);
            if (label_iter != labels_map_ptr->end())
            {
               unsigned int component_index=label_iter->second;
               if (component_index==component_counter)
               {
                  cc_texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,255,255,255);
                  n_nonzero_pixels++;
               }
            }
         } // loop over py index
      } // loop over px index

//      if (n_nonzero_pixels < 100) continue;

      string output_image_filename=cc_subdir+"connected_component_"+
         stringfunc::integer_to_string(
            26*(level-1)+component_counter,2)+"_"+
         stringfunc::integer_to_string(level,3)+".png";
      cc_texture_rectangle_ptr->write_curr_frame(output_image_filename);
//      cout << "Exported " << output_image_filename << endl;

   } // loop over component_counter

   delete blank_twoDarray_ptr;

   exit(-1);
}

// ---------------------------------------------------------------------
// Member function fill_labels_map() works with *cc_twoDarray_ptr
// which is assumed to have previously been filled with integer valued
// connected component labels.  It counts the number of distinct cc
// labels and enters them into STL map member *labels_map_ptr.

unsigned int connected_components::fill_labels_map()
{
//   cout << "inside connected_components::fill_labels_map()" << endl;
//   print_cc_twoDarray();

   labels_map_ptr->clear();

// Note:  In this method, labels_map independent var = cc_twoDarray_label
// and dependent var = component counter

   unsigned int n_connected_components=0;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int cc_label=cc_twoDarray_ptr->get(px,py);
         if (cc_label > 0)
         {
            label_iter=labels_map_ptr->find(cc_label);
            if (label_iter == labels_map_ptr->end())
            {
               (*labels_map_ptr)[cc_label]=n_connected_components++;
//               cout << "cc_label = " << cc_label << endl;
            }
         }
      }
   }
//   cout << "n_connected_components = " << n_connected_components << endl;
//   cout << "labels_map_ptr->size() = " << labels_map_ptr->size() << endl;
   
   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function count_all_cc_pixels() works with
// *cc_twoDarray_ptr which is assumed to have previously been filled
// with integer valued connected component labels.  It scans through
// *cc_twoDarray_ptr and counts the number of pixels associated with
// each distinct cc label.  Results are returned within member
// *labels_map_ptr.  

int connected_components::count_all_cc_pixels()
{
//   cout << "inside connected_components::count_all_cc_pixels()" << endl;

   labels_map_ptr->clear();

// Note:  In this method, labels_map independent var = cc_twoDarray_label
// and dependent var = number of matching pixels

   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int cc_label=cc_twoDarray_ptr->get(px,py);
         if (cc_label > 0)
         {
            label_iter=labels_map_ptr->find(cc_label);
            if (label_iter == labels_map_ptr->end())
            {
               (*labels_map_ptr)[cc_label]=1;
            }
            else
            {
               label_iter->second=label_iter->second+1;
            }
         }
      } // loop over py
   } // loop over px
   int n_connected_components=labels_map_ptr->size();
   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function purge_small_ccs() works with *cc_twoDarray_ptr
// which is assumed to have previously been filled with integer valued
// connected component labels.  It resets entries in *cc_twoDarray_ptr
// to zero which correspond to connected components whose pixel
// numbers are less than min_n_pixels.  This method returns the number
// of non-purged "big" connectedc components.

int connected_components::purge_small_ccs(int min_n_pixels)
{
//   cout << "inside connected_components::purge_small_ccs()" << endl;

//   int n_connected_components=count_all_cc_pixels();
//   cout << "Initial n_connected_components = "
//        << n_connected_components << endl;
//   cout << "labels_map_ptr->size() = " << labels_map_ptr->size() << endl;
   
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int cc_label=cc_twoDarray_ptr->get(px,py);

         label_iter=labels_map_ptr->find(cc_label);
         if (label_iter == labels_map_ptr->end()) continue;

         int n_pixels=label_iter->second;
         if (n_pixels < min_n_pixels)
         {
            cc_twoDarray_ptr->put(px,py,0);
         }
      } // py loop
   } // px loop
//   print_cc_twoDarray();

   return fill_labels_map();
}

// ---------------------------------------------------------------------
// Member function retain_only_largest_cc() works with
// *cc_twoDarray_ptr which is assumed to have previously been filled
// with integer valued connected component labels.  It resets all
// entries in *cc_twoDarray_ptr to zero which do not correspond to the
// largest connected component.  And it resets the largest connected
// component label as well as cc_label_offset to 1.

int connected_components::retain_only_largest_cc()
{
//   cout << "inside connected_components::retain_only_largest_cc()" << endl;

   int n_connected_components=count_all_cc_pixels();

//   cout << "Initial n_connected_components = "
//        << n_connected_components << endl;

//   if (n_connected_components <= 1) return n_connected_components;

// Identify cc_label with greatest number of pixels:

   int max_cc_label=-1;
   int max_n_pixels=0;
   for (label_iter=labels_map_ptr->begin(); label_iter != 
           labels_map_ptr->end(); label_iter++)
   {
//      cout << "curr_label = " << label_iter->first 
//           << " n_pixels = " << label_iter->second << endl;
      int curr_n_pixels=label_iter->second;
      if (curr_n_pixels > max_n_pixels)
      {
         max_n_pixels=curr_n_pixels;
         max_cc_label=label_iter->first;
      }
   }

   cc_label_offset += 1-n_connected_components;
   
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int cc_label=cc_twoDarray_ptr->get(px,py);

         if (cc_label <= 0) 
         {
            continue;
         }
         else if (cc_label != max_cc_label) 
         {
            cc_twoDarray_ptr->put(px,py,0);
         }
         else
         {
            cc_twoDarray_ptr->put(px,py,cc_label_offset);
         }
      } // py loop
   } // px loop
//   print_cc_twoDarray();

   labels_map_ptr->clear();
   (*labels_map_ptr)[cc_label_offset]=cc_label_offset;

   return 1;
}

// =========================================================================
// Connected component descriptor member functions
// =========================================================================

// Member function compute_shape_descriptors_recursively()

void connected_components::compute_shape_descriptors_recursively(
   int level,bool RLE_flag)
{
//   cout << "inside connected_components::compute_shape_descriptors_recursively()" << endl;
//   cout << "level = " << level << endl;
//   timefunc::initialize_timeofday_clock();

   vector<int> total_horiz_crossings;
   total_horiz_crossings.reserve(height);

// Initialize each parent node's area, perimeter and Euler number to
// sum of its children nodes' areas, perimeters and Euler numbers:

   TREENODE_PTR treenode_ptr=tree_ptr->reset_curr_treenodes_map_ptr(level);
   while (treenode_ptr != NULL)
   {
      extremal_region* extremal_region_ptr=treenode_ptr->get_data_ptr();
      
//      int cc_ID=treenode_ptr->get_ID();
//      cout << "cc_ID = " << cc_ID << endl;
      
      int total_child_area=0;
      int total_child_perim=0;
      int total_child_Euler_number=0;
      unsigned int total_min_px=POSITIVEINFINITY;
      unsigned int total_max_px=NEGATIVEINFINITY;
      unsigned int total_min_py=POSITIVEINFINITY;
      unsigned int total_max_py=NEGATIVEINFINITY;

      for (unsigned int py=0; py<height; py++)
      {
         total_horiz_crossings[py]=0;
      }

      double total_child_px_sum=0;
      double total_child_py_sum=0;
      double total_child_z_sum=0;

      double total_child_sqr_px_sum=0;
      double total_child_sqr_py_sum=0;
      double total_child_px_py_sum=0;
      double total_child_sqr_z_sum=0;

      double total_child_cube_px_sum=0;
      double total_child_sqr_px_py_sum=0;
      double total_child_sqr_py_px_sum=0;
      double total_child_cube_py_sum=0;
      double total_child_cube_z_sum=0;

      double total_child_quartic_z_sum=0;

      unsigned int min_px,min_py,max_px,max_py;
      TREENODE_PTR child_treenode_ptr=treenode_ptr->reset_child_treenode_ptr();
      while (child_treenode_ptr != NULL)
      {
         extremal_region* child_extremal_region_ptr=
            child_treenode_ptr->get_data_ptr();

         total_child_area += child_extremal_region_ptr->get_pixel_area();
         total_child_perim += child_extremal_region_ptr->get_pixel_perim();
         total_child_Euler_number += 
            child_extremal_region_ptr->get_Euler_number();

         child_extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);
         total_min_px=basic_math::min(total_min_px,min_px);
         total_max_px=basic_math::max(total_max_px,max_px);
         total_min_py=basic_math::min(total_min_py,min_py);
         total_max_py=basic_math::max(total_max_py,max_py);

         for (unsigned int py=min_py; py<=max_py; py++)
         {
            total_horiz_crossings[py] += 
               child_extremal_region_ptr->get_horiz_crossings(py);
         }

         total_child_px_sum += child_extremal_region_ptr->get_px_sum();
         total_child_py_sum += child_extremal_region_ptr->get_py_sum();
         total_child_z_sum += child_extremal_region_ptr->get_z_sum();

         total_child_sqr_px_sum += child_extremal_region_ptr->get_sqr_px_sum();
         total_child_sqr_py_sum += child_extremal_region_ptr->get_sqr_py_sum();
         total_child_px_py_sum += child_extremal_region_ptr->get_px_py_sum();
         total_child_sqr_z_sum += child_extremal_region_ptr->get_sqr_z_sum();

         total_child_cube_px_sum += child_extremal_region_ptr->
            get_cube_px_sum();
         total_child_sqr_px_py_sum += child_extremal_region_ptr->
            get_sqr_px_py_sum();
         total_child_sqr_py_px_sum += child_extremal_region_ptr->
            get_sqr_py_px_sum();
         total_child_cube_py_sum += child_extremal_region_ptr->
            get_cube_py_sum();
         total_child_cube_z_sum += child_extremal_region_ptr->
            get_cube_z_sum();

         total_child_quartic_z_sum += child_extremal_region_ptr->
            get_quartic_z_sum();

         child_treenode_ptr=treenode_ptr->get_next_child_treenode_ptr();
      } // loop over children treenodes of current treenode
      
      extremal_region_ptr->set_pixel_area(total_child_area);
      extremal_region_ptr->set_pixel_perim(total_child_perim);
      extremal_region_ptr->set_Euler_number(total_child_Euler_number);
      extremal_region_ptr->set_bbox(
         total_min_px,total_min_py,total_max_px,total_max_py);

      for (unsigned int py=total_min_py; py<=total_max_py; py++)
      {
         extremal_region_ptr->set_horiz_crossings(
            py,total_horiz_crossings[py]);
      }

      extremal_region_ptr->set_px_sum(total_child_px_sum);
      extremal_region_ptr->set_py_sum(total_child_py_sum);
      extremal_region_ptr->set_z_sum(total_child_z_sum);

      extremal_region_ptr->set_sqr_px_sum(total_child_sqr_px_sum);
      extremal_region_ptr->set_sqr_py_sum(total_child_sqr_py_sum);
      extremal_region_ptr->set_px_py_sum(total_child_px_py_sum);
      extremal_region_ptr->set_sqr_z_sum(total_child_sqr_z_sum);

      extremal_region_ptr->set_cube_px_sum(total_child_cube_px_sum);
      extremal_region_ptr->set_sqr_px_py_sum(total_child_sqr_px_py_sum);
      extremal_region_ptr->set_sqr_py_px_sum(total_child_sqr_py_px_sum);
      extremal_region_ptr->set_cube_py_sum(total_child_cube_py_sum);
      extremal_region_ptr->set_cube_z_sum(total_child_cube_z_sum);

      extremal_region_ptr->set_quartic_z_sum(total_child_quartic_z_sum);

      treenode_ptr=tree_ptr->get_next_treenode_ptr();

   } // while loop over treenodes at current level

// Loop over all pixels within *cc_twoDarray_ptr.  Focus only upon
// those pixels whose values have changed since the previous level:

   changed_pixels_map_ptr->clear();

   unsigned int mdim=cc_twoDarray_ptr->get_xdim();
   unsigned int ndim=cc_twoDarray_ptr->get_ydim();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         int curr_cc_ID=cc_twoDarray_ptr->get(px,py);
         if (curr_cc_ID <= 0) continue;

         int prev_cc_ID=prev_cc_twoDarray_ptr->get(px,py);
         if (prev_cc_ID > 0) continue;

// Only analyze extremal region pixels whose values have changed since
// the previous iteration:
         
//         cout << "px = " << px << " py = " << py
//              << " curr_cc_ID = " << curr_cc_ID 
//              << " prev_cc_ID = " << prev_cc_ID << endl;

         treenode_iter=curr_treenodes_map_ptr->find(curr_cc_ID);
//         if (treenode_iter==curr_treenodes_map_ptr->end())
//         {
//            cout << "ERROR!!!" << endl;
//            cout << "curr_cc_ID not found within *curr_treenodes_map_ptr!"
//                 << endl;
//            exit(-1);
//         }
         TREENODE_PTR curr_treenode_ptr=treenode_iter->second;
         extremal_region* extremal_region_ptr=
            curr_treenode_ptr->get_data_ptr();

// Increment current area by one for each new pixel which wasn't
// present in previous level:

         extremal_region_ptr->increment_pixel_area();
            
// Recompute lower left and upper right bounding box pixel coordinates
// for current connected region:

         extremal_region_ptr->update_bbox(px,py);
         extremal_region_ptr->update_XY_moments(px,py);
         extremal_region_ptr->update_Z_moments(
            get_ptwoDarray_ptr()->get(px,py));
         
//         cout << "px = " << px << " py = " << py
//              << " curr_cc = " << curr_cc_ID
//              << " prev_cc = " << prev_cc_ID
//              << " z = " << get_ptwoDarray_ptr()->get(px,py)
//              << endl;

         four_horiz_neighbors.clear();
         if (px-1 >= 0)
         {
            four_horiz_neighbors.push_back(INT_PAIR(px-1,py));
         }
         if (px+1 < mdim)
         {
            four_horiz_neighbors.push_back(INT_PAIR(px+1,py));
         }

// Count number of horizontal neighbors to changed pixel which are
// non-null valued.  Changed pixel's contributions to
// horiz_crossings STL vector is linear function of number of
// "sibling" and "children" horizontal neighbors:

         int n_curr_neighbors=0;
         int n_prev_neighbors=0;
         for (unsigned int f=0; f<four_horiz_neighbors.size(); f++)
         {
            int neighbor_px=four_horiz_neighbors[f].first;
            int neighbor_py=four_horiz_neighbors[f].second;
            int curr_neighbor_cc_ID=cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (curr_neighbor_cc_ID > 0) n_curr_neighbors++;
            int prev_neighbor_cc_ID=prev_cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (prev_neighbor_cc_ID > 0) n_prev_neighbors++;
         }
         int n_sibling_neighbors=n_curr_neighbors-n_prev_neighbors;
         int n_children_neighbors=n_prev_neighbors;

         int delta_horiz_crossings=
            2-n_sibling_neighbors-2*n_children_neighbors;
//         cout << "n_sibling_neighbors = " << n_sibling_neighbors
//              << " n_children_neighbors = " << n_children_neighbors
//              << endl;
         extremal_region_ptr->append_horiz_crossings(
            py,delta_horiz_crossings);

// Count number of vertical neighbors to changed pixel which are
// non-null valued.  Changed pixel's contributions to extremal
// region's perimeter is linear combo of "sibling" and "children"
// four-neighbors:

         four_vert_neighbors.clear();
         if (py-1 >= 0)
         {
            four_vert_neighbors.push_back(INT_PAIR(px,py-1));
         }
         if (py+1 < ndim)
         {
            four_vert_neighbors.push_back(INT_PAIR(px,py+1));
         }

         for (unsigned int f=0; f<four_vert_neighbors.size(); f++)
         {
            int neighbor_px=four_vert_neighbors[f].first;
            int neighbor_py=four_vert_neighbors[f].second;
            int curr_neighbor_cc_ID=cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (curr_neighbor_cc_ID > 0) n_curr_neighbors++;
            int prev_neighbor_cc_ID=prev_cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (prev_neighbor_cc_ID > 0) n_prev_neighbors++;
         }
         n_sibling_neighbors=n_curr_neighbors-n_prev_neighbors;
         n_children_neighbors=n_prev_neighbors;
//            cout << "n_sibling_neighbors = " << n_sibling_neighbors
//                 << " n_children_neighbors = " << n_children_neighbors
//                 << endl;

         extremal_region_ptr->update_pixel_perim(
            n_children_neighbors,n_sibling_neighbors);

// Store recently changed pixel coordinates as functions of connected
// component IDs within *changed_pixels_map_ptr.  Later compute change
// in Euler number once all changed pixels are known:

         curr_int_pair.first=px;
         curr_int_pair.second=py;

         CHANGED_PIXELS_MAP::iterator iter=changed_pixels_map_ptr->find(
            curr_cc_ID);
         if (iter==changed_pixels_map_ptr->end()) 
         {
            curr_changed_pixel_coords.clear();
            curr_changed_pixel_coords.push_back(curr_int_pair);
            (*changed_pixels_map_ptr)[curr_cc_ID]=curr_changed_pixel_coords;
         }
         else
         {
            iter->second.push_back(curr_int_pair);
         }

//            cout << "px = " << px << " py = " << py 
//                 << " changed_pixels_map_ptr->size() = "
//                 << changed_pixels_map_ptr->size() << endl;

      } // loop over py index
   } // loop over px index

// Iterate over independent variable connected component ID within
// *changed_pixels_map_ptr.  Compute change in Euler number for each
// connected component:

//   cout << "Before computing change in Euler numbers:" << endl;
//   print_changed_pixels_map(changed_pixels_map_ptr);

   visited_twoDarray_ptr->clear_values();

   for (changed_pixel_iter=changed_pixels_map_ptr->begin();
        changed_pixel_iter != changed_pixels_map_ptr->end(); 
        changed_pixel_iter++)
   {
      int cc_label=changed_pixel_iter->first;
      treenode_iter=curr_treenodes_map_ptr->find(cc_label);
      TREENODE_PTR curr_treenode_ptr=treenode_iter->second;
      extremal_region* extremal_region_ptr=curr_treenode_ptr->get_data_ptr();

// Compute median number of horizontal crossings for each connected
// component which has changed since the previous level:

//      int n_horiz_crossings=
           compute_median_horiz_crossing(cc_label,extremal_region_ptr);
//      cout << "cc_label = " << cc_label
//           << " n_horiz_crossings = " << n_horiz_crossings << endl;
      
      double d_Euler=binaryimagefunc::compute_delta_Euler_number(
         changed_pixel_iter->second,cc_label,cc_twoDarray_ptr,
         visited_twoDarray_ptr);
//      cout << "cc_label = " << cc_label << " d_Euler = " << d_Euler << endl;

      extremal_region_ptr->increment_Euler_number(d_Euler);

      if (RLE_flag) RLE_extremal_region(extremal_region_ptr);      
//      cout << "extremal region = " << *extremal_region_ptr << endl;

   } // loop over changed pixels map iterator

//   cout << "connected_components::compute_shape_descriptors_recursively() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function compute_median_horiz_crossing() counts the number
// of oscillations within *cc_twoDarray_ptr from zero to
// nonzero-valued pixels along 3 different horizontal lines cutting
// through *extremal_region_ptr.  It returns the median of
// these 3 horizontal crossing numbers.

int connected_components::compute_median_horiz_crossing(
   int cc_ID,extremal_region* extremal_region_ptr)
{
//   cout << "inside connected_components::compute_median_horiz_crossing()"
//        << endl;
       
   int min_py=extremal_region_ptr->get_min_py();
   int cc_height=extremal_region_ptr->get_pixel_height();

   int horiz_crossings0=
      extremal_region_ptr->get_horiz_crossings(min_py+cc_height/6.0);
   int horiz_crossings1=
      extremal_region_ptr->get_horiz_crossings(min_py+3*cc_height/6.0);
   int horiz_crossings2=
      extremal_region_ptr->get_horiz_crossings(min_py+5*cc_height/6.0);
   int median_horiz_crossings=mathfunc::median_value(
      horiz_crossings0,horiz_crossings1,horiz_crossings2);
   extremal_region_ptr->set_n_horiz_crossings(median_horiz_crossings);
   return median_horiz_crossings;
}

// ---------------------------------------------------------------------
// Member function compute_shape_descriptors()

void connected_components::compute_shape_descriptors(int level)
{
//   cout << "inside connected_components::compute_shape_descriptors()" << endl;
//   cout << "level = " << level << endl;

//   TREENODE_PTR treenode_ptr=
      tree_ptr->reset_curr_treenodes_map_ptr(level);

// Loop over all pixels within *cc_twoDarray_ptr.  Focus only upon
// those pixels whose values have changed since the previous level:

   changed_pixels_map_ptr->clear();

   unsigned int mdim=cc_twoDarray_ptr->get_xdim();
   unsigned int ndim=cc_twoDarray_ptr->get_ydim();
   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         int curr_cc_ID=cc_twoDarray_ptr->get(px,py);
         if (curr_cc_ID <= 0) continue;

/*         
         TREENODE_PTR curr_treenode_ptr=tree_ptr->get_treenode_ptr(
            level,curr_cc_ID);
         extremal_region* extremal_region_ptr=
            curr_treenode_ptr->get_data_ptr();

// Increment current area by one for each new pixel which wasn't
// present in previous level:

         extremal_region_ptr->increment_pixel_area();
            
// Recompute lower left and upper right bounding box pixel coordinates
// for current connected region:

         extremal_region_ptr->update_bbox(px,py);

//            cout << "px = " << px << " py = " << py
//                 << " curr_cc = " << curr_cc_ID
//                 << " prev_cc = " << prev_cc_ID
//                 << endl;

// Compute median number of horizontal crossings:

         compute_median_horiz_crossing(curr_cc_ID,extremal_region_ptr);

         four_neighbors.clear();
         if (px-1 >= 0)
         {
            four_neighbors.push_back(INT_PAIR(px-1,py));
         }
         if (px+1 < mdim)
         {
            four_neighbors.push_back(INT_PAIR(px+1,py));
         }
         if (py-1 >= 0)
         {
            four_neighbors.push_back(INT_PAIR(px,py-1));
         }
         if (py+1 < ndim)
         {
            four_neighbors.push_back(INT_PAIR(px,py+1));
         }
//            cout << "four_neighbors.size() = " << four_neighbors.size()
//                 << endl;

// Count number of 4-neighbor children and siblings which are non-null
// valued:

         int n_curr_neighbors=0;
         int n_prev_neighbors=0;
         for (unsigned int f=0; f<four_neighbors.size(); f++)
         {
            int neighbor_px=four_neighbors[f].first;
            int neighbor_py=four_neighbors[f].second;
            int curr_neighbor_cc_ID=cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (curr_neighbor_cc_ID > 0) n_curr_neighbors++;
            int prev_neighbor_cc_ID=prev_cc_twoDarray_ptr->get(
               neighbor_px,neighbor_py);
            if (prev_neighbor_cc_ID > 0) n_prev_neighbors++;
         }
         int n_sibling_neighbors=n_curr_neighbors-n_prev_neighbors;
         int n_children_neighbors=n_prev_neighbors;
//            cout << "n_sibling_neighbors = " << n_sibling_neighbors
//                 << " n_children_neighbors = " << n_children_neighbors
//                 << endl;

         extremal_region_ptr->update_pixel_perim(
            n_children_neighbors,n_sibling_neighbors);

// Store recently changed pixel coordinates as functions of connected
// component IDs within *changed_pixels_map_ptr.  Later compute change
// in Euler number once all changed pixels are known:

         curr_int_pair.first=px;
         curr_int_pair.second=py;

         CHANGED_PIXELS_MAP::iterator iter=changed_pixels_map_ptr->find(
            curr_cc_ID);
         if (iter==changed_pixels_map_ptr->end()) 
         {
            curr_changed_pixel_coords.clear();
            curr_changed_pixel_coords.push_back(curr_int_pair);
            (*changed_pixels_map_ptr)[curr_cc_ID]=curr_changed_pixel_coords;
         }
         else
         {
            iter->second.push_back(curr_int_pair);
         }

//            cout << "px = " << px << " py = " << py 
//                 << " changed_pixels_map_ptr->size() = "
//                 << changed_pixels_map_ptr->size() << endl;

*/


      } // loop over py index
   } // loop over px index

/*
// Iterate over indepent variable connected component ID within
// *changed_pixels_map_ptr.  Compute change in Euler number for each
// connected component:

   visited_twoDarray_ptr->clear_values();
   for (CHANGED_PIXELS_MAP::iterator iter=changed_pixels_map_ptr->begin();
        iter != changed_pixels_map_ptr->end(); iter++)
   {
      int cc_ID=iter->first;
      TREENODE_PTR curr_treenode_ptr=tree_ptr->get_treenode_ptr(level,cc_ID);
      extremal_region* extremal_region_ptr=curr_treenode_ptr->get_data_ptr();

      double d_Euler=binaryimagefunc::compute_delta_Euler_number(
         iter->second,cc_ID,cc_twoDarray_ptr,visited_twoDarray_ptr);
      extremal_region_ptr->increment_Euler_number(d_Euler);

   } // loop over changed pixels map iterator

*/

}

// =========================================================================
// Connected component randomness measure member functions
// =========================================================================

// Member function compute_interior_exterior_std_devs()

bool connected_components::compute_interior_exterior_std_devs(
   double& sigma_all,double& sigma_interior,double& sigma_exterior)
{
//   cout << "inside connected_components::compute_interior_exterior_std_devs()" 
//        << endl;

   vector<double> p_values,exterior_p_values,interior_p_values;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         double curr_p=get_ptwoDarray_ptr()->get(px,py);
         p_values.push_back(curr_p);

         int color_index=cc_twoDarray_ptr->get(px,py);
         if (color_index <= 0) 
         {
            exterior_p_values.push_back(curr_p);
         }
         else
         {
            interior_p_values.push_back(curr_p);
         }
      } // loop over py index
   } // loop over px index

   const unsigned int min_size=100;

   if (exterior_p_values.size() < min_size ||
   interior_p_values.size() < min_size)
   {
      return false;
   }

//   double mu_interior=mathfunc::mean(interior_p_values);
//   double mu_exterior=mathfunc::mean(exterior_p_values);
   sigma_all=mathfunc::std_dev(p_values);
   sigma_interior=mathfunc::std_dev(interior_p_values);
   sigma_exterior=mathfunc::std_dev(exterior_p_values);

//   ratio_interior=fabs(mu_interior-mu_exterior)/sigma_interior;
//   ratio_exterior=fabs(mu_interior-mu_exterior)/sigma_exterior;

   return true;
}

// ---------------------------------------------------------------------
// Member function compute_interior_exterior_entropies()

bool connected_components::compute_interior_exterior_entropies(
   double& entropy_all,double& entropy_interior,double& entropy_exterior)
{
//   cout << "inside connected_components::compute_interior_exterior_entropies()" 
//        << endl;

   vector<double> p_values,exterior_p_values,interior_p_values;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         double curr_p=get_ptwoDarray_ptr()->get(px,py);
         p_values.push_back(curr_p);

         int color_index=cc_twoDarray_ptr->get(px,py);
         if (color_index <= 0) 
         {
            exterior_p_values.push_back(curr_p);
         }
         else
         {
            interior_p_values.push_back(curr_p);
         }
      } // loop over py index
   } // loop over px index

   const unsigned int min_size=100;

   if (exterior_p_values.size() < min_size ||
   interior_p_values.size() < min_size)
   {
      return false;
   }

   int n_output_bins=256;
   prob_distribution all_prob(p_values,n_output_bins);
   prob_distribution interior_prob(interior_p_values,n_output_bins);
   prob_distribution exterior_prob(exterior_p_values,n_output_bins);
   
   entropy_all=all_prob.entropy();
   entropy_interior=interior_prob.entropy();
   entropy_exterior=exterior_prob.entropy();

   entropy_all /= log(256.0);
   entropy_interior /= log(256.0);
   entropy_exterior /= log(256.0);

   return true;
}

// =========================================================================
// Extremal region tree traversal member functions
// =========================================================================

// Member function identify_stable_extremal_regions() loops over all leaf
// nodes within member *tree_ptr.  From each leaf node, it climbs
// vertically towards the tree's root node.  For each extremal region
// along the way, it compares pixel widths, heights and number of
// holes.  If these quantities vary too much in the vertical
// direction, the extremal region is NOT regarded as stable.  This
// method returns an STL vector containing pointers to stable tree
// nodes.

vector<connected_components::TREENODE_PTR> 
connected_components::identify_stable_extremal_regions(int object_ID)
{
   cout << "inside connected_components::identify_stable_extremal_regions()" 
        << endl;
   cout << "object_ID = " << object_ID << endl;

   vector<TREENODE_PTR> stable_region_node_ptrs;

   TREENODE_PTR leafnode_ptr=tree_ptr->reset_leafnode_ptr();
//   cout << "n_leaf nodes = " << tree_ptr->get_n_leaf_nodes() << endl;

   int n_rejected=0;
   while (leafnode_ptr != NULL)
   {

// Climb tree from current leaf to its root node:

      TREENODE_PTR currnode_ptr=leafnode_ptr;

      while (currnode_ptr != NULL)
      {
//         if (currnode_ptr->get_previously_visited_flag()) break;

         TREENODE_PTR parent_node_ptr=currnode_ptr->get_parent_node_ptr();
         if (parent_node_ptr==NULL) break;

         TREENODE_PTR grandparent_node_ptr=currnode_ptr->get_parent_node_ptr();
         if (grandparent_node_ptr==NULL) break;

//         int curr_level=currnode_ptr->get_level();

         extremal_region* extremal_region_ptr=
            currnode_ptr->get_data_ptr();
         double char_prob=extremal_region_ptr->get_object_prob(object_ID);

// Don't bother to analyze current treenode if it has previously been
// visited for the current object ID!

//         if (char_prob > -2 ) break;

         int n_siblings=currnode_ptr->get_n_siblings();
         int pixel_width=extremal_region_ptr->get_pixel_width();
         int pixel_height=extremal_region_ptr->get_pixel_height();
         int n_holes=extremal_region_ptr->get_n_holes();

         extremal_region* parent_region_ptr=parent_node_ptr->get_data_ptr();
         double parent_char_prob=parent_region_ptr->get_object_prob(object_ID);
         int n_parent_siblings=parent_node_ptr->get_n_siblings();
         int parent_width=parent_region_ptr->get_pixel_width();
         int parent_height=parent_region_ptr->get_pixel_height();
         int parent_n_holes=parent_region_ptr->get_n_holes();

         extremal_region* grandparent_region_ptr=grandparent_node_ptr->
            get_data_ptr();
         double grandparent_char_prob=grandparent_region_ptr->
            get_object_prob(object_ID);
         int n_grandparent_siblings=grandparent_node_ptr->get_n_siblings();
         int grandparent_width=grandparent_region_ptr->get_pixel_width();
         int grandparent_height=grandparent_region_ptr->get_pixel_height();
         int grandparent_n_holes=grandparent_region_ptr->get_n_holes();

         int n_children=currnode_ptr->getNumChildren();

         if (char_prob > 0 && parent_char_prob > 0 &&
             grandparent_char_prob > 0)
         {
//            cout << "char_prob = " << char_prob
//                 << " parent_char_prob = " << parent_char_prob
//                 << " gp_char_prob = " << grandparent_char_prob
//                 << endl;

            TREENODE_PTR child_node_ptr=
               currnode_ptr->reset_child_treenode_ptr();

// FAKE FAKE:  Fri Sep 7, 2012 at 12:23 pm
// Change n_children > 1 to n_children > 2

            if (n_siblings > 0 || n_parent_siblings > 0 || 
                n_grandparent_siblings > 0 || n_children > 2)
//                  n_grandparent_siblings > 0 || n_children > 1)
            {
//               cout << "n_siblings = " << n_siblings
//                    << " n_parent_siblings = " << n_parent_siblings
//                    << " n_grandparent_siblings = "
//                    << n_grandparent_siblings
//                    << " n_children = " << n_children << endl;
               n_rejected++;
//                  cout << endl;
            }
            else if (child_node_ptr != NULL)
            {
               extremal_region* child_region_ptr=child_node_ptr->
                  get_data_ptr();
               
               double child_char_prob=child_region_ptr->get_object_prob(
                  object_ID);
               int child_width=child_region_ptr->get_pixel_width();
               int child_height=child_region_ptr->get_pixel_height();
               int child_n_holes=child_region_ptr->get_n_holes();

               int n_grandchildren=child_node_ptr->getNumChildren();
               TREENODE_PTR grandchild_node_ptr=
                  child_node_ptr->reset_child_treenode_ptr();
               if (grandchild_node_ptr != NULL && n_grandchildren==1)
               {
                  extremal_region* grandchild_region_ptr=grandchild_node_ptr->
                     get_data_ptr();

                  double grandchild_char_prob=
                     grandchild_region_ptr->get_object_prob(object_ID);
                  int grandchild_width=
                     grandchild_region_ptr->get_pixel_width();
                  int grandchild_height=
                     grandchild_region_ptr->get_pixel_height();
                  int grandchild_n_holes=grandchild_region_ptr->get_n_holes();

//                  cout << "child_char_prob = " << child_char_prob
//                       << " gc_char_prob = " << grandchild_char_prob 
//                       << endl;

//                  cout << "n_holes = " << n_holes 
//                       << " parent_n_holes = " << parent_n_holes 
//                       << " grandparent_n_holes =  " << grandparent_n_holes 
//                       << endl;

//                  cout << "pixel_width = " << pixel_width
//                       << " parent_width = " << parent_width
//                       << " grandparent_width = " << grandparent_width
//                       << endl;

//                  cout << "pixel_height = " << pixel_height
//                       << " parent_height = " << parent_height
//                       << " grandparent_height = " << grandparent_height
//                       << endl;

                  if (child_char_prob > 0 &&
                  pixel_width==parent_width &&
                  pixel_height==parent_height &&
                  n_holes==parent_n_holes &&
                  pixel_width==child_width &&
                  pixel_height==child_height &&
                  n_holes==child_n_holes &&
                  pixel_width==grandparent_width &&
                  pixel_height==grandparent_height &&
                  n_holes==grandparent_n_holes &&
                  grandchild_char_prob > 0 &&
                  pixel_width==grandchild_width &&
                  pixel_height==grandchild_height &&
                  n_holes==grandchild_n_holes)
                  {
//                     cout.precision(3);
//                     cout << "leaf ID=" << leafnode_ptr->get_ID()
//                          << "  node_ID=" << currnode_ptr->get_ID()
//                          << "  level=" << curr_level
//                          << "  siblings=" << n_siblings 
//                          << "  w=" << pixel_width
//                          << "  h=" << pixel_height
//                          << "  holes=" << n_holes
//                          << "  prob=" << char_prob << endl;
//                     cout.precision(10);
          
                     stable_region_node_ptrs.push_back(currnode_ptr);
                  }
                  else
                  {
                     n_rejected++;
                  }
               }
               else
               {
                  n_rejected++;
               }
            }
         }
         else
         {
            n_rejected++;
         }
         
         currnode_ptr->set_previously_visited_flag(true);
         currnode_ptr=currnode_ptr->get_parent_node_ptr();

      } // currnode_ptr while loop
//         outputfunc::enter_continue_char();
//      cout << "-------------------" << endl;

      leafnode_ptr=tree_ptr->get_next_leafnode_ptr();
   } // leafnode_ptr while loop

   cout << "total n_nodes = " << tree_ptr->size() << endl;
   cout << "n_leaf nodes = " << tree_ptr->get_n_leaf_nodes() << endl;
   cout << "n_stable_regions = " << stable_region_node_ptrs.size() << endl;
   cout << "n_unstable regions rejected = " << n_rejected << endl;

   return stable_region_node_ptrs;
}

// =========================================================================
// COMPONENT TREE COMPUTATION MEMBER FUNCTIONS
// =========================================================================

void connected_components::initialize_vector_union_find()
{
//   cout << "inside connected_components::initialize_vector_union_find()" 
//        << endl;
   vector_union_find_ptr->purgeNodes();

   int n_nodes=width*height;
   vector_union_find_ptr->initializeNodes(n_nodes);
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int node_ID=graphicsfunc::get_pixel_ID(px,py,width);
         vector_union_find_ptr->CreateNode(node_ID,-1);
      } // loop over px 
   } // loop over py
}

// ---------------------------------------------------------------------
// Member function mini_recompute_connected_components() focuses upon
// pixels in *pbinary_twoDarray_ptr which have changed since the
// previous threshold setting.  

void connected_components::mini_recompute_connected_components()
{
//   cout << "inside connected_components::mini_recompute_connected_components()"
//        << endl;
//   timefunc::initialize_timeofday_clock();

   unsigned int qx,qy;
   const int tmp_label=99999;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         if (pbinary_twoDarray_ptr->get(px,py) <= 0) continue;
         if (cc_twoDarray_ptr->get(px,py) > 0) continue;

// Temporarily reset cc label to temporary dummy value.  Also, create
// new nontrivial node within vector_union_find for changed pixel:

         cc_twoDarray_ptr->put(px,py,tmp_label);

         int curr_node_ID=graphicsfunc::get_pixel_ID(px,py,width);
         vector_union_find_ptr->MakeSet(curr_node_ID);

// Get 4-neighbors around currently changed pixel location:

         graphicsfunc::compute_four_neighbor_IDs(
            px,py,width,height,neighbor_IDs);

         for (unsigned int n=0; n<neighbor_IDs.size(); n++)
         {
            int neighbor_ID=neighbor_IDs[n];
            graphicsfunc::get_pixel_px_py(neighbor_ID,width,qx,qy);

            if (cc_twoDarray_ptr->get(qx,qy) > 0)
            {
               vector_union_find_ptr->Link(neighbor_ID,curr_node_ID);
            }
//            int neighbor_label=cc_twoDarray_ptr->get(qx,qy);
//            cout << "qx = " << qx << " qy = " << qy
//                 << " neighbor_label = " << neighbor_label << endl;
         } // loop over index n labeling neighbors to current pixel

      } // loop over py
   } // loop over px

//   print_pbinary_twoDarray();
//   print_cc_twoDarray();

//   cout << "connected_components::recompute_connected_components() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function recompute_connected_components() focuses upon pixels
// in *pbinary_twoDarray_ptr which have changed since the previous
// threshold setting.  

void connected_components::recompute_connected_components()
{
//   cout << "inside connected_components::recompute_connected_components()"
//        << endl;
//   timefunc::initialize_timeofday_clock();

//   print_pbinary_twoDarray();
//   print_cc_twoDarray();

   for (unsigned int n=0; n<vector_union_find_ptr->get_n_nodes(); n++)
   {
      extremal_region* extremal_region_ptr=
         static_cast<extremal_region*>(vector_union_find_ptr->get_data_ptr(n));
      if (extremal_region_ptr != NULL)
         extremal_region_ptr->set_visited_flag(false);
   }

   neighbor_IDs.clear();
   vector<int> neighbor_node_IDs;

   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         if (pbinary_twoDarray_ptr->get(px,py) <= 0) continue;
         int curr_label=cc_twoDarray_ptr->get(px,py);
         if (curr_label > 0) continue;
         int curr_node_ID=graphicsfunc::get_pixel_ID(px,py,width);

//         cout << "----------" << endl;
//         cout << "px = " << px << " py = " << py << " changed" << endl;
//         cout << "curr_node_ID = " << curr_node_ID << endl;
//         print_cc_twoDarray();
//         print_rootnode_twoDarray();

// Temporarily reset cc label to dummy 99999 values.  Also, create new
// nontrivial node within vector_union_find for changed pixel:

         int tmp_label=99999;
         cc_twoDarray_ptr->put(px,py,tmp_label);

         vector_union_find_ptr->MakeSet(curr_node_ID);
         extremal_region* extremal_region_ptr=new extremal_region(tmp_label);
         extremal_region_ptr->set_image_height(height);
         vector_union_find_ptr->set_data_ptr(curr_node_ID,extremal_region_ptr);

// Get 4-neighbors around currently changed pixel location:

         graphicsfunc::compute_four_neighbor_IDs(
            px,py,width,height,neighbor_IDs);

         neighbor_node_IDs.clear();
         for (unsigned int n=0; n<neighbor_IDs.size(); n++)
         {
            int neighbor_ID=neighbor_IDs[n];

            unsigned int qx,qy;
            graphicsfunc::get_pixel_px_py(neighbor_ID,width,qx,qy);
            int neighbor_label=cc_twoDarray_ptr->get(qx,qy);
            if (neighbor_label > 0)
            {
               neighbor_node_IDs.push_back(neighbor_ID);
            }

//            cout << "qx = " << qx << " qy = " << qy
//                 << " neighbor_label = " << neighbor_label << endl;
         } // loop over index n labeling four neighbors to current pixel

         if (neighbor_node_IDs.size()==0) continue;

// Accumulate properties from neighbors of changed pixel within 
// extremal region corresponding to root ID.  This step effectively
// corresponds to summing "children" or "previous" properties for a
// "new" extremal region.  Contributions from the changed, new pixels
// to properties are added to root extremal region within
// update_connected_component_properties():

         int neighbor_pixel_area=0;
         int neighbor_pixel_perim=0;
         int neighbor_Euler_number=0;

         double neighbor_px_sum=0;
         double neighbor_py_sum=0;
         double neighbor_z_sum=0;

         double neighbor_sqr_px_sum=0;
         double neighbor_sqr_py_sum=0;
         double neighbor_px_py_sum=0;
         double neighbor_sqr_z_sum=0;

         double neighbor_cube_px_sum=0;
         double neighbor_sqr_px_py_sum=0;
         double neighbor_sqr_py_px_sum=0;
         double neighbor_cube_py_sum=0;
         double neighbor_cube_z_sum=0;

         unsigned int neighbor_min_px=POSITIVEINFINITY;
         unsigned int neighbor_min_py=POSITIVEINFINITY;
         unsigned int neighbor_max_px=NEGATIVEINFINITY;
         unsigned int neighbor_max_py=NEGATIVEINFINITY;

//         vector<int> all_neighbor_node_IDs;
         map<int,int> visited_neighbor_root_IDs_map;

         for (unsigned int f=0; f<neighbor_node_IDs.size(); f++)
         {
            int neighbor_ID=neighbor_node_IDs[f];
            unsigned int qx,qy;
            graphicsfunc::get_pixel_px_py(neighbor_ID,width,qx,qy);
//            int neighbor_label=cc_twoDarray_ptr->get(qx,qy);
//            cout << "f = " << f << " of " << neighbor_node_IDs.size() 
//                 << endl;
//            cout << "   neighbor_ID = " << neighbor_ID 
//                 << " neighbor_label = " << neighbor_label
//                 << " qx = " << qx << " qy = " << qy 
//                 << endl;

            extremal_region* neighbor_extremal_region_ptr=
               static_cast<extremal_region*>(
                  vector_union_find_ptr->get_root_data_ptr(neighbor_ID));

            unsigned int min_px,min_py,max_px,max_py;
            neighbor_extremal_region_ptr->get_bbox(
               min_px,min_py,max_px,max_py);
            neighbor_min_px=basic_math::min(neighbor_min_px,min_px);
            neighbor_min_py=basic_math::min(neighbor_min_py,min_py);
            neighbor_max_px=basic_math::max(neighbor_max_px,max_px);
            neighbor_max_py=basic_math::max(neighbor_max_py,max_py);
//            cout << "neighbor_min_px = " << neighbor_min_px
//                 << " neighbor_max_px = " << neighbor_max_px << endl;
//            cout << "neighbor_min_py = " << neighbor_min_py
//                 << " neighbor_max_py = " << neighbor_max_py << endl;
            
            int neighbor_root_ID=vector_union_find_ptr->Find(neighbor_ID);
//            cout << "neighbor_root_ID = " << neighbor_root_ID << endl;


            vector_union_find_ptr->Link(neighbor_ID,curr_node_ID);

//            int root_ID=vector_union_find_ptr->Find(curr_node_ID);
//            cout << "root_ID = " << root_ID << endl;
            
            map<int,int>::iterator vl_iter=visited_neighbor_root_IDs_map.find(
               neighbor_root_ID);
            if (vl_iter==visited_neighbor_root_IDs_map.end())
            {
               visited_neighbor_root_IDs_map[neighbor_root_ID]=1;
            }
            else
            {
//               cout << "Previously visited this neighbor!" << endl;
               continue;
            }
        
            neighbor_pixel_area += neighbor_extremal_region_ptr->
               get_pixel_area();

//            cout << "d_area = " << neighbor_extremal_region_ptr->
//               get_pixel_area()
//                 << " neighbor area integral = " << neighbor_pixel_area
//                 << endl;

            neighbor_pixel_perim += neighbor_extremal_region_ptr->
               get_pixel_perim();
            neighbor_Euler_number += neighbor_extremal_region_ptr->
               get_Euler_number();

            neighbor_px_sum += neighbor_extremal_region_ptr->get_px_sum();
            neighbor_py_sum += neighbor_extremal_region_ptr->get_py_sum();
            neighbor_z_sum += neighbor_extremal_region_ptr->get_z_sum();

            neighbor_sqr_px_sum += neighbor_extremal_region_ptr->
               get_sqr_px_sum();
            neighbor_sqr_py_sum += neighbor_extremal_region_ptr->
               get_sqr_py_sum();
            neighbor_px_py_sum += neighbor_extremal_region_ptr->
               get_px_py_sum();
            neighbor_sqr_z_sum += neighbor_extremal_region_ptr->
               get_sqr_z_sum();

            neighbor_cube_px_sum += neighbor_extremal_region_ptr->
               get_cube_px_sum();
            neighbor_sqr_px_py_sum += neighbor_extremal_region_ptr->
               get_sqr_px_py_sum();
            neighbor_sqr_py_px_sum += neighbor_extremal_region_ptr->
               get_sqr_py_px_sum();
            neighbor_cube_py_sum += neighbor_extremal_region_ptr->
               get_cube_py_sum();
            neighbor_cube_z_sum += neighbor_extremal_region_ptr->
               get_cube_z_sum();

/*
// Copy all extremal region pixel IDs into STL vector all_neighbor_node_IDs:

            int root_neighbor_node_ID=
               neighbor_extremal_region_ptr->reset_pixel_iterator();
            while (root_neighbor_node_ID >= 0)
            {
               all_neighbor_node_IDs.push_back(root_neighbor_node_ID);
               root_neighbor_node_ID=neighbor_extremal_region_ptr->
                  get_next_pixel_ID();
            }
//            cout << "all_neighbor_node_IDs.size() = "
//                 << all_neighbor_node_IDs.size() << endl;
            
//            for (unsigned int a=0; a<all_neighbor_node_IDs.size(); a++)
//            {
//               cout << "neighbor node ID = " << all_neighbor_node_IDs[a]
//                    << endl;
//            }
*/

            extremal_region* root_extremal_region_ptr=
               static_cast<extremal_region*>(
                  vector_union_find_ptr->get_root_data_ptr(curr_node_ID));
//         cout << "root_extremal_region_ptr = "
//              << root_extremal_region_ptr << endl;

            root_extremal_region_ptr->set_pixel_area(neighbor_pixel_area);
            root_extremal_region_ptr->set_pixel_perim(neighbor_pixel_perim);
            root_extremal_region_ptr->set_Euler_number(neighbor_Euler_number);
         
            root_extremal_region_ptr->update_bbox(
               neighbor_min_px,neighbor_min_py,
               neighbor_max_px,neighbor_max_py);
         
            root_extremal_region_ptr->set_px_sum(neighbor_px_sum);
            root_extremal_region_ptr->set_py_sum(neighbor_py_sum);
            root_extremal_region_ptr->set_z_sum(neighbor_z_sum);

            root_extremal_region_ptr->set_sqr_px_sum(neighbor_sqr_px_sum);
            root_extremal_region_ptr->set_sqr_py_sum(neighbor_sqr_py_sum);
            root_extremal_region_ptr->set_px_py_sum(neighbor_px_py_sum);
            root_extremal_region_ptr->set_sqr_z_sum(neighbor_sqr_z_sum);

            root_extremal_region_ptr->set_cube_px_sum(
               neighbor_cube_px_sum);
            root_extremal_region_ptr->set_sqr_px_py_sum(
               neighbor_sqr_px_py_sum);
            root_extremal_region_ptr->set_sqr_py_px_sum(
               neighbor_sqr_py_px_sum);
            root_extremal_region_ptr->set_cube_py_sum(neighbor_cube_py_sum);
            root_extremal_region_ptr->set_cube_z_sum(neighbor_cube_z_sum);

//            root_extremal_region_ptr->insert_pixels(all_neighbor_node_IDs);

         } // loop over index f labeling neighbors to current pixel in 
           //  some existing connected component

      } // loop over py
   } // loop over px
//   cout << "connected_components::recompute_connected_components() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;
}

// ---------------------------------------------------------------------
// Member function update_connected_component_properties() loops
// over all pixels within *cc_twoDarray_ptr.  It works only with 
// pixels whose values have changed since the previous level.
// Four-neighbors to changed pixels are checked for those which
// corresponded to extremal regions in the previous level.  

// Note:  As of Sun Aug 5, 2012 at 1:06 pm, this method is SLOW!!!


void connected_components::update_connected_component_properties()
{
//   cout << endl;
//   cout << "inside connected_components::update_connected_component_properties()"
//        << endl;

   labels_map_ptr->clear();

   changed_pixels_map_ptr->clear();
   updated_changed_pixels_map_ptr->clear();
   int n_connected_components=0;

   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         int curr_label=cc_twoDarray_ptr->get(px,py);
         if (curr_label <= 0) continue;

         int prev_label=prev_cc_twoDarray_ptr->get(px,py);
         if (prev_label > 0) continue;

//         cout << endl;
//         cout << "Changed pixel: px = " << px << " py = " << py << endl;

// Count number of 4-neighbors around currently changed pixel
// location corresponding to "new" pixels at current level that did
// not exist within *cc_twoDarray_ptr at previous level:

         graphicsfunc::compute_four_neighbor_IDs(
            px,py,width,height,neighbor_IDs);

         int n_curr_neighbors=0;
         int n_prev_neighbors=0;
         for (unsigned int n=0; n<neighbor_IDs.size(); n++)
         {
            int neighbor_ID=neighbor_IDs[n];

            unsigned int qx,qy;
            graphicsfunc::get_pixel_px_py(neighbor_ID,width,qx,qy);
            int curr_neighbor_label=cc_twoDarray_ptr->get(qx,qy);
            if (curr_neighbor_label > 0) n_curr_neighbors++;

            int prev_neighbor_label=prev_cc_twoDarray_ptr->get(qx,qy);
            if (prev_neighbor_label > 0) n_prev_neighbors++;
         } // loop over index n labeling four neighbors to current pixel

         int n_new_neighbors=n_curr_neighbors-n_prev_neighbors;

//         cout << "px = " << px << " py = " << py 
//              << " n_new_neighbors = " << n_new_neighbors
//              << " n_prev_neighbors = " << n_prev_neighbors
//              << endl;

// Incorporate property changes from changed pixels at current level
// to root extremal region:

         int curr_node_ID=graphicsfunc::get_pixel_ID(px,py,width);
         int root_node_ID=vector_union_find_ptr->Find(curr_node_ID);
//         cout << "curr_node_ID = " << curr_node_ID 
//              << " root_node_ID = " << root_node_ID << endl;

         label_iter=labels_map_ptr->find(root_node_ID);
         int cc_label;
         if (label_iter==labels_map_ptr->end())
         {
            n_connected_components++;
            cc_label=n_connected_components+cc_label_offset;
            (*labels_map_ptr)[root_node_ID]=cc_label;
         }
         else
         {
            cc_label=label_iter->second;
         }
         cc_twoDarray_ptr->put(px,py,cc_label);

         extremal_region* root_extremal_region_ptr=
            static_cast<extremal_region*>(
               vector_union_find_ptr->get_root_data_ptr(curr_node_ID));
         root_extremal_region_ptr->increment_pixel_area(1);
         root_extremal_region_ptr->update_pixel_perim(
            n_prev_neighbors,n_new_neighbors);
         root_extremal_region_ptr->update_bbox(px,py);
         root_extremal_region_ptr->update_XY_moments(px,py);

// FAKE FAKE:  Sun Jul 29, 2012 at 5:28 pm
// Change following line in order to run componenttest main program!

         root_extremal_region_ptr->update_Z_moments(0);
//         root_extremal_region_ptr->update_Z_moments(
//            get_ptwoDarray_ptr()->get(px,py));

//         root_extremal_region_ptr->insert_pixel(curr_node_ID);

// Store recently changed pixel coordinates as functions of connected
// component IDs within *changed_pixels_map_ptr.  Later compute change
// in Euler number once all changed pixels are known:

         curr_int_pair.first=px;
         curr_int_pair.second=py;

         CHANGED_PIXELS_MAP::iterator iter=changed_pixels_map_ptr->find(
            cc_label);
         if (iter==changed_pixels_map_ptr->end()) 
         {
            curr_changed_pixel_coords.clear();
            curr_changed_pixel_coords.push_back(curr_int_pair);
            (*changed_pixels_map_ptr)[cc_label]=curr_changed_pixel_coords;
         }
         else
         {
            iter->second.push_back(curr_int_pair);
         }
//         cout << "px = " << px << " py = " << py 
//              << " cc_label = " << cc_label
//              << " changed_pixels_map_ptr->size() = "
//              << changed_pixels_map_ptr->size() << endl;

      } // loop over py
   } // loop over px
}

// ---------------------------------------------------------------------
// Member function update_connected_component_labels() assigns unique
// connected component labels to pixels in *cc_twoDarray_ptr based
// upon root node IDs within *vector_union_find_ptr.  Sequential
// integer cc labels are unique at each threshold level.  

int connected_components::update_connected_component_labels()
{
//   cout << "inside connected_components::update_connected_component_labels()"
//        << endl;
//   timefunc::initialize_timeofday_clock();

   labels_map_ptr->clear();

   int cc_label=-1;
   int prev_root_ID=-1;
   int n_connected_components=0;
   for (unsigned int px=0; px<width; px++)
   {
      for (unsigned int py=0; py<height; py++)
      {
         if (int(cc_twoDarray_ptr->get(px,py))==0) continue;
         
         int node_ID=graphicsfunc::get_pixel_ID(px,py,width);
         int root_ID=vector_union_find_ptr->Find(node_ID);
//         cout << "root_ID = " << root_ID << endl;

         if (root_ID == prev_root_ID)
         {
            cc_twoDarray_ptr->put(px,py,cc_label);
            continue;
         }
         prev_root_ID=root_ID;

         label_iter=labels_map_ptr->find(root_ID);
         if (label_iter==labels_map_ptr->end())
         {
            n_connected_components++;
            cc_label=n_connected_components+cc_label_offset;
            (*labels_map_ptr)[root_ID]=cc_label;
//            cout << "root_ID = " << root_ID 
//                 << " cc_label = " << cc_label << endl;
         }
         else
         {
            cc_label=label_iter->second;
         }
         cc_twoDarray_ptr->put(px,py,cc_label);

//         vector_union_find_ptr->set_data_label(node_ID,cc_label);
      } // loop over py
   } // loop over px 
   cc_label_offset += n_connected_components;

//   print_cc_twoDarray();
//   cout << "connected_components::update_connected_component_labels() time = " 
//        << timefunc::elapsed_timeofday_time() << endl;

//   cout << "n_connected_components = " << n_connected_components << endl;
   return n_connected_components;
}

// ---------------------------------------------------------------------
// Member function update_changed_pixels_map() iterates through
// *changed_pixels_map_ptr.  It replaces original connected component
// labels within the map's independent variables with their
// updated counterparts following the call to
// update_connected_component_labels().

void connected_components::update_changed_pixels_map()
{
//   cout << "inside connected_components::update_changed_pixels_map()"
//        << endl;
   for (changed_pixel_iter=changed_pixels_map_ptr->begin();
        changed_pixel_iter != changed_pixels_map_ptr->end();
        changed_pixel_iter++)
   {
//      int old_cc_label=changed_pixel_iter->first;
      vector<INT_PAIR> V=changed_pixel_iter->second;
      int px=V[0].first;
      int py=V[0].second;
      int new_cc_label=cc_twoDarray_ptr->get(px,py);
//      cout << "old cc label = " << old_cc_label
//           << " new cc label = " << new_cc_label 
//           << endl;
      (*updated_changed_pixels_map_ptr)[new_cc_label]=V;
   }
}

// =========================================================================
// TEMPORARY MEMBER FUNCTIONS FOR TESTING ONLY
// =========================================================================

void connected_components::instantiate_binary_image(int width,int height)
{
//   cout << "inside connected_components::instantiate_binary_image()" << endl;
//   cout << "width = " << width << " height = " << height << endl;

   this->width=width;
   this->height=height;

   delete pbinary_twoDarray_ptr;
   pbinary_twoDarray_ptr=new twoDarray(width,height);

   delete cc_twoDarray_ptr;
   cc_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);
   cc_twoDarray_ptr->clear_values();

   delete prev_cc_twoDarray_ptr;
   prev_cc_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);
   prev_cc_twoDarray_ptr->clear_values();

   delete visited_twoDarray_ptr;
   visited_twoDarray_ptr=new twoDarray(width+1,height+1);
   visited_twoDarray_ptr->clear_values();

   cc_label_offset=0;

   print_cc_twoDarray();
}

// ---------------------------------------------------------------------
void connected_components::set_binary_image_first()
{
   pbinary_twoDarray_ptr->clear_values();
   cc_twoDarray_ptr->clear_values();
}

/*
// ---------------------------------------------------------------------
void connected_components::set_binary_image_second()
{
   pbinary_twoDarray_ptr->clear_values();

   pbinary_twoDarray_ptr->put(0,0,1);
   pbinary_twoDarray_ptr->put(1,0,1);
   pbinary_twoDarray_ptr->put(2,0,1);
   pbinary_twoDarray_ptr->put(3,0,1);
   pbinary_twoDarray_ptr->put(4,0,1);

   pbinary_twoDarray_ptr->put(0,2,1);
   pbinary_twoDarray_ptr->put(1,2,1);
   pbinary_twoDarray_ptr->put(3,2,1);
   pbinary_twoDarray_ptr->put(4,2,1);

   pbinary_twoDarray_ptr->put(0,4,1);
   pbinary_twoDarray_ptr->put(1,4,1);
   pbinary_twoDarray_ptr->put(2,4,1);
   pbinary_twoDarray_ptr->put(3,4,1);
   pbinary_twoDarray_ptr->put(4,4,1);

   pbinary_twoDarray_ptr->put(0,1,1);
   pbinary_twoDarray_ptr->put(1,1,1);
   pbinary_twoDarray_ptr->put(2,1,1);
   pbinary_twoDarray_ptr->put(4,1,1);

   pbinary_twoDarray_ptr->put(0,3,1);
   pbinary_twoDarray_ptr->put(2,3,1);
   pbinary_twoDarray_ptr->put(3,3,1);
   pbinary_twoDarray_ptr->put(4,3,1);
}
*/

/*
// ---------------------------------------------------------------------
void connected_components::set_binary_image_second()
{
   pbinary_twoDarray_ptr->clear_values();

   pbinary_twoDarray_ptr->put(0,0,1);
   pbinary_twoDarray_ptr->put(1,0,1);
   pbinary_twoDarray_ptr->put(2,0,1);
   pbinary_twoDarray_ptr->put(3,0,1);
   pbinary_twoDarray_ptr->put(4,0,1);

   pbinary_twoDarray_ptr->put(0,2,1);
   pbinary_twoDarray_ptr->put(1,2,1);
   pbinary_twoDarray_ptr->put(2,2,1);
   pbinary_twoDarray_ptr->put(3,2,1);
   pbinary_twoDarray_ptr->put(4,2,1);

   pbinary_twoDarray_ptr->put(0,4,1);
   pbinary_twoDarray_ptr->put(1,4,1);
   pbinary_twoDarray_ptr->put(2,4,1);
   pbinary_twoDarray_ptr->put(3,4,1);
   pbinary_twoDarray_ptr->put(4,4,1);

   pbinary_twoDarray_ptr->put(0,1,1);
   pbinary_twoDarray_ptr->put(2,1,1);
   pbinary_twoDarray_ptr->put(4,1,1);

   pbinary_twoDarray_ptr->put(0,3,1);
   pbinary_twoDarray_ptr->put(2,3,1);
   pbinary_twoDarray_ptr->put(4,3,1);
}
*/

// ---------------------------------------------------------------------
void connected_components::set_binary_image_second()
{
   pbinary_twoDarray_ptr->clear_values();

   pbinary_twoDarray_ptr->put(0,0,1);
   pbinary_twoDarray_ptr->put(2,0,1);
   pbinary_twoDarray_ptr->put(3,0,1);
   pbinary_twoDarray_ptr->put(0,1,1);
   pbinary_twoDarray_ptr->put(3,3,1);
}

// ---------------------------------------------------------------------
void connected_components::set_binary_image_third()
{
   pbinary_twoDarray_ptr->clear_values();

   pbinary_twoDarray_ptr->put(0,0,1);
   pbinary_twoDarray_ptr->put(1,0,1);
   pbinary_twoDarray_ptr->put(2,0,1);
   pbinary_twoDarray_ptr->put(3,0,1);

   pbinary_twoDarray_ptr->put(0,1,1);
   pbinary_twoDarray_ptr->put(3,1,1);

   pbinary_twoDarray_ptr->put(0,3,1);
   pbinary_twoDarray_ptr->put(2,3,1);
   pbinary_twoDarray_ptr->put(3,3,1);
}

// ---------------------------------------------------------------------
void connected_components::set_binary_image_fourth()
{
   pbinary_twoDarray_ptr->clear_values();

   pbinary_twoDarray_ptr->put(0,0,1);
   pbinary_twoDarray_ptr->put(1,0,1);
   pbinary_twoDarray_ptr->put(2,0,1);
   pbinary_twoDarray_ptr->put(3,0,1);

   pbinary_twoDarray_ptr->put(0,1,1);
   pbinary_twoDarray_ptr->put(3,1,1);

   pbinary_twoDarray_ptr->put(0,2,1);
   pbinary_twoDarray_ptr->put(3,2,1);

   pbinary_twoDarray_ptr->put(0,3,1);
   pbinary_twoDarray_ptr->put(1,3,1);
   pbinary_twoDarray_ptr->put(2,3,1);
   pbinary_twoDarray_ptr->put(3,3,1);

}

// ---------------------------------------------------------------------
void connected_components::copy_cc_onto_prev_cc()
{
   cc_twoDarray_ptr->copy(prev_cc_twoDarray_ptr);
}


