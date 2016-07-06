// ==========================================================================
// Program QUANTIZE_COLORS
// ==========================================================================
// Last updated on 9/4/12; 9/8/12; 9/18/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/RGB_analyzer.h"


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("grey");

   string ppt_signs_subdir="./images/ppt_signs/";
   string output_subdir=ppt_signs_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();

//   int video_start=1;
//   int video_stop=4;
//   int video_start=2;
//   int video_stop=2;
   int video_start=5;
   int video_stop=5;
   int delta_video=2;

   cout << "Enter starting video pass number ( >= 5):" << endl;
   cin >> video_start;
   cout << "Enter stopping video pass number ( >= 19):" << endl;
   cin >> video_stop;

   for (int video_number=video_start; video_number <=video_stop; 
        video_number += delta_video)
   {
//      int video_number=1;
//      cout << "Enter video number:" << endl;
//      cin >> video_number;

      string video_subdir=output_subdir+
         "vid_"+stringfunc::integer_to_string(video_number,2)+"/";
      filefunc::dircreate(video_subdir);

      string image_subdir=ppt_signs_subdir+"videos/vid";
      image_subdir += stringfunc::integer_to_string(video_number,2)+"_frames/";

      vector<string> image_filenames=filefunc::image_files_in_subdir(
         image_subdir);

      int i_start=0;
      int i_stop=image_filenames.size()-1;
      cout << "Last image number = " << image_filenames.size() << endl;
      cout << "Enter starting image number:" << endl;
      cin >> i_start;
      cout << "Enter stopping image number:" << endl;
      cin >> i_stop;
      i_start--;
      i_stop--;

      for (int i=i_start; i<=i_stop; i++)
      {
         string image_filename=image_filenames[i];
         cout << "image_filename = " << image_filename << endl;
         int frame_number=i+1;
         
         texture_rectangle_ptr->reset_texture_content(image_filename);
         quantized_texture_rectangle_ptr->reset_texture_content(
            image_filename);

         RGB_analyzer_ptr->quantize_texture_rectangle_colors(
            quantized_texture_rectangle_ptr);

//         string init_quantized_filename=video_subdir+
//            "frame_"+stringfunc::number_to_string(frame_number)+".jpg";

//         quantized_texture_rectangle_ptr->write_curr_frame(
//            init_quantized_filename);
//         string banner="Exported "+init_quantized_filename;
//         outputfunc::write_big_banner(banner);

//         int n_filter_iters=1;
//         int n_filter_iters=2;
//         int n_filter_iters=3;
         int n_filter_iters=4;
         for (int filter_iter=0; filter_iter < n_filter_iters; filter_iter++)
         {
            RGB_analyzer_ptr->smooth_quantized_image(
               texture_rectangle_ptr,quantized_texture_rectangle_ptr);
         }
         string filtered_quantized_filename=video_subdir+
            "filtered_"+stringfunc::number_to_string(frame_number)+".jpg";
         quantized_texture_rectangle_ptr->write_curr_frame(
            filtered_quantized_filename);
         string banner="Exported "+filtered_quantized_filename;
         outputfunc::write_big_banner(banner);

         int R_grey=192;
         int G_grey=192;
         int B_grey=192;
         RGB_analyzer_ptr->identify_all_greyish_pixels(
            quantized_texture_rectangle_ptr,R_grey,G_grey,B_grey);
         string greys_filename=video_subdir+
            "greys_"+stringfunc::number_to_string(frame_number)+".jpg";
         quantized_texture_rectangle_ptr->write_curr_frame(greys_filename);

      } // loop over index i labeling image filenames
   } // loop over video_number 

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete quantized_texture_rectangle_ptr;
}

