// ==========================================================================
// Bldgstrandfuncs namespace method definitions
// ==========================================================================
// Last modified on 6/21/05
// ==========================================================================

#include <set>
#include <string>
#include <vector>
#include "urban/building_info.h"
#include "urban/bldgstrandfuncs.h"
#include "urban/urbanfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

namespace bldgstrandfunc
{
   Strand<building_info*>* set_video_strand()
      {

// First instantiate an array of new building_info pointers which
// point to dynamically allocated building_infos containing relative
// height info, etc extracted from some video sequence:

         const int length=5;
         building_info** building_info_ptr_ptr=new building_info*[length];

         for (int i=0; i<length; i++)
         {
            building_info_ptr_ptr[i]=new building_info(i);
         }

// As of 3/18/05, we hardwire in info for strand 18 in cityblock #8
// from chunk 66-72:

         building_info_ptr_ptr[0]->set_relative_height(
            building_info::unknown);
         building_info_ptr_ptr[1]->set_relative_height(
            building_info::greater_than);
         building_info_ptr_ptr[2]->set_relative_height(
            building_info::greater_than);
         building_info_ptr_ptr[3]->set_relative_height(
            building_info::equal_to);
         building_info_ptr_ptr[4]->set_relative_height(
            building_info::equal_to);

         building_info_ptr_ptr[0]->set_spine_dir(
            building_info::parallel);
         building_info_ptr_ptr[1]->set_spine_dir(
            building_info::perpendicular);
         building_info_ptr_ptr[2]->set_spine_dir(
            building_info::perpendicular);
         building_info_ptr_ptr[3]->set_spine_dir(
            building_info::perpendicular);
         building_info_ptr_ptr[4]->set_spine_dir(
            building_info::perpendicular);

         building_info_ptr_ptr[0]->set_tall_tree_posn(
            building_info::in_back);
         building_info_ptr_ptr[1]->set_tall_tree_posn(
            building_info::in_back);
         building_info_ptr_ptr[2]->set_tall_tree_posn(
            building_info::in_back);
         building_info_ptr_ptr[3]->set_tall_tree_posn(
            building_info::in_back);
         building_info_ptr_ptr[4]->set_tall_tree_posn(
            building_info::in_back);

         building_info_ptr_ptr[0]->set_small_shrub_posn(
            building_info::on_left);
         building_info_ptr_ptr[1]->set_small_shrub_posn(
            building_info::none);
         building_info_ptr_ptr[2]->set_small_shrub_posn(
            building_info::none);
         building_info_ptr_ptr[3]->set_small_shrub_posn(
            building_info::none);
         building_info_ptr_ptr[4]->set_small_shrub_posn(
            building_info::none);

         const int strand_ID=-1;	// Truth !
         Strand<building_info*>* video_strand_ptr=new Strand<building_info*>(
            strand_ID);
         for (int i=0; i<length; i++)
         {
            video_strand_ptr->append_node(building_info_ptr_ptr[i]);
         }
         return video_strand_ptr;
      }   

// ---------------------------------------------------------------------
// Member function display_strand_contents

// Note: this method should eventually go into the general templatized
// Strand class...

   void display_strand_contents(Strand<building_info*>* strand_ptr)
      {
         for (Mynode<building_info*>* bldgnode_ptr=
                 strand_ptr->get_start_ptr(); bldgnode_ptr != NULL;
              bldgnode_ptr=bldgnode_ptr->get_nextptr())
         {
            building_info* bldg_info_ptr=bldgnode_ptr->get_data();    
            cout << *bldg_info_ptr << endl;
         } // loop over buildings in current strand
         cout << "==================================================" << endl;
      } 

// ---------------------------------------------------------------------
// Member function score_strand_agreement_with_video_data evaluates
// how closely the general building information within input
// *strand_ptr matches that within *video_strand_ptr.  It compares
// relative building heights, rooftop spine directions relative to the
// street, gross positions for tall trees and gross positions for
// small shrubs.  This method computes and returns the number of
// differences between the candidate strand and the video strand.

   int score_strand_agreement_with_video_data(
      Strand<building_info*> const *strand_ptr,
      Strand<building_info*> const *video_strand_ptr,
      int first_video_bldg_offset)
      {
         int strand_length=strand_ptr->size();
         
         int n_differences=0;

         Mynode<building_info*> const *currnode_ptr=
            strand_ptr->get_start_ptr(); 
//         Mynode<building_info*> const *video_node_ptr=
//            video_strand_ptr->get_start_ptr(); 
         Mynode<building_info*> const *video_node_ptr=
            video_strand_ptr->get_node_ahead(
               video_strand_ptr->get_start_ptr(),first_video_bldg_offset);

         for (int i=0; i<strand_length; i++)
         {
            building_info* curr_building_info_ptr=currnode_ptr->get_data();
            building_info* video_building_info_ptr=video_node_ptr->get_data();
            
// Compare relative building heights:

            building_info::Relationship curr_rel_height=
               curr_building_info_ptr->get_relative_height();
            building_info::Relationship video_rel_height=
               video_building_info_ptr->get_relative_height();

            if (curr_rel_height != video_rel_height)
            {
               n_differences++;
            }

// Compare spine directions:

            building_info::Spine_Direction curr_spine_dir=
               curr_building_info_ptr->get_spine_dir();
            building_info::Spine_Direction video_spine_dir=
               video_building_info_ptr->get_spine_dir();

            if (curr_spine_dir != video_spine_dir)
            {
               n_differences++;
            }

// Compare tall tree gross positions:

            building_info::Gross_Spatial_Direction curr_tall_tree_posn=
               curr_building_info_ptr->get_tall_tree_posn();
            building_info::Gross_Spatial_Direction video_tall_tree_posn=
               video_building_info_ptr->get_tall_tree_posn();

            if (curr_tall_tree_posn != video_tall_tree_posn)
            {
//               n_differences++;
            }

// Compare small shrub gross positions:

            building_info::Gross_Spatial_Direction curr_small_shrub_posn=
               curr_building_info_ptr->get_small_shrub_posn();
            building_info::Gross_Spatial_Direction video_small_shrub_posn=
               video_building_info_ptr->get_small_shrub_posn();

            if (curr_small_shrub_posn != video_small_shrub_posn)
            {
//               n_differences++;
            }
            
//            cout << "Building comparison i = " << i << endl;
//            cout << "curr_rel_height = " << curr_rel_height
//                 << " video_rel_height = " << video_rel_height << endl;
            
            currnode_ptr=currnode_ptr->get_nextptr();
            video_node_ptr=video_node_ptr->get_nextptr();
         } // loop over index i labeling strand buildings
         return n_differences;
      } 

// ---------------------------------------------------------------------
// Method draw_3D_strand colors all of the buildings within input
// *starnd_ptr according to annotation_value.  This method was created
// in order to display candidate building strands which survive
// various comparison tests with the ground video strand.

   void draw_3D_strand(
      Network<building*> const *buildings_network_ptr,
      Strand<building_info*> const *strand_ptr,
      string xyzp_filename,double annotation_value)
      {
         for (const Mynode<building_info*>* bldgnode_ptr=
                 strand_ptr->get_start_ptr(); bldgnode_ptr != NULL;
              bldgnode_ptr=bldgnode_ptr->get_nextptr())
         {
            building_info* bldg_info_ptr=bldgnode_ptr->get_data();    

            int n=bldg_info_ptr->get_building_ID();
            urbanfunc::draw_particular_3D_building(
               n,buildings_network_ptr,xyzp_filename,annotation_value);
         } // loop over buildings within *strand_ptr
      }

} // bldgstrandfunc namespace

