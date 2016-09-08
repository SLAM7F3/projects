// ====================================================================
// Program ADIENCE_IMGS 

// 			 ./adience_images
// ====================================================================
// Last updated on 7/31/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

//   string faces_rootdir = "/data/TrainingImagery/faces/";
   string faces_rootdir = "/media/pcho/DataTransfer/faces/";
//   string faces_rootdir = "/media/DataTransfer/faces/";
   string adiencefaces_subdir = faces_rootdir + "AdienceFaces/";
   string faces_subdir = adiencefaces_subdir + "faces/";
   string output_faces_subdir="resized_adiencefaces/";
   filefunc::dircreate(output_faces_subdir);

   int max_xdim = 106;
   int max_ydim = 106;
   int face_counter = 0;

   vector<string> fold_filenames;
   int f_start = 0;
   int f_stop = 4;
   for(int f = f_start; f <= f_stop; f++)
   {
      string curr_fold_filename = adiencefaces_subdir + "fold_"+
         stringfunc::number_to_string(f)+"_data.txt";
      fold_filenames.push_back(curr_fold_filename);
      
      filefunc::ReadInfile(curr_fold_filename);
      for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
      {
         if(i%100 == 0)
         {
            double progress_frac = double(i)/filefunc::text_line.size();
            cout << "f = " << f << " of " << f_stop << endl;
            outputfunc::print_elapsed_and_remaining_time(progress_frac);
         }

         vector<string> substrings = 
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
//         cout << "substrings.size = " << substrings.size() << endl;

         if(substrings.size() < 13) continue;
         
         string user_id = substrings[0];
         string image_basename = substrings[1];
         string face_id = substrings[2];

         string gender = "female";
         if(substrings[5] == "m")
         {
            gender = "male";
         }
         
         int px = stringfunc::string_to_number(substrings[6]);
         int py = stringfunc::string_to_number(substrings[7]);
         int x_extent = stringfunc::string_to_number(substrings[8]);
         int y_extent = stringfunc::string_to_number(substrings[9]);

         string user_subdir=faces_subdir+user_id+"/";
         string image_filename=user_subdir+"coarse_tilt_aligned_face."+
            face_id+"."+image_basename;

//         cout << user_id << " "
//              << image_basename << " "
//              << gender << " "
//              << px << " "
//              << py << " "
//              << x_extent << " "
//              << y_extent << " "
//              << endl;
//         cout << "   " << image_filename << endl;
         int px_min = px;
         int px_max = px+x_extent;
         int py_min = py;
         int py_max = py+y_extent;

         int xdim, ydim;
         if(!imagefunc::get_image_width_height(image_filename,xdim,ydim))
         {
            continue;
         }
         int sdim = basic_math::min(xdim, ydim);
         int xoffset = 0.5 * (xdim - sdim);
         int yoffset = 0.5 * (ydim - sdim);

//         Magick::Image IM_image;
//         videofunc::import_IM_image(image_filename, IM_image);

         string output_filename = output_faces_subdir+gender+"_"+
            stringfunc::integer_to_string(face_counter++,5)+".jpg";

         imagefunc::crop_image(
            image_filename, output_filename,
            sdim, sdim, xoffset, yoffset);

//         cout << "sdim = " << sdim << " max_xdim = " << max_xdim
//              << " max_ydim = " << max_ydim << endl;
         
//         videofunc::downsize_image(
//            output_filename, max_xdim, max_ydim);

//         videofunc::crop_image(IM_image, sdim, sdim, xoffset, yoffset);
//         videofunc::resize_image(IM_image, max_xdim, max_ydim);
//         videofunc::export_IM_image(output_filename, IM_image);

      } // loop over index i labeling lines in data fold text file
   } // loop over index f labeling fold filename
   
}

