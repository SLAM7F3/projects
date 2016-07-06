// ==========================================================================
// Program BLACK_SIGNS is a playpen for experimenting with
// nominating TOC12 skull and eat ppt sign regions based primarily
// upon their color and edge contents.
// ==========================================================================
// Last updated on 10/15/12; 10/16/12; 12/23/13; 6/7/14; 1/23/16
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "image/compositefuncs.h"
#include "video/connected_components.h"
#include "image/extremal_regions_group.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "video/mserfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/RGB_analyzer.h"
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

   string ppt_signs_subdir="./images/ppt_signs/";
   string symbols_input_subdir=ppt_signs_subdir;
   string learned_funcs_subdir="./learned_functions/";

   string output_subdir=ppt_signs_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);

   extremal_regions_group regions_group;
   twoDarray* cc_twoDarray_ptr=NULL;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* mser_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* edges_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* extremal_regions_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* candidate_MSERs_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* black_flooded_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* binary_texture_rectangle_ptr=
      new texture_rectangle();
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
   curr_sign_properties.sign_hue="blue2";
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
   curr_sign_properties.colors_to_find.push_back("lightred");
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
//   curr_sign_properties.sign_hue="grey";
   curr_sign_properties.sign_hue="black";
   curr_sign_properties.bbox_color="brick";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("black");
//   curr_sign_properties.colors_to_find.push_back("darkgrey");
   curr_sign_properties.min_aspect_ratio=0.1;
   curr_sign_properties.max_aspect_ratio=4;
   curr_sign_properties.min_compactness=0.02;
   curr_sign_properties.max_compactness=0.56;
   curr_sign_properties.min_n_holes=0;
//   curr_sign_properties.min_n_holes=1;
   curr_sign_properties.max_n_holes=7;
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
//   curr_sign_properties.sign_hue="grey";
   curr_sign_properties.sign_hue="black";
   curr_sign_properties.bbox_color="gold";
   curr_sign_properties.colors_to_find.clear();
   curr_sign_properties.colors_to_find.push_back("black");
//   curr_sign_properties.colors_to_find.push_back("darkgrey");
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
   for (unsigned int s=0; s<sign_properties.size(); s++)
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

   int sign_ID_start=7;
//   int sign_ID_stop=7;
//   int sign_ID_start=8;
   int sign_ID_stop=8;

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
   for (unsigned int s=0; s<sign_properties.size(); s++)
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

      bool RGB_pixels_flag = true;
      text_detector* text_detector_ptr=new text_detector(
         dictionary_subdir,RGB_pixels_flag);
      text_detector_ptrs.push_back(text_detector_ptr);
      text_detector_ptr->import_inverse_sqrt_covar_matrix();
   } // loop over index s labeling TOC12 symbols

// -----------------------------------------------

   twoDarray* xderiv_twoDarray_ptr=NULL;
   twoDarray* yderiv_twoDarray_ptr=NULL;
   twoDarray* gradient_mag_twoDarray_ptr=NULL;

   int video_start=12;
   int video_stop=12;
   int delta_video=1;

   cout << "Enter starting video pass number ( >= 5):" << endl;
   cin >> video_start;
   cout << "Enter stopping video pass number ( <= 20):" << endl;
   cin >> video_stop;

   timefunc::initialize_timeofday_clock();

   extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr=
      new extremal_regions_group::ID_REGION_MAP;

// Loop over multiple video clips starts here:

   int image_counter=0;
   for (int video_number=video_start; video_number <=video_stop; 
        video_number += delta_video)
   {
      string video_subdir=output_subdir+
         "vid_"+stringfunc::integer_to_string(video_number,2)+"/";
      filefunc::dircreate(video_subdir);

      string image_subdir=ppt_signs_subdir+"videos/vid";
      image_subdir += stringfunc::integer_to_string(video_number,2)+"_frames/";
      cout << "image_subdir = " << image_subdir << endl;

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
//      cout << "i_start = " << i_start << " i_stop = " << i_stop << endl;

// Loop over individual video frames starts here:

      for (int i=i_start; i<=i_stop; i++)
      {
         string orig_image_filename=image_filenames[i];
//          cout << "Original image_filename = " << orig_image_filename << endl;

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

         vector<polygon> bright_bbox_polygons,dark_bbox_polygons,
            bbox_polygons,Ng_bbox_polygons;
         vector<int> Ng_bbox_color_indices;

         unsigned int orig_xdim,orig_ydim,xdim,ydim;
         string image_filename;
         imagefunc::get_image_width_height(
            orig_image_filename,orig_xdim,orig_ydim);
         if (orig_xdim >= 640)
         {
            string dirname=filefunc::getdirname(orig_image_filename);
            string basename=filefunc::getbasename(orig_image_filename);

            string subdirname=dirname+"downsized/";
            filefunc::dircreate(subdirname);
            image_filename=subdirname+"downsized_"+basename;

            double aspect_ratio=double(orig_xdim)/double(orig_ydim);

            xdim=640;
            ydim=xdim/aspect_ratio;
            videofunc::resize_image(
               orig_image_filename,orig_xdim,orig_ydim,
               xdim,ydim,image_filename);
         }
         else
         {
            image_filename=orig_image_filename;
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

            mser_texture_rectangle_ptr->reset_texture_content(image_filename);
            edges_texture_rectangle_ptr->reset_texture_content(image_filename);
            candidate_MSERs_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            extremal_regions_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            black_flooded_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            binary_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            black_grads_texture_rectangle_ptr->reset_texture_content(
               image_filename);
            white_grads_texture_rectangle_ptr->reset_texture_content(
               image_filename);

// Compute edge map.  Require strong edge content within genuine TOC12
// signs:

            edges_texture_rectangle_ptr->convert_color_image_to_luminosity();
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
            
            double spatial_resolution=0.25;
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

            double step_distance=-2;
            typedef map<int,int> GRADSTEP_MAP;
            GRADSTEP_MAP::iterator black_iter,white_iter;

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

// Try identifying blackish pixels and then flood-filling their
// neighbors:

            int black_flood_R=128;
            int black_flood_G=0;
            int black_flood_B=128;
            black_flooded_texture_rectangle_ptr->floodfill_black_pixels(
               black_flood_R,black_flood_G,black_flood_B);
            string black_flooded_filename=video_subdir+
               "black_flooded_"+stringfunc::number_to_string(frame_number)+
               ".jpg";
            black_flooded_texture_rectangle_ptr->write_curr_frame(
               black_flooded_filename);

// Convert flood-filled black pixels into binary image:

            for (unsigned int py=0; py<ydim; py++)
            {
               for (unsigned int px=0; px<xdim; px++)
               {
                  int R,G,B;
                  black_flooded_texture_rectangle_ptr->
                     get_pixel_RGB_values(px,py,R,G,B);
                  if (R==black_flood_R && G==black_flood_G &&
                  B==black_flood_B)
                  {
                     R=G=B=255;
                  }
                  else
                  {
                     R=G=B=0;
                  }
                  binary_texture_rectangle_ptr->
                     set_pixel_RGB_values(px,py,R,G,B);
               } // loop over px
            } // loop over py

            string binary_quantized_filename=video_subdir+
               "binary_black_"+stringfunc::number_to_string(frame_number)+
               ".jpg";
            binary_texture_rectangle_ptr->write_curr_frame(
               binary_quantized_filename);

// Extract connected components from flood-filled black blobs image in
// *connected_components_ptr:

            connected_components* connected_components_ptr=
               new connected_components();

            int color_channel_ID=-1;
            connected_components_ptr->reset_image(
               binary_quantized_filename,color_channel_ID,0);
            filefunc::deletefile(binary_quantized_filename);

            int index=0;
            int threshold=128;
            int level=threshold;
            bool RLE_flag=true;
            bool invert_binary_values_flag=false;
            bool export_connected_regions_flag=false;
//            bool export_connected_regions_flag=true;
            int n_components=
               connected_components_ptr->compute_connected_components(
                  index,threshold,level,RLE_flag,invert_binary_values_flag,
                  export_connected_regions_flag);
            cout << "n_components = " << n_components << endl;
            twoDarray* black_cc_twoDarray_ptr=
               connected_components_ptr->get_cc_twoDarray_ptr();
            connected_components::TREE_PTR tree_ptr=connected_components_ptr->
               get_tree_ptr();
//            cout << "tree.size() = " << tree_ptr->size() << endl;

            connected_components::TREENODES_MAP* treenodes_map_ptr=
               connected_components_ptr->get_treenodes_map_ptr();
            regions_group.destroy_all_regions(black_regions_map_ptr);
            for (connected_components::TREENODES_MAP::iterator treenodes_iter=
                    treenodes_map_ptr->begin(); treenodes_iter != 
                    treenodes_map_ptr->end(); treenodes_iter++)
            {
               connected_components::TREENODE_PTR treenode_ptr=
                  treenodes_iter->second;
               extremal_region* extremal_region_ptr=
                  treenode_ptr->get_data_ptr();
               (*black_regions_map_ptr)[extremal_region_ptr->get_ID()]=
                  extremal_region_ptr;

               unsigned int min_px,min_py,max_px,max_py;
               extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

//               cout << "dark region ID=" << extremal_region_ptr->get_ID()
//                    << " min_px=" << min_px << " max_px=" << max_px
//                    << " min_py=" << min_py << " max_py=" << max_py
//                    << endl;
            }
//            cout << "black_regions_map.size() = "
//                 << black_regions_map_ptr->size() << endl;

// Search for hot edges which have "black"-colored pixels on their
// cool sides:

            int flood_R=255;
            int flood_G=0;
            int flood_B=255;

            unsigned int px,py,qx,qy;
            int stepped_R,stepped_G,stepped_B;
            for (black_iter=black_gradstep_map.begin(); 
                 black_iter != black_gradstep_map.end(); black_iter++)
            {
               int pixel_ID=black_iter->first;
               int stepped_pixel_ID=black_iter->second;
               graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
               graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
               black_flooded_texture_rectangle_ptr->get_pixel_RGB_values(
                  qx,qy,stepped_R,stepped_G,stepped_B);
               if (stepped_R==black_flood_R && stepped_G==black_flood_G && 
                   stepped_B==black_flood_B)
               {
                  black_grads_texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,flood_R,flood_G,flood_B);
               }
            } // loop over black_gradstep_map iterator
            
            string black_grads_filename=video_subdir+"black_grads_"
               +stringfunc::number_to_string(frame_number)+".jpg";
            black_grads_texture_rectangle_ptr->write_curr_frame(
               black_grads_filename);

// Compute locally bright MSERs:

            regions_group.purge_dark_and_bright_regions();

            regions_group.extract_MSERs(image_filename);
            regions_group.instantiate_twoDarrays(ptwoDarray_ptr);
            regions_group.update_bright_cc_twoDarray();
            cc_twoDarray_ptr=regions_group.get_bright_cc_twoDarray_ptr();

            mserfunc::draw_MSERs(cc_twoDarray_ptr,mser_texture_rectangle_ptr);
            string msers_filename=video_subdir+"msers_"+
               stringfunc::number_to_string(frame_number)+".jpg";
            cout << "msers_filename = " << msers_filename << endl;
            mser_texture_rectangle_ptr->write_curr_frame(msers_filename);

// Coalesce touching bright MSERs:

            extremal_regions_group::ID_REGION_MAP* 
               coalesced_bright_region_map_ptr=
               regions_group.coalesce_bright_touching_regions();

            mserfunc::draw_MSERs(cc_twoDarray_ptr,mser_texture_rectangle_ptr);
            string coalesced_msers_filename=video_subdir+"coalesced_msers_"+
               stringfunc::number_to_string(frame_number)+".jpg";
            mser_texture_rectangle_ptr->write_curr_frame(
               coalesced_msers_filename);

// Print bounding boxes around each bright, coalesced MSER region:

            for (extremal_regions_group::ID_REGION_MAP::iterator iter=
                    coalesced_bright_region_map_ptr->begin();
                 iter != coalesced_bright_region_map_ptr->end();
                 iter++)
            {
               extremal_region* extremal_region_ptr=iter->second;
               unsigned int min_px,min_py,max_px,max_py;
               extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

//               cout << "bright region ID=" << extremal_region_ptr->get_ID()
//                    << " min_px=" << min_px << " max_px=" << max_px
//                    << " min_py=" << min_py << " max_py=" << max_py
//                    << endl;
            }

// Identify border pixels around each coalesced bright MSER & store
// their extremal region IDs within *bright_cc_borders_twoDarray_ptr:

//            cout << "Identifying border pixels around coalesced bright MSERs:" 
//                 << endl;
            int border_thickness=3;
            regions_group.identify_bright_border_pixels(
               coalesced_bright_region_map_ptr,border_thickness);
            twoDarray* bright_cc_borders_twoDarray_ptr=
               regions_group.get_bright_cc_borders_twoDarray_ptr();

// Display border pixels surrounding coalesced bright MSERs:

            for (unsigned int ry=0; ry<ydim; ry++)
            {
               for (unsigned int rx=0; rx<xdim; rx++)
               {
                  int cc_ID=bright_cc_borders_twoDarray_ptr->get(rx,ry);
                  int R,G,B;
                  R=G=B=0;
                  if (cc_ID >= 1)	
                  {
//                     cout << "cc_ID = " << cc_ID << endl;
                     colorfunc::Color curr_color=colorfunc::get_color(
                        cc_ID%15);
                     colorfunc::RGB curr_RGB=
                        colorfunc::get_RGB_values(curr_color);
                     R=curr_RGB.first*255;
                     G=curr_RGB.second*255;
                     B=curr_RGB.third*255;
                  }
                  candidate_MSERs_texture_rectangle_ptr->
                     set_pixel_RGB_values(rx,ry,R,G,B);
               } // loop over rx 
            } // loop over ry
            
            string borders_filename=video_subdir+"borders_"+
               stringfunc::number_to_string(frame_number)+".jpg";
            candidate_MSERs_texture_rectangle_ptr->write_curr_frame(
               borders_filename);

// Establish links between bright, coalesced MSERs with black borders
// and black flooded regions whose borders overlap bright MSERs:

            for (unsigned int ry=0; ry<ydim; ry++)
            {
               for (unsigned int rx=0; rx<xdim; rx++)
               {
                  int cc_borders_ID=bright_cc_borders_twoDarray_ptr->
                     get(rx,ry);     
                  if (cc_borders_ID <= 0) continue;

                  int black_cc_ID=black_cc_twoDarray_ptr->get(rx,ry);
                  if (black_cc_ID < 0) continue;

                  int gr_R,gr_G,gr_B;
                  black_grads_texture_rectangle_ptr->get_pixel_RGB_values(
                     rx,ry,gr_R,gr_G,gr_B);
                  if (gr_R==flood_R && gr_G==flood_G && gr_B==flood_B)
                  {
                     regions_group.add_bright_dark_neighbor_pair(
                        cc_borders_ID,black_cc_ID);
                  }
               } // loop over rx 
            } // loop over ry

//            cout << "regions_group.get_n_bright_dark_region_neighbors() = "
//                 << regions_group.get_n_bright_dark_region_neighbors()
//                 << endl;
//            cout << "regions_group.get_n_dark_bright_region_neighbors() = "
//                 << regions_group.get_n_dark_bright_region_neighbors()
//                 << endl;
//            regions_group.print_bright_dark_neighbor_pairs();
        
// Form bounding box polygons around coalesced, bright MSERs which are
// adjacent to flooded black regions:
            
            for (extremal_regions_group::ID_REGION_MAP::iterator iter=
                    coalesced_bright_region_map_ptr->begin();
                 iter != coalesced_bright_region_map_ptr->end(); iter++)
            {
               extremal_region* bright_extremal_region_ptr=iter->second;
               

               int bright_cc_ID=iter->first;
               bool bright_neighbor_flag=
                  regions_group.get_bright_neighbor_pair_flag(bright_cc_ID);

               if (bright_cc_ID >= 1 && bright_neighbor_flag)
               {
                  unsigned int left_pu,right_pu,top_pv,bottom_pv;
                  bright_extremal_region_ptr->
                     get_bbox(left_pu,bottom_pv,right_pu,top_pv);

//                  cout << "bright region ID=" 
//                       << bright_extremal_region_ptr->get_ID()
//                       << " left_pu=" << left_pu << " right_pu=" << right_pu
//                       << " bottom_pv=" << bottom_pv << " top_pv=" << top_pv
//                       << endl;

// Reject bright bbox if it is very small or large:

                  if ((right_pu-left_pu <= 10) && 
                  (fabs(bottom_pv-top_pv) <=10)) 
                  {
                     cout << "Bright bbox too small!" << endl;
                     bright_extremal_region_ptr->set_bbox_polygon_ptr(NULL);
                     continue;
                  }
                  else if ((right_pu-left_pu >= xdim-2) ||
                  (fabs(bottom_pv-top_pv) >= ydim-2)) 
                  {
                     cout << "Bright bbox too large!" << endl;
                     bright_extremal_region_ptr->set_bbox_polygon_ptr(NULL);
                     continue;
                  }

                  double left_u,right_u,bottom_v,top_v;
                  mser_texture_rectangle_ptr->get_uv_coords(
                     left_pu,top_pv,left_u,top_v);
                  mser_texture_rectangle_ptr->get_uv_coords(
                     right_pu,bottom_pv,right_u,bottom_v);
                  
                  vector<threevector> vertices;
                  vertices.push_back(threevector(left_u,top_v));
                  vertices.push_back(threevector(left_u,bottom_v));
                  vertices.push_back(threevector(right_u,bottom_v));
                  vertices.push_back(threevector(right_u,top_v));
                  polygon curr_bright_bbox_polygon(vertices);
                  bright_bbox_polygons.push_back(curr_bright_bbox_polygon);
                  bright_extremal_region_ptr->set_bbox_polygon_ptr(
                     &curr_bright_bbox_polygon);
               }
            } // loop over coalesced bright MSERs

// Form bounding box polygons around flooded black regions which are
// adjacent to coalesced, bright MSERs:

            for (extremal_regions_group::ID_REGION_MAP::iterator iter=
                    black_regions_map_ptr->begin();
                 iter != black_regions_map_ptr->end(); iter++)
            {
               extremal_region* dark_region_ptr=iter->second;
               int dark_cc_ID=iter->first;
               bool dark_neighbor_flag=
                  regions_group.get_dark_neighbor_pair_flag(dark_cc_ID);

               if (dark_cc_ID >= 1 && dark_neighbor_flag)
               {
                  unsigned int left_pu,right_pu,top_pv,bottom_pv;
                  dark_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);

// Reject dark bbox if it is very small or large:

//                  cout << "dark region ID=" 
//                       << dark_region_ptr->get_ID()
//                       << " left_pu=" << left_pu << " right_pu=" << right_pu
//                       << " bottom_pv=" << bottom_pv << " top_pv=" << top_pv
//                       << endl;

                  if ((right_pu-left_pu <= 10) && 
                  (fabs(bottom_pv-top_pv) <=10)) 
                  {
                     cout << "Dark bbox too small!" << endl;
                     dark_region_ptr->set_bbox_polygon_ptr(NULL);
                     continue;
                  }
                  else if ((right_pu-left_pu >= xdim-2) ||
                  (fabs(bottom_pv-top_pv) >= ydim-2)) 
                  {
                     cout << "Dark bbox too large!" << endl;
                     dark_region_ptr->set_bbox_polygon_ptr(NULL);
                     continue;
                  }

                  double left_u,right_u,bottom_v,top_v;
                  mser_texture_rectangle_ptr->get_uv_coords(
                     left_pu,top_pv,left_u,top_v);
                  mser_texture_rectangle_ptr->get_uv_coords(
                     right_pu,bottom_pv,right_u,bottom_v);

                  vector<threevector> vertices;
                  vertices.push_back(threevector(left_u,top_v));
                  vertices.push_back(threevector(left_u,bottom_v));
                  vertices.push_back(threevector(right_u,bottom_v));
                  vertices.push_back(threevector(right_u,top_v));
                  polygon curr_dark_bbox_polygon(vertices);
                  dark_bbox_polygons.push_back(curr_dark_bbox_polygon);
                  dark_region_ptr->set_bbox_polygon_ptr(
                     &curr_dark_bbox_polygon);
               }
            } // loop over flooded black regions

// Coalesce adjacent black and bright regions' bounding boxes:

            regions_group.merge_adjacent_dark_bright_bboxes(
               black_regions_map_ptr,coalesced_bright_region_map_ptr);

            delete coalesced_bright_region_map_ptr;

// Form bounding box polygons around adjacent bright MSER & flooded
// black regions:

//            cout << "Forming merged bbox polygons" << endl;
            for (extremal_regions_group::ID_REGION_MAP::iterator iter=
                    black_regions_map_ptr->begin();
                 iter != black_regions_map_ptr->end(); iter++)
            {
               extremal_region* dark_region_ptr=iter->second;
               if (dark_region_ptr->get_bbox_polygon_ptr()==NULL) continue;

               int dark_cc_ID=iter->first;
               bool dark_neighbor_flag=
                  regions_group.get_dark_neighbor_pair_flag(dark_cc_ID);

               if (!(dark_cc_ID >= 1 && dark_neighbor_flag)) continue;

               unsigned int left_pu,right_pu,top_pv,bottom_pv;
               dark_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);

               double left_u,right_u,bottom_v,top_v;
               mser_texture_rectangle_ptr->get_uv_coords(
                  left_pu,top_pv,left_u,top_v);
               mser_texture_rectangle_ptr->get_uv_coords(
                  right_pu,bottom_pv,right_u,bottom_v);

               vector<threevector> vertices;
               vertices.push_back(threevector(left_u,top_v));
               vertices.push_back(threevector(left_u,bottom_v));
               vertices.push_back(threevector(right_u,bottom_v));
               vertices.push_back(threevector(right_u,top_v));
               polygon bbox_polygon(vertices);
               bbox_polygons.push_back(bbox_polygon);

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
               text_detector_ptr->clear_avg_window_features_vector();

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
                     cout << " Ng char probability = " << Ng_char_prob << endl
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
                     Ng_bbox_polygons.push_back(bbox_polygon);
                     int color_index=colorfunc::get_color_index(
                        curr_sign_properties.bbox_color);
                     Ng_bbox_color_indices.push_back(color_index);
                  }
               } // perform_Ng_classification_flag conditional

            } // loop over adjacent bright MSER and flooded black regions


// Export image where bright MSERs with black borders are colored red
// and black regions adjacent to MSERs are colored blue:

//             cout << "Exporting red-blue image:" << endl;
            int R,G,B;
            for (unsigned int ry=0; ry<ydim; ry++)
            {
               for (unsigned int rx=0; rx<xdim; rx++)
               {
                  int bright_cc_ID=cc_twoDarray_ptr->get(rx,ry);
                  int dark_cc_ID=black_cc_twoDarray_ptr->get(rx,ry);
                  bool bright_neighbor_flag=
                     regions_group.get_bright_neighbor_pair_flag(
                        bright_cc_ID);
                  bool dark_neighbor_flag=
                     regions_group.get_dark_neighbor_pair_flag(
                        dark_cc_ID);
                
                  R=G=B=0;
                  if (dark_cc_ID >= 1 && dark_neighbor_flag)
                  {
                     R=G=0;
                     B=255;
                  }
                  if (bright_cc_ID >= 1 && bright_neighbor_flag)
                  {
                     R=255;
                     G=B=0;
                  }

                  mser_texture_rectangle_ptr->set_pixel_RGB_values(
                     rx,ry,R,G,B);
               } // loop over rx 
            } // loop over ry
          
            string regions_filename=video_subdir+"regions_"+
               stringfunc::number_to_string(frame_number)+".jpg";
            mser_texture_rectangle_ptr->write_curr_frame(regions_filename);

            delete connected_components_ptr;

// Draw bright and dark region bboxes:

            string bright_bboxes_filename=video_subdir+
               "bright_bboxes_"+stringfunc::number_to_string(frame_number)
               +".jpg";
            string dark_bboxes_filename=video_subdir+
               "dark_bboxes_"+stringfunc::number_to_string(frame_number)
               +".jpg";
            string bboxes_filename=video_subdir+
               "bboxes_"+stringfunc::number_to_string(frame_number)+".jpg";
            int thickness=1;

            texture_rectangle_ptr->reset_texture_content(image_filename);
            videofunc::display_polygons(
               bright_bbox_polygons,texture_rectangle_ptr,-1,thickness);
            texture_rectangle_ptr->reset_texture_content(image_filename);
            videofunc::display_polygons(
               dark_bbox_polygons,texture_rectangle_ptr,-1,thickness);
            texture_rectangle_ptr->reset_texture_content(image_filename);
            videofunc::display_polygons(
               bbox_polygons,texture_rectangle_ptr,-1,thickness);

            texture_rectangle_ptr->reset_texture_content(image_filename);
            cout << "Ng_bbox_polygons.size() = " << Ng_bbox_polygons.size()
                 << endl;
            for (unsigned int b=0; b<Ng_bbox_polygons.size(); b++)
            {
               videofunc::display_polygon(
                  Ng_bbox_polygons[b],texture_rectangle_ptr,
                  Ng_bbox_color_indices[b],thickness);
            }
         } // loop over curr_sign_ID

// Export color-coded bounding boxes indicating detections following
// Ng classification:

         string Ng_bboxes_filename=video_subdir+
            "Ng_bboxes_"+stringfunc::integer_to_string(frame_number,4)+".jpg";
         texture_rectangle_ptr->write_curr_frame(Ng_bboxes_filename);

         cout << "n_Ng_rejections = " << n_Ng_rejections << endl;
         cout << "n_Ng_acceptances = " << n_Ng_acceptances << endl;

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

   regions_group.purge_dark_and_bright_regions();
   regions_group.destroy_all_regions(black_regions_map_ptr);
   delete black_regions_map_ptr;

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete mser_texture_rectangle_ptr;  
   delete edges_texture_rectangle_ptr;
   delete extremal_regions_texture_rectangle_ptr;
   delete candidate_MSERs_texture_rectangle_ptr;
   delete black_flooded_texture_rectangle_ptr;
   delete black_grads_texture_rectangle_ptr;
   delete binary_texture_rectangle_ptr;

   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete gradient_mag_twoDarray_ptr;

   regions_group.set_bright_cc_twoDarray_ptr(NULL);
}

