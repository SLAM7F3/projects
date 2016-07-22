// ==========================================================================
// Program RELABEL_NON_CHIPS

//			   relabel_non_chips

// ==========================================================================
// Last updated on 2/2/16
// ==========================================================================

#include  <algorithm>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;


// ==========================================================================

int main(int argc, char* argv[])
{
   string input_subdir="/home/pcho/Desktop/cleaned_svhn/non/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_subdir);
   for(int i = 0; i < image_filenames.size(); i++)
   {
      string curr_basename=filefunc::getbasename(image_filenames[i]);
      string filename = curr_basename.substr(0,12);
      string incorrectly_labeled_chip_basename=filename;
      string correctly_labeled_chip_basename = "non_"+filename.substr(2,10);

      string unix_cmd = "mv "+incorrectly_labeled_chip_basename+" "+
         correctly_labeled_chip_basename;
      cout << unix_cmd << endl;
   }
   

}

