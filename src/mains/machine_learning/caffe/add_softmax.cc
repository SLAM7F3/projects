// ========================================================================
// Program ADD_SOFTMAX is a highly specialized utility which inserts
// male/female/non-face softmax classification probabilities into
// cropped screenshots of Michael Yee's graph viewer displaying
// network nodes colored according to their activations for a
// particular test image.  We wrote ADD_SOFTMAX primarily for movie
// generation purposes.

//                         ./add_softmax

// ========================================================================
// Last updated on 9/6/16; 9/8/16
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
   string network_subdir = "./vis_facenet/network/";
   string activations_subdir = network_subdir + "activations/";
   string image_activations_subdir = activations_subdir + "images/";

//   int class_index = 0; // male face
   int class_index = 1; // female face
//   int class_index = 2; // non-face

   cout <<  "Enter class index : 0 = male face, 1 = female face, 2 = non-face"
        << endl;
   cin >> class_index;

   string caffe_subdir="/home/pcho/programs/c++/git/projects/src/mains/machine_learning/caffe/";

   string composites_subdir;
   if(class_index == 0)
   {
      composites_subdir = caffe_subdir + 
         "/screen_shots/2016/Sep/Sep5/male/composites/";
   }
   else if (class_index == 1)
   {
      composites_subdir = caffe_subdir + 
         "/screen_shots/2016/Sep/Sep5/female/composites/";
   }

   string annotated_composites_subdir = composites_subdir+"annnotated/";
   filefunc::dircreate(annotated_composites_subdir);
   
   vector<string> composites_filenames = filefunc::image_files_in_subdir(
      composites_subdir);
   for(unsigned int f = 0; f < composites_filenames.size(); f++)
   {
      string composite_basename=filefunc::getbasename(composites_filenames[f]);
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         composite_basename, "_.");
      string image_index_str = substrings[1];
      string image_activation_filename = image_activations_subdir+
         "image_activations_"+image_index_str+".dat";
      vector< vector<double> > row_numbers = filefunc::ReadInRowNumbers(
         image_activation_filename);
      int n_lines = row_numbers.size();

      double non_face_prob = row_numbers[n_lines - 3].at(1);
      double male_face_prob = row_numbers[n_lines - 2].at(1);
      double female_face_prob = row_numbers[n_lines - 1].at(1);

      cout << "f = " << f
           << " composite_filename = " 
           << filefunc::getbasename(composites_filenames[f]) 
           << " male_prob = " << male_face_prob
           << " female_prob = " << female_face_prob
           << " nonface_prob = " << non_face_prob
           << endl;

      texture_rectangle *tr_ptr = new texture_rectangle(
         composites_filenames[f], NULL);
      texture_rectangle *text_tr_ptr = new texture_rectangle(
         composites_filenames[f], NULL);
      int fontsize = 25;

      double gender_prob;
      string curr_textline;
      vector<twovector> xy_start;
      if(class_index == 0)
      {
         curr_textline = "Male face prob = ";
         xy_start.push_back(twovector( 450, 20));
         gender_prob = male_face_prob;
      }
      else if (class_index == 1)
      {
         curr_textline = "Female face prob = ";
         xy_start.push_back(twovector( 440, 20)); 
         gender_prob = female_face_prob;

      }
      curr_textline += stringfunc::number_to_string(gender_prob,3);

      vector<string> text_lines;
      text_lines.push_back(curr_textline);

      vector<colorfunc::Color> text_colors;
      text_colors.push_back(colorfunc::white);

      videofunc::annotate_image_with_text(
         tr_ptr, text_tr_ptr, fontsize,
         text_lines, xy_start, text_colors);

// For movie generation purposes, we want annotated composite subnet
// images to be ordered so that those with the highest gender
// classifications are lexicographically first:

      string annotated_composite_basename = 
         annotated_composites_subdir+
         "annotated_composite_"+
         stringfunc::number_to_string(1-gender_prob,3)+"_"+
         image_index_str+".png";
      text_tr_ptr->write_curr_frame(
         annotated_composite_basename);

      string banner="Exported "+annotated_composite_basename;
      outputfunc::write_banner(banner);
      
      delete tr_ptr;
      delete text_tr_ptr;

   }// loop over index f labeling input composite image filenames
}

   
