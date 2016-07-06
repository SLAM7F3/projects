// ==========================================================================
// Program GENERATE_FILE_LIST imports all image files from a specified
// folder.  It exports a text file containing their full paths which is
// needed as input to caffe's feature extractor binary.

// 			./generate_file_list

// ==========================================================================
// Last updated on 11/20/15; 11/28/15; 2/12/16
// ==========================================================================

#include <stdint.h>
#include <byteswap.h>
#include <iostream>
#include <string>
#include <vector>

#include "math/constants.h"
#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"


int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::string;
   using std::vector;

   string output_filename="./file_list.txt";
   ofstream output_stream;
   filefunc::openfile(output_filename, output_stream);

   string bundler_subdir = 
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler";
//   string caffe_subdir = 
//      "/home/cho/programs/c++/svn/projects/src/mains/machine_learning/caffe";
//   string images_subdir = bundler_subdir+"/NewsWrap_tsne/images/";
//   string images_subdir = bundler_subdir+
//      "/NewsWrap_tsne/images/thumbnails_256/";
   string images_subdir = bundler_subdir+"/roadsigns2/images/thumbnails/";
   //   string images_subdir = bundler_subdir+"/tidmarsh_deep/images/thumbnails/";
//   string images_subdir = caffe_subdir+"/images/";

   cout << "Enter full path to subdirectory containing input imagery thumbnails:" << endl;
   cin >> images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);

   vector<string> image_filenames = filefunc::image_files_in_subdir(
      images_subdir);

   for(unsigned int i = 0; i < image_filenames.size(); i++)
   {
      output_stream << image_filenames[i] << "  0" << endl;
   }

   filefunc::closefile(output_filename,output_stream);

   string banner="Exported image paths to "+output_filename;
   outputfunc::write_banner(banner);
   banner="Move "+output_filename+" to an appropriate subdir of ~/software/caffe_public/examples/_temp/";
   outputfunc::write_banner(banner);
}

