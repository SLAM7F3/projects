// ==========================================================================
// Program SPAM is a relatively minor variant of NEWSGROUPS.  It
// imports O(100K) email messages from the 2005 Trec Public Spam
// corpus which have been labeled as either "spam" or "ham" = "not
// spam".  SPAM randomly takes 80% of the input corpus to be training data.
// It utilizes a naive bayes classifier to then label the remaining
// 20% test data as "ham" or "spam".  

// We empirically find that the naive bayes classifier's success
// fraction is approximately 97%.

//			       spam

// ==========================================================================
// Last updated on 9/27/15
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
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
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
// Method find_email_starting_linenumber() parses the email
// currently assumed to have been imported into
// filefunc::text_line.  It searches for the sentinel "Return-Path: " within
// the current email and returns its line number.

int find_email_starting_linenumber()
{
   int start_linenumber = 0;

   for(unsigned int l = 0; l < filefunc::text_line.size(); l++)
   {
      start_linenumber = l;
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(filefunc::text_line[l]);
            
      if(substrings.size() > 0)
      {
         if(substrings[0] == "Return-Path:") break;
      }
   }

// FAKE FAKE:  Sun Sep 27 at 10:50 am

// Experiment with using entire emails including their headers...

   start_linenumber = 0;

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
   const int ascii_exclamation_point = 21;

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
      else if (curr_ascii == ascii_exclamation_point)
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
   timefunc::initialize_timeofday_clock();

// Import stop word list downloaded from web and augmented by us:

   string trec_datadir = "./data/trec05p-1/";
   string stoplist_filename=trec_datadir+"stop_list.txt";
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

// Import lists of ham and spam filenames:

   vector<string> spam_filenames, ham_filenames;


   string index_file = trec_datadir + "full/index";
   filefunc::ReadInfile(index_file);
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string curr_filename = substrings[1];
      curr_filename = trec_datadir + 
         curr_filename.substr(3,curr_filename.size() - 3);
      if(substrings[0] == "ham")
      {
         ham_filenames.push_back(curr_filename);
      }
      else
      {
         spam_filenames.push_back(curr_filename);
      }
   }

// Decompose ham and spam filenames into training and testing sets:

   double training_frac = 0.8;

   int n_ham_filenames = ham_filenames.size();
   int n_spam_filenames = spam_filenames.size();
   vector<int> ham_filename_IDs = mathfunc::random_sequence(n_ham_filenames);
   vector<int> spam_filename_IDs = mathfunc::random_sequence(n_spam_filenames);

   int n_ham_training_filenames = training_frac * n_ham_filenames;
   int n_ham_testing_filenames = n_ham_filenames - n_ham_training_filenames;
   int n_spam_training_filenames = training_frac * n_spam_filenames;
   int n_spam_testing_filenames = n_spam_filenames - n_spam_training_filenames;
   int n_all_training_filenames = n_ham_training_filenames + 
      n_spam_training_filenames;

   vector<int> ham_training_filename_IDs, spam_training_filename_IDs;
   vector<int> ham_testing_filename_IDs, spam_testing_filename_IDs;
   
   for(int i = 0; i < n_ham_training_filenames; i++)
   {
      ham_training_filename_IDs.push_back(ham_filename_IDs[i]);
   }
   for(int i = n_ham_training_filenames; i < n_ham_filenames; i++)
   {
      ham_testing_filename_IDs.push_back(ham_filename_IDs[i]);
   }

   for(int i = 0; i < n_spam_training_filenames; i++)
   {
      spam_training_filename_IDs.push_back(spam_filename_IDs[i]);
   }
   for(int i = n_spam_training_filenames; i < n_spam_filenames; i++)
   {
      spam_testing_filename_IDs.push_back(spam_filename_IDs[i]);
   }

   std::sort(ham_training_filename_IDs.begin(), 
             ham_training_filename_IDs.end());
   std::sort(ham_testing_filename_IDs.begin(), 
             ham_testing_filename_IDs.end());

   std::sort(spam_training_filename_IDs.begin(), 
             spam_training_filename_IDs.end());
   std::sort(spam_testing_filename_IDs.begin(), 
             spam_testing_filename_IDs.end());

   cout << "ham_filenames.size() = " << ham_filenames.size() << endl;
   cout << "ham_training_filename_IDs.size() = " 
        << ham_training_filename_IDs.size() << endl;
   cout << "ham_testing_filename_IDs.size() = " 
        << ham_testing_filename_IDs.size() << endl;

   cout << "spam_filenames.size() = " << spam_filenames.size() << endl;
   cout << "spam_training_filename_IDs.size() = " 
        << spam_training_filename_IDs.size() << endl;
   cout << "spam_testing_filename_IDs.size() = " 
        << spam_testing_filename_IDs.size() << endl;

   typedef map<int, vector<int> > EMAILS_MAP;
// Independent int var: 0 if ham, 1 if spam
// Dependent vector var: List of email filename IDs
   EMAILS_MAP training_filenames_map, testing_filenames_map;

   training_filenames_map[0] = ham_training_filename_IDs;
   training_filenames_map[1] = spam_training_filename_IDs;

   testing_filenames_map[0] = ham_testing_filename_IDs;
   testing_filenames_map[1] = spam_testing_filename_IDs;

   for(int i = 0; i < 2; i++)
   {
      string banner;
      if(i == 0)
      {
         banner="Processing ham training emails:";
      }
      else
      {
         banner="Processing spam training emails:";
      }
      outputfunc::write_banner(banner);
      
// Loop over each file within ham & spam training groups starts here:

      vector<int> training_filename_IDs = training_filenames_map[i];
      for(unsigned int f = 0; f < training_filename_IDs.size(); f++)
      {
         outputfunc::update_progress_fraction(
            f, 1000, training_filename_IDs.size());
         int curr_filename_ID=training_filename_IDs[f];
         string curr_filename;
         if(i == 0)
         {
            curr_filename=ham_filenames[curr_filename_ID];
         }
         else
         {
            curr_filename=spam_filenames[curr_filename_ID];
         }
         filefunc::ReadInfile(curr_filename);

         int start_linenumber = find_email_starting_linenumber();
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
            
         } // loop over index l labeling lines within current email
	   //  starting after "Return-Path:"
      } // loop over index f labeling training files in ham/spam subgroup
   } // loop over index i labeling ham/spam emails

   vector<string> word;
   vector<int> word_frequency;
   for (word_iter=multi_doc_word_map.begin(); 
        word_iter != multi_doc_word_map.end(); word_iter++)
   {
      word.push_back(word_iter->first);
      word_frequency.push_back(word_iter->second);
   }
   templatefunc::Quicksort_descending(word_frequency,word);

// Export ordered word frequency list to output "bag of words" file:

   string bag_filename=trec_datadir+"all_training_docs.wordbag";
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
   cout << "Number of stop words = " << stop_word_map.size() << endl;
   cout << "Number of distinct, significant words found in all training articles = "
        << n_vocabulary << endl;
   cout << "Minimum frequency for significant word in all docs = "
        << min_word_freq << endl;
   string banner="Exported word bag for all imported documents to "+
      bag_filename;
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();

   typedef map<pair<string,int>, double> WORD_HAMSPAM_PROB_MAP;
// Independent pair var: (cleaned substring, ham/spam ID)
// Dependent double var: Conditional probability for word given ham/spam ID
   WORD_HAMSPAM_PROB_MAP word_hamspam_prob_map;

   vector<double> hamspam_probs;
   for(int i = 0; i < 2; i++)
   {
      EMAILS_MAP::iterator iter = training_filenames_map.find(i);
      vector<int> training_filename_IDs = iter->second;

// First assign a priori probabilities to ham/spam based upon their
// number of training articles:

      double curr_prob = double(training_filename_IDs.size()) / 
         n_all_training_filenames;

      hamspam_probs.push_back(curr_prob);
      cout << "i = " << i << " hamspam_prob = " << hamspam_probs[i] << endl;

// Next compute frequencies for cleaned words within current
// hamspam subgroup:

      WORD_MAP curr_hamspam_word_map;

      for(unsigned int f = 0; f < training_filename_IDs.size(); f++)
      {
         outputfunc::update_progress_fraction(
            f, 1000, training_filename_IDs.size());

         int curr_filename_ID=training_filename_IDs[f];
         string curr_filename;
         if(i == 0)
         {
            curr_filename=ham_filenames[curr_filename_ID];
         }
         else
         {
            curr_filename=spam_filenames[curr_filename_ID];
         }
         filefunc::ReadInfile(curr_filename);

         int start_linenumber = find_email_starting_linenumber();
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

               word_iter = curr_hamspam_word_map.find(cleaned_substring);
               if(word_iter == curr_hamspam_word_map.end())
               {
                  curr_hamspam_word_map[cleaned_substring] = 1;
               }
               else
               {
                  word_iter->second = word_iter->second + 1;
               }

            } // loop over index s labeling current article substrings
         } // loop over index l labeling lines in current article
      } // loop over index f labeling current newsgroup articles

// Compute total number of cleaned words for current hamspam subgroup:

      int n_cleaned_words_in_hamspam_subgroup = 0;
      for(WORD_MAP::iterator word_iter = curr_hamspam_word_map.begin();
          word_iter != curr_hamspam_word_map.end(); word_iter++)
      {
         n_cleaned_words_in_hamspam_subgroup += word_iter->second;
      }
      cout << "n_cleaned_words_in_hamspam_subgroup (counting repetitions) = " 
           << n_cleaned_words_in_hamspam_subgroup << endl;
      
// Compute conditional probabilities for all words within entire
// training vocabulary corpus given current hamspam subgroup:

      for(WORD_MAP::iterator word_iter = multi_doc_word_map.begin();
          word_iter != multi_doc_word_map.end(); word_iter++)
      {
         string cleaned_substring = word_iter->first;

         WORD_MAP::iterator hamspam_word_iter = 
            curr_hamspam_word_map.find(cleaned_substring);
         double numer = 1.0;
         if(hamspam_word_iter != curr_hamspam_word_map.end())
         {
            numer += hamspam_word_iter->second;
         }
         double curr_conditional_prob = numer / 
            (n_cleaned_words_in_hamspam_subgroup + n_vocabulary);

         pair<string, int> P;
         P.first = cleaned_substring;
         P.second = i;

//         cout << "cleaned_substring = " << P.first 
//              << " hamspam ID = " << P.second
//              << " numer = " << numer
//              << " conditional prob = " << curr_conditional_prob
//              << " multi_doc_word_map.size = "
//              << multi_doc_word_map.size() 
//              << endl;

         word_hamspam_prob_map[P] = curr_conditional_prob;
         
      } // loop over all words within entire vocabulary corpus
   } // loop over index i labeling hamspam subgroup

// Loop over testing emails starts here:

   int n_predictions = 0;
   int n_correct_predictions = 0;
   for(EMAILS_MAP::iterator iter = testing_filenames_map.begin();
       iter != testing_filenames_map.end(); iter++)
   {
      int testing_hamspam_ID = iter->first;
      vector<int> testing_filename_IDs = iter->second;

      for(unsigned int f = 0; f < testing_filename_IDs.size(); f++)
      {
         outputfunc::update_progress_fraction(
            f, 1000, testing_filename_IDs.size());

         int curr_filename_ID=testing_filename_IDs[f];
         string curr_filename;
         if(testing_hamspam_ID == 0)
         {
            curr_filename=ham_filenames[curr_filename_ID];
         }
         else
         {
            curr_filename=spam_filenames[curr_filename_ID];
         }
         filefunc::ReadInfile(curr_filename);

         WORD_MAP curr_email_word_map;

         int start_linenumber = find_email_starting_linenumber();
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

               word_iter = curr_email_word_map.find(cleaned_substring);
               if(word_iter == curr_email_word_map.end())
               {
                  curr_email_word_map[cleaned_substring] = 1;
               }
               else
               {
                  word_iter->second = word_iter->second + 1;
               }
            } // loop over index s labeling current article substrings
         } // loop over index l labeling lines in current article

         vector<int> hamspam_IDs;
         vector<double> hamspam_log_probs;
         for(int j = 0; j < 2; j++)
         {
            hamspam_IDs.push_back(j);
            double curr_logprob = log(hamspam_probs[j]);
            for(WORD_MAP::iterator word_iter = curr_email_word_map.begin();
                word_iter != curr_email_word_map.end(); word_iter++)
            {
               string curr_word = word_iter->first;
               int word_count_in_email = word_iter->second;

               pair<string, int> P;
               P.first = curr_word;
               P.second = j;

               WORD_HAMSPAM_PROB_MAP::iterator prob_iter = 
                  word_hamspam_prob_map.find(P);
               if(prob_iter != word_hamspam_prob_map.end())
               {
                  curr_logprob += word_count_in_email * 
                     log(prob_iter->second);

//                  cout << "curr_word = " << curr_word
//                       << " word_count_in_email = " << word_count_in_email
//                       << " cond prob = " << prob_iter->second
//                       << " curr_logprob = " << curr_logprob
//                       << endl;
               }
            } // loop over iterator for curr_email_word_map
            hamspam_log_probs.push_back(curr_logprob);
//            cout << "j = " << j 
//                 << " hamspam_log_prob = " << hamspam_log_probs.back() 
//                 << endl;
         } // loop over index j labeling all newsgroups

         templatefunc::Quicksort_descending(hamspam_log_probs, hamspam_IDs);

         int predicted_hamspam_ID = hamspam_IDs.front();
         if(predicted_hamspam_ID == testing_hamspam_ID)
         {
            n_correct_predictions++;
         }
         n_predictions++;
         double prediction_success_frac = double(n_correct_predictions)/
            n_predictions;

         cout << n_predictions 
              << " HamSpam ID: Predicted = " << hamspam_IDs.front()
              << " Actual = " << testing_hamspam_ID 
              << " Success frac = " << prediction_success_frac
              << endl;

      } // loop over index f labeling testing filenames for curr newsgroup
   } // loop over testing_filenames_map iterator

   cout << "n_ham_testing_filenames = " << n_ham_testing_filenames << endl;
   cout << "n_spam_testing_filenames = " << n_spam_testing_filenames << endl;
   outputfunc::print_elapsed_time();

// Compute and report most likely words given that email is ham or
// spam:

   vector<string> ham_words, spam_words;
   vector<double> ham_conditional_probs, spam_conditional_probs;
   for(WORD_HAMSPAM_PROB_MAP::iterator iter = word_hamspam_prob_map.begin();
       iter != word_hamspam_prob_map.end(); iter++)
   {
      pair<string,int> P = iter->first;
      double curr_prob = iter->second;
      if(P.second == 0)
      {
         ham_words.push_back(P.first);
         ham_conditional_probs.push_back(curr_prob);
      }
      else
      {
         spam_words.push_back(P.first);
         spam_conditional_probs.push_back(curr_prob);
      }
   }
   
   templatefunc::Quicksort_descending(ham_conditional_probs, ham_words);
   templatefunc::Quicksort_descending(spam_conditional_probs, spam_words);

   string output_filename = trec_datadir+"ham_spam_words.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);

   int n_output_words = 50;
   for(int i = 0; i < n_output_words; i++)
   {
      outstream << "i = " << i << " ham_word = " << ham_words[i]
                << "    ham conditional prob = " << ham_conditional_probs[i]
                << endl;
   }

   for(int i = 0; i < n_output_words; i++)
   {
      outstream << "i = " << i << " spam_word = " << spam_words[i]
                << "    spam conditional prob = " << spam_conditional_probs[i]
                << endl;
   }
   filefunc::closefile(output_filename, outstream);
   banner="Exported most probable ham & spam words to "+output_filename;
   outputfunc::write_banner(banner);

   outputfunc::print_elapsed_time();
}



