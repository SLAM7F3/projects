// ========================================================================
// Program PNG2JPG generates an executable script which calls
// ImageMagick's CONVERT in order to transform PNG files within a
// specified input directory to JPG format.

//				png2jpg

// ========================================================================
// Last updated on 1/19/12; 1/20/12
// ========================================================================

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
int main( int argc, char** argv )
{

   string pngs_subdir;
   cout << "Enter full path to subdirectory holding input PNG files:" 
        << endl;
   cin >> pngs_subdir;

   filefunc::add_trailing_dir_slash(pngs_subdir);
   string jpgs_subdir=pngs_subdir+"jpgs/";
   filefunc::dircreate(jpgs_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   vector<string> png_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,pngs_subdir);

   string script_filename="convert_png_2_jpg";
   ofstream outstream;
   filefunc::openfile(script_filename,outstream);

   for (int i=0; i<png_filenames.size(); i++)
   {
      string png_filename=png_filenames[i];
      string basename=filefunc::getbasename(png_filename);
      string prefix=stringfunc::prefix(basename);
      string jpg_filename=jpgs_subdir+prefix+".jpg";
      string unix_command="convert -verbose -quality 99 "+png_filename+" "
         +jpg_filename;
      outstream << unix_command << endl;
   }

   filefunc::closefile(script_filename,outstream);

   string unix_command_str="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command_str);
   
   string banner="Wrote script file "+script_filename;
   outputfunc::write_banner(banner);
}
