// ==========================================================================
// Program SIFTWORDS_2_TEXT imports Voronoi cell histograms generated
// by program IMAGE_WORD_HISTOGRAMS for some number of input images.
// Each SIFT word that has a nonzero frequency within an input image
// "document" is converted into a 5-digit integer string (0-16383).
// The integer string is exported to an output text file with a
// frequency equaling that in the Voronoi cell histogram.  The
// output file represents a "text document" equivalent for an image.
// ==========================================================================
// Last updated on 8/20/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();      

   string sift_keys_subdir="/data/sift_keyfiles/";
   string voronoi_subdir=sift_keys_subdir+"voronoi/";
   string textfiles_subdir=voronoi_subdir+"text_files/";

// Import Voronoi cell histograms for input images:

   cout << "Importing Voronoi cell histograms:" << endl;
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("voronoi");
   vector<string> voronoi_cell_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,voronoi_subdir);
   int n_images=voronoi_cell_filenames.size();
   cout << "n_images = " << n_images << endl;

//   n_images=1000;
//   n_images=10000;
   cout << "n_images = " << n_images << endl;

// Import image voronoi cell occupancy histograms from input text files:

   int n_start=0;
//   int n_start=87600;
   int n_stop=n_images;
   ofstream textstream;
   for (int n=n_start; n<n_stop; n++)
   {
      if (n > 0 && n%100==0) cout << n << " " << flush;
      if (n%1000==0) cout << endl;
      
      vector<double> column_values=filefunc::ReadInNumbers(
         voronoi_cell_filenames[n]);

      string basename=filefunc::getbasename(voronoi_cell_filenames[n]);
      string prefix=stringfunc::prefix(basename);
      string output_text_filename=textfiles_subdir+basename+".txt";
      filefunc::openfile(output_text_filename,textstream);
      for (unsigned int c=0; c<column_values.size(); c++)
      {
         if (column_values[c]==0) continue;
         for (int j=0; j<column_values[c]; j++)
         {
            textstream << stringfunc::integer_to_string(c,5) << " ";
         }
         textstream << endl;
      } // loop over index c labeling SIFT words
      textstream << endl;
      filefunc::closefile(output_text_filename,textstream);

   } // loop over index n effectively labeling image "documents"
   cout << endl;

   cout << "At end of program SIFTWORDS_2_TEXT" << endl;
   outputfunc::print_elapsed_time();
}
