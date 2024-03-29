// ========================================================================
// Program ADD_SOFTMAX is a highly specialized utility which inserts
// male/female/non-face softmax classification probabilities into
// cropped screenshots of Michael Yee's graph viewer displaying
// network nodes colored according to their activations for a
// particular test image.  We wrote ADD_SOFTMAX primarily for movie
// generation purposes.

//                         ./add_softmax

// ========================================================================
// Last updated on 9/6/16; 9/8/16; 9/15/16; 9/17/16
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
   string facenet_model_label;
   cout << "Enter facenet model label: (e.g. 2e, 2n, 2r, 2t)" << endl;
   cin >> facenet_model_label;

   string network_subdir = "./vis_facenet/network/";
   string base_activations_subdir = network_subdir + "activations/";
   string activations_subdir = base_activations_subdir + "model_"+
      facenet_model_label+"/";
   string image_activations_subdir = activations_subdir + "images/";

//   int class_index = 0; // male face
   int class_index = 1; // female face
//   int class_index = 2; // non-face

   cout <<  "Enter class index : 0 = male face, 1 = female face, 2 = non-face, 3 = face_vs_nonface"
        << endl;
   cin >> class_index;

   string caffe_subdir="/home/pcho/programs/c++/git/projects/src/mains/machine_learning/caffe/";

   string composites_subdir;
   if(class_index == 0)
   {
      composites_subdir = caffe_subdir + 
         "/screen_shots/2016/Sep/Sep16/male/composited_images/";
   }
   else if (class_index == 1)
   {
      composites_subdir = caffe_subdir + 
         "/screen_shots/2016/Sep/Sep16/female/composited_images/";
   }
   else if (class_index == 2)
   {
      composites_subdir = caffe_subdir + 
         "/screen_shots/2016/Sep/Sep16/nonface/composited_images/";
   }

   string annotated_composites_subdir = composites_subdir+"annotated/";
   filefunc::dircreate(annotated_composites_subdir);
   string face_vs_nonface_subdir = composites_subdir+"face_vs_nonface/";
   filefunc::dircreate(face_vs_nonface_subdir);
   
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
      double face_prob = male_face_prob + female_face_prob;

      cout << "f = " << f
           << " activation_filename = "
           << image_activation_filename 
           << " composite_filename = " 
           << filefunc::getbasename(composites_filenames[f])  << endl;
      cout << "   male_prob = " << male_face_prob
           << " female_prob = " << female_face_prob
           << " nonface_prob = " << non_face_prob
           << endl;

      texture_rectangle *tr_ptr = new texture_rectangle(
         composites_filenames[f], NULL);
      texture_rectangle *text_tr_ptr = new texture_rectangle(
         composites_filenames[f], NULL);
      int fontsize = 25;

      double prob;
      string curr_textline;
      vector<twovector> xy_start;

      if(class_index == 0)
      {
         curr_textline = "Male face prob = ";
         xy_start.push_back(twovector( 450, 20));
         prob = male_face_prob;
         curr_textline += stringfunc::number_to_string(prob,5);
      }
      else if (class_index == 1)
      {
         curr_textline = "Female face prob = ";
         xy_start.push_back(twovector( 440, 20)); 
         prob = female_face_prob;
         curr_textline += stringfunc::number_to_string(prob,5);
      }
      else if (class_index == 2)
      {
         curr_textline = "Non face prob = ";
         xy_start.push_back(twovector( 440, 20)); 
         prob = non_face_prob;
         curr_textline += stringfunc::number_to_string(prob,5);
      }

      curr_textline = "Face prob = ";
      xy_start.push_back(twovector( 440, 20)); 
      curr_textline += stringfunc::number_to_string(face_prob,5);
      
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

/*
      string annotated_composite_basename = 
         annotated_composites_subdir+
         "annotated_composite_"+
         stringfunc::number_to_string(1-prob,6)+"_"+
         image_index_str+".png";
*/

      string annotated_composite_basename = 
         face_vs_nonface_subdir+
         "face_vs_nonface_"+
         stringfunc::number_to_string(1-face_prob,6)+"_"+
         image_index_str+".png";


      text_tr_ptr->write_curr_frame(
         annotated_composite_basename);

      string banner="Exported "+annotated_composite_basename;
      outputfunc::write_banner(banner);
      
      delete tr_ptr;
      delete text_tr_ptr;

   }// loop over index f labeling input composite image filenames
}

   
