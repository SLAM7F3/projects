// ========================================================================
// Program GENERATE_MOVIE_SCRIPTS

//                      ./generate_movie_scripts

// ========================================================================
// Last updated on 9/15/16
// ========================================================================

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "math/lttriple.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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
   string facenet_model_label;
   cout << "Enter facenet model label: (e.g. 2e, 2n, 2r)" << endl;
   cin >> facenet_model_label;

   string network_subdir = "./vis_facenet/network/";
   string base_activations_subdir = network_subdir + "activations/";
   string activations_subdir = base_activations_subdir + "model_"+
      facenet_model_label+"/";
   string image_activations_subdir = activations_subdir + "images/";
   string renorm_image_activations_subdir = image_activations_subdir 
      + "renormalized/";
   string node_activations_subdir = activations_subdir + "nodes/";

   string starting_subdir = 
      "/home/pcho/programs/c++/git/projects/src/mains/machine_learning/caffe/screen_shots/2016/Sep/Sep14/";
   string screen_shots_subdir = starting_subdir+"male/";
   string cropped_images_subdir=screen_shots_subdir+"cropped_images/";
   filefunc::dircreate(cropped_images_subdir);
   string composite_images_subdir=screen_shots_subdir+"composited_images/";
   filefunc::dircreate(composite_images_subdir);

   vector<string> activation_image_filenames = 
      filefunc::image_files_in_subdir(screen_shots_subdir);

   ofstream crop_stream, composite_stream;
   string crop_script_filename=screen_shots_subdir+"crop_images";
   string composite_script_filename=screen_shots_subdir+"composite_images";
   filefunc::openfile(crop_script_filename, crop_stream);
   filefunc::openfile(composite_script_filename, composite_stream);

   for(unsigned int f = 0; f < activation_image_filenames.size(); f++)
   {
      string curr_basename = filefunc::getbasename(
         activation_image_filenames[f]);
      string cropped_basename="cropped_"+curr_basename;
      cout << "f = " << f << " curr_basename = " << curr_basename << endl;
      string cropped_image_filename=cropped_images_subdir+
         cropped_basename;
      
      string unix_cmd = "convert "+curr_basename+
         " -crop 900x1085+660+45 "+cropped_image_filename;
      crop_stream << unix_cmd << endl;

      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_basename, "._");
      string testimg_index_str = substrings[1];

      string image_activations_filename=image_activations_subdir+
         "image_activations_"+testimg_index_str+".dat";
      filefunc::ReadInfile(image_activations_filename);
      string imagechip_filename = filefunc::text_line[0];
      cout << "image_activations_filename = " << image_activations_filename
           << endl;
      cout << "image chip filename = " << imagechip_filename << endl;
      cout << endl;

      string composite_image_filename=composite_images_subdir+
         "composite_"+testimg_index_str+".png";
      unix_cmd="composite -geometry  +575+25 "+imagechip_filename+
         " "+cropped_image_filename+" "+composite_image_filename;
      composite_stream << unix_cmd << endl;
   }
   filefunc::closefile(crop_script_filename, crop_stream);
   filefunc::make_executable(crop_script_filename);

   filefunc::closefile(composite_script_filename, composite_stream);
   filefunc::make_executable(composite_script_filename);

   string banner="Exported "+crop_script_filename;
   outputfunc::write_banner(banner);
   banner="Exported "+composite_script_filename;
   outputfunc::write_banner(banner);


}

   
