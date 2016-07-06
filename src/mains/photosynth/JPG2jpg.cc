// ==========================================================================
// Program JPG2jpg takes in some subdirectory which is assumed to
// contain a set of images with .JPG suffixes.  It generates an
// executable script which forms soft links to files with the same
// prefix but with .jpg suffixes.  We wrote this script in order to
// avoid headaches with Noah's Bundler codes which seem to want .jpg
// rather than .JPG input files.

//				  JPG2jpg				

// ==========================================================================
// Last updated 12/17/10; 10/25/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   string subdir;
   cout << "Enter subdirectory path:" << endl;
   cin >> subdir;
   filefunc::add_trailing_dir_slash(subdir);
   string jpg_subdir=subdir+"jpg/";

   vector<string> suffixes;
   suffixes.push_back("JPG");
   
   vector<string> JPG_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         suffixes,subdir);

   string output_filename="link_JPG_to_jpg";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int i=0; i<JPG_filenames.size(); i++)
   {
//      cout << i <<  " JPG_filename = " << JPG_filenames[i] << endl;
      string dirname=filefunc::getdirname(JPG_filenames[i]);
      string basename=filefunc::getbasename(JPG_filenames[i]);
      string prefix=stringfunc::prefix(basename);
      string jpg_filename=jpg_subdir+prefix+".jpg";

//      string prefix=stringfunc::prefix(JPG_filenames[i]);
//      string jpg_filename=prefix+".jpg";
      string curr_ln_cmd="ln -s "+JPG_filenames[i]+"  "+jpg_filename;
      outstream << curr_ln_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   
   string unix_cmd="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Wrote soft link script to "+output_filename;
   outputfunc::write_big_banner(banner);
}

