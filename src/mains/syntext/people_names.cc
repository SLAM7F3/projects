// ==========================================================================
// Program PEOPLE_NAMES combines female and male first names with last
// names in order to synthesize reasonable-sounding full names.  Some
// full names are randomly spelled with all capital letters.  Others
// have their first and/or middle names replaced with initials.
// ==========================================================================
// Last updated on 3/5/16; 3/7/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
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

   vector<string> input_filenames;
   input_filenames.push_back("female_first_names.dat");
   input_filenames.push_back("male_first_names.dat");
   input_filenames.push_back("last_names.dat");

   vector<string> female_first_names;
   vector<string> male_first_names;
   vector<string> last_names;

   for(int iter = 0; iter < 3; iter++)
   {
      filefunc::ReadInfile(input_filenames[iter]);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         string curr_name = substrings[0];

         if(iter==0)
         {
            female_first_names.push_back(curr_name);
         }
         else if (iter==1)
         {
            male_first_names.push_back(curr_name);
         }
         else
         {
            last_names.push_back(curr_name);
         }
      }
   } // loop over iter labeling female, male and last names

   int n_female_first_names = female_first_names.size();
   int n_male_first_names = male_first_names.size();
   int n_last_names = last_names.size();

   cout << "female_first_names.size() = " << n_female_first_names << endl;
   cout << "male_first_names.size() = " << n_male_first_names << endl;
   cout << "last_names.size() = " << n_last_names << endl;

   int n_full_names = 3000;
   cout << "Enter number of full names to synthesize:" << endl;
   cin >> n_full_names;

   string output_filename="full_names.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);


   for(int i = 0; i < n_full_names; i++)
   {
      if(i%1000==0)
      {
         double progress_frac = double(i)/(n_last_names);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string first_name, middle_name;
      string last_name = last_names[i];

      double capitalize_random = nrfunc::ran1();
      if(capitalize_random < 0.50)
      {
         last_name = stringfunc::capitalize_just_first_letter(last_name);         
      }

      if(i%2 == 0)
      {
         first_name = female_first_names[nrfunc::ran1()*n_female_first_names];
         middle_name = female_first_names[nrfunc::ran1()
                                          *n_female_first_names];
      }
      else
      {
         first_name = male_first_names[nrfunc::ran1()*n_male_first_names];
         middle_name = male_first_names[nrfunc::ran1()*n_male_first_names];
      }
      
      if(capitalize_random < 0.50)
      {
         first_name = stringfunc::capitalize_just_first_letter(first_name);
         middle_name = stringfunc::capitalize_just_first_letter(middle_name);
      }

      double middle_name_random = nrfunc::ran1();
      string full_name = first_name+" "+middle_name+" "+last_name;      
      if(middle_name_random < 0.05)
      {
         string first_initial = first_name.substr(0,1)+".";
         string middle_initial = middle_name.substr(0,1)+".";
         full_name = first_initial+" "+middle_initial+" "+last_name;      
      }
      else if(middle_name_random < 0.25)
      {
         string middle_initial = middle_name.substr(0,1)+".";
         full_name = first_name+" "+middle_initial+" "+last_name;      
      }
      else if(middle_name_random < 0.75)
      {
         full_name = first_name+" "+last_name;      
      }

//      cout << i << "  " << full_name << endl;
      outstream << full_name << endl;
   } // loop over index i labeling last names

   filefunc::closefile(output_filename, outstream);

   string shuffled_output_filename="shuffled_"+output_filename;
   string unix_cmd="shuf "+output_filename+" > "+shuffled_output_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported synthesized phrases to "+shuffled_output_filename;
   outputfunc::write_banner(banner);
}

