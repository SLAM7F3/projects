// ========================================================================
// Program MONTAGE_MOVIEFRAMES is a specialized utility which we wrote
// in order to generate montages of annotated female and male
// activation displays for movie making purposes.

//                         ./montage_movieframes

// ========================================================================
// Last updated on 9/17/16
// ========================================================================

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv) 
{
   string caffe_subdir="/home/pcho/programs/c++/git/projects/src/mains/machine_learning/caffe/";

   string Sep14_subdir = caffe_subdir + "screen_shots/2016/Sep/Sep14/";
   string female_annotated_subdir = Sep14_subdir +
      "female/composited_images/annotated/";
   string male_annotated_subdir = Sep14_subdir + 
      "male/composited_images/annotated/";
   string montages_subdir = Sep14_subdir + "montages/";
   filefunc::dircreate(montages_subdir);

   vector<string> female_filenames = filefunc::image_files_in_subdir(
      female_annotated_subdir);
   vector<string> male_filenames = filefunc::image_files_in_subdir(
      male_annotated_subdir);

   for(unsigned int f = 0; f < female_filenames.size(); f++)
   {
      string montage_filename=montages_subdir + 
         "montage_"+stringfunc::integer_to_string(f,3)+".png";
      string unix_cmd = "montage -geometry 900x1085 "+female_filenames[f]+" "+
         male_filenames[f]+" "+montage_filename;
      sysfunc::unix_command(unix_cmd);
      cout << "Exported "+montage_filename << endl;
      
   }// loop over index f labeling input female and male filenames
}

   
