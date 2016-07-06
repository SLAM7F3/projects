// ==========================================================================
// Program EXTREMAL_REGIONS implements Davis King's union-find
// approach to efficiently building connected component trees.
// Connected components are linked via lightweight pointers
// to children [parents] at levels below [above] them in the 
// tree.  Connected components also have a few properties (area,
// perimeter, number of holes, bounding boxes) which are calculated
// incrementally following Matas & Neumann (CVPR 12).  

// EXTREMAL_REGIONS exports bounding boxes around extremal regions
// calculated iteratively as a threshold descends/ascends through a
// greyscale image.
// ==========================================================================
// Last updated on 4/14/14; 4/15/14; 4/17/14; 4/21/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <dlib/disjoint_subsets.h>

#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "color/colortext.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "math/ltduple.h"
#include "numrec/nrfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;    
using std::cout;
using std::endl;
using std::flush;
using std::set;
using std::string;
using std::vector;

typedef set<int> INT_SET;
// independent int = pixel ID

typedef struct 
{
   bool active_flag;
   int build_index;
   INT_SET pixel_IDs;
   double max_value,min_value;
   bounding_box bbox;
   int pixel_perim,Euler_number;
} extremal_region_t;

typedef struct _connected_component
{
   int ID;
   _connected_component* parent_cc_ptr;
   set<_connected_component*> children_cc_ptrs;
   INT_SET pixel_IDs;
   bounding_box bbox;
   int pixel_perim,n_holes;
   colorfunc::Color color;
} connected_component_t;

typedef set<connected_component_t*> CC_SET;

typedef map<int,connected_component_t*> CCS_MAP;
// independent int = connected component ID
// dependent var = ptr to connected component

typedef map<int,vector<connected_component_t*> > LEVELS_CCS_MAP;
// independent int = threshold level
// dependent var = STL vector containing connected components at level


// ==========================================================================
// Forward method declarations:

twoDarray* instantiate_test_ptwoDarray(long& seed);
void print_array_values(
   twoDarray* ptwoDarray_ptr,twoDarray* cc_twoDarray_ptr);
void display_region_as_array(extremal_region_t* region_ptr,
                             twoDarray* ptwoDarray_ptr);
void print_extremal_region(extremal_region_t* region_ptr,
                           twoDarray* ptwoDarray_ptr);
void print_connected_component(connected_component_t* cc_ptr,
                               twoDarray* ptwoDarray_ptr);
connected_component_t* construct_connected_component(
   extremal_region_t* region_ptr);
vector<connected_component_t*>
convert_extremal_regions_to_connected_components(
   bool descending_threshold_flag,bool test_ptwoDarray_flag,
   bool print_extremal_regions_data_flag,int curr_threshold,
   unsigned int min_cc_pixel_size,unsigned int max_cc_pixel_size,
   vector<extremal_region_t>& extremal_regions,
   twoDarray* ptwoDarray_ptr,twoDarray* qtwoDarray_ptr,
   twoDarray* prev_level_cctwoDarray_ptr,
   twoDarray* curr_level_cctwoDarray_ptr,CCS_MAP& ccs_map);
void initialize_extremal_regions(
   twoDarray* ptwoDarray_ptr,vector<extremal_region_t>& extremal_regions);
void generate_pixel_neighbor_edges(
   bool descending_threshold_flag,int MAX_POSSIBLE_PIXEL_VALUE,
   twoDarray* ptwoDarray_ptr,
   vector<DUPLE>& neighboring_pixel_edges,
   vector<int>& neighboring_pixel_values);

int construct_extremal_regions(
   bool descending_threshold_flag,bool test_ptwoDarray_flag,
   bool print_extremal_regions_data_flag,int MAX_POSSIBLE_PIXEL_VALUE,
   twoDarray* ptwoDarray_ptr,vector<extremal_region_t>& extremal_regions,
   vector<DUPLE>& neighboring_pixel_edges,
   vector<int>& neighboring_pixel_values,
   LEVELS_CCS_MAP& levels_ccs_map);

void print_connected_components_tree(
   LEVELS_CCS_MAP& levels_ccs_map,twoDarray* ptwoDarray_ptr);
void print_connected_components_for_particular_level(
   int curr_threshold,int prev_threshold,
   vector<connected_component_t*>& cc_ptrs,
   twoDarray* ptwoDarray_ptr,twoDarray* qtwoDarray_ptr,
   twoDarray* curr_level_cctwoDarray_ptr);

void export_all_cc_images(
   bool descending_threshold_flag,LEVELS_CCS_MAP& levels_ccs_map,
   texture_rectangle* texture_rectangle_ptr);
void export_connected_components_for_particular_level(
   bool descending_threshold_flag,int threshold,
   vector<connected_component_t*>& cc_ptrs,
   texture_rectangle* texture_rectangle_ptr);

void compute_inverse_compactness_distribution(
   bool descending_threshold_flag,LEVELS_CCS_MAP& levels_ccs_map);
void inverse_compactness_per_threshold_level(
   vector<connected_component_t*>& cc_ptrs,
   vector<double>& inverse_compactness);


// ==========================================================================

// Method instantiate_test_ptwoDarray() hardwires the following array
// with 6 rows and 7 columns into a dynamically generated twoDarray:

//     col =  0  1  2  3  4  5  6

// row = 0    6  3  2  5  3  4  7 

// row = 1    4  9  8  0  1  6  5

// row = 2    2  3  0  2  0  1  2

// row = 3    7  5  0  1  7  6  3

// row = 4    4  3  1  2  9  9  4

// row = 5    5  8  3  0  4  0  5

twoDarray* instantiate_test_ptwoDarray(long& seed)
{
   unsigned int width=2;
   unsigned int height=2;

//   unsigned int width=7;
//   unsigned int height=6;

//   unsigned int width=8;
//   unsigned int height=7;
   twoDarray* ptwoDarray_ptr=new twoDarray(width,height);

/*
   ptwoDarray_ptr->clear_values();
   ptwoDarray_ptr->put(1,1,9);
   ptwoDarray_ptr->put(2,2,8);
   ptwoDarray_ptr->put(2,3,8);
*/

/*
   ptwoDarray_ptr->put(0,0,6);
   ptwoDarray_ptr->put(1,0,3);
   ptwoDarray_ptr->put(2,0,2);
   ptwoDarray_ptr->put(3,0,5);
   ptwoDarray_ptr->put(4,0,3);
   ptwoDarray_ptr->put(5,0,4);
   ptwoDarray_ptr->put(6,0,7);

   ptwoDarray_ptr->put(0,1,4);
   ptwoDarray_ptr->put(1,1,9);
   ptwoDarray_ptr->put(2,1,8);
   ptwoDarray_ptr->put(3,1,0);
   ptwoDarray_ptr->put(4,1,1);
   ptwoDarray_ptr->put(5,1,6);
   ptwoDarray_ptr->put(6,1,5);

   ptwoDarray_ptr->put(0,2,2);
   ptwoDarray_ptr->put(1,2,3);
   ptwoDarray_ptr->put(2,2,0);
   ptwoDarray_ptr->put(3,2,2);
   ptwoDarray_ptr->put(4,2,0);
   ptwoDarray_ptr->put(5,2,1);
   ptwoDarray_ptr->put(6,2,2);

   ptwoDarray_ptr->put(0,3,7);
   ptwoDarray_ptr->put(1,3,5);
   ptwoDarray_ptr->put(2,3,0);
   ptwoDarray_ptr->put(3,3,1);
   ptwoDarray_ptr->put(4,3,7);
   ptwoDarray_ptr->put(5,3,6);
   ptwoDarray_ptr->put(6,3,3);

   ptwoDarray_ptr->put(0,4,4);
   ptwoDarray_ptr->put(1,4,3);
   ptwoDarray_ptr->put(2,4,1);
   ptwoDarray_ptr->put(3,4,2);
   ptwoDarray_ptr->put(4,4,9);
   ptwoDarray_ptr->put(5,4,9);
   ptwoDarray_ptr->put(6,4,4);

   ptwoDarray_ptr->put(0,5,5);
   ptwoDarray_ptr->put(1,5,8);
   ptwoDarray_ptr->put(2,5,3);
   ptwoDarray_ptr->put(3,5,0);
   ptwoDarray_ptr->put(4,5,4);
   ptwoDarray_ptr->put(5,5,0);
   ptwoDarray_ptr->put(6,5,5);
*/

   cout << "Enter random number generator seed:" << endl;
   cin >> seed;
   nrfunc::init_default_seed(seed);

   for (unsigned int c=0; c<width; c++)
   {
      for (unsigned int r=0; r<height; r++)
      {
         int curr_value=10*nrfunc::ran1();
         ptwoDarray_ptr->put(c,r,curr_value);
      }
   }

   return ptwoDarray_ptr;
}

// --------------------------------------------------------------------------
// Method print_array_values() takes in *ptwoDarray_ptr which is
// assumed to contain integer values (possibly equaling -1).  It
// prints out the matrix corresponding to *ptwoDarray_ptr where
// negative values are replaced by period symbols.  If
// cc_twoDarray_ptr != NULL, distinct connected components are
// colored.

void print_array_values(
   twoDarray* ptwoDarray_ptr,twoDarray* cc_twoDarray_ptr)
{
   Color::Modifier fg_default(Color::FG_DEFAULT);
   Color::Modifier fg_red(Color::FG_RED);
   Color::Modifier fg_orange(Color::FG_ORANGE);
   Color::Modifier fg_yellow(Color::FG_YELLOW);
   Color::Modifier fg_green(Color::FG_GREEN);
   Color::Modifier fg_cyan(Color::FG_CYAN);
   Color::Modifier fg_blue(Color::FG_BLUE);
   Color::Modifier fg_purple(Color::FG_PURPLE);  
   Color::Modifier fg_black(Color::FG_BLACK);
   Color::Modifier fg_white(Color::FG_WHITE);
   Color::Modifier fg_grey(Color::FG_GREY);

   Color::Modifier bg_def(Color::BG_DEFAULT);
   Color::Modifier bg_red(Color::BG_RED);
   Color::Modifier bg_orange(Color::BG_ORANGE);
   Color::Modifier bg_yellow(Color::BG_YELLOW);
   Color::Modifier bg_green(Color::BG_GREEN);
   Color::Modifier bg_cyan(Color::BG_CYAN);
   Color::Modifier bg_blue(Color::BG_BLUE);
   Color::Modifier bg_purple(Color::BG_PURPLE);
   Color::Modifier bg_black(Color::BG_BLACK);
   Color::Modifier bg_white(Color::BG_WHITE);
   Color::Modifier bg_grey(Color::BG_GREY);

   vector<Color::Modifier> text_colors;
   text_colors.push_back(fg_red);		
   text_colors.push_back(fg_green);		
   text_colors.push_back(fg_blue);		
   text_colors.push_back(fg_orange);		
   text_colors.push_back(fg_cyan);		
   text_colors.push_back(fg_purple);	       

   for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
      {
         double curr_p=ptwoDarray_ptr->get(px,py);
         int curr_color=-1;
         if (cc_twoDarray_ptr != NULL)
         {
            int curr_cc=cc_twoDarray_ptr->get(px,py);
            curr_color=curr_cc % text_colors.size();
         }
         
         if (curr_p < 0)
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
               cout << fg_default << " ";
            }
         }
      } // loop over px
      cout << endl;
   } // loop over py
} 

// --------------------------------------------------------------------------
// Method display_region_as_array() takes in some extremal region
// within *region_ptr.  Looping over the region's pixel IDs, it fills
// a new twoDarray *qtwoDarray_ptr with region pixel values.  A
// subsequent call to print_array_values() then displays the region in
// matrix form.  

void display_region_as_array(extremal_region_t* region_ptr,
                             twoDarray* ptwoDarray_ptr)
{
   twoDarray* qtwoDarray_ptr = new twoDarray(ptwoDarray_ptr);
   qtwoDarray_ptr->initialize_values(-1);

   for (INT_SET::iterator iter=region_ptr->pixel_IDs.begin(); 
        iter != region_ptr->pixel_IDs.end(); iter++)
   {
      int pixel_ID=*iter;
      unsigned int width = ptwoDarray_ptr->get_mdim();
      unsigned int px,py;
      graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
      int pixel_value = ptwoDarray_ptr->get(px,py);
      qtwoDarray_ptr->put(px,py,pixel_value);
   }
   print_array_values(qtwoDarray_ptr,NULL);
   delete qtwoDarray_ptr;
}

// --------------------------------------------------------------------------
// Method print_extremal_region() writes out various parameters
// associated with input extremal region *region_ptr.

void print_extremal_region(extremal_region_t* region_ptr,
                           twoDarray* ptwoDarray_ptr)
{
   cout << "Extremal region index = " << region_ptr->build_index << endl;
   cout << "Number pixels in extremal region = "
        << region_ptr->pixel_IDs.size() << endl;
   cout << "Max_value = " << region_ptr->max_value
        << " min_value = " << region_ptr->min_value
        << endl;

   int pixel_ID_counter=0;
   for (INT_SET::iterator iter=region_ptr->pixel_IDs.begin(); 
        iter != region_ptr->pixel_IDs.end(); iter++)
   {
      int pixel_ID=*iter;
      unsigned int width = ptwoDarray_ptr->get_mdim();
      unsigned int px,py;
      graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
      int pixel_value = ptwoDarray_ptr->get(px,py);
      cout << "pixel_ID_counter=" << pixel_ID_counter
           << "  px=" << px << " py=" << py 
           << "  pixel_value = " << pixel_value 
           << endl;
      pixel_ID_counter++;
   }
}

// --------------------------------------------------------------------------
// Method print_connected_component() writes out various parameters
// associated with input connected component *cc_ptr.

void print_connected_component(connected_component_t* cc_ptr,
                               twoDarray* ptwoDarray_ptr)
{
   cout << "Connected component ID = " << cc_ptr->ID << endl;

   if (cc_ptr->parent_cc_ptr != NULL)
      cout << "  Parent cc ID = " << cc_ptr->parent_cc_ptr->ID << endl;

   if (cc_ptr->children_cc_ptrs.size() > 0)
   {
      cout << "  Children cc IDs: ";

      typedef set<_connected_component*> CC_SET;
      for (CC_SET::iterator iter=cc_ptr->children_cc_ptrs.begin();
           iter != cc_ptr->children_cc_ptrs.end(); iter++)
      {
         int child_cc_ID = (*iter)->ID;
         cout << child_cc_ID << "  " << flush;
      }
   }
   cout << endl;
   cout << "  N_pixels  = " << cc_ptr->pixel_IDs.size() << endl;
   cout << "  Pixel perimeter  = " << cc_ptr->pixel_perim << endl;

   Color::Modifier fg_red(Color::FG_RED);
   Color::Modifier fg_default(Color::FG_DEFAULT);
   int n_4_holes = cc_ptr->n_holes;
   if (n_4_holes > 0)
   {
      cout << fg_red;
   }
   cout << "N_4_holes = " << n_4_holes << fg_default << endl;
   
/*

   cout << "  Max_value = " << cc_ptr->max_value
        << " min_value = " << cc_ptr->min_value
        << endl;

   int pixel_ID_counter=0;
   for (INT_SET::iterator iter=cc_ptr->pixel_IDs.begin(); 
        iter != cc_ptr->pixel_IDs.end(); iter++)
   {
      int pixel_ID=*iter;
      unsigned int width = ptwoDarray_ptr->get_mdim();
      unsigned int px,py;
      graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
      int pixel_value = ptwoDarray_ptr->get(px,py);
      cout << "pixel_ID_counter=" << pixel_ID_counter
           << "  px=" << px << " py=" << py 
           << "  pixel_value = " << pixel_value 
           << endl;
      pixel_ID_counter++;
   }
*/
}

// --------------------------------------------------------------------------
// Method construct_connected_component() transfers common information
// from a "working" extremal region to an "archived" connected
// component. 

connected_component_t* construct_connected_component(
   extremal_region_t* region_ptr)
{
   connected_component_t* cc_ptr=new connected_component_t; 

   cc_ptr->parent_cc_ptr=NULL;
   cc_ptr->pixel_IDs=region_ptr->pixel_IDs;
   cc_ptr->bbox=region_ptr->bbox;
   cc_ptr->pixel_perim=region_ptr->pixel_perim;
   cc_ptr->n_holes=basic_math::max(0,1-region_ptr->Euler_number);

   return cc_ptr;
}

// --------------------------------------------------------------------------
// Method convert_extremal_regions_to_connected_components() 

vector<connected_component_t*>  
convert_extremal_regions_to_connected_components(
   bool descending_threshold_flag,bool test_ptwoDarray_flag,
   bool print_extremal_regions_data_flag,int curr_threshold,
   unsigned int min_cc_pixel_size,unsigned int max_cc_pixel_size,
   vector<extremal_region_t>& extremal_regions,
   twoDarray* ptwoDarray_ptr,twoDarray* qtwoDarray_ptr,
   twoDarray* prev_level_cctwoDarray_ptr,
   twoDarray* curr_level_cctwoDarray_ptr,CCS_MAP& ccs_map)
{
//   cout << "inside convert_extremal_regions_to_connected_components()" << endl;
//   cout << "curr_threshold = " << curr_threshold << endl;
   
   qtwoDarray_ptr->initialize_values(-1);

   vector<connected_component_t*> cc_ptrs;
   for (unsigned int e=0; e<extremal_regions.size(); e++)
   {
      extremal_region_t* region_ptr=&extremal_regions.at(e);
      if (!region_ptr->active_flag) continue;
      if (descending_threshold_flag && 
          region_ptr->min_value < curr_threshold) continue;
      if (!descending_threshold_flag && 
          region_ptr->min_value > curr_threshold) continue;

      if (!test_ptwoDarray_flag)
      {

// We want to minimize expensive connected component construction.  So
// we impose a few basic criteria which extremal regions corresponding
// to characters are very likely to satisfy:


// 1.  Ignore any extremal region containing too few or too many
// pixels to be of interest:

         if (region_ptr->pixel_IDs.size() < min_cc_pixel_size) continue;
         if (region_ptr->pixel_IDs.size() > max_cc_pixel_size) continue;

// 2.  Ignore any extremal region whose brightness variation exceeds
// some threshold fraction of the maximum possible brightness
// difference:

         const double max_value_spread = 0.4 * 256;
//         const double max_value_spread = 0.5 * 256;
         if (region_ptr->max_value - region_ptr->min_value > max_value_spread)
            continue;

// 3.  Ignore any extremal regions whose "inverse compactness" =
// perimeter / sqrt(area) exceeds some large threshold:

         double inverse_compactness=double(region_ptr->pixel_perim)/
            sqrt(region_ptr->pixel_IDs.size());
         const double max_inverse_compactness_for_chars = 10;
//         const double max_inverse_compactness_for_chars = 12;
         if (inverse_compactness > max_inverse_compactness_for_chars) continue;

// 4.  Ignore any extremal regions whose number of 4-holes exceeds
// some threshold:

         int n_four_holes=basic_math::max(0,1-region_ptr->Euler_number);
         const int max_n_four_holes = 3;
         if (n_four_holes > max_n_four_holes) continue;

      }
      
      connected_component_t* cc_ptr=construct_connected_component(region_ptr);
      cc_ptrs.push_back(cc_ptr);

      cc_ptr->ID=ccs_map.size();
      ccs_map[cc_ptr->ID]=cc_ptr;

      if (print_extremal_regions_data_flag)
	print_extremal_region(region_ptr,ptwoDarray_ptr);

//      cout << "Extremal region ID = " << region_ptr->ID << endl;
//      cout << "Number pixels in extremal region = "
//           << region_ptr->pixel_IDs.size() << endl;
//      cout << endl;

      unsigned int width = ptwoDarray_ptr->get_mdim();
      unsigned int px,py;

      for (INT_SET::iterator iter=region_ptr->pixel_IDs.begin(); 
           iter != region_ptr->pixel_IDs.end(); iter++)
      {
         int pixel_ID=*iter;
         graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
         int pixel_value = ptwoDarray_ptr->get(px,py);
         qtwoDarray_ptr->put(px,py,pixel_value);
         curr_level_cctwoDarray_ptr->put(px,py,cc_ptr->ID);

// Assign parent-child IDs to vertically related connected components:

         int child_cc_ID=prev_level_cctwoDarray_ptr->get(px,py);
         CCS_MAP::iterator cc_iter=ccs_map.find(child_cc_ID);
         if (cc_iter != ccs_map.end())
         {
            connected_component_t* child_cc_ptr=cc_iter->second;
            child_cc_ptr->parent_cc_ptr=cc_ptr;
//            cout << "child cc ID = " << child_cc_ptr->ID
//                 << " parent cc ID = " << cc_ptr->ID << endl;
            cc_ptr->children_cc_ptrs.insert(child_cc_ptr);
         }
      }

   } // loop over index e labeling all extremal regions

/*
   cout << "Original array:" << endl;
   print_array_values(ptwoDarray_ptr,NULL);

   cout << endl;
   cout << "Current connected components:" << endl;
   print_array_values(qtwoDarray_ptr,curr_level_cctwoDarray_ptr);
   cout << endl;

   cout << "Curr_level_cctwoDarray = " << endl;
   print_array_values(curr_level_cctwoDarray_ptr,NULL);
   cout << endl;

   cout << "Prev_level_cctwoDarray = " << endl;
   print_array_values(prev_level_cctwoDarray_ptr,NULL);

   cout << "====================================================" 
        << endl;
*/

   return cc_ptrs;
}

// --------------------------------------------------------------------------
// Method initialize_extremal_regions() sets the ith entry within STL
// vector extremal_regions equal to the ith pixel within the image
// corresponding to *ptwoDarray_ptr.  Each extremal_region
// consequently begins as just a single pixel.

void initialize_extremal_regions(
   twoDarray* ptwoDarray_ptr,vector<extremal_region_t>& extremal_regions)
{
//   cout << "inside initialize_extremal_regions()" << endl;

   unsigned int width=ptwoDarray_ptr->get_mdim();
   unsigned int height=ptwoDarray_ptr->get_ndim();

   extremal_regions.clear();

   for (unsigned int pixel_ID=0; pixel_ID<width*height; pixel_ID++)
   {
      unsigned int px,py;
      graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
      int pixel_value=ptwoDarray_ptr->get(px,py);

      extremal_region_t curr_region;
      curr_region.active_flag=true;
      curr_region.build_index=pixel_ID;
      curr_region.pixel_IDs.insert(pixel_ID);
      curr_region.max_value=pixel_value;
      curr_region.min_value=pixel_value;
      curr_region.bbox=bounding_box(px,px+1,py,py+1);
      curr_region.pixel_perim=4;
      curr_region.Euler_number=1;	// n_8_ccs - n_4_holes = 1 - 0 = 1

      extremal_regions.push_back(curr_region);
   } // loop over pixel_ID
}

// --------------------------------------------------------------------------
// Method generate_pixel_neighbor_edges() takes in STL vector
// neighboring_pixel_edges which holds DUPLEs of pixel IDs that
// represent edges between neighboring pixels.  It also imports STL
// vector neighboring_pixel_values() which represents neighboring
// pixel values as
// lo_pixel_value*MAX_POSSIBLE_PIXEL_VALUE+hi_pixel_value.  This
// effectively encodes neighboring pixel values as a DUPLE.

void generate_pixel_neighbor_edges(
   bool descending_threshold_flag,int MAX_POSSIBLE_PIXEL_VALUE,
   twoDarray* ptwoDarray_ptr,
   vector<DUPLE>& neighboring_pixel_edges,
   vector<int>& neighboring_pixel_values)
{
   cout << "inside generate_pixel_neighbor_edges()" << endl;

   unsigned int width=ptwoDarray_ptr->get_mdim();
   unsigned int height=ptwoDarray_ptr->get_ndim();

   double min_p,max_p;
   ptwoDarray_ptr->minmax_values(min_p,max_p);
//   cout << "min_p = " << min_p << " max_p = " << max_p << endl;

// Fill STL vectors neighboring_pixel_values and
// neighboring_pixel_edges:

   unsigned int qx,qy;
   int curr_pixel_ID,next_pixel_ID;
   int curr_pixel_value,next_pixel_value;
   int curr_neighbor_pixel_values;
   DUPLE curr_neighbor_pixel_edge;

   for (unsigned int py=0; py<height; py++)
   {
      for (unsigned int px=0; px<width; px++)
      {
         curr_pixel_ID=graphicsfunc::get_pixel_ID(px,py,width);
         curr_pixel_value=ptwoDarray_ptr->get(px,py);

// Loop over neighboring pixels located to the northeast, east,
// southeast and south of current pixel.  We also consider the current
// pixel to be its own neighbor.  Add the neighbors' pixel
// values and edges to STL vectors neighboring_pixel_values and
// neighboring_pixel_edges:

         unsigned int n_neighbor_dirs=5;
         for (unsigned int neighbor_dir=0; neighbor_dir<n_neighbor_dirs; 
              neighbor_dir++)
         {
            next_pixel_ID=-1;

// Northeast neighbor         

            if (neighbor_dir==0 && py > 0 && px < width-1)
            {
               qx=px+1;
               qy=py-1;
               next_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
            }

// East neighbor         

            if (neighbor_dir==1 && px < width-1)
            {
               qx=px+1;
               qy=py;
               next_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
            }

// Southeast neighbor         

            if (neighbor_dir==2 && py < height-1 && px < width-1)
            {
               qx=px+1;
               qy=py+1;
               next_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
            }

// South neighbor         

            if (neighbor_dir==3 && py < height-1)
            {
               qx=px;
               qy=py+1;
               next_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
            }
         
// Current pixel as its own neighbor

            if (neighbor_dir==4)
            {
               qx=px;
               qy=py;
               next_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
            }

            if (next_pixel_ID < 0) continue;

            next_pixel_value=ptwoDarray_ptr->get(qx,qy);

            if ((descending_threshold_flag && curr_pixel_value < 
                 next_pixel_value)  ||
                (!descending_threshold_flag && curr_pixel_value >
                 next_pixel_value))
            {
               curr_neighbor_pixel_values=
                  curr_pixel_value*MAX_POSSIBLE_PIXEL_VALUE+next_pixel_value;
               curr_neighbor_pixel_edge=DUPLE(curr_pixel_ID,next_pixel_ID);
            }
            else
            {
               curr_neighbor_pixel_values=
                  next_pixel_value*MAX_POSSIBLE_PIXEL_VALUE+curr_pixel_value;
               curr_neighbor_pixel_edge=DUPLE(next_pixel_ID,curr_pixel_ID);
            }
         
            neighboring_pixel_values.push_back(curr_neighbor_pixel_values);
            neighboring_pixel_edges.push_back(curr_neighbor_pixel_edge);

/*
  cout << "neighbor_dir = " << neighbor_dir << endl;
  cout << "curr_pixel_value = " << curr_pixel_value
  << " next_pixel_value = " << next_pixel_value << endl;
  cout << "curr_pixel_ID = " << curr_pixel_ID
  << " next_pixel_ID = " << next_pixel_ID << endl << endl;
*/

         } // loop over neighbor_dir
         
      } // loop over px index
   } // loop over py index

// Sort edges between neighboring pixels such that edges between
// brighter [or darker] pixels come first:

   if (descending_threshold_flag)
   {
      templatefunc::Quicksort_descending(
         neighboring_pixel_values,neighboring_pixel_edges);
   }
   else
   {
      templatefunc::Quicksort(
         neighboring_pixel_values,neighboring_pixel_edges);
   }
}

// --------------------------------------------------------------------------
// Method construct_extremal_regions() takes in greyscale image within
// input *ptwoDarray_ptr.  It also imports sorted neighboring pixel
// edge information within STL vectors neighboring_pixel_edges &
// neighboring_pixel_values.  Working with a union-find object, it
// marches through the pixel neighbor edges and adds pixels to growing
// connected components.  Whenever a new threshold level is crossed,
// the current set of connected components is printed out.

// This method returns the number of connected components
// within the tree.

int construct_extremal_regions(
   bool descending_threshold_flag,bool test_ptwoDarray_flag,
   bool print_extremal_regions_data_flag,int MAX_POSSIBLE_PIXEL_VALUE,
   twoDarray* ptwoDarray_ptr,vector<extremal_region_t>& extremal_regions,
   vector<DUPLE>& neighboring_pixel_edges,
   vector<int>& neighboring_pixel_values,
   LEVELS_CCS_MAP& levels_ccs_map)
{
   cout << "inside construct_extremal_regions()" << endl;

// Instantiate twoDarrays to hold IDs for connected components found
// at current and previous threshold levels.  We'll use these
// twoDarrays to establish parent-child relationships between extremal
// regions at different threshold levels:

   twoDarray* curr_level_cctwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
   twoDarray* prev_level_cctwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
   twoDarray* working_cc_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);

   curr_level_cctwoDarray_ptr->initialize_values(-1);
   prev_level_cctwoDarray_ptr->initialize_values(-1);

   unsigned int width=ptwoDarray_ptr->get_mdim();
   unsigned int height=ptwoDarray_ptr->get_ndim();

   unsigned int n_edges=neighboring_pixel_values.size();
   cout << "neighboring_pixel_edges.size() = " << n_edges << endl;
   outputfunc::enter_continue_char();

/*
// FAKE FAKE:
//   n_edges=20;
//   cout << "Enter number of neighboring pixel edges to analyze:" << endl;
//   cin >> n_edges;

   for (unsigned int e=0; e<n_edges; e++)
   {
      outputfunc::update_progress_fraction(e,n_edges/100,n_edges);

      int neighbor_pixel_value=neighboring_pixel_values[e];
      int pixel_hi_value,pixel_lo_value;

      if (descending_threshold_flag)
      {
         pixel_lo_value=neighbor_pixel_value/MAX_POSSIBLE_PIXEL_VALUE;
         pixel_hi_value=neighbor_pixel_value%MAX_POSSIBLE_PIXEL_VALUE;
      }
      else
      {
         pixel_hi_value=neighbor_pixel_value/MAX_POSSIBLE_PIXEL_VALUE;
         pixel_lo_value=neighbor_pixel_value%MAX_POSSIBLE_PIXEL_VALUE;
      }
      
      int lo_pixel_ID=neighboring_pixel_edges[e].first;
      int hi_pixel_ID=neighboring_pixel_edges[e].second;
      
      unsigned int lo_px,lo_py,hi_px,hi_py;
      graphicsfunc::get_pixel_px_py(lo_pixel_ID,width,lo_px,lo_py);
      graphicsfunc::get_pixel_px_py(hi_pixel_ID,width,hi_px,hi_py);


      cout << "Edge:" << e ;
      if (descending_threshold_flag)
      {
         cout << " lo_value=" << pixel_lo_value
              << " hi_value=" << pixel_hi_value
              << " lo_px,lo_py=" << lo_px << "," << lo_py << "  "
              << " hi_px,hi_py=" << hi_px << "," << hi_py << "  ";
      }
      else
      {
         cout << " hi_value=" << pixel_hi_value
              << " lo_value=" << pixel_lo_value
              << " hi_px,hi_py=" << hi_px << "," << hi_py << "  "
              << " lo_px,lo_py=" << lo_px << "," << lo_py << "  ";
      }
      cout << endl;

   } // loop over index e labeling neighboring pixel edges
*/

   cout << "====================================================" 
        << endl;

   double min_p,max_p;
   ptwoDarray_ptr->minmax_values(min_p,max_p);
//   cout << "min_p = " << min_p << " max_p = " << max_p << endl;

   int curr_threshold;
   if (descending_threshold_flag)
   {
      curr_threshold=max_p;
   }
   else
   {
      curr_threshold=min_p;
   }

// Instantiate union-find datastructure to keep track of extremal
// region merging.  As of 4/13/14, we use dlib's very light-weight
// "disjoint_subsets" data structure rather than our own heavier,
// slower "union_find" class:

   dlib::disjoint_subsets dlib_union_find;

// Initialize union_find set so that it contains one integer
// for each pixel within the input image:

   dlib_union_find.set_size(width*height);

// Instantiate "working" twoDarray:

   twoDarray* qtwoDarray_ptr=new twoDarray(ptwoDarray_ptr);

// Instantiate "working" STL map to keep track of all connected
// components as function of their IDs:

   CCS_MAP ccs_map;

   outputfunc::print_elapsed_time(); 

   int n_ccs=0;

// On 4/17/14, Fredrik indicated that the smallest characters which 
// need to be detected will be roughly 20 pixels x 20 pixels in size:

   const unsigned int min_cc_pixel_size=35;	// Smallest chars

   cout << "min_cc_pixel_size = " << min_cc_pixel_size << endl;
   const unsigned int max_cc_pixel_size=0.1 * width * height;
   cout << "max_cc_pixel_size = " << max_cc_pixel_size << endl;

   for (unsigned int e=0; e<n_edges; e++)
   {
      outputfunc::update_progress_fraction(e,n_edges/10,n_edges);

// Retrieve index values for the 2 pixels connected by the current
// edge:

      int lo_pixel_ID = neighboring_pixel_edges[e].first;
      int hi_pixel_ID = neighboring_pixel_edges[e].second;

      int root_lo_ID=dlib_union_find.find_set(lo_pixel_ID);
      int root_hi_ID=dlib_union_find.find_set(hi_pixel_ID);
    
/*
  cout << "e = " << e 
  << " hi_pixel_ID = " << hi_pixel_ID 
  << " lo_pixel_ID = " << lo_pixel_ID
  << endl;
  cout << "root_hi_ID = " << root_hi_ID
  << " root_lo_ID = " << root_lo_ID << endl;
*/
      
// Check if extremal regions containing these 2 pixels have already
// been merged.  If so, there's nothing to do:

      if (root_lo_ID == root_hi_ID) continue;

      extremal_region_t* lo_region_ptr=&extremal_regions[root_lo_ID];
      extremal_region_t* hi_region_ptr=&extremal_regions[root_hi_ID];

// Check if either *lo_region_ptr or *high_region_ptr has crossed an
// intensity threshold boundary.  If so, print out current set of
// extremal regions BEFORE we potentially add *merged_region_ptr to
// the output list of extremal regions:

      if (descending_threshold_flag &&
          (lo_region_ptr->min_value < curr_threshold ||
           hi_region_ptr->min_value < curr_threshold))
      {
         vector<connected_component_t*> cc_ptrs=
            convert_extremal_regions_to_connected_components(
               descending_threshold_flag,test_ptwoDarray_flag,
               print_extremal_regions_data_flag,curr_threshold,
               min_cc_pixel_size,max_cc_pixel_size,extremal_regions,
               ptwoDarray_ptr,qtwoDarray_ptr,
               prev_level_cctwoDarray_ptr,curr_level_cctwoDarray_ptr,ccs_map);
         levels_ccs_map[curr_threshold]=cc_ptrs;
         n_ccs += cc_ptrs.size();

         curr_threshold = basic_math::min(
            lo_region_ptr->min_value,hi_region_ptr->min_value);

// This copy is somewhat expensive.  But most time is occurring in
// preceding convert_extremal_regions_to_connected_components call.

         curr_level_cctwoDarray_ptr->copy(prev_level_cctwoDarray_ptr);
      }

      if (!descending_threshold_flag &&
          (lo_region_ptr->min_value > curr_threshold ||
           hi_region_ptr->min_value > curr_threshold))
      {
         vector<connected_component_t*> cc_ptrs=
            convert_extremal_regions_to_connected_components(
               descending_threshold_flag,test_ptwoDarray_flag,
               print_extremal_regions_data_flag,curr_threshold,
               min_cc_pixel_size,max_cc_pixel_size,extremal_regions,
               ptwoDarray_ptr,qtwoDarray_ptr,
               prev_level_cctwoDarray_ptr,curr_level_cctwoDarray_ptr,ccs_map);
         levels_ccs_map[curr_threshold]=cc_ptrs;
         n_ccs += cc_ptrs.size();

         curr_threshold = basic_math::max(
            lo_region_ptr->max_value,hi_region_ptr->max_value);
         curr_level_cctwoDarray_ptr->copy(prev_level_cctwoDarray_ptr);
      }

      int merged_root_ID = dlib_union_find.merge_sets(root_lo_ID,root_hi_ID);

      int submerged_root_ID=root_lo_ID;
      if (merged_root_ID==root_lo_ID)
      {
         submerged_root_ID=root_hi_ID;
      }
      extremal_region_t* merged_region_ptr=&extremal_regions[merged_root_ID];
      extremal_region_t* submerged_region_ptr=
         &extremal_regions[submerged_root_ID];

/*
  cout << "merged_root_ID = " << merged_root_ID << endl;
  cout << "submerged_root_ID = " << submerged_root_ID << endl;
  cout << "merged_region_ptr->min_value = "
  << merged_region_ptr->min_value 
  << " submerged_region_ptr->min_value = "
  << submerged_region_ptr->min_value << endl;
*/

      submerged_region_ptr->active_flag=false;
      merged_region_ptr->active_flag=true;
      
// Update data for merged extremal region within *merged_region_ptr:

      merged_region_ptr->build_index = basic_math::min(
         merged_region_ptr->build_index,submerged_region_ptr->build_index);

      merged_region_ptr->max_value = basic_math::max(
         merged_region_ptr->max_value,submerged_region_ptr->max_value);
      merged_region_ptr->min_value = basic_math::min(
         merged_region_ptr->min_value,submerged_region_ptr->min_value);

      merged_region_ptr->bbox.update_bounds(&submerged_region_ptr->bbox);

// If new bbox has aspect ratio significantly inconsistent with unity,
// it most likely does not enclose a character:

      const double max_bbox_aspect_ratio=2.5;
      const double min_bbox_aspect_ratio=1.0/2.5;
      
      double bbox_aspect_ratio=merged_region_ptr->bbox.get_aspect_ratio();
      if (bbox_aspect_ratio > max_bbox_aspect_ratio ||
          bbox_aspect_ratio < min_bbox_aspect_ratio)
      {
         merged_region_ptr->active_flag=false;
      }

// Append all pixel_IDs from extremal region corresponding to
// submerged_root_ID onto pixel_IDs from extremal region corresponding
// to merged_root_ID:

      unsigned int tmp_px,tmp_py; 
      int qx,qy,pfill=1;
      for (INT_SET::iterator iter=submerged_region_ptr->pixel_IDs.begin(); 
           iter != submerged_region_ptr->pixel_IDs.end(); iter++)
      {
         int submerged_region_pixel_ID=*iter;
         graphicsfunc::get_pixel_px_py(
            submerged_region_pixel_ID,width,tmp_px,tmp_py);
         int px=tmp_px;
         int py=tmp_py;
//          int submerged_pixel_value = ptwoDarray_ptr->get(px,py);

// Record *ptwoDarray_ptr values for 8-neighbors of pixel (px,py)
// within STL vector neighbor_values:

         int n_pixel_four_neighbors=0;
         vector<int> binary_neighbor_values;
         for (unsigned int n=0; n<9; n++)
         {
            if (n==0)		
            {
               qx=px-1;
               qy=py-1;
            }
            else if (n==1)	
            {
               qx=px;
               qy=py-1;
            }
            else if (n==2)	
            {
               qx=px+1;
               qy=py-1;
            }
            else if (n==3)	
            {
               qx=px-1;
               qy=py;
            }
            else if (n==4)	
            {
               qx=px;
               qy=py;
            }
            else if (n==5)	
            {
               qx=px+1;
               qy=py;
            }
            else if (n==6)     
            {
               qx=px-1;
               qy=py+1;
            }
            else if (n==7)	
            {
               qx=px;
               qy=py+1;
            }
            else if (n==8)	
            {
               qx=px+1;
               qy=py+1;
            }

            if (!ptwoDarray_ptr->pixel_inside_working_region(qx,qy)) 
            {
               binary_neighbor_values.push_back(0);
            }
            else
            {
               int neighbor_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,width);
               INT_SET::iterator iter=
                  merged_region_ptr->pixel_IDs.find(neighbor_pixel_ID);
               if (iter != merged_region_ptr->pixel_IDs.end())
               {
                  binary_neighbor_values.push_back(pfill);
                  if (is_odd(n)) n_pixel_four_neighbors++;
               }
	       else
	       {
                  binary_neighbor_values.push_back(0);
	       }
	       
            } // (qx,qy) inside working region conditional
         } // loop over index n labeling 8-neighbors of submerged pixel 

//         cout << "Submerged_region_pixel_ID = "
//              << submerged_region_pixel_ID 
//              << " has n_pixel_four_neighbors = " << n_pixel_four_neighbors
//              << endl;

         merged_region_ptr->pixel_perim += 4-2*n_pixel_four_neighbors;

// Q1: Why does submerged_pixel_value sometimes NOT equal
// curr_threshold? 
// A1:  Submerged_pixel_value >= curr_threshold

         double delta_Euler_number=binaryimagefunc::
            delta_Euler_number_for_single_pixel(
               px,py,pfill,binary_neighbor_values);
         merged_region_ptr->Euler_number += delta_Euler_number;

         merged_region_ptr->pixel_IDs.insert(submerged_region_pixel_ID);

      } // loop over iter index labeling submerged region pixel IDs
   } // loop over index e labeling neighboring pixel edges
   cout << endl;

   vector<connected_component_t*> cc_ptrs=
      convert_extremal_regions_to_connected_components(
         descending_threshold_flag,test_ptwoDarray_flag,
         print_extremal_regions_data_flag,curr_threshold,
         min_cc_pixel_size,max_cc_pixel_size,extremal_regions,
         ptwoDarray_ptr,qtwoDarray_ptr,prev_level_cctwoDarray_ptr,
         curr_level_cctwoDarray_ptr,ccs_map);
   levels_ccs_map[curr_threshold]=cc_ptrs;

   delete curr_level_cctwoDarray_ptr;
   delete prev_level_cctwoDarray_ptr;
   delete working_cc_twoDarray_ptr;
   delete qtwoDarray_ptr;

   return n_ccs;
}
 
// --------------------------------------------------------------------------
void print_connected_components_tree(
   bool descending_threshold_flag,
   LEVELS_CCS_MAP& levels_ccs_map,twoDarray* ptwoDarray_ptr)
{
   int prev_threshold=-999;

   twoDarray* qtwoDarray_ptr = new twoDarray(ptwoDarray_ptr);
   qtwoDarray_ptr->initialize_values(-1);
   twoDarray* curr_level_cctwoDarray_ptr = new twoDarray(ptwoDarray_ptr);
   curr_level_cctwoDarray_ptr->initialize_values(-1);

   if (descending_threshold_flag)
   {
      for (LEVELS_CCS_MAP::reverse_iterator iter=levels_ccs_map.rbegin();
           iter != levels_ccs_map.rend(); iter++)
      {
         int curr_threshold=iter->first;
         vector<connected_component_t*> cc_ptrs=iter->second;
         print_connected_components_for_particular_level(
            curr_threshold,prev_threshold,cc_ptrs,
            ptwoDarray_ptr,qtwoDarray_ptr,curr_level_cctwoDarray_ptr);
         prev_threshold=curr_threshold;
      } // loop over levels_ccs_map reverse_iterator
   }
   else
   {
      for (LEVELS_CCS_MAP::iterator iter=levels_ccs_map.begin();
           iter != levels_ccs_map.end(); iter++)
      {
         int curr_threshold=iter->first;
         vector<connected_component_t*> cc_ptrs=iter->second;
         print_connected_components_for_particular_level(
            curr_threshold,prev_threshold,cc_ptrs,
            ptwoDarray_ptr,qtwoDarray_ptr,curr_level_cctwoDarray_ptr);
         prev_threshold=curr_threshold;
      } // loop over levels_ccs_map reverse_iterator
   } // descending_threshold_flag conditional
   
   cout << endl;
   cout << "Original array:" << endl;
   print_array_values(ptwoDarray_ptr,NULL);
   cout << endl;

   cout << "Current connected components:" << endl;
   print_array_values(qtwoDarray_ptr,curr_level_cctwoDarray_ptr);
   cout << endl;

   cout << "Connected component IDs: " << endl;
   print_array_values(curr_level_cctwoDarray_ptr,NULL);
   cout << endl;

   cout << "============================================================="
        << endl;

   delete qtwoDarray_ptr;
   delete curr_level_cctwoDarray_ptr;
}

// --------------------------------------------------------------------------
void print_connected_components_for_particular_level(
   int curr_threshold,int prev_threshold,
   vector<connected_component_t*>& cc_ptrs,
   twoDarray* ptwoDarray_ptr,twoDarray* qtwoDarray_ptr,
   twoDarray* curr_level_cctwoDarray_ptr)
{
   if (curr_threshold != prev_threshold && prev_threshold >=0)
   {
      cout << "curr_threshold = " << curr_threshold
           << " prev_threshold = " << prev_threshold << endl;

      prev_threshold=curr_threshold;

      cout << endl;
      cout << "Original array:" << endl;
      print_array_values(ptwoDarray_ptr,NULL);
      cout << endl;

      cout << "Current connected components:" << endl;
      print_array_values(qtwoDarray_ptr,curr_level_cctwoDarray_ptr);
      cout << endl;

      cout << "Connected component IDs: " << endl;
      print_array_values(curr_level_cctwoDarray_ptr,NULL);
      cout << endl;

      cout << "---------------------------------------------------------"
           << endl;
      cout << "Threshold level = " << curr_threshold << endl << endl;

      qtwoDarray_ptr->initialize_values(-1);
      curr_level_cctwoDarray_ptr->initialize_values(-1);

   } // curr_threshold != prev_threshold conditional

   unsigned int px,py;
   unsigned int width = ptwoDarray_ptr->get_mdim();
   for (unsigned int i=0; i<cc_ptrs.size(); i++)
   {
      for (INT_SET::iterator iter=cc_ptrs[i]->pixel_IDs.begin(); 
           iter != cc_ptrs[i]->pixel_IDs.end(); iter++)
      {
         int pixel_ID=*iter;
         graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
         int pixel_value = ptwoDarray_ptr->get(px,py);
         qtwoDarray_ptr->put(px,py,pixel_value);
         curr_level_cctwoDarray_ptr->put(px,py,cc_ptrs[i]->ID);
      }

      print_connected_component(cc_ptrs[i],ptwoDarray_ptr);
   } // loop over index i labeling cc_ptrs
}

// --------------------------------------------------------------------------
// Method export_all_cc_images() loops over all levels within input
// STL map levels_ccs_map.  For each it extracts the threshold and
// associated connected components.  Each level's ccs are visualized
// within a colored image.

void export_all_cc_images(
   bool descending_threshold_flag,LEVELS_CCS_MAP& levels_ccs_map,
   texture_rectangle* texture_rectangle_ptr)
{

   if (descending_threshold_flag)
   {
      for (LEVELS_CCS_MAP::reverse_iterator iter=levels_ccs_map.rbegin();
           iter != levels_ccs_map.rend(); iter++)
      {
         int curr_threshold=256-iter->first;
         vector<connected_component_t*> cc_ptrs=iter->second;
         export_connected_components_for_particular_level(
            descending_threshold_flag,curr_threshold,cc_ptrs,
            texture_rectangle_ptr);
      } // loop over levels_ccs_map reverse_iterator
   }
   else
   {
      for (LEVELS_CCS_MAP::iterator iter=levels_ccs_map.begin();
           iter != levels_ccs_map.end(); iter++)
      {
         int curr_threshold=iter->first;
         vector<connected_component_t*> cc_ptrs=iter->second;
         export_connected_components_for_particular_level(
            descending_threshold_flag,curr_threshold,cc_ptrs,
            texture_rectangle_ptr);
      } // loop over levels_ccs_map reverse_iterator
   } // descending_threshold_flag conditional
}

// --------------------------------------------------------------------------
// Method export_all_cc_images() loops over all levels within input
// STL map levels_ccs_map.  For each it extracts the threshold and
// associated connected components.  Each level's ccs are visualized
// within a colored image.

void export_connected_components_for_particular_level(
   bool descending_threshold_flag,int threshold,
   vector<connected_component_t*>& cc_ptrs,
   texture_rectangle* texture_rectangle_ptr)
{
//   cout << "inside export_connected_components_for_particular_level()"
//        << endl;
//   cout << "threshold = " << threshold << endl;

// Create output subdirectories:

   string output_subdir="./connected_components/";
   filefunc::dircreate(output_subdir);
   if (descending_threshold_flag)
   {
      output_subdir += "bright_regions/";
   }
   else
   {
      output_subdir += "dark_regions/";  
   }
   filefunc::dircreate(output_subdir);
   
   texture_rectangle_ptr->clear_all_RGB_values();
   unsigned int width = texture_rectangle_ptr->getWidth();

   for (unsigned int cc=0; cc<cc_ptrs.size(); cc++)
   {
      connected_component_t* cc_ptr=cc_ptrs[cc];
      
      if (cc_ptr->children_cc_ptrs.size()==0)
      {
         int color_index=cc_ptr->ID % 17;
         cc_ptr->color=colorfunc::get_color(color_index);
      }
      else
      {
         CC_SET::iterator iter=cc_ptr->children_cc_ptrs.begin();
         connected_component_t* child_cc_ptr=*iter;         
         cc_ptr->color=child_cc_ptr->color;
      }
      colorfunc::RGB cc_rgb=colorfunc::get_RGB_values(cc_ptr->color);
      int R=255*cc_rgb.first;
      int G=255*cc_rgb.second;
      int B=255*cc_rgb.third;

      unsigned int px,py;
      for (INT_SET::iterator iter=cc_ptr->pixel_IDs.begin(); 
           iter != cc_ptr->pixel_IDs.end(); iter++)
      {
         int pixel_ID=*iter;
         graphicsfunc::get_pixel_px_py(pixel_ID,width,px,py);
         texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,R,G,B);
      } // loop over pixel_IDs

   } // loop over index cc labeling connected components for particular level

   string image_filename=output_subdir+"ccs_"+stringfunc::integer_to_string(
      threshold,3)+".jpg";
   texture_rectangle_ptr->write_curr_frame(image_filename);
   string banner="Exported "+image_filename;
   outputfunc::write_banner(banner);
   
}



// --------------------------------------------------------------------------
// Method 

// Note: Circle has minimal inverse compactness = 2 * sqrt(PI) = 3.54.  
// As of 4/13/24, we roughly estimate that most genuine characters
// have inverse compactness values less than 10.

void compute_inverse_compactness_distribution(
   bool descending_threshold_flag,LEVELS_CCS_MAP& levels_ccs_map)
{
   vector<double> inverse_compactness;
   
   if (descending_threshold_flag)
   {
      for (LEVELS_CCS_MAP::reverse_iterator iter=levels_ccs_map.rbegin();
           iter != levels_ccs_map.rend(); iter++)
      {
         vector<connected_component_t*> cc_ptrs=iter->second;
         inverse_compactness_per_threshold_level(cc_ptrs,inverse_compactness);
      } // loop over levels_ccs_map reverse_iterator
   }
   else
   {
      for (LEVELS_CCS_MAP::iterator iter=levels_ccs_map.begin();
           iter != levels_ccs_map.end(); iter++)
      {
         vector<connected_component_t*> cc_ptrs=iter->second;
         inverse_compactness_per_threshold_level(cc_ptrs,inverse_compactness);
      } // loop over levels_ccs_map reverse_iterator
   } // descending_threshold_flag conditional

   prob_distribution prob_inverse_compactness(
      inverse_compactness,100);
   prob_inverse_compactness.writeprobdists(false,true);
}


// --------------------------------------------------------------------------
// Method 

void inverse_compactness_per_threshold_level(
   vector<connected_component_t*>& cc_ptrs,
   vector<double>& inverse_compactness)
{
   for (unsigned int cc=0; cc<cc_ptrs.size(); cc++)
   {
      connected_component_t* cc_ptr=cc_ptrs[cc];
      inverse_compactness.push_back(
         double(cc_ptr->pixel_perim)/
         sqrt(double(cc_ptr->pixel_IDs.size())) );
   } // loop over index cc labeling connected components for particular level
}

// ==========================================================================
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   string image_filename=photo_ptr->get_filename();

// Always work with full resolution images.  But for development
// purposes, we crop out chunks which do not take prohibitively 
// long to process:

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
     image_filename,NULL);
   texture_rectangle_ptr->convert_color_image_to_luminosity();

//   bool print_extremal_regions_data_flag=true;
   bool print_extremal_regions_data_flag=false;

   twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();

//   bool test_ptwoDarray_flag=true;
   bool test_ptwoDarray_flag=false;

   long seed;
   if (test_ptwoDarray_flag)
   {
      ptwoDarray_ptr=instantiate_test_ptwoDarray(seed);
   }
   int width=ptwoDarray_ptr->get_mdim();
   int height=ptwoDarray_ptr->get_ndim();

   cout << "Input image width = " << width << endl;
   cout << "Input image height = " << height << endl;
   cout << "N_pixels = width * height = " << width*height << endl;

   bool descending_threshold_flag=true;
   string threshold_str;
   cout << "Enter 'd' or 'a' for descending/ascending threshold:" << endl;
   cout << "  For bright characters on dark backgrounds, choose 'd'" << endl;
   cout << "  For dark characters on bright backgrounds, choose 'a'" << endl;
   
   cin >> threshold_str;
   if (threshold_str=="a")
   {
      descending_threshold_flag=false;
   }

   timefunc::initialize_timeofday_clock();

// Instantiate STL vector to hold all extremal regions:

   vector<extremal_region_t> extremal_regions;
   initialize_extremal_regions(ptwoDarray_ptr,extremal_regions);

// As of 2/6/14, we assume all pixel intensities correspond to 8-bit
// values:

   const int MAX_POSSIBLE_PIXEL_VALUE=256;

// Represent edge between two neighboring pixels by a DUPLE of the
// pixels' IDs:

   vector<DUPLE> neighboring_pixel_edges;
   neighboring_pixel_edges.reserve(5*width*height);

// Represent neighboring pixel values as
// lo_pixel_value*MAX_POSSIBLE_PIXEL_VALUE+hi_pixel_value which effectively
// encodes the values as a DUPLE:

   vector<int> neighboring_pixel_values;
   neighboring_pixel_edges.reserve(5*width*height);

   generate_pixel_neighbor_edges(
      descending_threshold_flag,MAX_POSSIBLE_PIXEL_VALUE,
      ptwoDarray_ptr,neighboring_pixel_edges,neighboring_pixel_values);
   outputfunc::print_elapsed_time();

// Instantiate STL map to keep track of connected components as
// function of threshold level:

   LEVELS_CCS_MAP levels_ccs_map;

   int n_ccs=construct_extremal_regions(
      descending_threshold_flag,test_ptwoDarray_flag,
      print_extremal_regions_data_flag,MAX_POSSIBLE_PIXEL_VALUE,
      ptwoDarray_ptr,extremal_regions,
      neighboring_pixel_edges,neighboring_pixel_values,
      levels_ccs_map);

   cout << "Number of 'active' connected components within tree = "
        << n_ccs << endl;
   outputfunc::print_elapsed_time();


   if (test_ptwoDarray_flag)
   {
      print_connected_components_tree(
         descending_threshold_flag,levels_ccs_map,ptwoDarray_ptr);
      delete ptwoDarray_ptr;
      ptwoDarray_ptr=NULL;
      cout << "seed = " << seed << endl;
   }
   else
   {
//   compute_inverse_compactness_distribution(
//      descending_threshold_flag,levels_ccs_map);
      outputfunc::enter_continue_char();

      string blank_filename="blank.jpg";
      texture_rectangle_ptr->generate_blank_image_file(
         width,height,blank_filename,0.5);
      export_all_cc_images(
         descending_threshold_flag,levels_ccs_map,texture_rectangle_ptr);
   }
   
   delete texture_rectangle_ptr;
   photogroup_ptr->destroy_all_photos();
   delete photogroup_ptr;

   cout << "Number of 'active' connected components within tree = "
        << n_ccs << endl;
   outputfunc::print_elapsed_time();
}

