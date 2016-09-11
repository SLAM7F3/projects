// ==========================================================================
// Program TRAINING_PERFORMANCE imports a caffe.bin.INFO file generated
// by caffe finetuning run on a GPU machine.  It plots the loss
// function as a function of training epoch number for the training
// set.  It also plots the accuracy value as a function of training
// epoch number for the training and validation sets.

//			   training_performance

// ==========================================================================
// Last updated on 8/15/16; 8/16/16; 8/28/16; 9/11/16
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
#include "filter/filterfuncs.h"
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

   int n_training_images_per_epoch = 386925;
//   int n_training_images_per_epoch = 359992;
//   int n_training_images_per_epoch = 260788;  
//    int n_training_images_per_epoch = 184440;  

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
      "Sep10_2r_T3/";
//      "Sep10_2e_T1/";
//       "Sep8_2r_T1/";
//      "Sep8_2q_T3/";
//       "Sep8_2e_T1/";
//       "Sep7_2e_T3/";
//       "Sep7_2e_T1/";


   string log_filename=dated_subdir+"caffe.bin.INFO";
   bool strip_comments_flag = false;
   filefunc::ReadInfile(log_filename, strip_comments_flag);

   string caffe_model_name="caffe";
   string solver_type="SGD";
   int batch_size = -1;
   double curr_epoch, base_learning_rate = -1;
   double weight_decay;
   double min_time_in_secs = 9.9E32;
   double max_time_in_secs = -min_time_in_secs;
   vector<double> training_epoch;
   vector<double> validation_iter, validation_epoch;
   vector<double> training_accuracy, smoothed_training_accuracy;
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
      else if(substrings[0] == "weight_decay:"){
         weight_decay = stringfunc::string_to_number(substrings[1]);
         cout << "weight_decay = " << weight_decay << endl;
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
         string timestamp = substrings[1];
         vector<string> substrings = 
            stringfunc::decompose_string_into_substrings(timestamp, ":");
         int hours = stringfunc::string_to_number(substrings[0]);
         int minutes = stringfunc::string_to_number(substrings[1]);
         double seconds = stringfunc::string_to_number(substrings[2]);
         double curr_time_in_secs = hours * 3600 + minutes * 60 + seconds;
         min_time_in_secs = basic_math::min(
            min_time_in_secs, curr_time_in_secs);
         max_time_in_secs = basic_math::max(
            max_time_in_secs, curr_time_in_secs);
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

   double elapsed_time_in_secs = max_time_in_secs - min_time_in_secs;
   double frac_day = elapsed_time_in_secs / (3600*24);
   int hours, minutes;
   double secs;
   timefunc::frac_day_to_hms(frac_day, hours, minutes, secs);
   string training_time_str = " Training time: "+
      stringfunc::number_to_string(hours)+" hrs "+
      stringfunc::number_to_string(minutes)+" mins ";
   cout << training_time_str << endl;

// Temporally smooth noisy training_accuracy values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx);
   vector<double> h;
   h.reserve(gaussian_size);
   filterfunc::gaussian_filter(dx, sigma, h);

   bool wrap_around_input_values = false;
   filterfunc::brute_force_filter(
      training_accuracy, h, smoothed_training_accuracy, 
      wrap_around_input_values);

   prob_distribution loss_prob(loss,50);
   double almost_max_loss = loss_prob.find_x_corresponding_to_pcum(0.995);

// Plot training loss versus epoch number

   metafile loss_metafile;
   loss_metafile.set_thickness(2);
   loss_metafile.set_legend_flag(true);

//   string learning_curves_subdir="./learning_curves/";
//   filefunc::dircreate(learning_curves_subdir);
   string learning_curves_subdir = dated_subdir;
   string lr_str = "_lr_"+stringfunc::number_to_string(base_learning_rate);
   string wd_str = "_wd_"+stringfunc::number_to_string(weight_decay);
   string training_imgs_str = "_"+stringfunc::number_to_string(n_training_images/1000)+"K_training_imgs";
   string meta_filename=learning_curves_subdir+"loss_function"+training_imgs_str+lr_str+"_"+solver_type;

   string horizontal_label="Training-from-scratch epoch";
   if(caffe_model_name != "caffe")
   {
      horizontal_label = "Fine tuning epoch from pre-trained "+
         caffe_model_name+" model";
   }
   
   loss_metafile.set_parameters(
      meta_filename,"Loss vs model training",
      horizontal_label, "Loss function value",
      0, training_epoch.back(), 0.0, almost_max_loss, 0.2, 0.1);

   string n_images_str = stringfunc::number_to_string(n_training_images)
      +" training images";
   string base_learning_rate_str = "Base learning rate="
      +stringfunc::number_to_string(base_learning_rate);
   string weight_decay_str = "Weight decay="
      +stringfunc::number_to_string(weight_decay);
   string batch_size_str = "Batch size="+stringfunc::number_to_string(
      batch_size);
   string solver_str = solver_type+" solver";
   string subtitle = training_time_str + "; "+base_learning_rate_str+"; "
      +weight_decay_str+"; "+batch_size_str;
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
      meta_filename,"Model accuracy vs model training",
      horizontal_label, "Model accuracy",
      0, training_epoch.back(), 0.0, 1.0, 0.2, 0.1);

   n_images_str = stringfunc::number_to_string(n_validation_images)+
      " validation images";
   subtitle = training_time_str + "; "+base_learning_rate_str+"; "
      +weight_decay_str+"; "+batch_size_str;
   accuracy_metafile.set_subtitle(subtitle);

   accuracy_metafile.openmetafile();
   accuracy_metafile.write_header();

   accuracy_metafile.set_legendlabel("Training accuracy");
   accuracy_metafile.write_curve(training_epoch, training_accuracy, 
                                   colorfunc::blue);
   accuracy_metafile.set_legendlabel("Smoothed training accuracy");
   accuracy_metafile.write_curve(training_epoch, smoothed_training_accuracy, 
                                   colorfunc::cyan);
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

