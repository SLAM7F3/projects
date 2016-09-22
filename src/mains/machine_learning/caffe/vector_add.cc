// ========================================================================
// Program VECTOR_ADD is a playground for testing linear relationships
// among fc5 descriptors extracted from female/male test image chips.

// 				./vector_add

// ========================================================================
// Last updated on 9/22/16
// ========================================================================

#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ------------------------------------------------------------------
// First 6170 feature descriptors correspond to testing and validation
// face image chips

int main(int argc, char** argv) 
{
   string network_subdir = "./vis_facenet/network/";
   string base_activations_subdir = network_subdir + "activations/";
   string activations_subdir = base_activations_subdir + "model_2t/";
   string features_subdir = activations_subdir + "features/";
   string image_engine_subdir = "/data/ImageEngine/faces/";

   string feature_layer_name = "fc5";
//   cout << "Enter name of major layer (e.g. fc5, fc6) " << endl;
//   cout << "  for which image features should be exported:" << endl;
//   cin >> feature_layer_name;
   string image_features_subdir = features_subdir + feature_layer_name;
   filefunc::add_trailing_dir_slash(image_features_subdir);
// For fc5 layer of model 2t, eigenvalue/max_eigenvalue < 1E-3 for d >= 86
// For fc6 layer of model 2t, eigenvalue/max_eigenvalue < 1E-4 for d >= 24

   int d_max = 86; // fc5 layer
//   int d_max = 24; // fc6 layer

   ofstream lookup_stream;
   string lookup_filename = image_features_subdir+"features_vs_images.dat";
   filefunc::ReadInfile(lookup_filename);

   typedef std::map<string,  genvector* > FACE_FEATURES_MAP;
// independent string contains basename for test image chip
// dependent var contains pointer to genvector holding feature descriptor
   FACE_FEATURES_MAP face_features_map;
   FACE_FEATURES_MAP::iterator face_features_iter_f1, face_features_iter_f2,
      face_features_iter_m1, face_features_iter_m2;


   vector<string> image_basenames, feature_basenames;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      image_basenames.push_back(substrings[1]);
      feature_basenames.push_back(substrings[0]);
   }

   for(unsigned int i = 0; i < image_basenames.size(); i++)
   {
      string feature_filename = image_features_subdir+feature_basenames[i];
      filefunc::ReadInfile(feature_filename);
      
      genvector *curr_descriptor = new genvector(d_max);
      face_features_map[image_basenames[i]] = curr_descriptor;
      
      for(int j = 0; j < d_max; j++)
      {
         double curr_value = stringfunc::string_to_number(
            filefunc::text_line[j+2]);
         curr_descriptor->put(j, curr_value);
      }
   } // loop over index i labeling image basenames
   

   vector<string> left_male_faces, left_female_faces;
   vector<string> right_male_faces, right_female_faces;
   vector<string> forward_male_faces, forward_female_faces;


   left_female_faces.push_back(
      "female_face_20552_00_09251_96x96.jpg");
   left_female_faces.push_back(
      "female_face_21633_00_17247_96x96.jpg");
   left_female_faces.push_back(
      "female_face_11192_00_228730_96x96.jpg");
   left_female_faces.push_back(
      "female_face_09753_00_199144_96x96.jpg");

   forward_female_faces.push_back(
      "female_face_01756_00_40222_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_09321_00_190569_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_04020_00_111608_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_08880_00_178498_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_04913_00_136665_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_01695_00_39062_96x96.jpg");
   forward_female_faces.push_back(
      "female_face_05038_00_142108_96x96.jpg");
   
   right_female_faces.push_back(
      "female_face_10033_00_205292_96x96.jpg");
   right_female_faces.push_back(
      "female_face_10686_00_216595_96x96.jpg");
   right_female_faces.push_back(
      "female_face_04951_00_138187_96x96.jpg");
   right_female_faces.push_back(
      "female_face_05701_00_157577_96x96.jpg");
   

   left_male_faces.push_back(
      "male_face_01645_00_38074_96x96.jpg");
   left_male_faces.push_back(
      "male_face_02764_00_68905_96x96.jpg");
   left_male_faces.push_back(
      "male_face_09170_00_183706_96x96.jpg");
   left_male_faces.push_back(
      "male_face_04633_00_128076_96x96.jpg");

   forward_male_faces.push_back(
      "male_face_11397_00_233631_96x96.jpg");
   forward_male_faces.push_back(
      "male_face_11271_00_230493_96x96.jpg");
   forward_male_faces.push_back(
      "male_face_01456_00_27242_96x96.jpg");
   forward_male_faces.push_back(
      "male_face_22237_00_39926_96x96.jpg");
   forward_male_faces.push_back(
      "male_adience_face_16514_00_51198_96x96.jpg");
   forward_male_faces.push_back(
      "male_face_03114_00_77895_96x96.jpg");
   forward_male_faces.push_back(
      "male_face_04973_00_138891_96x96.jpg");

   right_male_faces.push_back(
      "male_face_04131_00_116286_96x96.jpg");
   right_male_faces.push_back(
      "male_face_01555_00_33810_96x96.jpg");
   right_male_faces.push_back(
      "male_face_01636_00_37818_96x96.jpg");
   right_male_faces.push_back(
      "male_face_04975_00_139062_96x96.jpg");
   right_male_faces.push_back(
      "male_face_03875_00_101231_96x96.jpg");
   right_male_faces.push_back(
      "male_face_10308_00_210188_96x96.jpg");

// Combine all female and male face test images into single STL
// vectors:

   vector<string> female_faces;
   for(unsigned int i = 0; i < left_female_faces.size(); i++)
   {
      female_faces.push_back(left_female_faces[i]);
   }
   for(unsigned int i = 0; i < forward_female_faces.size(); i++)
   {
      female_faces.push_back(forward_female_faces[i]);
   }
   for(unsigned int i = 0; i < right_female_faces.size(); i++)
   {
      female_faces.push_back(right_female_faces[i]);
   }
   
   vector<string> male_faces;
   for(unsigned int i = 0; i < left_male_faces.size(); i++)
   {
      male_faces.push_back(left_male_faces[i]);
   }
   for(unsigned int i = 0; i < forward_male_faces.size(); i++)
   {
      male_faces.push_back(forward_male_faces[i]);
   }
   for(unsigned int i = 0; i < right_male_faces.size(); i++)
   {
      male_faces.push_back(right_male_faces[i]);
   }

   cout << "face_features_map.size = " << face_features_map.size()
        << endl;

   genvector female_delta(d_max), male_delta(d_max), total_delta(d_max);
   vector<double> total_delta_mags;
   vector<string> f1_filenames, f2_filenames, m1_filenames, m2_filenames;

   for(unsigned int f1 = 0; f1 < female_faces.size(); f1++)
   {
      face_features_iter_f1 = face_features_map.find(female_faces[f1]);
      genvector *f1_descriptor = face_features_iter_f1->second;

      for(unsigned int f2 = f1+1; f2 < female_faces.size(); f2++)
      {
         face_features_iter_f2 = face_features_map.find(female_faces[f2]);
         genvector *f2_descriptor = face_features_iter_f2->second;
         female_delta = *f1_descriptor - *f2_descriptor;
         
         for(unsigned int m1 = 0; m1 < male_faces.size(); m1++)
         {
            face_features_iter_m1 = face_features_map.find(male_faces[m1]);
            genvector *m1_descriptor = face_features_iter_m1->second;

            for(unsigned int m2 = m1+1; m2 < male_faces.size(); m2++)
            {
               face_features_iter_m2 = face_features_map.find(male_faces[m2]);
               genvector *m2_descriptor = face_features_iter_m2->second;

               male_delta = *m1_descriptor - *m2_descriptor;
               double curr_score = (female_delta - male_delta).magnitude();
               total_delta_mags.push_back(curr_score);

               f1_filenames.push_back(face_features_iter_f1->first);
               f2_filenames.push_back(face_features_iter_f2->first);
               m1_filenames.push_back(face_features_iter_m1->first);
               m2_filenames.push_back(face_features_iter_m2->first);
            } // loop over index m2 
         } // loop over index m1 
      } // loop over index f2
   } // loop over index f1

   templatefunc::Quicksort(
      total_delta_mags, f1_filenames, f2_filenames, 
      m1_filenames, m2_filenames);

   cout << "total_delta_mags.size = " << total_delta_mags.size() << endl;
   cout << "f1_filenames.size = " << f1_filenames.size() << endl;
   cout << "f2_filenames.size = " << f2_filenames.size() << endl;
   cout << "m1_filenames.size = " << m1_filenames.size() << endl;
   cout << "m2_filenames.size = " << m2_filenames.size() << endl;
   
   for(unsigned int i = 0; i < total_delta_mags.size(); i++)
   {
      double curr_score = total_delta_mags[i];
      const double max_score = 0.65;
      if(curr_score > max_score) continue;

      string f1_pathname = image_engine_subdir+f1_filenames[i];
      string f2_pathname = image_engine_subdir+f2_filenames[i];
      string m1_pathname = image_engine_subdir+m1_filenames[i];
      string m2_pathname = image_engine_subdir+m2_filenames[i];
      

      cout << "i = " << i 
           << " total delta mag = " << total_delta_mags[i] << endl;
      cout << endl;

      string unix_cmd="montageview NO_DISPLAY "+f1_pathname+" "+
         f2_pathname+" "+m1_pathname+" "+m2_pathname;
      sysfunc::unix_command(unix_cmd);
   }
   


}
