// ==========================================================================
// Program NONTEXT_EXTREMAL_REGIONS is a specialized variant of
// SHAPE_DESCRIPTORS.  It is designed to output extremal regions for
// text and non-text images which are only 32 pixels high.
// GENERATE_EXTREMAL_REGIONS works with images containing bright
// characters against dark backgrounds or vice versa (or else non-text
// input imagery).  It varies a binary threshold from 0 to 255 and
// generates connected binary component images.  If the number of
// connected components or fill fraction is too large or small, this
// program ignores the candidate image.  Otherwise, it retains some
// a small fraction of extremal region image examples corresponding to
// random threshold settings.  

//			./nontext_extremal_regions

// NOTE: THIS PROGRAM IS SUPERCEDED BY GENERATE_EXTREMAL_REGIONS

// ==========================================================================
// Last updated on 6/15/12
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "image/binaryimagefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

#include "graphs/graph.h"
#include "graphs/graph_hierarchy.h"
#include "datastructures/Linkedlist.h"
#include "graphs/node.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   connected_components* connected_components_ptr=new connected_components();

   bool text_char_flag=false;
//   bool invert_binary_values_flag=false; 	// dark chars
   bool invert_binary_values_flag=true; 	// bright chars

   double random_threshold;
   string renorm_subdir="./training_data/nontext_images/";
   connected_components_ptr->set_min_n_connected_components(1);
   connected_components_ptr->set_max_n_connected_components(1);
   connected_components_ptr->set_min_fill_frac(0.1);
   connected_components_ptr->set_max_fill_frac(0.9);
   random_threshold=0.005;

// As of Weds, 6/13/2012, we have 640 [932] examples of 32-pixel high images
// containing dark [bright] chars against bright [dark] backgrounds

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");

   bool search_all_children_dirs_flag=false;
   vector<string> jpeg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,renorm_subdir,search_all_children_dirs_flag);

   int i_start=0;
   int i_stop=15000;	// non-text
   int i_skip=1;
   for (int i=i_start; i<i_stop; i += i_skip)
   {
      cout << "i = " << i 
           << " jpeg_filename = " << jpeg_filenames[i] << endl;
      
      connected_components_ptr->reset_image(jpeg_filenames[i]);

      int threshold=255*nrfunc::ran1();
      int n_connected_components=
         connected_components_ptr->compute_connected_components(
            i,threshold,invert_binary_values_flag);

      if (n_connected_components != 1) 
      {
         string output_filenames_prefix="connected_regions_"
            +stringfunc::integer_to_string(i,3);
         string output_subdir="./connected_components/";
         vector<string> output_filenames=
            filefunc::files_in_subdir_matching_substring(
               output_subdir,output_filenames_prefix);
         
         for (int f=0; f<output_filenames.size(); f++)
         {
            filefunc::deletefile(output_filenames[f]);
         }
      }
  
   } // loop over index i labeling input 32x32 jpeg files

   delete connected_components_ptr;
}

