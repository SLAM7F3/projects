// ==========================================================================
// Header file for BLDGSTRANDFUNCS namespace
// ==========================================================================
// Last modified on 4/18/05
// ==========================================================================

#ifndef STRANDFUNCS_H
#define STRANDFUNCS_H

#include <string>
#include "network/Network.h"
#include "network/Strand.h"
class building;
class building_info;

namespace bldgstrandfunc
{
   Strand<building_info*>* set_video_strand();
   void display_strand_contents(Strand<building_info*>* strand_ptr);
   int score_strand_agreement_with_video_data(
      Strand<building_info*> const *strand_ptr,
      Strand<building_info*> const *video_strand_ptr,
      int first_video_bldg_offset=0);
   void draw_3D_strand(
      Network<building*> const *buildings_network_ptr,
      Strand<building_info*> const *strand_ptr,
      std::string xyzp_filename,double annotation_value);

} // strandfunc namespace

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // strandfuncs.h



