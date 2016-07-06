// ==========================================================================
// Program SMALLVIEW
// ==========================================================================
// Last updated on 8/25/13; 10/6/13
// ==========================================================================

#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   string image_filename;
   vector<string> param;
   if (filefunc::parameter_input(argc,argv,param))
   {
      image_filename=param[0];
   }
   else
   {
      exit(-1);
   }
   string basename=filefunc::getbasename(image_filename);

   string tmp_subdir="/tmp/";
   string file_suffix="jpg";
   filefunc::purge_files_with_suffix_in_subdir(tmp_subdir,file_suffix);

   string downsized_image_filename=tmp_subdir+basename;
   int max_xdim=500;
   int max_ydim=500;

   videofunc::downsize_image(
      image_filename,max_xdim,max_ydim,downsized_image_filename);

   string unix_cmd="display "+downsized_image_filename+" &";
   sysfunc::unix_command(unix_cmd);
}


