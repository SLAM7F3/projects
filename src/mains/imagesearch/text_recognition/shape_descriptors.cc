// ==========================================================================
// Program SHAPE_DESCRIPTORS computes connected components within
// binary thresholded versions of an input image for thresholds
// ranging from 255 down to 0.  It also follows Neumann and Matas,
// "Real-time scene text localization and recognition", CVPR 2012 and
// computes descriptors for the extremal regions.
// ==========================================================================
// Last updated on 6/11/12; 6/12/12; 6/13/12; 6/15/12
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
   connected_components_ptr->set_min_n_connected_components(1);
   connected_components_ptr->set_max_n_connected_components(2);
//   connected_components_ptr->set_max_n_connected_components(4);

//   connected_components_ptr->set_min_fill_frac(0.20);
//   connected_components_ptr->set_min_fill_frac(0.25);
//   connected_components_ptr->set_min_fill_frac(0.30);
//   connected_components_ptr->set_min_fill_frac(0.33);
   connected_components_ptr->set_min_fill_frac(0.35);

//   connected_components_ptr->set_max_fill_frac(0.45);
   connected_components_ptr->set_max_fill_frac(0.55);
//   connected_components_ptr->set_max_fill_frac(0.60);

//   connected_components_ptr->set_max_fill_frac(0.65);
//   connected_components_ptr->set_max_fill_frac(0.66);
//   connected_components_ptr->set_max_fill_frac(0.70);
//   connected_components_ptr->set_max_fill_frac(0.75);

   bool invert_binary_values_flag=false; 	// dark chars
//   bool invert_binary_values_flag=true; 	// bright chars

// As of Weds, 6/13/2012, we have 640 [932] examples of 32-pixel high images
// containing dark [bright] chars against bright [dark] backgrounds

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   string renorm_subdir="./training_data/char_intensity/";
   if (invert_binary_values_flag)
   {
      renorm_subdir += "dark_chars_bright_background/";
   }
   else
   {
      renorm_subdir += "bright_chars_dark_background/";
   }
   renorm_subdir="./training_data/nontext_images/";


   bool search_all_children_dirs_flag=false;
   vector<string> jpeg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,renorm_subdir,search_all_children_dirs_flag);

   int i_start=0;
//   int i_start=201;
//   int i_stop=200;
//   int i_stop=jpeg_filenames.size();
   int i_stop=2000;	// non-text
   int i_skip=1;
//   int i_skip=5;
   for (int i=i_start; i<i_stop; i += i_skip)
   {
      cout << "i = " << i 
           << " jpeg_filename = " << jpeg_filenames[i] << endl;
      
      connected_components_ptr->reset_image(jpeg_filenames[i]);
   
      int threshold_start=255;
      int threshold_stop=0;
      int d_threshold=-1;
      for (int threshold=threshold_start; threshold >= threshold_stop; 
           threshold += d_threshold)
      {
         int n_connected_components=
            connected_components_ptr->compute_connected_components(
               i,threshold,invert_binary_values_flag);
//         cout << "threshold = " << threshold 
//              << " n_connected_components = " << n_connected_components
//              << endl;
      } // loop over threshold index
      
      string output_filenames_prefix="connected_regions_"
         +stringfunc::integer_to_string(i,3);
      string output_subdir="./connected_components/";
      vector<string> output_filenames=
         filefunc::files_in_subdir_matching_substring(
            output_subdir,output_filenames_prefix);
      int n_output_filenames=output_filenames.size();

      for (int f=0; f<n_output_filenames; f++)
      {
         if (nrfunc::ran1() > 0.025)
         {
            filefunc::deletefile(output_filenames[f]);
         }
      }


   } // loop over index i labeling input 32x32 jpeg files

   delete connected_components_ptr;
}

