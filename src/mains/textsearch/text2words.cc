// ========================================================================
// Program TEXT2WORDS imports all text files from a specified
// subdirectory as well as a stop list of common words.  It loops over
// all strings within all of the input documents.  TEXT2WORDS drops
// any non-letter characters it encounters within a string and
// converts all upper-case letters to lower-case.  It also discards
// very short strings and stems all surviving strings.  TEXT2WORDS
// exports a "Bag of words" file which is ordered according to cleaned
// string frequency.

// 				text2words

// ========================================================================
// Last updated on 12/24/12; 12/29/12; 5/29/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

//   int ascii_hyphen=stringfunc::unsigned_char_to_ascii_integer('-');
//   int ascii_a=stringfunc::unsigned_char_to_ascii_integer('a');
//   int ascii_z=stringfunc::unsigned_char_to_ascii_integer('z');
//   int ascii_A=stringfunc::unsigned_char_to_ascii_integer('A');
//   int ascii_Z=stringfunc::unsigned_char_to_ascii_integer('Z');

   // Ascii_hyphen = 45
   // Ascii_A = 65
   // Ascii_Z = 90
   // Ascii_a = 97
   // Ascii_z = 122

   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;
//   const int ascii_hyphen=45;

   int doc_level=0;
//   cout << "Enter document level (0 = raw docs, 1 = docs generated from topics):" << endl;
//   cin >> doc_level;

   bool astro_flag=false;
   bool reuters_flag=true;

   string reuters_subdir=
      "/data_third_disk/text_docs/reuters/export/";
   string text_subdir=reuters_subdir+"text/50K_docs/";
   string topic_docs_subdir=text_subdir+"topic_docs/";

//   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
//   string astro_subdir=arXiv_subdir+"astro/";
//   string reuters_subdir=arXiv_subdir+"reuters/export/";
//   string text_subdir=reuters_subdir+"text/";
//   string ccs_subdir=text_subdir+"connected_components/";
//   string topic_docs_subdir=ccs_subdir+"topic_docs/";

   if (doc_level==1)
   {
      text_subdir=topic_docs_subdir;
   }

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> text_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,text_subdir);
   int n_text_files=text_filenames.size();

// Import stop word list downloaded from web and augmented by us:

   string stoplist_filename="./stop_list.txt";
   filefunc::ReadInfile(stoplist_filename);

   typedef map<string,int> WORD_MAP;
   WORD_MAP::iterator word_iter,stop_word_iter;
   WORD_MAP multi_doc_word_map,stop_word_map;


   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string stop_word=filefunc::text_line[i];
      stop_word=stringfunc::remove_leading_whitespace(stop_word);
      stop_word=stringfunc::remove_trailing_whitespace(stop_word);
      stop_word_map[stop_word]=-1;
//      cout << i << "   stop_word = " << stop_word 
//           << " word size = " << stop_word.size() << endl;
   }
   cout << "stop_word_map.size() = " << stop_word_map.size() << endl;

// Loop over all strings within all text files starts here:

   for (int n=0; n<n_text_files; n++)
   {
      string text_filename=text_filenames[n];
      if (n%100==0)
      {
         cout << "n = " << n << " of " << n_text_files << ": File="+
            filefunc::getbasename(text_filename) << endl;
      }
      filefunc::ReadInfile(text_filename);

      bool introduction_found_flag=false;      
      bool reuters_end_found_flag=false;
      for (unsigned int i=0; i<filefunc::text_line.size() && !introduction_found_flag; 
           i++)
      {
         string curr_line=filefunc::text_line[i];
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line);
         for (unsigned int s=0; s<substrings.size(); s++)
         {
            string curr_substring=substrings[s];

// Ignore numbers:

            if (stringfunc::is_number(curr_substring)) continue;

// Search for keywords near end of Reuters' articles:

            if (reuters_flag)
            {
               if (curr_substring=="(Reporting" ||
               curr_substring=="(reporting" || curr_substring=="(Reported" ||
               curr_substring=="(reported")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Editing" ||
               curr_substring=="(editing" || curr_substring=="(Edited" ||
               curr_substring=="(edited")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Additional" ||
               curr_substring=="(additional" || curr_substring=="(Added" ||
               curr_substring=="(added")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Writing" ||
               curr_substring=="(writing" || curr_substring=="(Written" ||
               curr_substring=="(written")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Created" ||
               curr_substring=="(created")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Compiled" ||
               curr_substring=="(compiled")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Via" ||
               curr_substring=="(via")
               {
                  reuters_end_found_flag=true;
               }
               else if (curr_substring=="(Sources:" ||
               curr_substring=="(sources" || curr_substring=="(SOURCES:" ||
               curr_substring=="(SOURCE:"|| curr_substring=="(source:" ||
               curr_substring=="Sources:" || curr_substring=="sources:" ||
               curr_substring=="SOURCES:" || curr_substring=="SOURCE:")
               {
                  reuters_end_found_flag=true;
               }
            } // reuters_flag conditional
            if (reuters_end_found_flag) break;

// Remove non-letters from strings:

            vector<int> ascii_string=stringfunc::decompose_string_to_ascii_rep(
               curr_substring);

            string cleaned_substring;
            for (unsigned int s=0; s<ascii_string.size(); s++)
            {
               int curr_ascii=ascii_string[s];

/*
               if (curr_ascii==ascii_hyphen)
               {
//                  cout << "pre_hyphen string = " << curr_substring << "  ";
                  if (s+1 < substrings.size()-1)
                  {
//                     cout << "post_hyphen string = " << substrings[s+1] 
//                          << endl;
                  }
                  else
                  {
                     cout << "pre_hyphen string = " << curr_substring << endl;
                  }
               }
*/

               if (curr_ascii >= ascii_a && curr_ascii <= ascii_z)
               {
                  cleaned_substring.push_back(curr_substring[s]);
               }

// Convert uppercase letters to lowercase:

               if (curr_ascii >= ascii_A && curr_ascii <= ascii_Z)
               {
                  curr_ascii += ascii_a-ascii_A;
                  char new_char=stringfunc::ascii_integer_to_char(curr_ascii);
                  cleaned_substring.push_back(new_char);
               }
            }
            cleaned_substring=
               stringfunc::remove_leading_whitespace(cleaned_substring);
            cleaned_substring=
               stringfunc::remove_trailing_whitespace(cleaned_substring);

            if (astro_flag)
            {
               if (cleaned_substring=="introduction")
               {
                  cout << "Introduction found" << endl;
                  introduction_found_flag=true;
               }
               else if (cleaned_substring=="pacs")
               {
                  cout << "PACS found" << endl;
                  introduction_found_flag=true;
               }
               else if (cleaned_substring=="keywords")
               {
                  cout << "Keywords found" << endl;
                  introduction_found_flag=true;
               }
               else if (cleaned_substring=="contents")
               {
                  cout << "contents found" << endl;
                  introduction_found_flag=true;
               }
               else if (cleaned_substring=="subject")
               {
                  cout << "subject found" << endl;
                  introduction_found_flag=true;
               }
               if (introduction_found_flag) break;
            }
            
// Ignore short strings:

            if (cleaned_substring.size() < 3) continue;

// Ignore "words" containing "garbage":

            vector<string> garbage_strings;
            garbage_strings.push_back("qqq");
            garbage_strings.push_back("xxx");

            bool garbage_continue_flag=false;
            for (unsigned int g=0; g<garbage_strings.size(); g++)
            {
               int garbage_location=stringfunc::first_substring_location(
                  cleaned_substring,garbage_strings[g]);
               if (garbage_location > 0) 
               {
                  cout << "garbage_string = " << garbage_strings[g]
                       << " cleaned_substring = " << cleaned_substring
                       << endl;
                  garbage_continue_flag=true;
//                  outputfunc::enter_continue_char();
               }
            }
            if (garbage_continue_flag) continue;

// As of Sun, Dec 23, 2012, we no longer stem cleaned substrings:

            string stemmed_substring=cleaned_substring;

/*

// Stem cleaned substring:

            string stemmed_substring=stringfunc::stem_word(cleaned_substring);
            stemmed_substring=
               stringfunc::remove_leading_whitespace(stemmed_substring);
            stemmed_substring=
               stringfunc::remove_trailing_whitespace(stemmed_substring);
*/

// Ignore stemmed strings which match "stop" words:

            stop_word_iter=stop_word_map.find(stemmed_substring);
            if (stop_word_iter != stop_word_map.end()) continue;

            word_iter=multi_doc_word_map.find(stemmed_substring);
            if (word_iter==multi_doc_word_map.end())
            {
               multi_doc_word_map[stemmed_substring]=1;
            }
            else
            {
               word_iter->second=word_iter->second+1;
            }
         }
      } // loop over index i labeling line within current text file

      if (astro_flag && !introduction_found_flag)
      {
         cout << "No intro found" << endl;
         string unix_cmd="mv "+text_filename+" "+text_subdir+"no_intro/";
         cout << "unix_cmd = " << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
//         outputfunc::enter_continue_char();
      }
      
   } // loop over index n labeling input text files

   vector<int> word_frequency;
   vector<string> word;
   for (word_iter=multi_doc_word_map.begin(); 
        word_iter != multi_doc_word_map.end(); word_iter++)
   {
      word.push_back(word_iter->first);
      word_frequency.push_back(word_iter->second);
   }
   templatefunc::Quicksort_descending(word_frequency,word);

// Export ordered word frequency list to output "bag of words" file:

   string bag_filename=text_subdir+"all_docs.wordbag";
   ofstream bagstream;
   filefunc::openfile(bag_filename,bagstream);
   bagstream << "# ID  stemmed_word   corpus_freq" << endl << endl;

//   int min_word_freq=3;	// astro
   int min_word_freq=20;	// reuters
   if (doc_level==1) min_word_freq=2;
   
   int word_ID=0;
   for (unsigned int i=0; i<word.size(); i++)
   {
      if (word_frequency[i] < min_word_freq) continue;
      bagstream << word_ID << "    " << word[i] << "    " 
                << word_frequency[i] << endl;
      word_ID++;
   }
   filefunc::closefile(bag_filename,bagstream);

   cout << endl << endl;
   cout << "Number of imported documents = " << n_text_files << endl;
   cout << "Number of stop words = " << stop_word_map.size() << endl;
   cout << "Number of significant words found in documents = "
        << word_ID << endl;
   cout << "Minimum frequency for significant word in all docs = "
        << min_word_freq << endl;
   string banner="Exported word bag for all imported documents to "+
      bag_filename;
   outputfunc::write_big_banner(banner);
}

