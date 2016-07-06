// ========================================================================
// Program KEYWORD_ATTRIBUTES queries the user to enter some keyword.
// It then scans all text files corresponding to URLs within
// the images table of the imagery database that match onto a
// particular graph_hierarchy_ID.  Any text file whose title/abstract
// contains a stemmed version of the input keyword is considered to
// have the keyword as an attribute.  Keyword attribute information is
// inserted into the image_attributes table.
// ========================================================================
// Last updated on 12/15/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
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

   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;
   const int ascii_hyphen=45;

   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
   string astro_subdir=arXiv_subdir+"astro/";
   string text_subdir=astro_subdir+"txt/";

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int hierarchy_ID=7;	// Astro1K
   int graph_ID=0;

   vector<int> campaign_IDs,mission_IDs,image_IDs,datum_IDs;
   vector<string> URLs,text_filenames;
   
   imagesdatabasefunc::get_image_URLs(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      campaign_IDs,mission_IDs,image_IDs,datum_IDs,URLs);

   for (int i=0; i<URLs.size(); i++)
   {
      string text_filename=filefunc::getbasename(URLs[i]);
      string prefix=stringfunc::prefix(text_filename);
      text_filenames.push_back(text_subdir+prefix+".txt");
//      cout << "text_filename = " << text_filenames.back() << endl;
   } // loop over index i 


   int min_keyword_freq=3;
   int n_text_files=text_filenames.size();

// Loop over all strings within all text documents starts here:

   string keyword="neutrino";
   cout << "Enter keyword:" << endl;
   cin >> keyword;
   
   int n_docs_containing_keyword=0;
   for (int n=0; n<n_text_files; n++)
   {
      string text_filename=text_filenames[n];
      filefunc::ReadInfile(text_filename);

      int n_keywords_in_doc=0;
      bool introduction_found_flag=false;      
      for (int i=0; i<filefunc::text_line.size() && !introduction_found_flag; 
           i++)
      {
         string curr_line=filefunc::text_line[i];
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line);
         for (int s=0; s<substrings.size(); s++)
         {
            string curr_substring=substrings[s];

// Ignore numbers:

            if (stringfunc::is_number(curr_substring)) continue;

// Remove non-letters from strings:

            vector<int> ascii_string=stringfunc::decompose_string_to_ascii_rep(
               curr_substring);

            string cleaned_substring;
            for (int s=0; s<ascii_string.size(); s++)
            {
               int curr_ascii=ascii_string[s];
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
         

            if (cleaned_substring=="introduction")
            {
               introduction_found_flag=true;
            }
            else if (cleaned_substring=="pacs")
            {
               introduction_found_flag=true;
            }
            else if (cleaned_substring=="keywords")
            {
               introduction_found_flag=true;
            }
            else if (cleaned_substring=="contents")
            {
               introduction_found_flag=true;
            }
            else if (cleaned_substring=="subject")
            {
               introduction_found_flag=true;
            }

            if (introduction_found_flag) break;


            string stemmed_substring=stringfunc::stem_word(cleaned_substring);
            string stemmed_keyword=stringfunc::stem_word(keyword);

//            if (cleaned_substring==keyword)
            if (stemmed_substring==stemmed_keyword)
            {
               n_keywords_in_doc++;
            }

         } // loop over index s labeling substring in current line
      } // loop over index i labeling line within current text file

      string attribute_key="Astro_keyword_\""+keyword+"\"";
      string attribute_value="absent";
      if (n_keywords_in_doc > 0)
      {
         cout << "n = " << n << " keyword found in " 
              << filefunc::getbasename(text_filename) << endl;
         cout << "n_keywords_in_doc = " << n_keywords_in_doc << endl;
         cout << endl;
         n_docs_containing_keyword++;
         attribute_value="present";
      }

      imagesdatabasefunc::insert_image_attribute(
         postgis_db_ptr,campaign_IDs[n],mission_IDs[n],
         image_IDs[n],datum_IDs[n],
         attribute_key,attribute_value);

   } // loop over index n labeling input text files

   cout << "n_docs_containing_keyword = " 
        << n_docs_containing_keyword << endl;
}

