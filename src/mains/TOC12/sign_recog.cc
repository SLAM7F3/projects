// ==========================================================================
// Program SIGN_RECOG is a main program which takes in sets of input
// video frames and searches for all 9 TOC12 ppt signs within them.

//				sign_recog

// ==========================================================================
// Last updated on 10/5/12; 10/16/12; 10/29/12
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "image/compositefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/RGB_analyzer.h"
#include "classification/signrecogfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::map;
using std::pair;
using std::string;
using std::vector;


typedef struct 
{
      string symbol_name,sign_hue,bbox_color;
      vector<string> colors_to_find;
      double min_aspect_ratio,max_aspect_ratio;
      double min_compactness,max_compactness;
      int min_n_holes,max_n_holes;
      int min_n_crossings,max_n_crossings;
      int min_n_significant_holes,max_n_significant_holes;
      bool black_interior_flag,white_interior_flag,purple_interior_flag;
      double min_gradient_mag,max_bbox_hue_frac,Ng_threshold;

} SIGN_PROPERTIES;


int main(int argc, char* argv[])
{
   cout.precision(12);

   bool PointGrey_camera_flag=true;

   string final_signs_subdir="./images/final_signs/";
//   string ppt_signs_subdir="./images/ppt_signs/";
   string symbols_input_subdir=final_signs_subdir;
   string learned_funcs_subdir="./learned_functions/";

   string output_subdir=final_signs_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);

   texture_rectangle* raw_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* selected_colors_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* binary_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* edges_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* black_grads_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* white_grads_texture_rectangle_ptr=
      new texture_rectangle();

// Initialize TOC12 sign properties:
   
   SIGN_PROPERTIES curr_sign_properties,prev_sign_properties;
   vector<SIGN_PROPERTIES> sign_properties;

// Yellow radiation TOC12 ppt sign:

   curr_sign_properties.symbol_name="yellow_radiation";
   curr_sign_properties.sign_hue="yellow";
   curr_sign_properties.bbox_color="blue";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("yellow");
   curr_sign_properties.colors_to_find.push_back("darkyellow");
   curr_sign_properties.colors_to_find.push_back("lightyellow");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=2.2;
   curr_sign_properties.min_compactness=0.055;
   curr_sign_properties.max_compactness=0.115;
   curr_sign_properties.min_n_holes=3;
   curr_sign_properties.max_n_holes=6;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=6;
   curr_sign_properties.min_n_significant_holes=2;
   curr_sign_properties.max_n_significant_holes=6;
   curr_sign_properties.black_interior_flag=true;
   curr_sign_properties.white_interior_flag=false;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.5;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Orange biohazard sign:

   curr_sign_properties.symbol_name="orange_biohazard";
   curr_sign_properties.sign_hue="orange";
   curr_sign_properties.bbox_color="cyan";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("orange");
   curr_sign_properties.colors_to_find.push_back("red");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=3.9;
   curr_sign_properties.min_compactness=0.045;
   curr_sign_properties.max_compactness=0.19;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=10;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=9;
   curr_sign_properties.black_interior_flag=true;
   curr_sign_properties.white_interior_flag=false;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.5;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Blue-purple radiation sign:

   curr_sign_properties.symbol_name="blue_radiation";
   curr_sign_properties.sign_hue="blue";
   curr_sign_properties.bbox_color="orange";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("blue");
   curr_sign_properties.colors_to_find.push_back("darkblue");
   curr_sign_properties.colors_to_find.push_back("greyblue");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=2.2;
   curr_sign_properties.min_compactness=0.055;
   curr_sign_properties.max_compactness=0.16;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=7;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=6;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=5;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=false;
   curr_sign_properties.purple_interior_flag=true;
   curr_sign_properties.min_gradient_mag=0.5;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Blue water TOC12 ppt sign:

   curr_sign_properties.symbol_name="blue_water";
   curr_sign_properties.sign_hue="blue";
   curr_sign_properties.bbox_color="yellow";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("blue");
   curr_sign_properties.colors_to_find.push_back("darkblue");
   curr_sign_properties.colors_to_find.push_back("greyblue");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=2.2;
   curr_sign_properties.min_compactness=0.055;
   curr_sign_properties.max_compactness=0.16;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=6;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=6;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=4;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.5;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Blue gasoline sign:

   curr_sign_properties.symbol_name="blue_gas";
   curr_sign_properties.sign_hue="blue";	// PointGrey cameras
//   curr_sign_properties.sign_hue="blue2";
   curr_sign_properties.bbox_color="white";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("blue");
   curr_sign_properties.colors_to_find.push_back("darkblue");
   curr_sign_properties.colors_to_find.push_back("greyblue");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=2.2;
   curr_sign_properties.min_compactness=0.045;
   curr_sign_properties.max_compactness=0.16;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=5;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=3;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.5;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Red stop sign:

   curr_sign_properties.symbol_name="red_stop";
   curr_sign_properties.sign_hue="red";
   curr_sign_properties.bbox_color="green";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("red");
//   curr_sign_properties.colors_to_find.push_back("lightred");
   curr_sign_properties.min_aspect_ratio=0.4;
   curr_sign_properties.max_aspect_ratio=2;
   curr_sign_properties.min_compactness=0.045;
   curr_sign_properties.max_compactness=0.23;
   curr_sign_properties.min_n_holes=0;
   curr_sign_properties.max_n_holes=5;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=4;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.3;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Green start sign:

   curr_sign_properties.symbol_name="green_start";
   curr_sign_properties.sign_hue="green";
   curr_sign_properties.bbox_color="purple";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("green");
   curr_sign_properties.colors_to_find.push_back("lightgreen");
   curr_sign_properties.colors_to_find.push_back("darkgreen");
   curr_sign_properties.colors_to_find.push_back("greygreen");
   curr_sign_properties.colors_to_find.push_back("yellow");
   curr_sign_properties.colors_to_find.push_back("lightyellow");
   curr_sign_properties.min_aspect_ratio=0.27;
   curr_sign_properties.max_aspect_ratio=2;
   curr_sign_properties.min_compactness=0.045;
   curr_sign_properties.max_compactness=0.23;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=17;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=14;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.3;
   curr_sign_properties.Ng_threshold=0.5;
   sign_properties.push_back(curr_sign_properties);

// Black & white skull sign:

   curr_sign_properties.symbol_name="bw_skull";
   curr_sign_properties.sign_hue="black";
   curr_sign_properties.bbox_color="brick";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("black");
   curr_sign_properties.colors_to_find.push_back("darkgrey");
   curr_sign_properties.min_aspect_ratio=0.1;
   curr_sign_properties.max_aspect_ratio=4;
   curr_sign_properties.min_compactness=0.02;
   curr_sign_properties.max_compactness=0.56;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=5;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=5;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.75;
   curr_sign_properties.max_bbox_hue_frac=0.33;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Black & white eat sign:

   curr_sign_properties.symbol_name="bw_eat";
   curr_sign_properties.sign_hue="black";
   curr_sign_properties.bbox_color="gold";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("black");
   curr_sign_properties.colors_to_find.push_back("darkgrey");
   curr_sign_properties.min_aspect_ratio=0.1;
   curr_sign_properties.max_aspect_ratio=4;
   curr_sign_properties.min_compactness=0.02;
   curr_sign_properties.max_compactness=0.56;
   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=5;
   curr_sign_properties.min_n_crossings=2;
   curr_sign_properties.max_n_crossings=8;
   curr_sign_properties.min_n_significant_holes=1;
   curr_sign_properties.max_n_significant_holes=5;
   curr_sign_properties.black_interior_flag=false;
   curr_sign_properties.white_interior_flag=true;
   curr_sign_properties.purple_interior_flag=false;
   curr_sign_properties.min_gradient_mag=0.75;
   curr_sign_properties.max_bbox_hue_frac=0.33;
   curr_sign_properties.Ng_threshold=0.25;
   sign_properties.push_back(curr_sign_properties);

// Import quantized RGB lookup tables for all TOC12 signs:

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   for (int s=0; s<sign_properties.size(); s++)
   {
      RGB_analyzer_ptr->import_quantized_RGB_lookup_table(
         sign_properties[s].sign_hue);
   }

   cout << endl;
   cout << "------------------------------------------------------" << endl;
   cout << "Yellow radiation sign ID = 0" << endl;
   cout << "Orange biohazard sign ID = 1" << endl;
   cout << "Blue-purple radiation sign ID = 2" << endl;
   cout << "Blue water sign ID = 3" << endl;
   cout << "Blue gasoline sign ID = 4" << endl;
   cout << "Red stop sign ID = 5" << endl;
   cout << "Green start sign ID = 6" << endl;
   cout << "Black-white skull sign ID = 7" << endl;
   cout << "Black-white eat sign ID = 8" << endl;
   cout << "------------------------------------------------------" << endl;   

//   int sign_ID=2;
//   cout << "Enter sign number:" << endl;
//   cin >> sign_ID;
//   int sign_ID_start=sign_ID;
//   int sign_ID_stop=sign_ID;

   int sign_ID_start=0;
   int sign_ID_stop=sign_properties.size()-1;

// Import probabilistic decision functions generated by an SVM with a
// linear kernel on 12K symbol and 96K non-symbol images:

   const int K_Ng=1024;
   const int nineK=9*K_Ng;

   typedef dlib::matrix<double, nineK, 1> Ng_sample_type;
   typedef dlib::linear_kernel<Ng_sample_type> Ng_kernel_type;
   Ng_sample_type Ng_sample;

   typedef dlib::probabilistic_decision_function<Ng_kernel_type> 
      Ng_probabilistic_funct_type;  
   typedef dlib::normalized_function<Ng_probabilistic_funct_type> 
      Ng_pfunct_type;
   Ng_pfunct_type Ng_pfunct;
   vector<Ng_pfunct_type> Ng_pfuncts;

   bool RGB_pixels_flag=false;
//   bool RGB_pixels_flag=true;
   cout << "RGB_pixels_flag = " << RGB_pixels_flag << endl;

   int D=64*3;	// RGB color
   if (!RGB_pixels_flag)
   {
      D=64;	// greyscale
   }

   vector<text_detector*> text_detector_ptrs;
   for (int s=0; s<sign_properties.size(); s++)
   {
      string symbol_name=sign_properties[s].symbol_name;
      string learned_Ng_pfunct_filename=learned_funcs_subdir;
      learned_Ng_pfunct_filename += symbol_name+"/";
      learned_Ng_pfunct_filename += "symbols_Ng_pfunct_";
      learned_Ng_pfunct_filename += "12000_96000";
      learned_Ng_pfunct_filename += ".dat";
      cout << "learned_Ng_pfunct_filename = "
           << learned_Ng_pfunct_filename << endl;
      ifstream fin6(learned_Ng_pfunct_filename.c_str(),ios::binary);
      deserialize(Ng_pfunct, fin6);
      Ng_pfuncts.push_back(Ng_pfunct);

// Import dictionary trained on symbol and non-symbol images:

      string symbol_filename=symbols_input_subdir+symbol_name+".png";
      string synthetic_subdir=symbols_input_subdir+"synthetic_symbols/";
      string synthetic_symbols_subdir=synthetic_subdir+symbol_name+"/";
      string dictionary_subdir=synthetic_symbols_subdir;
      cout << "dictionary_subdir = " << dictionary_subdir << endl;

      text_detector* text_detector_ptr=new text_detector(
         D,K_Ng,dictionary_subdir);
      text_detector_ptrs.push_back(text_detector_ptr);
      text_detector_ptr->import_mean_and_covar_matrices();
      text_detector_ptr->import_dictionary();
   } // loop over index s labeling TOC12 symbols

// -----------------------------------------------

   twoDarray* xderiv_twoDarray_ptr=NULL;
   twoDarray* yderiv_twoDarray_ptr=NULL;
   twoDarray* gradient_mag_twoDarray_ptr=NULL;

   vector<pair<int,int> > black_pixels,white_pixels;

   int video_start=22;
   int video_stop=22;
   int delta_video=1;

//   cout << "Enter starting video pass number ( >= 5):" << endl;
//   cin >> video_start;
//   cout << "Enter stopping video pass number ( <= 20):" << endl;
//   cin >> video_stop;

   timefunc::initialize_timeofday_clock();

// Loop over multiple video clips starts here:

   int image_counter=0;
   for (int video_number=video_start; video_number <=video_stop; 
        video_number += delta_video)
   {
      string video_subdir=output_subdir+
         "vid_"+stringfunc::integer_to_string(video_number,2)+"/";
      filefunc::dircreate(video_subdir);

      string image_subdir=final_signs_subdir+"videos/vid";
      image_subdir += stringfunc::integer_to_string(video_number,2)+"_frames/";

      vector<string> image_filenames=filefunc::image_files_in_subdir(
         image_subdir);

      int i_start=0;
      int i_stop=image_filenames.size()-1;
//      cout << "Last image number = " << image_filenames.size() << endl;
//      cout << "Enter starting image number:" << endl;
//      cin >> i_start;
//      cout << "Enter stopping image number:" << endl;
//      cin >> i_stop;
//      i_start--;
//      i_stop--;

// Loop over individual video frames starts here:

      for (int i=i_start; i<=i_stop; i++)
      {
         string orig_image_filename=image_filenames[i];
         cout << "Original image_filename = " << orig_image_filename << endl;

         if (image_counter >= 1)
         {
            double total_time=timefunc::elapsed_timeofday_time();
            cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
                 << total_time / 60.0 << " minutes" << endl;
            double avg_time_per_image=
               timefunc::elapsed_timeofday_time()/image_counter;
            cout << "***********************************************" << endl;
            cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image 
                 << " secs" << " n_images = " << image_counter << endl;
            cout << "***********************************************" << endl;
         }
         
         int frame_number=i+1;
         image_counter++;

// Crop white border surrounding PointGrey images captured within
// Tennis Bubble on Saturday, Oct 27, 2012:

         string undistorted_image_filename=orig_image_filename;
         if (PointGrey_camera_flag)
         {
            string cropped_image_filename=signrecogfunc::crop_white_border(
               orig_image_filename);
            undistorted_image_filename=
               signrecogfunc::radially_undistort_PointGrey_image(
                  cropped_image_filename,raw_texture_rectangle_ptr,
                  texture_rectangle_ptr);

            string raw_filename=video_subdir+
               "raw_"+stringfunc::number_to_string(frame_number)+".jpg";
            raw_texture_rectangle_ptr->write_curr_frame(raw_filename);

            string undistorted_filename=video_subdir+
               "undistorted_"+stringfunc::number_to_string(frame_number)
               +".jpg";
            texture_rectangle_ptr->write_curr_frame(undistorted_filename);
         }

         vector<polygon> bbox_polygons;
         vector<int> bbox_color_indices;

         int orig_xdim,orig_ydim,xdim,ydim;
         string image_filename;
         imagefunc::get_image_width_height(
            undistorted_image_filename,orig_xdim,orig_ydim);
         if (orig_xdim >= 1000 || orig_ydim >= 600)
         {
            string dirname=filefunc::getdirname(undistorted_image_filename);
            string basename=filefunc::getbasename(undistorted_image_filename);

            string subdirname=dirname+"downsized/";
            filefunc::dircreate(subdirname);
            image_filename=subdirname+"downsized_"+basename;

            xdim=orig_xdim/2;
            ydim=orig_ydim/2;
            videofunc::resize_image(
               undistorted_image_filename,orig_xdim,orig_ydim,
               xdim,ydim,image_filename);
         }
         else
         {
            image_filename=undistorted_image_filename;
            xdim=orig_xdim;
            ydim=orig_ydim;
         }

         cout << "image_filename = " << image_filename << endl;

         texture_rectangle_ptr->reset_texture_content(image_filename);

         int n_Ng_rejections=0;
         int n_Ng_acceptances=0;
         int candidate_char_counter=0;

// Loop over TOC12 sign IDs starts here:

         for (int curr_sign_ID=sign_ID_start; curr_sign_ID <= sign_ID_stop;
              curr_sign_ID++)
         {
            curr_sign_properties=sign_properties[curr_sign_ID];   

            if (curr_sign_ID > sign_ID_start)
            {
               prev_sign_properties=sign_properties[curr_sign_ID-1];
            }
            else
            {
               prev_sign_properties=curr_sign_properties;
            }
            
            string lookup_map_name=curr_sign_properties.sign_hue;
            string symbol_name=curr_sign_properties.symbol_name;

            cout << "======================================================"
                 << endl;
            cout << "symbol = " << symbol_name << endl;

            quantized_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            if (selected_colors_texture_rectangle_ptr != NULL)
               selected_colors_texture_rectangle_ptr->reset_texture_content(
                  image_filename);
            binary_texture_rectangle_ptr->reset_texture_content(
               image_filename);

            edges_texture_rectangle_ptr->reset_texture_content(image_filename);

            if (black_grads_texture_rectangle_ptr != NULL)
               black_grads_texture_rectangle_ptr->reset_texture_content(
                  image_filename);
            if (white_grads_texture_rectangle_ptr != NULL)
               white_grads_texture_rectangle_ptr->reset_texture_content(
                  image_filename);

            RGB_analyzer_ptr->quantize_texture_rectangle_colors(
               lookup_map_name,quantized_texture_rectangle_ptr);

            int n_filter_iters=4;
            for (int filter_iter=0; filter_iter < n_filter_iters; 
                 filter_iter++)
            {
               RGB_analyzer_ptr->smooth_quantized_image(
                  texture_rectangle_ptr,quantized_texture_rectangle_ptr);
            }
            string quantized_filename=video_subdir+
              "quantized_"+stringfunc::number_to_string(frame_number)+".jpg";
            cout << "quantized_filename = " << quantized_filename << endl;
            quantized_texture_rectangle_ptr->write_curr_frame(
               quantized_filename);

            RGB_analyzer_ptr->isolate_quantized_colors(
               quantized_texture_rectangle_ptr,
               curr_sign_properties.colors_to_find,
               selected_colors_texture_rectangle_ptr,
               binary_texture_rectangle_ptr);
         
            string selected_colors_filename=video_subdir+
               "selected_colors_"+stringfunc::number_to_string(frame_number)+
               ".jpg";
            cout << "Selected colors filename = " << selected_colors_filename
                 << endl;
            selected_colors_texture_rectangle_ptr->write_curr_frame(
               selected_colors_filename);

            string binary_quantized_filename=video_subdir+
               "binary_colors_"+stringfunc::number_to_string(frame_number)
               +".jpg";
            cout << "binary_quantized_filename = "
                 << binary_quantized_filename << endl;
            binary_texture_rectangle_ptr->write_curr_frame(
               binary_quantized_filename);

// Compute edge map.  Require strong edge content within genuine TOC12
// signs:

            bool compute_gradients_flag=false;
            if (curr_sign_ID==sign_ID_start && 
            curr_sign_properties.black_interior_flag)
            {
               edges_texture_rectangle_ptr->
                  convert_color_image_to_greyscale(); 
               compute_gradients_flag=true;
            }

// Note added on 9/29/12: For blue-purple radiation sign, should very
// likely convert to greyscale using just red cannel and not use
// luminosity!

            if (prev_sign_properties.black_interior_flag &&
            !curr_sign_properties.black_interior_flag)
            {
               edges_texture_rectangle_ptr->
                  convert_color_image_to_luminosity();
               compute_gradients_flag=true;
            }

            string greyscale_filename=video_subdir+
               "greyscale_"+stringfunc::number_to_string(frame_number)+".jpg";
            edges_texture_rectangle_ptr->write_curr_frame(greyscale_filename);
            
            if (compute_gradients_flag)
            {
               twoDarray* ptwoDarray_ptr=edges_texture_rectangle_ptr->
                  get_ptwoDarray_ptr();
               ptwoDarray_ptr->set_deltax(1);
               ptwoDarray_ptr->set_deltay(1);
               
               if (xderiv_twoDarray_ptr==NULL || 
               xderiv_twoDarray_ptr->get_xdim() != xdim ||
               xderiv_twoDarray_ptr->get_ydim() != ydim)
               {
                  xderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
                  yderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
                  gradient_mag_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
               }

               const double spatial_resolution=0.25;
               imagefunc::compute_x_y_deriv_fields(
                  spatial_resolution,ptwoDarray_ptr,
                  xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
               imagefunc::compute_gradient_magnitude_field(
                  xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
                  gradient_mag_twoDarray_ptr);
               imagefunc::threshold_intensities_below_cutoff(
                  gradient_mag_twoDarray_ptr,
                  curr_sign_properties.min_gradient_mag,0);

               edges_texture_rectangle_ptr->initialize_twoDarray_image(
                  gradient_mag_twoDarray_ptr,3,false);
               string edges_filename=video_subdir+
                  "edges_"+stringfunc::number_to_string(frame_number)+".jpg";
               edges_texture_rectangle_ptr->write_curr_frame(edges_filename);
            } // compute_gradients_flag conditional


            double step_distance=-2;
            if (curr_sign_properties.white_interior_flag) step_distance=2;

            typedef map<int,int> GRADSTEP_MAP;
            GRADSTEP_MAP::iterator gradstep_iter;

            GRADSTEP_MAP black_gradstep_map=
               imagefunc::compute_gradient_steps(
                  curr_sign_properties.min_gradient_mag,step_distance,
                  xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
                  gradient_mag_twoDarray_ptr);
            GRADSTEP_MAP white_gradstep_map=
               imagefunc::compute_gradient_steps(
                  curr_sign_properties.min_gradient_mag,-step_distance,
                  xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
                  gradient_mag_twoDarray_ptr);

         cout << "black_gradstep_map.size() = " << black_gradstep_map.size()
              << endl;
         cout << "white_gradstep_map.size() = " << white_gradstep_map.size()
              << endl;

            int px,py,qx,qy;
            double stepped_h,stepped_s,stepped_v;

            if (curr_sign_properties.black_interior_flag)
            {

// Search for hot edges which have "black"-colored pixels on their
// cool sides:

               black_pixels.clear();
               for (gradstep_iter=black_gradstep_map.begin(); 
                    gradstep_iter != black_gradstep_map.end(); gradstep_iter++)
               {
                  int pixel_ID=gradstep_iter->first;
                  int stepped_pixel_ID=gradstep_iter->second;
                  graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
                  graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
                  quantized_texture_rectangle_ptr->get_pixel_hsv_values(
                     qx,qy,stepped_h,stepped_s,stepped_v);
  
// Relax stepped_v value for PointGrey cameras:
          
                  if (stepped_v < 0.75)
                  {
                     black_pixels.push_back(pair<int,int>(px,py));

                     if (black_grads_texture_rectangle_ptr != NULL)
                     {
                        int gradstep_R=255;
                        int gradstep_G=0;
                        int gradstep_B=255;
                        black_grads_texture_rectangle_ptr->
                           set_pixel_RGB_values(
                              px,py,gradstep_R,gradstep_G,gradstep_B);
                     }
                  } // stepped_v < 0.5 conditional
               } // loop over black_gradstep_map iterator

               string black_grads_filename=video_subdir+"black_grads_"
                  +stringfunc::number_to_string(frame_number)+".jpg";
               cout << "black_grads_filename = " << black_grads_filename 
                    << endl;
               black_grads_texture_rectangle_ptr->write_curr_frame(
                  black_grads_filename);
            }
            else if (curr_sign_properties.white_interior_flag)
            {

// Search for hot edges which have "white"-colored pixels on their
// warm sides:

               white_pixels.clear();
               for (gradstep_iter=white_gradstep_map.begin(); 
                    gradstep_iter != white_gradstep_map.end(); gradstep_iter++)
               {
                  int pixel_ID=gradstep_iter->first;
                  int stepped_pixel_ID=gradstep_iter->second;
                  graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
                  graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
                  quantized_texture_rectangle_ptr->get_pixel_hsv_values(
                     qx,qy,stepped_h,stepped_s,stepped_v);
            
                  if (stepped_v > 0.5 && stepped_s < 0.3)
                  {
                     white_pixels.push_back(pair<int,int>(px,py));
                     if (white_grads_texture_rectangle_ptr != NULL)
                     {
                        int gradstep_R=255;
                        int gradstep_G=0;
                        int gradstep_B=255;
                        white_grads_texture_rectangle_ptr->
                           set_pixel_RGB_values(
                              px,py,gradstep_R,gradstep_G,gradstep_B);
                     }
                  } // stepped_v > 0.5 && stepped_s < 0.3 conditional
               } // loop over gradstep iterator

               string white_grads_filename=video_subdir+"white_grads_"
                  +stringfunc::number_to_string(frame_number)+".jpg";
               cout << "white_grads_filename = " << white_grads_filename 
                    << endl;
               white_grads_texture_rectangle_ptr->write_curr_frame(
                  white_grads_filename);

            } // black_interior_flag and white_interior_flag conditionals

            connected_components* connected_components_ptr=
               new connected_components();
            connected_components* inverse_connected_components_ptr=
               new connected_components();

            int color_channel_ID=-1;
            connected_components_ptr->reset_image(
               binary_quantized_filename,color_channel_ID,0);
            inverse_connected_components_ptr->reset_image(
               binary_quantized_filename,color_channel_ID,0);
            filefunc::deletefile(binary_quantized_filename);

            int index=0;
            int threshold=128;
            int level=threshold;
            bool RLE_flag=true;
            bool invert_binary_values_flag=false;
//            bool export_connected_regions_flag=false;
            bool export_connected_regions_flag=true;
            int n_components=
               connected_components_ptr->compute_connected_components(
                  index,threshold,level,RLE_flag,invert_binary_values_flag,
                  export_connected_regions_flag);
            cout << "n_components = " << n_components << endl;

            int n_inverse_components=
               inverse_connected_components_ptr->compute_connected_components(
                  index,threshold,level,RLE_flag,!invert_binary_values_flag,
                  export_connected_regions_flag);
            cout << "n_inverse_components = " << n_inverse_components << endl;

            vector<extremal_region*> extremal_region_ptrs=
               connected_components_ptr->select_extremal_regions(
                  level,
                  curr_sign_properties.min_aspect_ratio,
                  curr_sign_properties.max_aspect_ratio,
                  curr_sign_properties.min_compactness,
                  curr_sign_properties.max_compactness,
                  curr_sign_properties.min_n_holes,
                  curr_sign_properties.max_n_holes,
                  curr_sign_properties.min_n_crossings,
                  curr_sign_properties.max_n_crossings);

            cout << "extremal_region_ptrs.size() = " 
                 << extremal_region_ptrs.size() << endl;
         
            vector<extremal_region*> inverse_extremal_region_ptrs=
               inverse_connected_components_ptr->select_extremal_regions(
                  level,0.1,10,0.045,10,0,10,0,10);
            cout << "inverse_extremal_region_ptrs.size() = "
                 << inverse_extremal_region_ptrs.size() << endl;

            string sign_hue=curr_sign_properties.sign_hue;
            for (int r=0; r<extremal_region_ptrs.size(); r++)
            {
               extremal_region* extremal_region_ptr=extremal_region_ptrs[r];
//               cout << "r = " << r 
//                    << " extremal region = " << *extremal_region_ptr 
//                    << endl;

//               cout << "sign_hue = " << sign_hue << endl;
               if (sign_hue=="blue2") sign_hue="blue";

               if (sign_hue != "black")
               {
                  int dominant_hue_index=
                     RGB_analyzer_ptr->dominant_extremal_region_hue_content(
                        lookup_map_name,extremal_region_ptr,
                        quantized_texture_rectangle_ptr);
                  string dominant_hue_name=RGB_analyzer_ptr->
                     get_hue_given_index(dominant_hue_index);
//                  cout << "Dominant hue = " << dominant_hue_name 
//                       << " sign_hue = " << sign_hue << endl;

// Ignore candidate extremal region if its dominant hue does not agree
// with TOC12 ppt sign:

                  if (dominant_hue_name != sign_hue) continue;
               } // sign_hue != black conditional
            
               int left_pu,bottom_pv,right_pu,top_pv;
               extremal_region_ptr->get_bbox(
                  left_pu,bottom_pv,right_pu,top_pv);
            
// Reject bbox if it is very small:

//               cout << "right_pu-left_pu = " << right_pu-left_pu 
//                    << " bottom_pv-top_pv = " << bottom_pv-top_pv
//                    << endl;
               if ((right_pu-left_pu <= 10) && (bottom_pv-top_pv <=10)) 
                  continue;

// Next count number of significant holes within extremal region
// based upon inverse regions' bounding box coordinates:

               const int min_inverse_pixel_area=12;
               int n_significant_holes=0;
               for (int ir=0; ir<inverse_extremal_region_ptrs.size(); ir++)
               {
                  extremal_region* inverse_region_ptr=
                     inverse_extremal_region_ptrs[ir];

                  int inverse_pixel_area=inverse_region_ptr->get_pixel_area();
//               cout << "inverse_pixel_area = " << inverse_pixel_area
//                    << endl;
                  if (inverse_pixel_area <= min_inverse_pixel_area) continue;
               
                  int ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv;
                  inverse_region_ptr->get_bbox(
                     ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv);

//               cout << "ir_left_pu = " << ir_left_pu 
//                    << " left_pu = " << left_pu << endl;
//               cout << "ir_right_pu = " << ir_right_pu 
//                    << " right_pu = " << right_pu << endl;
//               cout << "ir_bottom_pv = " << ir_bottom_pv 
//                    << " bottom_pv = " << bottom_pv << endl;
//               cout << "ir_top_pv = " << ir_top_pv 
//                    << " top_pv = " << top_pv << endl;

                  if (ir_left_pu < left_pu) continue;
                  if (ir_right_pu > right_pu) continue;
                  if (ir_bottom_pv < bottom_pv) continue;
                  if (ir_top_pv > top_pv) continue;

                  n_significant_holes++;
               } // loop over index ir labeling inverse regions

               cout << "n_significant_holes = " << n_significant_holes
                    << endl;
               if (n_significant_holes < 
               curr_sign_properties.min_n_significant_holes ||
               n_significant_holes > 
               curr_sign_properties.max_n_significant_holes)
                  continue;

// Require TOC12 sign bbox to contain some minimal number of
// black-colored or white-colored pixels:

               int n_hole_pixels=0;
               bounding_box curr_bbox(left_pu,right_pu,bottom_pv,top_pv);
               if (curr_sign_properties.black_interior_flag)
               {
                  for (int p=0; p<black_pixels.size(); p++)
                  {
                     int px=black_pixels[p].first;
                     int py=black_pixels[p].second;
                     if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
                  }
               } 
               else if (curr_sign_properties.white_interior_flag)
               {
                  for (int p=0; p<white_pixels.size(); p++)
                  {
                     int px=white_pixels[p].first;
                     int py=white_pixels[p].second;
                     if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
                  }
               } // black,white interior flag conditionals
               cout << "n_hole_pixels = " << n_hole_pixels << endl;
               if (!curr_sign_properties.purple_interior_flag && 
               n_hole_pixels < 5) continue;

// Check blue signs for purple interior content:

               if (sign_hue=="blue")
               {
                  int n_purple_hole_pixels=0;
                  double h,s,v;
                  const double min_hue=-100; // blue-purple
                  const double max_hue=-20;  // red-purple
                  const double min_saturation=0.4;
                  const double min_value=0.5;
                  for (int py=bottom_pv; py<=top_pv; py++)
                  {
                     for (int px=left_pu; px<=right_pu; px++)
                     {
                        texture_rectangle_ptr->get_pixel_hsv_values(
                           px,py,h,s,v);
                        h=basic_math::phase_to_canonical_interval(
                           h,-180,180);
                        if (h > min_hue && h < max_hue && s > min_saturation
                        && v > min_value) n_purple_hole_pixels++;
                     } // loop over px index
                  } // loop over py index

                  if (curr_sign_properties.purple_interior_flag)
                  {
                     if (n_purple_hole_pixels < 5) continue;
                  }
                  else
                  {
                     if (n_purple_hole_pixels > 10) continue;
                  }
               } // sign_hue==blue conditional
   
// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

               int width=right_pu-left_pu+1;
               int height=fabs(bottom_pv-top_pv)+1;
               double aspect_ratio=double(width)/double(height);
//               cout << "width = " << width << " height = " << height 
//                    << " aspect_ratio = " << aspect_ratio << endl;

               int new_width,new_height;
               if (aspect_ratio > 1)
               {
                  new_width=32;
                  new_height=new_width/aspect_ratio;
               }
               else
               {
                  new_height=32;
                  new_width=aspect_ratio*new_height;
               }
               cout << "new_height = " << new_height
                    << " new_width = " << new_width << endl;

// Copy TOC12 sign-dependent greyscale values into ptwoDarray_ptr
// member of *texture_rectangle_ptr:

//               texture_rectangle_ptr->refresh_ptwoDarray_ptr();
               
// Conversion from RGB color to grey-scale is TOC12 sign dependent!

               if (!RGB_pixels_flag)
               {
                  if (symbol_name=="yellow_radiation" ||
                  symbol_name=="orange_biohazard")
                  {
                     texture_rectangle_ptr->
                        convert_color_image_to_greyscale(); 
                  }
                  else if (symbol_name=="blue_radiation")
                  {
                     bool generate_greyscale_image_flag=true;
                     texture_rectangle_ptr->
                        convert_color_image_to_single_color_channel(
                           1,generate_greyscale_image_flag);
                     // red channel
                  }
                  else
                  {
                     texture_rectangle_ptr->
                        convert_color_image_to_luminosity();
                  }
               } // !RGB_pixels_flag conditional

               twoDarray* qtwoDarray_ptr=texture_rectangle_ptr->
                  export_sub_twoDarray(left_pu,right_pu,bottom_pv,top_pv);
               qtwoDarray_ptr->init_coord_system(0,1,0,1);

               twoDarray* qnew_twoDarray_ptr=compositefunc::downsample(
                  new_width,new_height,qtwoDarray_ptr);
               delete qtwoDarray_ptr;

/*
// For debugging purposes only, export *qnew_twoDarray_ptr as new JPG
// image chip:

               texture_rectangle* subtexture_rectangle_ptr=new
                  texture_rectangle(new_width,new_height,1,3,NULL);
               subtexture_rectangle_ptr->generate_blank_image_file(
                  new_width,new_height,"blank.jpg",0.5);
               bool randomize_blue_values_flag=true;
               subtexture_rectangle_ptr->
                  convert_single_twoDarray_to_three_channels(
                     qnew_twoDarray_ptr,randomize_blue_values_flag);
               string candidate_char_patches_subdir="./candidate_patches/";
               filefunc::dircreate(candidate_char_patches_subdir);
               string patch_filename=candidate_char_patches_subdir+
                  "candidate_char_patch_"+stringfunc::integer_to_string(
                     candidate_char_counter++,3)+".jpg";
               cout << "patch_filename = " << patch_filename << endl;
               subtexture_rectangle_ptr->write_curr_frame(patch_filename);
               delete subtexture_rectangle_ptr;
*/

// Need to compute K_Ng features for 8x8 patches within
// *qnew_twoDarray_ptr.  Pool features within 3x3 sectors into
// single nineK=9xK_Ng vector.  Then compute probability rescaled chip
// corresponds to text character using learned Ng probability decision
// function...

               *(texture_rectangle_ptr->get_ptwoDarray_ptr()) = 
                  *qnew_twoDarray_ptr;
               delete qnew_twoDarray_ptr;

               text_detector* text_detector_ptr=
                  text_detector_ptrs[curr_sign_ID];

               text_detector_ptr->set_texture_rectangle_ptr(
                  texture_rectangle_ptr);
               text_detector_ptr->set_window_width(new_width);
               text_detector_ptr->set_window_height(new_height);
               text_detector_ptr->clear_window_features_vector();

//               bool perform_Ng_classification_flag=false;
               bool perform_Ng_classification_flag=true;
               if (perform_Ng_classification_flag)
               {
                  double Ng_char_prob=0;
                  bool flag=text_detector_ptr->average_window_features(0,0);
                  if (flag)
                  {
                     float* window_histogram=text_detector_ptr->
                        get_nineK_window_descriptor();
                     for (int k=0; k<nineK; k++)
                     {
                        Ng_sample(k)=window_histogram[k];
//                        cout << "k = " << k << " window_histogram[k] = "
//                             << window_histogram[k] << endl;
                     } // loop over index k labeling dictionary descriptors
                     Ng_char_prob=Ng_pfuncts[curr_sign_ID](Ng_sample);
                     cout << "Extremal region index r = " << r
                          << " Ng char probability = " << Ng_char_prob << endl
                          << endl;
//               outputfunc::enter_continue_char();
                  }
                  else
                  {
                     cout << "Averaged window features computation failed!" 
                          << endl;
                  }

// If we imported patch_filename into *texture_rectangle_ptr, we now
// need to reload image_filename into *texture_rectangle_ptr:

//                  texture_rectangle_ptr->reset_texture_content(image_filename);

                  if (Ng_char_prob < curr_sign_properties.Ng_threshold)
                  {
                     n_Ng_rejections++;
                     continue;
                  }
                  else
                  {
                     n_Ng_acceptances++;
                  }
               } // perform_Ng_classification_flag conditional

               double left_u,top_v,right_u,bottom_v;
               binary_texture_rectangle_ptr->get_uv_coords(
                  left_pu,top_pv,left_u,top_v);
               binary_texture_rectangle_ptr->get_uv_coords(
                  right_pu,bottom_pv,right_u,bottom_v);

               vector<threevector> vertices;
               vertices.push_back(threevector(left_u,top_v));
               vertices.push_back(threevector(left_u,bottom_v));
               vertices.push_back(threevector(right_u,bottom_v));
               vertices.push_back(threevector(right_u,top_v));
               polygon bbox(vertices);
               bbox_polygons.push_back(bbox);

               int color_index=colorfunc::get_color_index(
                  curr_sign_properties.bbox_color);
               bbox_color_indices.push_back(color_index);

            } // loop over index r labeling selected extremal regions

            delete connected_components_ptr;
            delete inverse_connected_components_ptr;

            outputfunc::enter_continue_char();

         } // loop over curr_sign_ID

         cout << "n_Ng_rejections = " << n_Ng_rejections << endl;
         cout << "n_Ng_acceptances = " << n_Ng_acceptances << endl;

         if (bbox_polygons.size()==0) continue;

// Reload original, colored image into *texture_rectangle_ptr:

         texture_rectangle_ptr->reset_texture_content(image_filename);

         int thickness=1;
         for (int b=0; b<bbox_polygons.size(); b++)
         {
            videofunc::display_polygon(
               bbox_polygons[b],texture_rectangle_ptr,
               bbox_color_indices[b],thickness);
         }
         string bboxes_filename=video_subdir+
            "bboxes_"+stringfunc::integer_to_string(frame_number,4)+".jpg";
         texture_rectangle_ptr->write_curr_frame(bboxes_filename);

      } // loop over index i labeling image filenames

      double total_time=timefunc::elapsed_timeofday_time();
      cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
           << total_time / 60.0 << " minutes" << endl;
      double avg_time_per_image=
         timefunc::elapsed_timeofday_time()/image_counter;
      cout << "***********************************************" << endl;
      cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image 
           << " secs" << " n_images = " << image_counter << endl;
      cout << "***********************************************" << endl;

   } // loop over video_number 

   delete RGB_analyzer_ptr;
   delete raw_texture_rectangle_ptr;
   delete texture_rectangle_ptr;
   delete edges_texture_rectangle_ptr;
   delete quantized_texture_rectangle_ptr;
   delete selected_colors_texture_rectangle_ptr;
   delete binary_texture_rectangle_ptr;
   delete black_grads_texture_rectangle_ptr;

   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete gradient_mag_twoDarray_ptr;
}

