// ==========================================================================
// Program NEWSGROUPS implements a bayesian learning approach to
// classify text articles as belonging to one of 20 newgroups.

//			       newsgroups

// ==========================================================================
// Last updated on 9/13/15; 9/14/15; 9/15/15
// ==========================================================================

#include  <algorithm>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "templates/mytemplates.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

typedef map<string,int> WORD_MAP;
// Independent string var: cleaned substring
// Dependent int var: Frequency count for cleaned substring in document corpus

// --------------------------------------------------------------------------
// Method find_article_starting_linenumber() parses the newsgroup
// article currently assumed to have been imported into
// filefunc::text_line.  It searches for the sentinel "Lines: " within
// the current article and returns its line number.

int find_article_starting_linenumber()
{
   int start_linenumber = 0;

   for(unsigned int l = 0; l < filefunc::text_line.size(); l++)
   {
      start_linenumber = l;
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(filefunc::text_line[l]);
            
      if(substrings.size() > 0)
      {
         if(substrings[0] == "Lines:") break;
      }
   }

   return start_linenumber;
}

// --------------------------------------------------------------------------
// Method clean_substring() takes in a string.  It returns an empty
// string if the input string is purely numerical.  It also removes
// any non-letters from the input string.  All uppercase letters are
// converted to lowercase letters.  Any leading or trailing whitespace
// is removed.  If the cleaned string corresponds to a stop-word, it
// is reset to an empty string.  The cleaned string is returned by
// this method.

string clean_substring(string curr_substring, WORD_MAP& stop_word_map)
{
   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;

   string cleaned_substring="";
   
// Ignore numbers:

//   if (stringfunc::is_number(curr_substring)) return cleaned_substring;

   vector<int> ascii_string=
      stringfunc::decompose_string_to_ascii_rep(curr_substring);
   for (unsigned int s=0; s<ascii_string.size(); s++)
   {
      int curr_ascii=ascii_string[s];

// Remove non-letters from strings.  Also convert uppercase letters to
// lowercase:

      if (curr_ascii >= ascii_A && curr_ascii <= ascii_Z)
      {
         curr_ascii += ascii_a-ascii_A;
         char new_char=stringfunc::ascii_integer_to_char(
            curr_ascii);
         cleaned_substring.push_back(new_char);
      }
      else if (curr_ascii >= ascii_a && curr_ascii <= ascii_z)
      {
         cleaned_substring.push_back(curr_substring[s]);
      }

   } // loop over index s labeling chars within curr_substring
               
   cleaned_substring=
      stringfunc::remove_leading_whitespace(cleaned_substring);
   cleaned_substring=
      stringfunc::remove_trailing_whitespace(cleaned_substring);

// Ignore cleaned strings which match "stop" words:

   WORD_MAP::iterator stop_word_iter=stop_word_map.find(cleaned_substring);
   if (stop_word_iter != stop_word_map.end()) cleaned_substring="";

   return cleaned_substring;
}

// --------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   cout.precision(8);
   nrfunc::init_time_based_seed();

// Import stop word list downloaded from web and augmented by us:

   string stoplist_filename="./data/stop_list.txt";
   filefunc::ReadInfile(stoplist_filename);

   WORD_MAP::iterator word_iter,stop_word_iter;
   WORD_MAP stop_word_map, multi_doc_word_map;

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

   string newsgroups_datadir = "./data/20_newsgroups/";

   vector<string> newsgroup_subdirs;
   newsgroup_subdirs.push_back("alt.atheism/");
   newsgroup_subdirs.push_back("comp.graphics/");
   newsgroup_subdirs.push_back("comp.os.ms-windows.misc/");
   newsgroup_subdirs.push_back("comp.sys.ibm.pc.hardware/");
   newsgroup_subdirs.push_back("comp.sys.mac.hardware/");
   newsgroup_subdirs.push_back("comp.windows.x/");
   newsgroup_subdirs.push_back("misc.forsale/");
   newsgroup_subdirs.push_back("rec.autos/");
   newsgroup_subdirs.push_back("rec.motorcycles/");
   newsgroup_subdirs.push_back("rec.sport.baseball/");
   newsgroup_subdirs.push_back("rec.sport.hockey/");
   newsgroup_subdirs.push_back("sci.crypt/");
   newsgroup_subdirs.push_back("sci.electronics/");
   newsgroup_subdirs.push_back("sci.med/");
   newsgroup_subdirs.push_back("sci.space/");
   newsgroup_subdirs.push_back("soc.religion.christian/");
   newsgroup_subdirs.push_back("talk.politics.guns/");
   newsgroup_subdirs.push_back("talk.politics.mideast/");
   newsgroup_subdirs.push_back("talk.politics.misc/");
   newsgroup_subdirs.push_back("talk.religion.misc/");
   int n_newsgroups = newsgroup_subdirs.size();

   typedef map<int, vector<string> > NEWSGROUP_ARTICLES_MAP;
   // Independent int var: Newsgroup ID
// Dependent vector var: List of newsgroup article filenames
   NEWSGROUP_ARTICLES_MAP training_filenames_map, testing_filenames_map;

// Initial loop over all newsgroups starts here:

   int n_all_training_articles = 0, n_all_testing_articles = 0;
   for(int i = 0; i < n_newsgroups; i++)
   {
      newsgroup_subdirs[i] = newsgroups_datadir + newsgroup_subdirs[i];
      cout << i << "  " << newsgroup_subdirs[i] << endl;

      vector<string> filenames = filefunc::files_in_subdir(
         newsgroup_subdirs[i]);
      
// Split article filenames for current newgroup into training (2/3)
// and testing (1/3) sets:

      vector<string> training_filenames, testing_filenames;
      for(unsigned int f = 0; f < filenames.size(); f++)
      {
         if(nrfunc::ran1() < 0.66)
         {
            training_filenames.push_back(filenames[f]);
            n_all_training_articles++;
         }
         else
         {
            testing_filenames.push_back(filenames[f]);
            n_all_testing_articles++;
         }
      }
      training_filenames_map[i] = training_filenames;
      testing_filenames_map[i] = testing_filenames;

// Loop over each file within a particular newsgroup starts here:

      for(unsigned int f = 0; f < training_filenames.size(); f++)
      {
         filefunc::ReadInfile(training_filenames[f]);

         int start_linenumber = find_article_starting_linenumber();
         for(unsigned int l = start_linenumber + 1; 
             l < filefunc::text_line.size(); l++)
         {
            vector<string> substrings = 
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[l]);
            for(unsigned int s = 0; s < substrings.size(); s++)
            {
               string cleaned_substring = clean_substring(
                  substrings[s], stop_word_map);

// Ignore cleaned substring if it's empty:

               if(cleaned_substring.size() == 0) continue;

               word_iter=multi_doc_word_map.find(cleaned_substring);
               if (word_iter==multi_doc_word_map.end())
               {
                  multi_doc_word_map[cleaned_substring]=1;
               }
               else
               {
                  word_iter->second=word_iter->second+1;
               }
            }
            
         } // loop over index l labeling lines within current newsgroup file
	   //  starting after "Lines:"
      } // loop over index f labeling training files in a particular newsgroup
   } // loop over index i labeling newsgroup subdirs

   for(NEWSGROUP_ARTICLES_MAP::iterator iter = training_filenames_map.begin();
       iter != training_filenames_map.end(); iter++)
   {
      cout << "newsgroup ID = " << iter->first
           << " n_training_filenames = " << iter->second.size()
           << endl;
   }

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

   string bag_filename=newsgroups_datadir+"all_training_docs.wordbag";
   ofstream bagstream;
   filefunc::openfile(bag_filename,bagstream);
   bagstream << "# ID  stemmed_word   corpus_freq" << endl << endl;

   int min_word_freq=3;
//   int max_word_freq = 3300;
   int word_ID=0;
   for (unsigned int i=0; i<word.size(); i++)
   {
      if (word_frequency[i] < min_word_freq) continue;
//      if (word_frequency[i] > max_word_freq) continue;
      bagstream << word_ID << "    " << word[i] << "    " 
                << word_frequency[i] << endl;
      word_ID++;
   }
   filefunc::closefile(bag_filename,bagstream);
   int n_vocabulary = word_ID;

   cout << endl << endl;
   cout << "Number of training articles = " << n_all_training_articles << endl;
   cout << "Number of testing articles = " << n_all_testing_articles << endl;
   cout << "Number of stop words = " << stop_word_map.size() << endl;
   cout << "Number of distinct, significant words found in all training articles = "
        << n_vocabulary << endl;
   cout << "Minimum frequency for significant word in all docs = "
        << min_word_freq << endl;
   string banner="Exported word bag for all imported documents to "+
      bag_filename;
   outputfunc::write_big_banner(banner);

   typedef map<pair<string,int>, double> WORD_NEWSGROUP_PROB_MAP;
// Independent pair var: (cleaned substring, newsgroup ID)
// Dependent int var: Conditional probability for word given newsgroup
   WORD_NEWSGROUP_PROB_MAP word_newsgroup_prob_map;

   vector<double> newsgroup_probs;
   for(int i = 0; i < n_newsgroups; i++)
   {
      cout << "i = " << i << " newsgroup = " << newsgroup_subdirs[i]
           << endl;

      NEWSGROUP_ARTICLES_MAP::iterator iter = training_filenames_map.find(i);
      vector<string> training_filenames = iter->second;

// First assign a priori probabilities to newsgroups based upon their
// number of training articles:

      double curr_prob = double(training_filenames.size()) / 
         n_all_training_articles;
      newsgroup_probs.push_back(curr_prob);
      cout << "newsgroup_prob = " << newsgroup_probs[i] << endl;

// Next compute frequencies for cleaned words within current
// newsgroup:

      WORD_MAP curr_newsgroup_word_map;

      for(unsigned int f = 0; f < training_filenames.size(); f++)
      {
         filefunc::ReadInfile(training_filenames[f]);

         int start_linenumber = find_article_starting_linenumber();
         for(unsigned int l = start_linenumber + 1; 
             l < filefunc::text_line.size(); l++)
         {
            vector<string> substrings = 
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[l]);
            for(unsigned int s = 0; s < substrings.size(); s++)
            {
               string cleaned_substring = clean_substring(
                  substrings[s], stop_word_map);

// Ignore cleaned substring if it's empty:

               if(cleaned_substring.size() == 0) continue;

// Ignore cleaned substring if it doesn't belong to bag of words:

               WORD_MAP::iterator word_iter = multi_doc_word_map.find(
                  cleaned_substring);
               if(word_iter == multi_doc_word_map.end()) continue;

               word_iter = curr_newsgroup_word_map.find(cleaned_substring);
               if(word_iter == curr_newsgroup_word_map.end())
               {
                  curr_newsgroup_word_map[cleaned_substring] = 1;
               }
               else
               {
                  word_iter->second = word_iter->second + 1;
               }

            } // loop over index s labeling current article substrings
         } // loop over index l labeling lines in current article
      } // loop over index f labeling current newsgroup articles

// Compute total number of cleaned words for current newsgroup:

      int n_cleaned_words_in_newsgroup = 0;
      for(WORD_MAP::iterator word_iter = curr_newsgroup_word_map.begin();
          word_iter != curr_newsgroup_word_map.end(); word_iter++)
      {
         n_cleaned_words_in_newsgroup += word_iter->second;
      }
      cout << "n_cleaned_words_in_newsgroup (counting repetitions) = " 
           << n_cleaned_words_in_newsgroup << endl;
      
// Compute conditional probabilities for all words within entire
// training vocabulary corpus given current newsgroup:

      for(WORD_MAP::iterator word_iter = multi_doc_word_map.begin();
          word_iter != multi_doc_word_map.end(); word_iter++)
      {
         string cleaned_substring = word_iter->first;

         WORD_MAP::iterator newsgroup_word_iter = 
            curr_newsgroup_word_map.find(cleaned_substring);
         double numer = 1.0;
         if(newsgroup_word_iter != curr_newsgroup_word_map.end())
         {
            numer += newsgroup_word_iter->second;
         }
         double curr_conditional_prob = numer / 
            (n_cleaned_words_in_newsgroup + n_vocabulary);

         pair<string, int> P;
         P.first = cleaned_substring;
         P.second = i;

//         cout << "cleaned_substring = " << P.first 
//              << " newsgroup ID = " << P.second
//              << " numer = " << numer
//              << " conditional prob = " << curr_conditional_prob
//              << " multi_doc_word_map.size = "
//              << multi_doc_word_map.size() 
//              << endl;

         word_newsgroup_prob_map[P] = curr_conditional_prob;
         
      } // loop over all words within entire vocabulary corpus
   } // loop over index i labeling newsgroups

// Loop over testing articles starts here:

   int n_predictions = 0;
   int n_correct_predictions = 0;
   for(NEWSGROUP_ARTICLES_MAP::iterator iter = testing_filenames_map.begin();
       iter != testing_filenames_map.end(); iter++)
   {
      int testing_newsgroup_ID = iter->first;
      vector<string> testing_filenames = iter->second;

      for(unsigned int f = 0; f < testing_filenames.size(); f++)
      {
         filefunc::ReadInfile(testing_filenames[f]);

         WORD_MAP curr_article_word_map;

         int start_linenumber = find_article_starting_linenumber();
         for(unsigned int l = start_linenumber + 1; 
             l < filefunc::text_line.size(); l++)
         {
            vector<string> substrings = 
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[l]);
            for(unsigned int s = 0; s < substrings.size(); s++)
            {
               string cleaned_substring = clean_substring(
                  substrings[s], stop_word_map);

// Ignore cleaned substring if it's empty:

               if(cleaned_substring.size() == 0) continue;

// Ignore cleaned substring if it doesn't belong to bag of words:

               WORD_MAP::iterator word_iter = multi_doc_word_map.find(
                  cleaned_substring);
               if(word_iter == multi_doc_word_map.end()) continue;

               word_iter = curr_article_word_map.find(cleaned_substring);
               if(word_iter == curr_article_word_map.end())
               {
                  curr_article_word_map[cleaned_substring] = 1;
               }
               else
               {
                  word_iter->second = word_iter->second + 1;
               }
            } // loop over index s labeling current article substrings
         } // loop over index l labeling lines in current article

         vector<int> newsgroup_IDs;
         vector<double> newsgroup_log_probs;
         for(int j = 0; j < n_newsgroups; j++)
         {
            newsgroup_IDs.push_back(j);
            double curr_logprob = log(newsgroup_probs[j]);
            for(WORD_MAP::iterator word_iter = curr_article_word_map.begin();
                word_iter != curr_article_word_map.end(); word_iter++)
            {
               string curr_word = word_iter->first;
               int word_count_in_article = word_iter->second;

               pair<string, int> P;
               P.first = curr_word;
               P.second = j;

               WORD_NEWSGROUP_PROB_MAP::iterator prob_iter = 
                  word_newsgroup_prob_map.find(P);
               if(prob_iter != word_newsgroup_prob_map.end())
               {
                  curr_logprob += word_count_in_article * 
                     log(prob_iter->second);

//                  cout << "curr_word = " << curr_word
//                       << " word_count_in_article = " << word_count_in_article
//                       << " cond prob = " << prob_iter->second
//                       << " curr_logprob = " << curr_logprob
//                       << endl;
               }
            } // loop over iterator for curr_article_word_map
            newsgroup_log_probs.push_back(curr_logprob);
//            cout << "j = " << j 
//                 << " newsgroup_log_prob = " << newsgroup_log_probs.back() 
//                 << endl;
         } // loop over index j labeling all newsgroups

         templatefunc::Quicksort_descending(newsgroup_log_probs, newsgroup_IDs);

         int predicted_newsgroup_ID = newsgroup_IDs.front();
         if(predicted_newsgroup_ID == testing_newsgroup_ID)
         {
            n_correct_predictions++;
         }
         n_predictions++;
         double prediction_success_frac = double(n_correct_predictions)/
            n_predictions;

         cout << n_predictions 
              << " Newsgroup ID: Predicted = " << newsgroup_IDs.front()
              << " Actual = " << testing_newsgroup_ID 
              << " Success frac = " << prediction_success_frac
              << endl;

      } // loop over index f labeling testing filenames for curr newsgroup
   } // loop over testing_filenames_map iterator

}



