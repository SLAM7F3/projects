// ==========================================================================
// Extremal_Regions_Group class member function definitions
// ==========================================================================
// Last modified on 10/16/12; 10/23/12; 4/5/14; 6/7/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "image/extremal_regions_group.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "image/graphicsfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/union_find.h"

using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void extremal_regions_group::allocate_member_objects()
{
   dark_region_id_map_ptr=new REGION_ID_MAP;
   bright_region_id_map_ptr=new REGION_ID_MAP;
   dark_id_region_map_ptr=new ID_REGION_MAP;
   bright_id_region_map_ptr=new ID_REGION_MAP;

   bright_regions_near_dark_region_map_ptr=new ANTI_REGIONS_NEAR_REGION_MAP;
   dark_regions_near_bright_region_map_ptr=new ANTI_REGIONS_NEAR_REGION_MAP;
}		       

void extremal_regions_group::initialize_member_objects()
{
   region_counter=0;
   dark_ID_offset=10000;
   mser_twoDarray_ptr=NULL;
   tmp_twoDarray_ptr=NULL;
   bright_cc_twoDarray_ptr=NULL;
   dark_cc_twoDarray_ptr=NULL;
   bright_cc_borders_twoDarray_ptr=NULL;
   dark_cc_borders_twoDarray_ptr=NULL;
}

extremal_regions_group::extremal_regions_group() 
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

extremal_regions_group::extremal_regions_group(const extremal_regions_group& 
erg) 
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(erg);
}

extremal_regions_group::~extremal_regions_group()
{
//   cout << "inside extremal_regions_group destructor" << endl;
   purge_dark_and_bright_regions();
   delete dark_region_id_map_ptr;
   delete bright_region_id_map_ptr;
   delete dark_id_region_map_ptr;
   delete bright_id_region_map_ptr;

   delete tmp_twoDarray_ptr;

   delete bright_cc_twoDarray_ptr;
   delete dark_cc_twoDarray_ptr;
   delete bright_cc_borders_twoDarray_ptr;
   delete dark_cc_borders_twoDarray_ptr;

   delete bright_regions_near_dark_region_map_ptr;
   delete dark_regions_near_bright_region_map_ptr;
}

void extremal_regions_group::purge_dark_and_bright_regions()
{
   destroy_all_regions(dark_region_id_map_ptr);
   destroy_all_regions(bright_region_id_map_ptr);
   destroy_all_regions(dark_id_region_map_ptr);
   destroy_all_regions(bright_id_region_map_ptr);
   destroy_all_regions(dark_regions_near_bright_region_map_ptr);
   destroy_all_regions(bright_regions_near_dark_region_map_ptr);
}

void extremal_regions_group::destroy_all_regions(
   REGION_ID_MAP* region_id_map_ptr)
{
   for (REGION_ID_MAP::iterator region_id_iter=region_id_map_ptr->begin(); 
        region_id_iter != region_id_map_ptr->end();
        region_id_iter++)
   {
      extremal_region* extremal_region_ptr=region_id_iter->first;
      delete extremal_region_ptr;
   }
   region_id_map_ptr->clear();
}

void extremal_regions_group::destroy_all_regions(
   ID_REGION_MAP* id_region_map_ptr)
{
   for (ID_REGION_MAP::iterator id_region_iter=id_region_map_ptr->begin(); 
        id_region_iter != id_region_map_ptr->end();
        id_region_iter++)
   {
      extremal_region* extremal_region_ptr=id_region_iter->second;
      delete extremal_region_ptr;
   }
   id_region_map_ptr->clear();
}

void extremal_regions_group::destroy_all_regions(
   ANTI_REGIONS_NEAR_REGION_MAP* anti_regions_near_region_map_ptr)
{
   for (ANTI_REGIONS_NEAR_REGION_MAP::iterator iter=
           anti_regions_near_region_map_ptr->begin(); 
        iter != anti_regions_near_region_map_ptr->end(); iter++)
   {
      ID_REGION_MAP* id_region_map_ptr=iter->second;
      destroy_all_regions(id_region_map_ptr);
   }
   anti_regions_near_region_map_ptr->clear();
}


// ---------------------------------------------------------------------
void extremal_regions_group::docopy(const extremal_regions_group& erg)
{
//   cout << "inside extremal_regions_group::docopy()" << endl;
}

// Overload = operator:

extremal_regions_group& extremal_regions_group::operator= (const extremal_regions_group& erg)
{
   if (this==&erg) return *this;
   docopy(erg);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const extremal_regions_group& erg)
{
   outstream << "inside extremal_regions_group::operator<<" << endl;
   outstream << endl;
   return outstream;
}

// =====================================================================
// Set & get member functions
// =====================================================================

int extremal_regions_group::get_n_dark_regions() const
{
   return dark_id_region_map_ptr->size();
//   return dark_region_id_map_ptr->size();
}

int extremal_regions_group::get_n_bright_regions() const
{
   return bright_id_region_map_ptr->size();
//   return bright_region_id_map_ptr->size();
}

extremal_region* extremal_regions_group::get_dark_region(int region_ID)
{
   dark_id_region_iter=dark_id_region_map_ptr->find(region_ID);
   if (dark_id_region_iter != dark_id_region_map_ptr->end())
   {
      extremal_region* extremal_region_ptr=dark_id_region_iter->second;
      return extremal_region_ptr;
   }
   else
   {
      return NULL;
   }
}

extremal_region* extremal_regions_group::get_bright_region(int region_ID)
{
   bright_id_region_iter=bright_id_region_map_ptr->find(region_ID);
   if (bright_id_region_iter != bright_id_region_map_ptr->end())
   {
      extremal_region* extremal_region_ptr=bright_id_region_iter->second;
      return extremal_region_ptr;
   }
   else
   {
      return NULL;
   }
}


// =====================================================================
// Extremal region manipulation member functions
// =====================================================================

// Member function add_dark_region()

void extremal_regions_group::add_dark_region(
   extremal_region* extremal_region_ptr)
{
//   cout << "inside extremal_regions_group::add_dark_region()" << endl;

   dark_region_id_iter=dark_region_id_map_ptr->find(extremal_region_ptr);
   if (dark_region_id_iter != dark_region_id_map_ptr->end())
   {
      (*dark_region_id_map_ptr)[extremal_region_ptr]=extremal_region_ptr->
         get_ID();
      (*dark_id_region_map_ptr)[region_counter]=extremal_region_ptr;
      region_counter++;
   }
}

// ---------------------------------------------------------------------
// Member function add_bright_region()

void extremal_regions_group::add_bright_region(
   extremal_region* extremal_region_ptr)
{
//   cout << "inside extremal_regions_group::add_bright_region()" << endl;

   bright_region_id_iter=bright_region_id_map_ptr->find(extremal_region_ptr);
   if (bright_region_id_iter != bright_region_id_map_ptr->end())
   {
      (*bright_region_id_map_ptr)[extremal_region_ptr]=extremal_region_ptr->
         get_ID();
      (*bright_id_region_map_ptr)[region_counter]=extremal_region_ptr;
      region_counter++;
   }
}

// ---------------------------------------------------------------------
// Member function delete_dark_region()

void extremal_regions_group::delete_dark_region(int dark_region_ID)
{
//   cout << "inside extremal_regions_group::delete_dark_region()" << endl;

   dark_id_region_iter=dark_id_region_map_ptr->find(dark_region_ID);
   if (dark_id_region_iter != dark_id_region_map_ptr->end())
   {
      extremal_region* extremal_region_ptr=dark_id_region_iter->second;

      dark_id_region_map_ptr->erase(dark_id_region_iter);

      dark_region_id_iter=dark_region_id_map_ptr->find(extremal_region_ptr);
      dark_region_id_map_ptr->erase(dark_region_id_iter);
   }
}

// ---------------------------------------------------------------------
// Member function delete_bright_region()

void extremal_regions_group::delete_bright_region(int bright_region_ID)
{
//   cout << "inside extremal_regions_group::delete_bright_region()" << endl;

   bright_id_region_iter=bright_id_region_map_ptr->find(bright_region_ID);
   if (bright_id_region_iter != bright_id_region_map_ptr->end())
   {
      extremal_region* extremal_region_ptr=bright_id_region_iter->second;

      bright_id_region_map_ptr->erase(bright_id_region_iter);

      bright_region_id_iter=bright_region_id_map_ptr->find(
         extremal_region_ptr);
      bright_region_id_map_ptr->erase(bright_region_id_iter);
   }
}

// ==========================================================================
// MSER member functions
// ==========================================================================

// Member function extract_MSERs() takes in an image and calls the Oxford MSER
// linux binary which outputs RLE runs for locally dark and bright
// maximally stable extremal regions.  This method decodes the MSER
// linux binary output and instantiates extremal region objects which
// are returned in STL maps.

void extremal_regions_group::extract_MSERs(string image_filename)
{
   cout << "inside extremal_regions_group:extract_MSERs()" << endl;

// Next call Oxford MSER linux binary on input image file:

   unsigned int xdim,ydim;
   imagefunc::get_image_width_height(image_filename,xdim,ydim);
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;

   string MSER_filename="./mser.output";
   string unix_cmd="mser.ln -i "+image_filename+" -o "+MSER_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::ReadInfile(MSER_filename);

   int line_counter=0;
   unsigned int n_dark_regions=stringfunc::string_to_number(
      filefunc::text_line[line_counter++]);
//   cout << "n_dark_regions = " << n_dark_regions << endl;

   int region_ID=1;
   vector<int> RLE_pixel_IDs;

   for (unsigned int i=1; i<=n_dark_regions; i++)
   {
      RLE_pixel_IDs.clear();
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      line_counter++;

      unsigned int n_triples=column_values[0];
      for (unsigned int n=0; n<n_triples; n++)
      {
         int row=column_values[3*n+1];
         int start_column=column_values[3*n+2];
         int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

         int start_pixel_ID=graphicsfunc::get_pixel_ID(
            start_column,row,xdim);
         int stop_pixel_ID=graphicsfunc::get_pixel_ID(
            stop_column,row,xdim);
         RLE_pixel_IDs.push_back(start_pixel_ID);
         RLE_pixel_IDs.push_back(stop_pixel_ID);

         if (start_pixel_ID > stop_pixel_ID ||
         start_pixel_ID < 0 || stop_pixel_ID < 0)
         {
            cout << "Error!  start_pixel_ID = " << start_pixel_ID
                 << " stop_pixel_ID = " << stop_pixel_ID << endl;
            outputfunc::enter_continue_char();
         }
      }
  
      extremal_region* extremal_region_ptr=new extremal_region(region_ID);
      extremal_region_ptr->set_bright_region_flag(false);
      extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
      (*dark_id_region_map_ptr)[region_ID]=extremal_region_ptr;
      region_ID++;
   } // loop over index i labeling dark MSERs

   RLE_pixel_IDs.clear();

   int n_bright_regions=stringfunc::string_to_number(
      filefunc::text_line[line_counter++]);
//   cout << "n_bright_regions = " << n_bright_regions << endl;

// Restart ID labeling for bright MSERs at 1:

   region_ID=1;
   for (unsigned int i=n_dark_regions+2; i<n_dark_regions+2+n_bright_regions; i++)
   {
      RLE_pixel_IDs.clear();
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      line_counter++;

      unsigned int n_triples=column_values[0];
      for (unsigned int n=0; n<n_triples; n++)
      {
         int row=column_values[3*n+1];
         int start_column=column_values[3*n+2];
         int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

         int start_pixel_ID=graphicsfunc::get_pixel_ID(
            start_column,row,xdim);
         int stop_pixel_ID=graphicsfunc::get_pixel_ID(
            stop_column,row,xdim);
         RLE_pixel_IDs.push_back(start_pixel_ID);
         RLE_pixel_IDs.push_back(stop_pixel_ID);

         if (start_pixel_ID > stop_pixel_ID ||
         start_pixel_ID < 0 || stop_pixel_ID < 0)
         {
            cout << "Error!  start_pixel_ID = " << start_pixel_ID
                 << " stop_pixel_ID = " << stop_pixel_ID << endl;
            outputfunc::enter_continue_char();
         }
      } // loop over index n labeling RLE triples
  
      extremal_region* extremal_region_ptr=new extremal_region(region_ID);
      extremal_region_ptr->set_bright_region_flag(true);
      extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
      (*bright_id_region_map_ptr)[region_ID]=extremal_region_ptr;
      region_ID++;
   } // loop over index i labeling bright MSERs

   cout << "dark_id_region_map_ptr->size() = "
        << dark_id_region_map_ptr->size() << " = " 
        << get_n_dark_regions() << endl;
   cout << "bright_id_region_map_ptr->size() = "
        << bright_id_region_map_ptr->size() << " = " 
        << get_n_bright_regions() << endl;
}

// ---------------------------------------------------------------------
// Method instantiate_twoDarrays()
      
void extremal_regions_group::instantiate_twoDarrays(
   twoDarray* mser_twoDarray_ptr)
{
//   cout << "inside extremal_regions_group::instantiate_twoDarrays()" << endl;
   
   if (tmp_twoDarray_ptr==NULL)
   {
      this->mser_twoDarray_ptr=mser_twoDarray_ptr;
      tmp_twoDarray_ptr=new twoDarray(mser_twoDarray_ptr);
      bright_cc_twoDarray_ptr=new twoDarray(mser_twoDarray_ptr);
      dark_cc_twoDarray_ptr=new twoDarray(mser_twoDarray_ptr);
      bright_cc_borders_twoDarray_ptr=new twoDarray(mser_twoDarray_ptr);
      dark_cc_borders_twoDarray_ptr=new twoDarray(mser_twoDarray_ptr);
   }
}

// ---------------------------------------------------------------------
// Method update_bright[dark]_cc_twoDarray() loops over
// *bright_id_region_map_ptr [*dark_id_region_map_ptr].  It sets
// bright [dark] MSER pixels equal to id_region
// [id_region+dark_ID_offset] within twoDarray *bright[dark]_cc_twoDarray_ptr.
      
void extremal_regions_group::update_bright_cc_twoDarray()
{
//   cout << "inside extremal_regions_group::update_bright_cc_twoDarray()"
//        << endl;
   update_cc_twoDarray(bright_id_region_map_ptr,bright_cc_twoDarray_ptr);
}
      
void extremal_regions_group::update_bright_cc_twoDarray(
   vector<extremal_region*> bright_extremal_region_ptrs)
{
   update_cc_twoDarray(bright_extremal_region_ptrs,bright_cc_twoDarray_ptr);
}

void extremal_regions_group::update_dark_cc_twoDarray()
{
   update_cc_twoDarray(dark_id_region_map_ptr,dark_cc_twoDarray_ptr);
}

void extremal_regions_group::update_dark_cc_twoDarray(
   vector<extremal_region*> dark_extremal_region_ptrs)
{
   update_cc_twoDarray(dark_extremal_region_ptrs,dark_cc_twoDarray_ptr);
}

void extremal_regions_group::update_cc_twoDarray(
   ID_REGION_MAP* id_region_map_ptr,twoDarray* cc_twoDarray_ptr)
{
//   cout << "inside extremal_regions_group::update_cc_twoDarray()" << endl;
//   cout << "cc_twoDarray_ptr = " << cc_twoDarray_ptr << endl;

   cc_twoDarray_ptr->clear_values();

//   cout << "id_region_map_ptr->size() = " << id_region_map_ptr->size() << endl;
   for (ID_REGION_MAP::iterator id_region_iter=id_region_map_ptr->begin();
        id_region_iter != id_region_map_ptr->end(); id_region_iter++)
   {
//      int id=id_region_iter->first;
      extremal_region* extremal_region_ptr=id_region_iter->second;
      int ID=extremal_region_ptr->get_ID();

      if (!extremal_region_ptr->get_bright_region_flag())
      {
         ID += dark_ID_offset;
      }
//      cout << "Extremal_region_ptr = " << extremal_region_ptr << endl;
      extremal_region_ptr->run_length_decode(cc_twoDarray_ptr,ID);      
   }

/*
   int n_cc_ID_pixels=0;
   for (unsigned int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
      {
         int cc_ID=cc_twoDarray_ptr->get(px,py);
         if (cc_ID > 0)
         {
//            cout << "px = " << px << " py = " << py 
//                 << " cc_ID = " << cc_ID << endl;
            n_cc_ID_pixels++;
         }
      } // loop over px
   } // loop over py
   cout << "n_cc_ID_pixels = " << n_cc_ID_pixels << endl;
//   outputfunc::enter_continue_char();
*/

}

void extremal_regions_group::update_cc_twoDarray(
   const vector<extremal_region*>& extremal_region_ptrs,
   twoDarray* cc_twoDarray_ptr)
{
//   cout << "inside extremal_regions_group::update_cc_twoDarray()" << endl;

   cc_twoDarray_ptr->clear_values();

   for (unsigned int e=0; e<extremal_region_ptrs.size(); e++)
   {
      extremal_region* extremal_region_ptr=extremal_region_ptrs[e];
      int ID=extremal_region_ptr->get_ID();
      if (!extremal_region_ptr->get_bright_region_flag())
      {
         ID += dark_ID_offset;
      }
      extremal_region_ptr->run_length_decode(cc_twoDarray_ptr,ID);      
   }

/*
   for (unsigned int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
      {
         int cc_ID=cc_twoDarray_ptr->get(px,py);
         if (cc_ID > 0)
         {
            cout << "px = " << px << " py = " << py 
                 << " cc_ID = " << cc_ID << endl;
         }
      } // loop over px
   } // loop over py
   outputfunc::enter_continue_char();
*/
}

// ---------------------------------------------------------------------
// Member function coalesce_touching_regions() takes in
// *cc_twoDarray_ptr which
// is assumed to be filled with connected component IDs > 0. We first 
// a UnionFind datastructure with the IDs for all extremal regions
// within input *extremal_region_map_ptr.  Looping over all pixels in
// *cc_twoDarray_ptr, we next associate pairs of extremal regions if
// the border region of one contains pixels that lie inside of the other.
// This method returns a compactified extremal_regions map which
// effectively contains the union of all the input extremal regions.
 
extremal_regions_group::ID_REGION_MAP* 
extremal_regions_group::coalesce_bright_touching_regions()
{
//   cout << "inside erg::coalesce_bright_touching_regions()" << endl;
   return coalesce_touching_regions(
      bright_cc_twoDarray_ptr,bright_id_region_map_ptr);
}

extremal_regions_group::ID_REGION_MAP* 
extremal_regions_group::coalesce_touching_regions(
   twoDarray* cc_twoDarray_ptr,ID_REGION_MAP* extremal_region_map_ptr)
{
//   cout << "inside extremal_regions_group::coalesce_touching_regions()" 
//        << endl;
         
   ID_REGION_MAP::iterator iter;

   union_find uf;
   for (iter=extremal_region_map_ptr->begin();
        iter != extremal_region_map_ptr->end(); iter++)
   {
      extremal_region* extremal_region_ptr=iter->second;
      uf.MakeSet(extremal_region_ptr->get_ID());
   }
//   cout << "uf.get_n_nodes() = " << uf.get_n_nodes() << endl;

// Link any two extremal regions which have neighboring pixels:

   int border_thickness=1;
   for (iter=extremal_region_map_ptr->begin();
        iter != extremal_region_map_ptr->end(); iter++)
   {
      extremal_region* extremal_region_ptr=iter->second;
      int curr_ID=extremal_region_ptr->get_ID();
            
      vector<pair<int,int> > border_pixels=
         extremal_region_ptr->compute_border_pixels(
            border_thickness,tmp_twoDarray_ptr);
      for (unsigned int bp=0; bp<border_pixels.size(); bp++)
      {
         int px=border_pixels[bp].first;
         int py=border_pixels[bp].second;
         int cc_ID=cc_twoDarray_ptr->get(px,py);
         if (cc_ID <= 0) continue;
         uf.Link(curr_ID,cc_ID);
      } // loop over bp
   } // iteration loop over extremal regions

   ID_REGION_MAP* coalesced_extremal_region_map_ptr=new ID_REGION_MAP;

   tmp_twoDarray_ptr->clear_values();
   for (unsigned int py=0; py<tmp_twoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<tmp_twoDarray_ptr->get_xdim(); px++)
      {
         int cc_ID=cc_twoDarray_ptr->get(px,py);
         int root_ID=uf.Find(cc_ID);
         tmp_twoDarray_ptr->put(px,py,root_ID);

         extremal_region* extremal_region_ptr=NULL;
         iter=coalesced_extremal_region_map_ptr->find(root_ID);
         if (iter==coalesced_extremal_region_map_ptr->end())
         {
            extremal_region_ptr=new extremal_region(root_ID);
            (*coalesced_extremal_region_map_ptr)[root_ID]=
               extremal_region_ptr;
         }
         else
         {
            extremal_region_ptr=iter->second;
         }   
         extremal_region_ptr->update_bbox(px,py);
      } // loop over px
   } // loop over py
   tmp_twoDarray_ptr->copy(cc_twoDarray_ptr);

   for (iter=coalesced_extremal_region_map_ptr->begin();
        iter != coalesced_extremal_region_map_ptr->end(); iter++)
   {
      extremal_region* extremal_region_ptr=iter->second;
      extremal_region_ptr->run_length_encode(cc_twoDarray_ptr);
   }

//   cout << "coalesced_extremal_region_map_ptr->size() = "
//        << coalesced_extremal_region_map_ptr->size() << endl;
   return coalesced_extremal_region_map_ptr;
}

// ---------------------------------------------------------------------
// Member function identify_border_pixels() locates border pixels
// around each locally bright MSER & stores their extremal region IDs
// within *cc_borders_twoDarray_ptr:

void extremal_regions_group::identify_bright_border_pixels(
   ID_REGION_MAP* id_region_map_ptr,int border_thickness)
{
//   cout << "inside extremal_regions_group::identify_bright_border_pixels()"
//        << endl;
   identify_border_pixels(
      border_thickness,id_region_map_ptr,
      bright_cc_borders_twoDarray_ptr);
}

void extremal_regions_group::identify_dark_border_pixels(
   ID_REGION_MAP* id_region_map_ptr,int border_thickness)
{
   identify_border_pixels(
      border_thickness,id_region_map_ptr,
      dark_cc_borders_twoDarray_ptr);
}

void extremal_regions_group::identify_border_pixels(
   int border_thickness,ID_REGION_MAP* extremal_region_map_ptr,
   twoDarray* cc_borders_twoDarray_ptr)
{
//   cout << "inside extremal_regions_group::identify_border_pixels()" << endl;

   cc_borders_twoDarray_ptr->initialize_values(-1);

   for (ID_REGION_MAP::iterator iter=extremal_region_map_ptr->begin();
        iter != extremal_region_map_ptr->end(); iter++)
   {
      extremal_region* extremal_region_ptr=iter->second;
      vector<pair<int,int> > border_pixels=
         extremal_region_ptr->compute_border_pixels(
            border_thickness,tmp_twoDarray_ptr);
      for (unsigned int bp=0; bp<border_pixels.size(); bp++)
      {
         unsigned int px=border_pixels[bp].first;
         unsigned int py=border_pixels[bp].second;
         cc_borders_twoDarray_ptr->put(
            px,py,extremal_region_ptr->get_ID());
      } // loop over index bp
//      cout << "extremal region ID = " << extremal_region_ptr->get_ID()
//           << " border_pixels.size() = " << border_pixels.size() << endl;
   } // iteration loop over extremal regions
}

// ==========================================================================
// Neighboring anti-region member functions
// ==========================================================================

// Member function add_bright_dark_neighbor_pair() takes in connected
// component IDs for a bright and dark region which are assumed to be
// adjacent neighbors.  It adds entries into member STL maps
// *bright[dark]_regions_near_dark[bright]_region_map_ptr.

void extremal_regions_group::add_bright_dark_neighbor_pair(
   int bright_cc_ID,int dark_cc_ID)
{
//   cout << "inside extremal_regions_group::add_bright_dark_neighbor_pair()"
//        << endl;

   if (bright_cc_ID <= 0 || dark_cc_ID <= 0) return;
   
   ANTI_REGIONS_NEAR_REGION_MAP::iterator bright_iter,dark_iter;

   ID_REGION_MAP *dark_id_region_map_ptr,*bright_id_region_map_ptr;
   bright_iter=dark_regions_near_bright_region_map_ptr->find(bright_cc_ID);
   if (bright_iter==dark_regions_near_bright_region_map_ptr->end())
   {
      dark_id_region_map_ptr=new ID_REGION_MAP;
   }
   else
   {
      dark_id_region_map_ptr=bright_iter->second;
   }
   extremal_region* dark_region_ptr=get_dark_region(dark_cc_ID);
   (*dark_id_region_map_ptr)[dark_cc_ID]=dark_region_ptr;
   (*dark_regions_near_bright_region_map_ptr)[bright_cc_ID]=
      dark_id_region_map_ptr;

   dark_iter=bright_regions_near_dark_region_map_ptr->find(dark_cc_ID);
   if (dark_iter==bright_regions_near_dark_region_map_ptr->end())
   {
      bright_id_region_map_ptr=new ID_REGION_MAP;
   }
   else
   {
      bright_id_region_map_ptr=dark_iter->second;
   }
   extremal_region* bright_region_ptr=get_bright_region(bright_cc_ID);
   (*bright_id_region_map_ptr)[bright_cc_ID]=bright_region_ptr;
   (*bright_regions_near_dark_region_map_ptr)[dark_cc_ID]=
      bright_id_region_map_ptr;

//   cout << "bright_regions_near_dark_region_map.size() = "
//        << bright_regions_near_dark_region_map_ptr->size() << endl;
//   cout << "dark_regions_near_bright_region_map.size() = "
//        << dark_regions_near_bright_region_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function get_bright_dark_neighbor_pair_flag() takes in IDs
// for bright and dark regions.  It returns true if the two "anti"
// regions are adjacent neighbors.

bool extremal_regions_group::get_bright_dark_neighbor_pair_flag(
   int bright_cc_ID,int dark_cc_ID)
{
   ANTI_REGIONS_NEAR_REGION_MAP::iterator dark_iter=
      bright_regions_near_dark_region_map_ptr->find(dark_cc_ID);
   if (dark_iter==bright_regions_near_dark_region_map_ptr->end())
   {
      return false;
   }

   ID_REGION_MAP* bright_id_region_map_ptr=dark_iter->second;
   bright_id_region_iter=bright_id_region_map_ptr->find(bright_cc_ID);
   if (bright_id_region_iter==bright_id_region_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool extremal_regions_group::get_bright_neighbor_pair_flag(int bright_cc_ID)
{
   ANTI_REGIONS_NEAR_REGION_MAP::iterator bright_iter=
      dark_regions_near_bright_region_map_ptr->find(bright_cc_ID);

   if (bright_iter==dark_regions_near_bright_region_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool extremal_regions_group::get_dark_neighbor_pair_flag(int dark_cc_ID)
{
   ANTI_REGIONS_NEAR_REGION_MAP::iterator dark_iter=
      bright_regions_near_dark_region_map_ptr->find(dark_cc_ID);

   if (dark_iter==bright_regions_near_dark_region_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function print_bright_dark_neighbor_pairs() 

void extremal_regions_group::print_bright_dark_neighbor_pairs()
{
   cout << "inside extremal_regions_group::print_bright_dark_neighbor_pairs()"
        << endl;
   
   ANTI_REGIONS_NEAR_REGION_MAP::iterator bright_iter,dark_iter;
   ID_REGION_MAP *dark_id_region_map_ptr,*bright_id_region_map_ptr;

   for (bright_iter=dark_regions_near_bright_region_map_ptr->begin();
        bright_iter != dark_regions_near_bright_region_map_ptr->end();
        bright_iter++)
   {
      int bright_cc_ID=bright_iter->first;
      if (bright_cc_ID <= 0) continue;
      dark_id_region_map_ptr=bright_iter->second;
      for (dark_id_region_iter=dark_id_region_map_ptr->begin();
           dark_id_region_iter != dark_id_region_map_ptr->end();
           dark_id_region_iter++)
      {
         int dark_cc_ID=dark_id_region_iter->first;
         cout << "bright cc ID = " << bright_cc_ID
              << " dark_cc_ID = " << dark_cc_ID << endl;
      }
   }

   for (dark_iter=bright_regions_near_dark_region_map_ptr->begin();
        dark_iter != bright_regions_near_dark_region_map_ptr->end();
        dark_iter++)
   {
      int dark_cc_ID=dark_iter->first;
      if (dark_cc_ID <= 0) continue;
      bright_id_region_map_ptr=dark_iter->second;
      for (bright_id_region_iter=bright_id_region_map_ptr->begin();
           bright_id_region_iter != bright_id_region_map_ptr->end();
           bright_id_region_iter++)
      {
         int bright_cc_ID=bright_id_region_iter->first;
         cout << "dark cc ID = " << dark_cc_ID
              << " bright_cc_ID = " << bright_cc_ID << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Member function merge_adjacent_dark_bright_bboxes()

void extremal_regions_group::merge_adjacent_dark_bright_bboxes(
   ID_REGION_MAP* black_regions_map_ptr,
   ID_REGION_MAP* coalesced_bright_region_map_ptr)
{
//   cout << "inside extremal_regions_group::merge_adjacent_dark_bright_bboxes()"
//        << endl;
   
   ANTI_REGIONS_NEAR_REGION_MAP::iterator dark_iter;
   ID_REGION_MAP* bright_id_region_map_ptr;

   for (dark_iter=bright_regions_near_dark_region_map_ptr->begin();
        dark_iter != bright_regions_near_dark_region_map_ptr->end();
        dark_iter++)
   {
      int dark_cc_ID=dark_iter->first;
      if (dark_cc_ID <= 0) continue;
      ID_REGION_MAP::iterator black_region_iter=black_regions_map_ptr->
         find(dark_cc_ID);
      if (black_region_iter==black_regions_map_ptr->end()) continue;
      
      extremal_region* dark_region_ptr=black_region_iter->second;

// Ignore any dark region bbox whose polygon pointer = NULL:

      if (dark_region_ptr->get_bbox_polygon_ptr()==NULL) continue;

      unsigned int min_px,min_py,max_px,max_py;
      dark_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

      if ((min_px < 0.5*NEGATIVEINFINITY) ||
      (max_px > 0.5*POSITIVEINFINITY) ||
      (min_py < 0.5*NEGATIVEINFINITY) ||
      (max_py > 0.5*POSITIVEINFINITY) )
      {
         dark_region_ptr->set_bbox_polygon_ptr(NULL);
         continue;
      }
      bounding_box dark_bbox(min_px,max_px,min_py,max_py);

      bright_id_region_map_ptr=dark_iter->second;
      for (bright_id_region_iter=bright_id_region_map_ptr->begin();
           bright_id_region_iter != bright_id_region_map_ptr->end();
           bright_id_region_iter++)
      {
         int bright_cc_ID=bright_id_region_iter->first;
         ID_REGION_MAP::iterator bright_iter=
            coalesced_bright_region_map_ptr->find(bright_cc_ID);
         if (bright_iter==coalesced_bright_region_map_ptr->end()) continue;

         extremal_region* bright_region_ptr=bright_iter->second;

// Ignore any bright region bbox whose polygon pointer = NULL:

         if (bright_region_ptr->get_bbox_polygon_ptr()==NULL) continue;

//         cout << "dark cc ID = " << dark_cc_ID
//              << " bright_cc_ID = " << bright_cc_ID << endl;

         unsigned int bright_min_px,bright_min_py,bright_max_px,bright_max_py;
         bright_region_ptr->get_bbox(
            bright_min_px,bright_min_py,bright_max_px,bright_max_py);

         if ((bright_min_px < 0.5*NEGATIVEINFINITY) ||
         (bright_max_px > 0.5*POSITIVEINFINITY) ||
         (bright_min_py < 0.5*NEGATIVEINFINITY) ||
         (bright_max_py > 0.5*POSITIVEINFINITY) )
         {
            bright_region_ptr->set_bbox_polygon_ptr(NULL);
            continue;
         }

         bounding_box bright_bbox(
            bright_min_px,bright_max_px,bright_min_py,bright_max_py);
         if (dark_bbox.overlap(bright_bbox))
         {
            dark_bbox.update_bounds(&bright_bbox);
         }
      }

//      cout << "Finally, dark_bbox = " << dark_bbox << endl;

      dark_region_ptr->set_bbox(
         dark_bbox.get_xmin(),dark_bbox.get_ymin(),
         dark_bbox.get_xmax(),dark_bbox.get_ymax());
   }

}

// ---------------------------------------------------------------------
void extremal_regions_group::print_id_region_map(
   ID_REGION_MAP* id_region_map_ptr)
{
   cout << "inside extremal_regions_group::print_id_region_map()" << endl;

   cout << "id_region_map_ptr->size() = " << id_region_map_ptr->size() << endl;
   for (ID_REGION_MAP::iterator id_region_iter=id_region_map_ptr->begin();
        id_region_iter != id_region_map_ptr->end(); id_region_iter++)
   {
      int id=id_region_iter->first;
      extremal_region* extremal_region_ptr=id_region_iter->second;
      int ID=extremal_region_ptr->get_ID();

      cout << "id_region_iter->first = " << id
           << " extremal region ID = " << ID << endl;
   }
//   outputfunc::enter_continue_char();
}

