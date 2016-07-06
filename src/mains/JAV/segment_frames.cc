// =======================================================================
// Program SEGMENT_FRAMES 
// =======================================================================
// Last updated on 10/17/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
// #include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/image_transforms.h>

#include "general/filefuncs.h"
#include "math/lttriple.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   string videos_subdir="./jpg_files/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      videos_subdir);
//   string substring="clip_";
//   vector<string> video_filenames=
//      filefunc::files_in_subdir_matching_substring(videos_subdir,substring);
//   filefunc::dircreate(videos_subdir+"jpg_frames/");

   typedef std::pair<int,int> DUPLE;
   typedef map<triple,vector<DUPLE>,lttriple> SEGMENTS_MAP;
// independent var: triple of RGB integers
// dependent var: STL vector of (pu,pv) pairs

   SEGMENTS_MAP segments_map;
   SEGMENTS_MAP::iterator iter;

   for (int i=0; i<image_filenames.size(); i++)
   {
      string image_filename=image_filenames[i];

      string image_basename=filefunc::getbasename(image_filename);      
      string image_prefix=stringfunc::prefix(image_basename);
//      string image_prefix=stringfunc::prefix(image_filename);

      string image_dirname=filefunc::getdirname(image_filename);

      string blurred_image_dirname=image_dirname+"blurred_images/";
      filefunc::dircreate(blurred_image_dirname);
      string segments_image_dirname=image_dirname+"segmented_images/";
      filefunc::dircreate(segments_image_dirname);
      string filtered_image_dirname=image_dirname+"filtered_images/";
      filefunc::dircreate(filtered_image_dirname);
      
      string ppm_image_filename=image_prefix+".ppm";
      string ppm_segments_filename=image_prefix+"_segments.ppm";

/*
// Blur input image:

      dlib::array2d<dlib::rgb_pixel> img,blurred_img;

      cout << "image_filename = " << image_filename << endl;
      dlib::load_image(img,image_filename.c_str());

//      double sigma=2;
      double sigma=3.5;
      dlib::gaussian_blur(img,blurred_img,sigma); 

      string blurred_image_filename="blurred_"+image_prefix+".png";
      cout << "blurred_image_filename = " << blurred_image_filename << endl;
      dlib::save_png(blurred_img,blurred_image_filename.c_str());

//      cout << "ppm_image_filename = " << ppm_image_filename << endl;
//      string unix_cmd="convert "+blurred_image_filename+" "+ppm_image_filename;


      string unix_cmd="mv "+blurred_image_filename+" "+blurred_image_dirname;
      sysfunc::unix_command(unix_cmd);
*/

      string unix_cmd="convert "+image_filename+" "+ppm_image_filename;
      sysfunc::unix_command(unix_cmd);

//   unix_cmd="Felzenszwalb_segment 1.5 600 50 "+ppm_image_filename+" "+
      unix_cmd="Felzenszwalb_segment 2 500 50 "+ppm_image_filename+" "+
         ppm_segments_filename;
      sysfunc::unix_command(unix_cmd);

      string segments_filename=stringfunc::prefix(ppm_segments_filename)
         +".jpg";
      unix_cmd="convert "+ppm_segments_filename+" "+segments_filename;
      sysfunc::unix_command(unix_cmd);

// Converted segmented image into STL vector of pixel segments:

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         segments_filename,NULL);
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
      int n_pixels=width*height;
//   cout << "width = " << width << " height = " << height << endl;

      int R,G,B;
      segments_map.clear();
      for (int py=0; py<height; py++)
      {   
         for (int px=0; px<width; px++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            DUPLE curr_duple(px,py);
            triple curr_RGB(R,G,B);

//         cout << "px = " << px << " py = " << py 
//              << " RGB = " << curr_RGB
//              << " R = " << R << " G = " << G << " B = " << B
//              << endl;
         
            iter=segments_map.find(curr_RGB);
            if (iter==segments_map.end())
            {
               vector<DUPLE> V;
               V.push_back(curr_duple);
               segments_map[curr_RGB]=V;
            }
            else
            {
               iter->second.push_back(curr_duple);
            }
         } // loop over px
      } // loop over py   

//   cout << "Initial segments_map.size() = " << segments_map.size() << endl;

      vector<int> segment_sizes;
      vector< vector<DUPLE> > segments;
   
      int min_segment_size=1000;
      for (iter=segments_map.begin(); iter != segments_map.end(); iter++)
      {
         int curr_segment_size=iter->second.size();
         if (curr_segment_size > min_segment_size)
         {
            segment_sizes.push_back(curr_segment_size);
            segments.push_back(iter->second);
         }
      } // loop over segments_map iterator

      templatefunc::Quicksort_descending(segment_sizes,segments);

      cout << "Nontrivial segment_sizes.size() = " 
           << segment_sizes.size() << endl;

// Reload original image into texture_rectangle:

      texture_rectangle_ptr->import_photo_from_file(image_filename);
      texture_rectangle* new_texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);
      new_texture_rectangle_ptr->clear_all_RGB_values();

      double pixel_frac=0;
//   double pixel_frac_threshold=0.66;
      double pixel_frac_threshold=0.75;

      for (int s=0; s<segment_sizes.size(); s++)
      {
         cout << "s = " << s << " segment_size = " << segment_sizes[s] << endl;
         pixel_frac += double(segment_sizes[s])/n_pixels;
         if (pixel_frac > pixel_frac_threshold) break;

         for (int t=0; t<segments[s].size(); t++)
         {
            int pu=segments[s].at(t).first;
            int pv=segments[s].at(t).second;
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            new_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
         }
      }
   
      string filtered_image_filename=filtered_image_dirname+"filtered_"+
         image_basename;
      new_texture_rectangle_ptr->write_curr_frame(filtered_image_filename);
      string banner="Exported "+filtered_image_filename;
      outputfunc::write_banner(banner);

      unix_cmd="mv "+segments_filename+" "+segments_image_dirname;
      sysfunc::unix_command(unix_cmd);

      filefunc::deletefile(ppm_image_filename);
      filefunc::deletefile(ppm_segments_filename);

      delete texture_rectangle_ptr;
      delete new_texture_rectangle_ptr;
   } // loop over index i labeling input image filenames

}

