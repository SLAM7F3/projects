// ==========================================================================
// Header file for extremal_regions_group class
// ==========================================================================
// Last modified on 10/13/12; 10/14/12; 10/15/12
// ==========================================================================

#ifndef EXTREMAL_REGIONS_GROUP_H
#define EXTREMAL_REGIONS_GROUP_H

#include <map>
#include "image/extremal_region.h"
#include "image/TwoDarray.h"

class extremal_regions_group 
{

  public:

   typedef std::map<extremal_region*,int> REGION_ID_MAP;

// independent var: extremal region pointer
// dependent int var: extremal region ID 

   typedef std::map<int,extremal_region*> ID_REGION_MAP;

// independent int var: extremal region ID 
// dependent var: extremal region pointer

   typedef std::map<int,ID_REGION_MAP*> ANTI_REGIONS_NEAR_REGION_MAP;

// independent int var: bright [dark] region ID
// dependent var: ID_REGION_MAP for neighboring dark [bright] regions


// Initialization, constructor and destructor functions:

   extremal_regions_group();
   extremal_regions_group(const extremal_regions_group& erg);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~extremal_regions_group();
   void purge_dark_and_bright_regions();
   void destroy_all_regions(REGION_ID_MAP* region_id_map_ptr);
   void destroy_all_regions(ID_REGION_MAP* id_region_map_ptr);
   void destroy_all_regions(
      ANTI_REGIONS_NEAR_REGION_MAP* anti_regions_near_region_map_ptr);

   extremal_regions_group& operator= (const extremal_regions_group& erg);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const extremal_regions_group& erg);

// Set and get member functions:

   ID_REGION_MAP* get_dark_id_region_map_ptr();
   ID_REGION_MAP* get_bright_id_region_map_ptr();
   int get_n_dark_regions() const;
   int get_n_bright_regions() const;
   extremal_region* get_dark_region(int region_ID);
   extremal_region* get_bright_region(int region_ID);

   void set_bright_cc_twoDarray_ptr(twoDarray* twoDarray_ptr);
   twoDarray* get_bright_cc_twoDarray_ptr();
   const twoDarray* get_bright_cc_twoDarray_ptr() const;
   twoDarray* get_dark_cc_twoDarray_ptr();
   const twoDarray* get_dark_cc_twoDarray_ptr() const;

   twoDarray* get_bright_cc_borders_twoDarray_ptr();
   const twoDarray* get_bright_cc_borders_twoDarray_ptr() const;
   twoDarray* get_dark_cc_borders_twoDarray_ptr();
   const twoDarray* get_dark_cc_borders_twoDarray_ptr() const;

   ANTI_REGIONS_NEAR_REGION_MAP*  
      get_bright_regions_near_dark_region_map_ptr();
   const ANTI_REGIONS_NEAR_REGION_MAP*  
      get_bright_regions_near_dark_region_map_ptr() const;
   ANTI_REGIONS_NEAR_REGION_MAP*  
      get_dark_regions_near_bright_region_map_ptr();
   const ANTI_REGIONS_NEAR_REGION_MAP*  
      get_dark_regions_near_bright_region_map_ptr() const;

   int get_n_bright_dark_region_neighbors() const;
   int get_n_dark_bright_region_neighbors() const;

// Extremal region manipulation member functions

   void add_dark_region(extremal_region* extremal_region_ptr);
   void add_bright_region(extremal_region* extremal_region_ptr);
   void delete_dark_region(int dark_region_ID);
   void delete_bright_region(int bright_region_ID);

// MSER member functions

   void extract_MSERs(std::string image_filename);

   void instantiate_twoDarrays(twoDarray* ptwoDarray_ptr);
   void update_bright_cc_twoDarray();
   void update_bright_cc_twoDarray(
      std::vector<extremal_region*> bright_extremal_region_ptrs);
   void update_dark_cc_twoDarray();
   void update_dark_cc_twoDarray(
      std::vector<extremal_region*> dark_extremal_region_ptrs);
   void update_cc_twoDarray(
      ID_REGION_MAP* region_id_map_ptr,twoDarray* cc_twoDarray_ptr);
   void update_cc_twoDarray(
      const std::vector<extremal_region*>& extremal_region_ptrs,
      twoDarray* cc_twoDarray_ptr);
   void print_id_region_map(ID_REGION_MAP* id_region_map_ptr);


   ID_REGION_MAP* coalesce_bright_touching_regions();
   ID_REGION_MAP* coalesce_touching_regions(
      twoDarray* cc_twoDarray_ptr,ID_REGION_MAP* extremal_region_map_ptr);

   void identify_bright_border_pixels(
      ID_REGION_MAP* id_region_map_ptr,int border_thickness);
   void identify_dark_border_pixels(
      ID_REGION_MAP* id_region_map_ptr,int border_thickness);
   void identify_border_pixels(
      int border_thickness,ID_REGION_MAP* extremal_region_map_ptr,
      twoDarray* cc_borders_twoDarray_ptr);
   
// Neighboring anti-region member functions

   void add_bright_dark_neighbor_pair(
      int bright_cc_ID,int dark_cc_ID);

   bool get_bright_dark_neighbor_pair_flag(
      int bright_cc_ID,int dark_cc_ID);
   bool get_bright_neighbor_pair_flag(int bright_cc_ID);
   bool get_dark_neighbor_pair_flag(int dark_cc_ID);

   void print_bright_dark_neighbor_pairs();

   void merge_adjacent_dark_bright_bboxes(
      ID_REGION_MAP* black_regions_map_ptr,
      ID_REGION_MAP* coalesced_bright_region_map_ptr);

  private:

   int region_counter,dark_ID_offset;
   REGION_ID_MAP *dark_region_id_map_ptr,*bright_region_id_map_ptr;
   REGION_ID_MAP::iterator dark_region_id_iter,bright_region_id_iter;

   ID_REGION_MAP *dark_id_region_map_ptr,*bright_id_region_map_ptr;
   ID_REGION_MAP::iterator dark_id_region_iter,bright_id_region_iter;

   twoDarray *mser_twoDarray_ptr,*tmp_twoDarray_ptr;
   twoDarray *bright_cc_twoDarray_ptr,*dark_cc_twoDarray_ptr;
   twoDarray *bright_cc_borders_twoDarray_ptr,*dark_cc_borders_twoDarray_ptr;

   ANTI_REGIONS_NEAR_REGION_MAP *bright_regions_near_dark_region_map_ptr,
      *dark_regions_near_bright_region_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const extremal_regions_group& erg);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline extremal_regions_group::ID_REGION_MAP* 
extremal_regions_group::get_dark_id_region_map_ptr()
{
   return dark_id_region_map_ptr;
}

inline extremal_regions_group::ID_REGION_MAP* 
extremal_regions_group::get_bright_id_region_map_ptr()
{
   return bright_id_region_map_ptr;
}

inline void extremal_regions_group::set_bright_cc_twoDarray_ptr(
   twoDarray* twoDarray_ptr)
{
   bright_cc_twoDarray_ptr=twoDarray_ptr;
}

inline twoDarray* extremal_regions_group::get_bright_cc_twoDarray_ptr()
{
   return bright_cc_twoDarray_ptr;
}

inline const twoDarray* 
extremal_regions_group::get_bright_cc_twoDarray_ptr() const
{
   return bright_cc_twoDarray_ptr;
}

inline twoDarray* extremal_regions_group::get_dark_cc_twoDarray_ptr()
{
   return dark_cc_twoDarray_ptr;
}

inline const twoDarray* 
extremal_regions_group::get_dark_cc_twoDarray_ptr() const
{
   return dark_cc_twoDarray_ptr;
}

inline twoDarray* 
extremal_regions_group::get_bright_cc_borders_twoDarray_ptr()
{
   return bright_cc_borders_twoDarray_ptr;
}

inline const twoDarray* 
extremal_regions_group::get_bright_cc_borders_twoDarray_ptr() const
{
   return bright_cc_borders_twoDarray_ptr;
}

inline twoDarray* 
extremal_regions_group::get_dark_cc_borders_twoDarray_ptr()
{
   return dark_cc_borders_twoDarray_ptr;
}

inline const twoDarray* 
extremal_regions_group::get_dark_cc_borders_twoDarray_ptr() const
{
   return dark_cc_borders_twoDarray_ptr;
}

inline extremal_regions_group::ANTI_REGIONS_NEAR_REGION_MAP* 
extremal_regions_group::get_bright_regions_near_dark_region_map_ptr()
{
   return bright_regions_near_dark_region_map_ptr;
}

inline const extremal_regions_group::ANTI_REGIONS_NEAR_REGION_MAP* 
extremal_regions_group::get_bright_regions_near_dark_region_map_ptr() const
{
   return bright_regions_near_dark_region_map_ptr;
}

inline extremal_regions_group::ANTI_REGIONS_NEAR_REGION_MAP*  
extremal_regions_group::get_dark_regions_near_bright_region_map_ptr()
{
   return dark_regions_near_bright_region_map_ptr;
}

inline const extremal_regions_group::ANTI_REGIONS_NEAR_REGION_MAP*  
extremal_regions_group::get_dark_regions_near_bright_region_map_ptr() const
{
   return dark_regions_near_bright_region_map_ptr;
}

inline int extremal_regions_group::get_n_bright_dark_region_neighbors() const
{
   return bright_regions_near_dark_region_map_ptr->size();
}

inline int extremal_regions_group::get_n_dark_bright_region_neighbors() const
{
   return dark_regions_near_bright_region_map_ptr->size();
}


#endif  // extremal_regions_group.h



