// ==========================================================================
// Program PHRASES imports a set of text files in various topic
// categories.  Looping every line in each file, PHRASES first removes
// any white space.  It then retrieves 1 - 15 words from the cleaned
// lines.  PHRASES all randomly intersperses house numbers among
// English words.  Cleaned "phrases" are exported to an output text
// file.
// ==========================================================================
// Last updated on 3/13/16; 3/15/16; 3/31/16; 4/5/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;


// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   string text_files_subdir="./text_files/";
   filefunc::dircreate(text_files_subdir);

// Import house numbers:

   string housenumbers_filename="./text_files/house_numbers.txt";
   filefunc::ReadInfile(housenumbers_filename);
   vector<string> numerals;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      numerals.push_back(filefunc::text_line[i]);
//      cout << "i = " << i << " numeral = " << numerals.back() << endl;
   }

   vector<string> categories;
   categories.push_back("comp_news");
   categories.push_back("reuters");
   categories.push_back("science");
   categories.push_back("stories");

   vector<int> n_words;   
   for(unsigned int c = 0; c < categories.size(); c++) 
   {
      string text_subdir="./text_docs/"+categories[c]+"/";

      vector<string> allowed_suffixes;
      allowed_suffixes.push_back("txt");
      vector<string> input_filenames = 
         filefunc::files_in_subdir_matching_specified_suffixes(
            allowed_suffixes, text_subdir);
      cout << "Imported " << input_filenames.size() << " text files" << endl;

      string output_filename=text_files_subdir+categories[c]+".txt";
      cout << "output_filename = " << output_filename << endl;
   
      ofstream outstream;
      filefunc::openfile(output_filename, outstream);

      vector<string> phrases;

// As of 3/31/16, we can use ImageMagick to generate either strings
// containing apostrophes or double quotes, but not both.  So we allow
// for apostrophes to appear inside our phrases.

// On 4/5/16, we discovered the painful way that a phrase cannot
// contain ` folllowed later by ' .  So we reject any forward
// apostrophes found within input strings:

      string separator_chars=" `\"\t\n";
//       string punctuation=".,:;'`-+_^=/~|!$#@%&*?{}()[]<>";

      int n_phrases = 0;
      for(unsigned int iter = 0; iter < input_filenames.size(); iter++)
      {
         filefunc::ReadInfile(input_filenames[iter]);
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            vector<string> curr_line_words = 
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[i], separator_chars);

//            double lambda = 0.075;
            double lambda = 0.1;
//            double lambda = 0.2;
//            double lambda = 0.5;
            unsigned int n_phrase_words = 1 + nrfunc::expdev(lambda);
            unsigned int max_n_phrase_words = 15;
            n_phrase_words = basic_math::min(
               n_phrase_words, max_n_phrase_words);
         
            if(curr_line_words.size() < n_phrase_words)
            {
               n_phrase_words = curr_line_words.size();
            }
      
            n_words.push_back(n_phrase_words);

            string curr_phrase;
            for(unsigned int w = 0; w < n_phrase_words; w++)
            {
               if(nrfunc::ran1() < 0.04)
               {
                  curr_phrase += numerals[nrfunc::ran1() * numerals.size()]
                     +" ";
               }
               curr_phrase += curr_line_words[w];

               if(w < n_phrase_words-1)
               {
                  curr_phrase += " ";
               }
            }
//             cout << i << "  " << n_phrase_words << endl;
//             cout << curr_phrase << endl;
            outstream << curr_phrase << endl;
            n_phrases++;
         }
      } // loop over index iter labeling input text files

      filefunc::closefile(output_filename, outstream);

      string shuffled_output_filename=text_files_subdir+
         "shuffled_"+filefunc::getbasename(output_filename);
      string unix_cmd="shuf "+output_filename+" > "+shuffled_output_filename;
      sysfunc::unix_command(unix_cmd);

      string banner="Exported "+stringfunc::number_to_string(n_phrases)
         +" synthesized phrases to "+shuffled_output_filename;
      outputfunc::write_banner(banner);

   } // loop over index c labeling text categories 

   prob_distribution prob_words(n_words, 100, 0);
   prob_words.write_density_dist(false,true);
}

