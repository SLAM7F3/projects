// ==========================================================================
// Program MSERS calls the Oxford MSER linux binary and parses its RLE
// output.  It exports a new version of each input image where locally
// bright [dark] maximally stable extremal regions are colored red
// [blue].
// ==========================================================================
// Last updated on 9/19/12; 9/20/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string ppt_signs_subdir="./images/ppt_signs/";
   string output_subdir=ppt_signs_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);

   int video_start=5;
   int video_stop=19;
//   int video_stop=26;
   int delta_video=1;
//   int delta_video=2;

   cout << "Enter starting video pass number ( >= 5):" << endl;
   cin >> video_start;
   cout << "Enter stopping video pass number ( >= 19):" << endl;
   cin >> video_stop;

   texture_rectangle* mser_texture_rectangle_ptr=new texture_rectangle();
   vector<extremal_region*> dark_extremal_region_ptrs,
      bright_extremal_region_ptrs;

   for (int video_number=video_start; video_number <=video_stop; 
        video_number += delta_video)
   {
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
         mser_texture_rectangle_ptr->reset_texture_content(image_filename);
  
         videofunc::extract_MSERs(
            image_filename,dark_extremal_region_ptrs,
            bright_extremal_region_ptrs);
         videofunc::update_MSER_twoDarray(
            dark_extremal_region_ptrs,bright_extremal_region_ptrs,
            mser_texture_rectangle_ptr);
         videofunc::draw_MSERs(mser_texture_rectangle_ptr);
       
         string MSER_output_filename=video_subdir+
            "MSERs_"+stringfunc::number_to_string(frame_number)+".jpg";
//         cout << "MSER_output_filename = " << MSER_output_filename << endl;
         mser_texture_rectangle_ptr->write_curr_frame(MSER_output_filename);

//         string banner="Exported "+MSER_output_filename;
//         outputfunc::write_big_banner(banner);

      } // loop over index i labeling image frames
      

   } // loop over video index

   delete mser_texture_rectangle_ptr;   

}

