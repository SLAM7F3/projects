// ========================================================================
// Program GENERATE_IMAGE_SIZES first links
// bundler_IO_subdir/stable_frames to bundler_IO_subdir/images.  It
// then calls mains/photosynth/generate_peter_inputs .
// GENERATE_IMAGE_SIZES next exports an image_sizes.dat file to
// bundler_IO_subdir for a set of WISP panoramas.  Each panorama is
// assumed to have a fixed, specified size in pixels.

//			    ./generate_image_sizes

// ========================================================================
// Last updated on 3/25/13; 6/21/13; 7/14/13; 8/8/13; 8/12/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
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
int main( int argc, char** argv )
{
   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string MayFieldtest_subdir="DIME/May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_io_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";

   string bundler_subdir="/data/DIME/bundler/";
   string bundler_IO_subdir=bundler_subdir+bundler_io_subdir;
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// First create soft link from bundler_IO_subdir/stable_frames to
// bundler_IO_subdir/images:

   string unix_cmd="ln -s "+bundler_IO_subdir+"stable_frames/ "+
      bundler_IO_subdir+"images";
   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="~/programs/c++/svn/projects/src/mains/photosynth/generate_peter_inputs ";
   unix_cmd += bundler_io_subdir;
   sysfunc::unix_command(unix_cmd);

   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   filefunc::ReadInfile(image_list_filename);
   int n_images=filefunc::text_line.size();

   string output_filename=bundler_IO_subdir+"image_sizes.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int width=40000;
   int height=2200;
   for (int n=0; n<n_images; n++)
   {
      outstream << n << "  " << width << "  " << height << endl;
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}

