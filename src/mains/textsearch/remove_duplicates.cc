// ==========================================================================
// Program REMOVE_DUPLICATES searches through all text files within a
// specified subdirectory.  It checks if a file and its immediate
// successor are identical via the unix "diff" command.  If so, the
// successor text file along with any html and jpg versions are moved 
// into duplicate subdirectories.  

// We wrote this little utility program after realizing that
// significant duplication exists within Michael Yee's Reuters data
// set.
// ==========================================================================
// Last updated on 12/23/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string reuters_subdir=
      "/media/66368D22368CF3F9/visualization/arXiv/reuters/export/";
   string text_subdir=reuters_subdir+"text/";
   string text_duplicates_subdir=text_subdir+"duplicates/";
   string html_subdir=reuters_subdir+"html/";
   string html_duplicates_subdir=html_subdir+"duplicates/";
   string jpg_subdir=reuters_subdir+"jpg/";
   string jpg_duplicates_subdir=jpg_subdir+"duplicates/";
   
   filefunc::dircreate(text_duplicates_subdir);
   filefunc::dircreate(html_duplicates_subdir);
   filefunc::dircreate(jpg_duplicates_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> txt_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,text_subdir);

   timefunc::initialize_timeofday_clock();

   int i_start=1;
   int i_stop=txt_filenames.size();
//   int i_stop=1000;
   string differences_filename="./diff.out";

   bool skip_next_flag=false;
   for (int i=i_start; i<i_stop; i++)
   {
      if (skip_next_flag)
      {
         skip_next_flag=false;
         continue;
      }
      
      cout << "Processing file " << i << " of " << i_stop << endl;

      string unix_cmd=
         "diff --brief "+txt_filenames[i-1]+" "+txt_filenames[i]
         +" > "+differences_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      
      sysfunc::unix_command(unix_cmd);

      string filename1=txt_filenames[i-1];
      string filename2=txt_filenames[i];
      string basename1=filefunc::getbasename(filename1);
      string basename2=filefunc::getbasename(filename2);

      if (filefunc::size_of_file_in_bytes(differences_filename)==0)
      {
         cout << basename1 << " and " << basename2
              << " are identical" << endl;

         unix_cmd="mv "+filename2+" "+text_duplicates_subdir;
         sysfunc::unix_command(unix_cmd);

         string duplicate_html_filename=
            html_subdir+filefunc::replace_suffix(basename2,"html");
         unix_cmd="mv "+duplicate_html_filename+" "+html_duplicates_subdir;
         sysfunc::unix_command(unix_cmd);

         string duplicate_jpg_filename=
            jpg_subdir+filefunc::replace_suffix(basename2,"jpg");
         unix_cmd="mv "+duplicate_jpg_filename+" "+jpg_duplicates_subdir;
         sysfunc::unix_command(unix_cmd);
         
         skip_next_flag=true;

//         outputfunc::enter_continue_char();
      }
      else
      {
//         cout << filename1 << " and " << filename2
//              << " differ" << endl;
      }

   } // loop over index i labeling input text files

}

