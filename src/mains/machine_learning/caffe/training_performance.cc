// ==========================================================================
// Program TRAINING_PERFORMANCE imports a caffe.bin.INFO file generated
// by caffe finetuning run on a GPU machine.  It plots the loss
// function as a function of training epoch number for the training
// set.  It also plots the accuracy value as a function of training
// epoch number for the training and validation sets.

//			   training_performance

// ==========================================================================
// Last updated on 7/24/16; 7/25/16; 7/26/16; 7/28/16
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
#include "plot/metafile.h"
#include "math/mypolynomial.h"
#include "math/prob_distribution.h"
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
   cout.precision(8);

   int n_training_images_per_epoch = 41821;  
                      // O(43K) unaugmented face gender images 
//   cout << "Enter number of training images per epoch (e.g. 2000 for mini, 20000 for full)"
//	<< endl;
//   cin >> n_images_per_epoch;

// "Batch" size for training data specified within within TRAIN_BATCH
// variable in our network dated run script:

   int n_training_images_per_iteration = 100;	
   double n_iters_per_epoch = 
      n_training_images_per_epoch / n_training_images_per_iteration; 

   double validation_frac = 0.1; //  10% of all labeled images are reserved for validation
   int n_training_images = n_training_images_per_epoch;
   int n_validation_images = validation_frac / (1 - validation_frac) * n_training_images;

   cout << "n_training_images_per_epoch = " 
        << n_training_images_per_epoch << endl;
   cout << "n_training_images_per_iteration = " 
        << n_training_images_per_iteration << endl;
   cout << "n_iterations_per_epoch = " << n_iters_per_epoch << endl;

   string faces_data_subdir = "/data/caffe/faces/";
   string trained_models_subdir = faces_data_subdir+"trained_models/";

   string dated_subdir = trained_models_subdir+
      "Jul27_41K_face_7layers_80-95dropout/";
//       "Jul27_41K_face_7layers_50dropout_0.01br_T3/";
//      "Jul27_41K_face_7layers_66dropout_T1/";
//      "Jul27_41K_face_7layers_T1/";
//      "Jul26_43K_face_13layers_T1/";
//      "Jul26_43K_face_7layers_fc512nodes_T1/";
//   string dated_subdir = trained_models_subdir+"Jul25_43K_face_7layers_T1/";
//   string dated_subdir = trained_models_subdir+"Jul25_43K_face_6layers_T3/";
//   string dated_subdir = trained_models_subdir+"Jul24_174K_augmented_vgg/";
//   string dated_subdir = trained_models_subdir+"Jul22_43K_vgg/";

   string log_filename=dated_subdir+"caffe.bin.INFO";
   bool strip_comments_flag = false;
   filefunc::ReadInfile(log_filename, strip_comments_flag);

   string caffe_model_name="caffe";
   string solver_type="SGD";
   int batch_size = -1;
   double curr_epoch, base_learning_rate = -1;
   vector<double> training_epoch;
   vector<double> validation_iter, validation_epoch;
   vector<double> training_accuracy;
   vector<double> validation_accuracy;
   vector<double> loss;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      if(substrings[0] == "name:"){
         string model_name = substrings[1].substr(1,substrings[1].size()-2);
         cout << "model_name = " << model_name << endl;
         if(model_name=="VGG_ILSVRC_16_layers")
         {
            caffe_model_name = "VGG-16";
         }
         else if (model_name=="CaffeNet")
         {
            caffe_model_name = "AlexNet";
         }
      }

      if(substrings[0] == "batch_size:" && batch_size < 0)
      {
         batch_size = stringfunc::string_to_number(substrings[1]);
      }

      if(substrings[0] == "base_lr:"){
         base_learning_rate = stringfunc::string_to_number(substrings[1]);
         cout << "base_learning_rate = " << base_learning_rate << endl;
      }
      else if(substrings[0] == "solver_type:"){
         solver_type = substrings[1];
         cout << "solver_type = " << solver_type << endl;
      }

      if(substrings.size() < 8) continue;

      if(substrings[4] == "Iteration" && substrings[6] == "loss" 
         && substrings[7] == "=")
      {
         int curr_iteration_number = stringfunc::string_to_number(
            substrings[5]);
         curr_epoch = curr_iteration_number / n_iters_per_epoch;
         training_epoch.push_back(curr_epoch);
         double curr_loss = stringfunc::string_to_number(substrings[8]);
         loss.push_back(curr_loss);
      }
      else if (substrings[4] == "Train" && substrings[5] == "net" 
               && substrings[6] == "output" && substrings[8] == "accuracy")
      {
         double curr_accuracy = stringfunc::string_to_number(substrings[10]);
         if(substrings[7] == "#0:")
         {
            training_accuracy.push_back(curr_accuracy);
         }
      }
      else if (substrings[4] == "Test" && substrings[5] == "net" 
               && substrings[6] == "output" && substrings[8] == "accuracy")
      {
         double curr_accuracy = stringfunc::string_to_number(substrings[10]);
         if(substrings[7] == "#0:")
         {
            validation_accuracy.push_back(curr_accuracy);
            validation_epoch.push_back(curr_epoch);
         }
      }

   } // loop over index i labeling imported logfile text lines

   cout << "training_epoch.size = " << training_epoch.size() << endl;
   cout << "training_accuracy.size = " << training_accuracy.size() << endl;
   cout << "validation_epoch.size = " << validation_epoch.size() << endl;
   cout << "validation_accuracy.size = " << validation_accuracy.size() 
        << endl;
   cout << "loss.sizes = " << loss.size() << endl;

   prob_distribution loss_prob(loss,50);
   double almost_max_loss = loss_prob.find_x_corresponding_to_pcum(0.995);

// Plot training loss versus epoch number

   metafile loss_metafile;
   loss_metafile.set_thickness(2);
   loss_metafile.set_legend_flag(true);

   string learning_curves_subdir="./learning_curves/";
   filefunc::dircreate(learning_curves_subdir);
   string lr_str = "_lr_"+stringfunc::number_to_string(base_learning_rate);
   string training_imgs_str = "_"+stringfunc::number_to_string(n_training_images/1000)+"K_training_imgs";
   string meta_filename=learning_curves_subdir+"loss_function"+training_imgs_str+lr_str+"_"+solver_type;

   loss_metafile.set_parameters(
      meta_filename,"Fine-tuning loss vs model training",
      "Fine tuning epoch from pre-trained "+caffe_model_name+" model",
      "Training metrics",
      0, training_epoch.back(), 0.0, almost_max_loss, 0.2, 0.1);

   string n_images_str = stringfunc::number_to_string(n_training_images)
      +" training images";
   string base_learning_rate_str = "Base learning rate="
      +stringfunc::number_to_string(base_learning_rate);
   string batch_size_str = "Batch size="+stringfunc::number_to_string(
      batch_size);
   string solver_str = solver_type+" solver";
   string subtitle = n_images_str + "; "+base_learning_rate_str+"; "
      +solver_str+"; "+batch_size_str;
   loss_metafile.set_subtitle(subtitle);
   
   loss_metafile.openmetafile();
   loss_metafile.write_header();

   loss_metafile.set_legendlabel("Loss");
   loss_metafile.write_curve(training_epoch, loss, colorfunc::orange);

   loss_metafile.set_legendlabel("");
   loss_metafile.closemetafile();

   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   banner="Exported "+meta_filename+".jpg";
   outputfunc::write_banner(banner);

// Plot training and validation accuracies versus epoch number:

   metafile accuracy_metafile;
   accuracy_metafile.set_thickness(2);
   accuracy_metafile.set_legend_flag(true);

   string validation_imgs_str;
   if(n_validation_images/1000 >= 1){
      validation_imgs_str = "_"+stringfunc::number_to_string(
         n_validation_images/1000)+"K_validation_imgs";
   }
   else{
      validation_imgs_str = "_"+stringfunc::number_to_string(
         n_validation_images)+"_validation_imgs";
   }
   meta_filename=learning_curves_subdir+"accuracy"+
      validation_imgs_str+lr_str+"_"+solver_type;

   accuracy_metafile.set_parameters(
      meta_filename,"Fine-tuning accuracy vs model training",
      "Fine tuning epoch from pre-trained "+caffe_model_name+" model",
      "Validation metrics",
      0, training_epoch.back(), 0.0, 1.0, 0.2, 0.1);

   n_images_str = stringfunc::number_to_string(n_validation_images)+
      " validation images";
   subtitle = n_images_str + "; "+base_learning_rate_str+"; "+solver_str
      +"; "+batch_size_str;
   accuracy_metafile.set_subtitle(subtitle);

   accuracy_metafile.openmetafile();
   accuracy_metafile.write_header();

   accuracy_metafile.set_legendlabel("Training accuracy");
   accuracy_metafile.write_curve(training_epoch, training_accuracy, 
                                   colorfunc::blue);
   accuracy_metafile.set_legendlabel("Validation accuracy");
   accuracy_metafile.write_curve(validation_epoch, validation_accuracy, 
                                   colorfunc::red);

   accuracy_metafile.set_legendlabel("");

   accuracy_metafile.closemetafile();

   banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   banner="Exported "+meta_filename+".jpg";
   outputfunc::write_banner(banner);

}

