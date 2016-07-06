// ==========================================================================
// Program HARVEST_NONDIGITS
// ==========================================================================
// Last updated on 2/2/16
// ==========================================================================

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/mypolynomial.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::set;
using std::string;
using std::vector;

// --------------------------------------------------------------------------
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string nonchars_subdir="./training_data/partial_digits_as_nonchars/";
   string pruned_digits_subdir = nonchars_subdir+"pruned_partial_digits/";
   string all_extended_chips_subdir = nonchars_subdir+"all_extended_chips/";
   string pruned_extended_chips_subdir = nonchars_subdir+
      "pruned_extended_chips/";
   filefunc::dircreate(pruned_extended_chips_subdir);
   
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      pruned_digits_subdir);
   for(int i = 0; i < image_filenames.size(); i++)
   {
      string curr_basename=filefunc::getbasename(image_filenames[i]);
      string extended_basename=stringfunc::prefix(curr_basename)+
         "_256x256_Feb2.jpg";
      cout << "i = " << i 
           << " curr_basename = " << curr_basename 
           << " extended_basename = " << extended_basename 
           << endl;
      string image_filename=pruned_digits_subdir+curr_basename;
      string new_extended_filename=pruned_extended_chips_subdir+
         extended_basename;
      string unix_cmd="cp "+image_filename+" "+new_extended_filename;
      sysfunc::unix_command(unix_cmd);
   }

}

