// ====================================================================
// Program RECOMBINE_SEGMENTED_TILES imports a list of input image
// files from a specified subdirectory. It also imports all their
// segmented image tiles from another specified subdirectory.  For
// each input image, RECOMBINE_SEGMENTED_TILES stitches together its
// segmented label and scores tiles into single mosaics.  The
// reconstructed segmentation label and scores images along with their
// montages are exported to specified output subdirectories.

// recombine_segmented_tiles /home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_05/halfsized

// ====================================================================
// Last updated on 5/20/16; 5/21/16; 6/16/16; 6/21/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
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
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

   string banner="Starting RECOMBINE_SEGMENTED_TILES program";
   outputfunc::write_big_banner(banner);

   const int tile_border = 100; // pixels

   if(argc != 2)
   {
      cout << "Must pass input_images_subdir (e.g. '/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_05/halfsized') as command-line argument" << endl;
      exit(-1);
   }
   string input_images_subdir(argv[1]);
   filefunc::add_trailing_dir_slash(input_images_subdir);

   string input_tiles_subdir=input_images_subdir+"tiles/segmentation_results/";
   string output_images_subdir=input_images_subdir+"recombined_segmentations/";
   string score_images_subdir=output_images_subdir+"segmentation_scores/";
   string montage_images_subdir=output_images_subdir+"montages/";
   filefunc::dircreate(output_images_subdir);
   filefunc::dircreate(score_images_subdir);
   filefunc::dircreate(montage_images_subdir);

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir);
   int n_images = image_filenames.size();
   cout << "input_images_subdir = " << input_images_subdir << endl;
   cout << "n_images = " << n_images << endl;

   typedef std::map<int, string > IMAGE_FILENAMES_MAP;
// indepedent int: image ID
// dependent string : path to original homogenized image

   IMAGE_FILENAMES_MAP image_filenames_map;
   IMAGE_FILENAMES_MAP::iterator image_filenames_map_iter;

   for(int i = 0; i < n_images; i++)
   {
      string basename=filefunc::getbasename(image_filenames[i]);
      string separator_chars="_.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
      string number_str="";
            for(unsigned int s= 0; s < substrings.size(); s++)
      {
         if(stringfunc::is_number(substrings[s]))
         {
            number_str=substrings[s];
            break;
         }
      }

      int image_ID = stringfunc::string_to_number(number_str);
      image_filenames_map[image_ID] = image_filenames[i];
//      cout << "i = " << i << " image_ID = " << image_ID
//           << " image_filenames[i] = " << image_filenames[i] << endl;
   }

   vector<string> image_tile_filenames = filefunc::image_files_in_subdir(
     input_tiles_subdir);
//   cout << "image_tile_filenames.size() = " << image_tile_filenames.size()
//        << endl;

   typedef std::pair<int,int> INT_PAIR;
   typedef std::map<int, std::vector<INT_PAIR> > TILES_MAP;
// indepedent int: image index
// dependent vector< pair<int,int> > : horizontal & vertical tile indices   
//                                     for a particular image
   TILES_MAP tiles_map;
   TILES_MAP::iterator tiles_map_iter;
   
   unsigned int istart = 0;
   unsigned int istop = image_tile_filenames.size();

// First fill hashmap with tile indices as a function of input image
// index:

//   cout << "istop = " << istop << endl;
   for (unsigned int i = istart; i < istop; i++)
   {
      string currtile_filename = image_tile_filenames[i];
      string curr_basename=filefunc::getbasename(currtile_filename);
      string separator_chars="_";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         curr_basename,separator_chars);
      int image_ID = stringfunc::string_to_number(substrings[1]);
      int tile_x_ID = stringfunc::string_to_number(substrings[2]);
      int tile_y_ID = stringfunc::string_to_number(substrings[3]);
//      cout << "i = " << i << " curr_basename = " << curr_basename << endl;
//      cout << "   image_ID = " << image_ID
//           << " tile_x_ID = " << tile_x_ID
//           << " tile_y_ID = " << tile_y_ID << endl;

      INT_PAIR P;
      P.first = tile_x_ID;
      P.second = tile_y_ID;

      tiles_map_iter = tiles_map.find(image_ID);
      if(tiles_map_iter == tiles_map.end())
      {
         vector<INT_PAIR> V;
         V.push_back(P);
         tiles_map[image_ID] = V;
      }
      else
      {
         vector<INT_PAIR>* V_ptr = &tiles_map_iter->second;
         V_ptr->push_back(P);
      }
   } // loop over index i labeling input tilenames

   int counter = 0;
   for(tiles_map_iter=tiles_map.begin(); tiles_map_iter != tiles_map.end();
       tiles_map_iter++)
   {
      counter++;
      double progress_frac = double(counter)/tiles_map.size();
      if(counter% 10 == 0)
      {
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      int image_ID = tiles_map_iter->first;
      vector<INT_PAIR>* V_ptr = &tiles_map_iter->second;
      
      string image_ID_str = stringfunc::integer_to_string(image_ID,5);
//      cout << "image_ID_str = " << image_ID_str << endl;

      image_filenames_map_iter = image_filenames_map.find(image_ID);
      if(image_filenames_map_iter == image_filenames_map.end()) 
      {
        cout << "No image corresponding to image_ID = " << image_ID
             << " found! " << endl;
        continue;
      }
      
      string curr_image_filename=image_filenames_map_iter->second;
//      cout << "curr_image_filename = " << curr_image_filename << endl;
      unsigned int image_xdim, image_ydim;
      imagefunc::get_image_width_height(
         curr_image_filename,image_xdim, image_ydim);
//      cout << "image_xdim = " << image_xdim << " image_ydim = " << image_ydim
//           << endl;

      int nx_max = 0;
      int ny_max = 0;
      for(unsigned int i = 0; i < V_ptr->size(); i++)
      {
         INT_PAIR P=V_ptr->at(i);
         int nx = P.first;
         int ny = P.second;
         nx_max = basic_math::max(nx_max, nx);
         ny_max = basic_math::max(ny_max, ny);
      }
//      cout << "nx_max = " << nx_max << " ny_max = " << ny_max << endl;
  
      texture_rectangle total_labels_image(curr_image_filename,NULL);
      texture_rectangle total_scores_image(curr_image_filename,NULL);

// Recall ny = 0 corresponds to BOTTOM row of tiles!

      for(int ny = 0; ny <= ny_max; ny++)
      {
         for(int nx = 0; nx <= nx_max; nx++)
         {
            string nx_label = stringfunc::integer_to_string(nx, 2);
            string ny_label = stringfunc::integer_to_string(ny, 2);
            string curr_tile_labels_filename = input_tiles_subdir+"tile_"
               +image_ID_str+"_"+nx_label+"_"+ny_label+"_segmented.png";
            string curr_tile_scores_filename = input_tiles_subdir+"tile_"
               +image_ID_str+"_"+nx_label+"_"+ny_label+"_score.png";

//            cout << "nx = " << nx << " ny = " << ny
//                 << " curr_tile_labels_filename = " 
//                 << curr_tile_labels_filename 
//                 << " curr_tile_scores_filename = " 
//                 << curr_tile_scores_filename 
//                 << endl;

            texture_rectangle curr_tile_labels_tr(
               curr_tile_labels_filename,NULL);
            texture_rectangle curr_tile_scores_tr(
               curr_tile_scores_filename,NULL);

            int tile_xdim=curr_tile_labels_tr.getWidth();
            int tile_ydim=curr_tile_labels_tr.getHeight();
            int reduced_tile_xdim = tile_xdim - 2 * tile_border;
            int reduced_tile_ydim = tile_ydim - 2 * tile_border;

            for(int py = 0; py < tile_ydim; py++)
            {
               if(ny > 0 && py > tile_ydim - tile_border) continue;
               if(ny < ny_max && py < tile_border) continue;

               int qy = image_ydim - tile_ydim + py - ny * reduced_tile_ydim;
               if(qy < 0 || qy >= int(image_ydim)) continue;

// (qx,qy) = (0,0) corresponds to top left corner of total
// reconstructed image:

               for(int px = 0; px < tile_xdim; px++)
               {
                  int qx = px + nx * reduced_tile_xdim;
                  if(qx < 0 || qx >= int(image_xdim)) continue;

                  int R, G, B;
                  curr_tile_labels_tr.get_pixel_RGB_values(px, py, R, G, B);
                  total_labels_image.set_pixel_RGB_values(qx, qy, R, G, B);
                  curr_tile_scores_tr.get_pixel_RGB_values(px, py, R, G, B);
                  total_scores_image.set_pixel_RGB_values(qx, qy, R, G, B);
               } // loop over px
            } // loop over py

         } // loop over nx
      } // loop over ny

      string output_labels_filename=output_images_subdir+
         "segmented_image_"+image_ID_str+".png";
      string output_scores_filename=score_images_subdir+
         "scores_image_"+image_ID_str+".png";
      string output_montage_filename=montage_images_subdir+
         "montage_image_"+image_ID_str+".png";
      total_labels_image.write_curr_frame(output_labels_filename);
      total_scores_image.write_curr_frame(output_scores_filename);

      string unix_cmd = "montage -tile 2x1 -geometry +10+6 "+
         output_labels_filename+" "+output_scores_filename+" "+
         output_montage_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      banner="Exported labels, scores and montage for "+curr_image_filename;
      outputfunc::write_banner(banner);
   } // loop over tile_map_iter
   
}

