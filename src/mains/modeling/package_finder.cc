// ==========================================================================
// Program PACKAGE_FINDER reads in a set of jpeg filenames from the
// current directory.  It then greps through all package files within
// the hardwired packages_subdir.  PACKAGE_FINDER locates the package
// file names which are associated with the input jpegs.  It generates
// an executable script for running program POINTMODELER which incorporates
// the package file names.

//				package_finder

// ==========================================================================
// Last updated on 1/23/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string mains_dir="/home/cho/programs/c++/svn/projects/src/mains/";
   string modeling_subdir=mains_dir+"modeling/";
   string packages_subdir=mains_dir+"photosynth/bundler/MIT2317/packages/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");

   string jpg_files_subdir="./";
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,jpg_files_subdir);

   string output_filename="run_pointmodeler";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << modeling_subdir+"pointmodeler \\" << endl;

   int n_jpg_filenames=jpg_filenames.size();
   string match_filename="./match.dat";
   for (int j=0; j<n_jpg_filenames; j++)
   {
//      cout << "j = " << j << " jpg_filenames[j] = " << jpg_filenames[j]
//           << endl;

      string unix_cmd="grep "+jpg_filenames[j]+" "+packages_subdir+"*.pkg ";
      unix_cmd += " > "+match_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      filefunc::ReadInfile(match_filename);
      filefunc::deletefile(match_filename);

      string separator_chars=":";
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[0],separator_chars);

      string package_filename=substrings[0];

      cout << "j = " << j 
           << " jpeg filename = " << jpg_filenames[j] << endl;
      cout << " package filename = " << package_filename
           << endl;
      cout << endl;

      outstream << "--region_filename "+packages_subdir+
         filefunc::getbasename(package_filename);
      if (j < n_jpg_filenames-1)
      {
         outstream << " \\";
      }
      outstream << endl;
         
   } // loop over index j labeling jpg filenames

   filefunc::closefile(output_filename,outstream);

// Make output script executable:

   string unix_cmd="chmod a+x "+output_filename;
   sysfunc::unix_command(unix_cmd);

/*
// Move output script:

   unix_cmd="mv "+output_filename+" "+modeling_subdir;
   sysfunc::unix_command(unix_cmd);
*/

}
