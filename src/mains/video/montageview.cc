// ==========================================================================
// Program MONTAGEVIEW imports some number of image filenames as
// command line arguments.  After downsizing each input image file, it
// concatenates them together and forms a single output image.  If the
// command line does not contain "NO_DISPLAY", MONTAGEVIEW pops open a
// window containing the concatenated montage image.
// ==========================================================================
// Last updated on 10/15/13; 12/29/13; 1/1/15; 4/25/16
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

   bool display_montage_flag=true;
   
   vector<string> params,image_filenames;
   if (filefunc::parameter_input(argc,argv,params))
   {
      for (unsigned int p=0; p<params.size(); p++)
      {
         if (params[p]=="NO_DISPLAY")
         {
            display_montage_flag=false;
         }
         else
         {
            image_filenames.push_back(params[p]);
         }
      }
   }
   else
   {
      exit(-1);
   }

//   int max_xdim=500;
//   int max_ydim=500;
//   int max_xdim=512;
//   int max_ydim=512;
//   int max_xdim=750;
//   int max_ydim=750;
   int max_xdim=1000;
   int max_ydim=1000;
//   int max_xdim=1250;
//   int max_ydim=1250;

   string montage_filename="montage_";
//    string unix_cmd="montage -geometry +4+4 -mode concatenate ";
   string unix_cmd="montage -geometry +10+6  ";
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      string basename=filefunc::getbasename(image_filenames[i]);
      string prefix=stringfunc::prefix(basename);
      montage_filename += prefix;
      if (i < image_filenames.size()-1)
      {
         montage_filename += "___";
      }

      string downsized_image_filename="/tmp/"+basename;
      videofunc::downsize_image(
         image_filenames[i],max_xdim,max_ydim,downsized_image_filename);

      unix_cmd += downsized_image_filename+" ";

   } // loop over index i labeling input image filenames
   montage_filename += ".jpg";
   unix_cmd += montage_filename;

//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

// Do NOT display montage if command line includes any string(s)
// beyond the names of the 2 input files:

   if (display_montage_flag)
   {
      unix_cmd="display "+montage_filename+" &";
      sysfunc::unix_command(unix_cmd);
   }
}


