// ==========================================================================
// Header file for mserfunc namespace
// ==========================================================================
// Last modified on 10/8/12; 10/15/12
// ==========================================================================

#ifndef MSERFUNCS_H
#define MSERFUNCS_H

#include <map>
#include <string>
#include <vector>
#include "math/basic_math.h"

class extremal_region;
class texture_rectangle;

namespace mserfunc
{

// MSER methods:

   typedef std::map<int,extremal_region*> EXTREMAL_REGIONS_MAP;
// independent int var = extremal_region ID

   void purge_regions_map(
      EXTREMAL_REGIONS_MAP* extremal_region_map_ptr);

   void extract_MSERs(
      std::string image_filename,
      EXTREMAL_REGIONS_MAP* dark_extremal_region_map_ptr,
      EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr);
   void extract_MSERs(
      std::string image_filename,
      std::vector<extremal_region*>& dark_extremal_region_ptrs,
      std::vector<extremal_region*>& bright_extremal_region_ptrs);

   void update_MSER_twoDarray(
      EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr,
      texture_rectangle* mser_texture_rectangle_ptr);
   void update_MSER_twoDarray(
      EXTREMAL_REGIONS_MAP* dark_extremal_region_map_ptr,
      EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr,
      texture_rectangle* mser_texture_rectangle_ptr);

   void update_MSER_twoDarray(
      const std::vector<extremal_region*>& bright_extremal_region_ptrs,
      texture_rectangle* mser_texture_rectangle_ptr);
   void update_MSER_twoDarray(
      const std::vector<extremal_region*>& dark_extremal_region_ptrs,
      const std::vector<extremal_region*>& bright_extremal_region_ptrs,
      texture_rectangle* mser_texture_rectangle_ptr);

   void draw_MSERs(twoDarray* cc_twoDarray_ptr,
   texture_rectangle* mser_texture_rectangle_ptr);

// MSER border methods:

   void identify_border_pixels(
      int border_thickness,twoDarray* tmp_twoDarray_ptr,
      EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr,
      twoDarray* cc_borders_twoDarray_ptr);
   EXTREMAL_REGIONS_MAP* coalesce_touching_regions(
      twoDarray* tmp_twoDarray_ptr,twoDarray* cc_twoDarray_ptr,
      EXTREMAL_REGIONS_MAP* extremal_region_map_ptr);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // mserfunc namespace

#endif // mserfuncs.h

