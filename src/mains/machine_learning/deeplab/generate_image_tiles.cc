7// ====================================================================
// Program GENERATE_IMAGE_TILES imports all pyramided image files
// exported by PYRAMID_TESTING_IMAGES from a specified input
// subdirectory.  It breaks apart each input image into a set of
// square tiles whose pixel size is set by image_tile_size.  Adjacent
// image tiles generally overlap their neighbors by tile_border
// pixels.  The image tiles are exported as jpeg files to a specified
// output subdirectory.

//                     generate_image_tiles

// ====================================================================
// Last updated on 5/25/16; 6/15/16; 6/20/16; 6/29/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

   const int tile_border = 100;	 // pixels
//   int image_tile_size = 1000;
//   int image_tile_size = 1400;	 
   int image_tile_size = 1500;	 
//   int image_tile_size = 1860;	 // max size allowed by caffe = 1864 ?
//   int image_tile_size = 2000;
//   cout << "Enter desired pixel size for square tiles to be exported:"
//        << endl;
//   cin >> image_tile_size;
   int reduced_image_tile_size = image_tile_size - 2 * tile_border;

   string faces_subdir = "/data/TrainingImagery/faces/";
   string root_subdir = faces_subdir+"labeled_data/";

   string input_images_basedir = "faces_07_testing_images";
   cout << "Enter input images basedir relative to " << root_subdir << ":" 
        << endl;
   cout << " (e.g. faces_11/testing_images)" << endl;
   cin >> input_images_basedir;

   string input_images_subdir = root_subdir+input_images_basedir;
   filefunc::add_trailing_dir_slash(input_images_subdir);

   string doublesized_subdir=input_images_subdir+"doublesized/";
   string fullsized_subdir=input_images_subdir+"fullsized/";
   string halfsized_subdir=input_images_subdir+"halfsized/";

   vector<string> sized_subdirs;
   sized_subdirs.push_back(doublesized_subdir);
   sized_subdirs.push_back(fullsized_subdir);
   sized_subdirs.push_back(halfsized_subdir);

   int s_stop = sized_subdirs.size();
   for(int s = 0; s < s_stop; s++) 
   {
      string output_tiles_subdir=sized_subdirs[s]+"tiles/";
      filefunc::dircreate(output_tiles_subdir);
  
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         sized_subdirs[s]);
      int n_images = image_filenames.size();
//      cout << "s = " << s << " n_images = " << n_images << endl;
//      cout << "sized_subdirs[s] = " << sized_subdirs[s] << endl;

      typedef std::map<int, std::string > IMAGE_FILENAMES_MAP;
// indepedent int: image ID
// dependent string > : path to original homogenized image

      IMAGE_FILENAMES_MAP image_filenames_map;
      IMAGE_FILENAMES_MAP::iterator image_filenames_map_iter;

      vector<int> image_IDs;
      for(int i = 0; i < n_images; i++)
      {
         string curr_basename=filefunc::getbasename(image_filenames[i]);
         string separator_chars="_.";
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               curr_basename,separator_chars);
//         cout << "substrings.size() = " << substrings.size() << endl;

         int image_ID = 0;
         if(substrings.size() == 3)
         {
            image_ID = stringfunc::string_to_number(substrings[1]);
         }
         else if(substrings.size() == 4)
         {
            image_ID = stringfunc::string_to_number(substrings[2]);
         }

         image_IDs.push_back(image_ID);
         image_filenames_map[image_ID] = image_filenames[i];

//         cout << "i = " << i << " curr_basename = " << curr_basename << endl;
//         cout << "   image_ID = " << image_ID << endl;
      }

      unsigned int outside_frame_value = 0;	// black
      unsigned int istart=0;
      unsigned int istop=n_images;
      for (unsigned int i = istart; i < istop; i++)
      {
         if(i%100 == 0)
         {
            double progress_frac = double(i + s * n_images)/
               double(s_stop * n_images);
            outputfunc::print_elapsed_and_remaining_time(progress_frac);
         }

         string currimage_filename = image_filenames[i];
         string currimage_suffix = stringfunc::suffix(currimage_filename);

// As of 4/7/16, we force input images to be in jpg rather than png
// format to avoid alpha channel headaches.  Delete original png file
// after its jpg replacement has been generated:

         string currjpg_filename = currimage_filename;
         if(currimage_suffix=="png" || currimage_suffix =="PNG")
         {
            string prefix = stringfunc::prefix(currimage_filename);
            currjpg_filename=prefix+".jpg";
            string unix_cmd = "convert "+currimage_filename+" "+
               currjpg_filename;
            cout << unix_cmd << endl;
            sysfunc::unix_command(unix_cmd);
            if(!filefunc::deletefile(currimage_filename))
            {
               cout << "Couldn't delete " << currimage_filename << endl;
               outputfunc::enter_continue_char();
            }
         }

         texture_rectangle curr_tr(currjpg_filename, NULL);
         if(curr_tr.get_VideoType() == texture_rectangle::unknown) continue;
         if(curr_tr.getNchannels() != 3) continue;
      
         cout << "  Tiling " << currjpg_filename << endl;

         unsigned int image_xdim, image_ydim;
         imagefunc::get_image_width_height(
            currjpg_filename,image_xdim, image_ydim);
         int xdim = image_xdim;
         int ydim = image_ydim;

         vector<int> px_start, px_stop;
         px_start.push_back(0);
         px_stop.push_back(image_tile_size);

         if(xdim > image_tile_size)
         {
            xdim -= image_tile_size;
            while(xdim > 0)
            {
               px_start.push_back(px_start.back() + reduced_image_tile_size);
               px_stop.push_back(px_start.back() + image_tile_size);
               xdim -= reduced_image_tile_size;
            }
         }
   
         vector<int> py_start, py_stop;
         py_start.push_back(0);
         py_stop.push_back(image_tile_size);

         if(ydim > image_tile_size)
         {
            ydim -= image_tile_size;
            while(ydim > 0)
            {
               py_start.push_back(py_start.back() + reduced_image_tile_size);
               py_stop.push_back(py_start.back() + image_tile_size);
               ydim -= reduced_image_tile_size;
            }
         }

         int nx_tiles = px_start.size();
         int ny_tiles = py_start.size();
//         cout << "  nx_tiles = " << nx_tiles << " ny_tiles = " << ny_tiles
//              << endl;
//         cout << "i = " << i << " curr_tr: width = " << curr_tr.getWidth()
//              << " height = " << curr_tr.getHeight() << endl;

         for(int nx = 0; nx < nx_tiles; nx++)
         {
            string nx_label = stringfunc::integer_to_string(nx, 2);
            for(int ny = 0; ny < ny_tiles; ny++)
            {
               string ny_label = stringfunc::integer_to_string(ny, 2);
               string image_label = stringfunc::integer_to_string(
                  image_IDs[i],5);
//               cout << "i = " << i << " image_IDs[i] = " << image_IDs[i]
//                    << " image_label = " << image_label
//                    << endl;
               string curr_tile_filename = 
                  output_tiles_subdir+"tile_"+image_label+"_"+
                  nx_label+"_"+ny_label+".jpg";

//            cout << "nx = " << nx << " ny = " << ny << endl;
//            cout << "px_start = " << px_start << " px_stop = " << px_stop
//                 << endl;
//            cout << "py_start = " << py_start << " py_stop = " << py_stop
//                 << endl;

               int px_offset = 0;
               int py_offset = 0;
               curr_tr.write_curr_subframe(
                  px_start[nx], px_stop[nx] - 1, 
                  py_start[ny], py_stop[ny] - 1,
                  px_offset, py_offset,
                  curr_tile_filename, outside_frame_value);
            
//               cout << "i = " << i << " Exported "+curr_tile_filename << endl;

            } // loop over index ny labeling vertical tile index
         } // loop over index nx labeling horizontal tile index

      } // loop over index i labeling images

      string banner="Image tiles exported to "+output_tiles_subdir;
      outputfunc::write_banner(banner);
   } // loop over index s labeling different sized subdirs
}

