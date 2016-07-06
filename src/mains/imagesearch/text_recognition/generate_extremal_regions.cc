// ==========================================================================
// Program GENERATE_EXTREMAL_REGIONS is a specialized variant of
// SHAPE_DESCRIPTORS.  It is designed to output extremal regions for
// text and non-text images which are only 32 pixels high.
// GENERATE_EXTREMAL_REGIONS works with images containing bright
// characters against dark backgrounds or vice versa (or else non-text
// input imagery).  It varies a binary threshold from 0 to 255 and
// generates connected binary component images.  If the number of
// connected components or fill fraction is too large or small, this
// program ignores the candidate image.  Otherwise, it retains some
// small fraction of extremal region image examples corresponding to
// random threshold settings.  

//		./generate_extremal_regions

// ==========================================================================
// Last updated on 6/23/12; 7/8/12; 7/11/12
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

   long s;
   cout << "Enter random integer seed s:" << endl;
   cin >> s;
   nrfunc::init_default_seed(s);

   connected_components* connected_components_ptr=new connected_components();

   bool text_char_flag=false;
//   bool text_char_flag=true;

//   bool invert_binary_values_flag=false; 	// bright chars
   bool invert_binary_values_flag=true; 	// dark chars

   double random_threshold;
   string renorm_subdir="./training_data/nontext_images/";
   if (text_char_flag)
   {
      renorm_subdir="./training_data/char_intensity/";
      if (invert_binary_values_flag)
      {
         renorm_subdir += "dark_chars_bright_background/";
      }
      else
      {
         renorm_subdir += "bright_chars_dark_background/";
      }

// Most genuine text characters correspond to at most 2 connected
// components:

      connected_components_ptr->set_min_n_connected_components(1);
      connected_components_ptr->set_max_n_connected_components(2);

// Set reasonable fill fraction bounds for genuine text characters:

      connected_components_ptr->set_min_fill_frac(0.35);
      connected_components_ptr->set_max_fill_frac(0.55);

      random_threshold=0.025;
   }
   else
   {
      connected_components_ptr->set_min_n_connected_components(1);
      connected_components_ptr->set_max_n_connected_components(1);
      connected_components_ptr->set_min_fill_frac(0.1);
      connected_components_ptr->set_max_fill_frac(0.9);
      random_threshold=0.005;
   } // text_char_flag conditional

// As of Weds, 6/13/2012, we have 640 [932] examples of 32-pixel high images
// containing dark [bright] chars against bright [dark] backgrounds

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");

   bool search_all_children_dirs_flag=false;
   vector<string> jpeg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,renorm_subdir,search_all_children_dirs_flag);

//   int i_start=0;
//   int i_stop=10;
//   int i_start=641;
//   int i_stop=200;
//   int i_stop=jpeg_filenames.size();
//   int i_stop=2000;	// non-text


//   int i_start=2001;	// non-text
//   int i_stop=3000;
//   int i_start=3001;	// non-text
//   int i_stop=12000;
//   int i_start=13001;	// non-text
//   int i_stop=23000;
//   int i_start=23001;	// non-text
//   int i_stop=43000;
   int i_start=1001;	// non-text
   int i_stop=49999;
   
   int i_skip=1;
   for (int i=i_start; i<i_stop; i += i_skip)
   {
      cout << "i = " << i 
           << " jpeg_filename = " << jpeg_filenames[i] << endl;
      
      int threshold_start,threshold_stop,d_threshold;
      if (invert_binary_values_flag)  // dark chars bright background
      {
         threshold_start=1;
         threshold_stop=254;
         d_threshold=1;
      }
      else	// bright chars dark background
      {
         threshold_start=254;
         threshold_stop=1;
         d_threshold=-1;
      }

      connected_components_ptr->reset_image(jpeg_filenames[i]);

      for (int threshold=threshold_start; threshold != threshold_stop; 
           threshold += d_threshold)
      {
         int level=-1;
         if (!invert_binary_values_flag)
         {
            level=255-threshold;
         }
         else
         {
            level=threshold;
         }

         int n_connected_components=
            connected_components_ptr->export_connected_components(
               i,threshold,level,invert_binary_values_flag);
//         cout << "threshold = " << threshold 
//              << " n_connected_components = " << n_connected_components
//              << endl;
      } // loop over threshold index

      string output_filenames_prefix="connected_regions_"+
         stringfunc::integer_to_string(i,5);
      vector<string> output_filenames=
         filefunc::files_in_subdir_matching_substring(
            connected_components_ptr->get_cc_subdir(),
            output_filenames_prefix);
      int n_output_filenames=output_filenames.size();

      for (int f=0; f<n_output_filenames; f++)
      {
         if (nrfunc::ran1() > random_threshold)
         {
            filefunc::deletefile(output_filenames[f]);
         }
      }
   } // loop over index i labeling input 32x32 jpeg files

   delete connected_components_ptr;
}

