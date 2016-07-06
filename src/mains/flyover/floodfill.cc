// ==========================================================================
// Program FLOODFILL imports some aerial flyover image.  It first
// median filters the input image in order to reduce high spatial
// frequency components in water regions.  FLOODFILL next performs
// flood-filling for multiple seed locations which were
// manually or automatically preselected.  Flood filling terminates
// whenever a candidate pixel patch has 1st and 2nd RGB moments that
// differ too much from the starting pixel patch's.  It also
// terminates when a candidate pixel patch's moments are too different
// from its predecessor neighbors.

// FLOODFILL exports the final flooded image along with a montage of
// the original and flooded images.


//				./floodfill

// ==========================================================================
// Last updated on 2/18/14; 2/19/14; 2/20/14; 2/21/14; 4/5/14
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>
#include "image/binaryimagefuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "geometry/plane.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<string> image_basenames;

   image_basenames.push_back("Boston/resized_images/shore1.jpg");
   image_basenames.push_back("Boston/resized_images/shore3.jpg");
   image_basenames.push_back("Boston/resized_images/marina_1.jpg");
   image_basenames.push_back("Boston/resized_images/marina_2.jpg");
   image_basenames.push_back("Boston/resized_images/marina_3.jpg");
   image_basenames.push_back("Boston/resized_images/marina_4.jpg");
   image_basenames.push_back("Boston/resized_images/marina_5.jpg");

   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam1.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam2.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam4.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam5.JPG");

   image_basenames.push_back(
      "HalfMoonBay/resized_images/009014-080712000452-Cam6.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009853-080712002012-Cam8.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009816-080712002043-Cam6.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009857-080712002009-Cam7.JPG");

   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam1.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam3.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam5.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam7.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam9.JPG");

   image_basenames.push_back(
      "Toronto/resized_images/036838-070512135926-Cam9.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042750-072912155521-Cam1.JPG");
   
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam1.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam2.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam3.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam4.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam5.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam6.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam7.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam8.JPG");

   image_basenames.push_back("Sam/resized_images/input0.JPG");
   image_basenames.push_back("Sam/resized_images/input1.JPG");
   image_basenames.push_back("Sam/resized_images/input2.JPG");
   image_basenames.push_back("Sam/resized_images/input4.JPG");
   image_basenames.push_back("Sam/resized_images/input5.JPG");
   image_basenames.push_back("Sam/resized_images/input6.JPG");
   image_basenames.push_back("Sam/resized_images/input7.JPG");
   image_basenames.push_back("Sam/resized_images/input8.JPG");

   for (unsigned int i=0; i<image_basenames.size(); i++)
   {
      cout << i << " --> " << filefunc::getbasename(image_basenames[i])
           << endl;
   }
   cout << endl;

   cout << "Enter input image index:" << endl;
   int image_index;
   cin >> image_index;

   
   double global_mu_threshold=45;
//   double global_mu_threshold=40;
//   double global_mu_threshold=30;
//   cout << "Enter global_threshold:" << endl;
//   cin >> global_mu_threshold;

   double local_mu_threshold=15;
//   double local_mu_threshold=10;
//   double local_mu_threshold=5;
//   cout << "Enter local_mu_threshold:" << endl;
//   cin >> local_mu_threshold;

//   double global_sigma_threshold=10;
   double global_sigma_threshold=6;
//   double global_sigma_threshold=4;
   cout << "Enter global_sigma_threshold:" << endl;
   cin >> global_sigma_threshold;

//   double local_sigma_threshold=5;
   double local_sigma_threshold=3;
//   double local_sigma_threshold=2;
   cout << "Enter local_sigma_threshold:" << endl;
   cin >> local_sigma_threshold;

   timefunc::initialize_timeofday_clock();      


   string images_subdir="./images/";
   string image_filename=images_subdir+image_basenames[image_index];
   cout << "image_filename = " << image_filename << endl;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* filtered_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->import_photo_from_file(image_filename);
   filtered_texture_rectangle_ptr->import_photo_from_file(image_filename);

   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();
   cout << "width = " << width
        << " height = " << height << endl;
   cout << "width*height = " << width*height << endl;

   twoDarray* segmentation_mask_twoDarray_ptr=new twoDarray(width,height);
   segmentation_mask_twoDarray_ptr->clear_values();

   twoDarray* mu_R_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* mu_G_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* mu_B_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* sigma_R_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* sigma_G_twoDarray_ptr=new twoDarray(width,height);
   twoDarray* sigma_B_twoDarray_ptr=new twoDarray(width,height);

   mu_R_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);

// Import manually selected features which emulate those automatically
// generated via Sam Friedman's codes:

   string features_subdir=filefunc::getdirname(image_filename);
   string prefix=filefunc::getprefix(image_filename);
//   cout << "prefix = " << prefix << endl;
   string features_filename=features_subdir+"features_2D_"+prefix+".txt";
//   cout << "features_filename = " << features_filename << endl;
   if (!filefunc::fileexist(features_filename))
   {
      cout << "No file containing features found in features_filename = "
           << features_filename << endl;
      exit(-1);
   }
   
   vector<vector<double> > row_values=filefunc::ReadInRowNumbers(
      features_filename);

   vector<twovector> seed_posns;
   for (unsigned int r=0; r<row_values.size(); r++)
   {
      double curr_u=row_values[r].at(3);
      double curr_v=row_values[r].at(4);

      unsigned int pu,pv;
      texture_rectangle_ptr->get_pixel_coords(curr_u,curr_v,pu,pv);
      seed_posns.push_back(twovector(pu,pv));
   }
 
   bool include_alpha_channel_flag=false;
   RGBA_array curr_RGBA_array=texture_rectangle_ptr->get_RGBA_twoDarrays(
      include_alpha_channel_flag);

// Experiment with median filtering to reduce high-spatial frequency
// fluctuations in water regions due to solar glints:

   int nsize=3;
//   int n_median_filtering_iters=1;
   int n_median_filtering_iters=3;

   for (int mfi=0; mfi<n_median_filtering_iters; mfi++)
   {
      cout << "Median filtering iteration " << mfi << endl;
      imagefunc::median_filter(nsize,curr_RGBA_array.first);
      imagefunc::median_filter(nsize,curr_RGBA_array.second);
      imagefunc::median_filter(nsize,curr_RGBA_array.third);
   }
   texture_rectangle_ptr->set_from_RGB_twoDarrays(curr_RGBA_array);

   string filtered_image_filename="filtered_image.jpg";
   texture_rectangle_ptr->write_curr_frame(filtered_image_filename);
   string banner="Exported filtered image to "+filtered_image_filename;
   outputfunc::write_big_banner(banner);

// Store median filtered RGB values within *filtered_texture_rectangle_ptr:

   filtered_texture_rectangle_ptr->set_from_RGB_twoDarrays(
      curr_RGBA_array);


   int flood_R=255;
   int flood_G=0;
   int flood_B=255;
   for (unsigned int seed=0; seed < seed_posns.size(); seed++)
   {
      int pu=seed_posns[seed].get(0);
      int pv=seed_posns[seed].get(1);
      cout << "pu = " << pu << " pv = " << pv << endl;

// For each new seed, reset *texture_rectangle_ptr to median filtered
// RGB values:

      texture_rectangle_ptr->set_from_RGB_twoDarrays(curr_RGBA_array);

// Verify seed pixel RGB doesn't equal flood RGB to avoid entering
// into infinite loop:

      int init_R,init_G,init_B;
      texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,init_R,init_G,init_B);
      if (init_R == flood_R && init_G == flood_G && init_B == flood_B) 
         continue;

      int bbox_size=7;
      texture_rectangle_ptr->floodFill(
         filtered_texture_rectangle_ptr,
         segmentation_mask_twoDarray_ptr,
         mu_R_twoDarray_ptr,
         mu_G_twoDarray_ptr,
         mu_B_twoDarray_ptr,
         sigma_R_twoDarray_ptr,
         sigma_G_twoDarray_ptr,
         sigma_B_twoDarray_ptr,
	 pu,pv,flood_R,flood_G,flood_B,
         bbox_size,local_mu_threshold,global_mu_threshold,
         local_sigma_threshold,global_sigma_threshold);

      int n_segmented_pixels=imagefunc::count_pixels_above_zmin(
         0.5,segmentation_mask_twoDarray_ptr);
      double cum_filled_frac=double(n_segmented_pixels)/(width*height);
      cout << "Cum filled pixels = " << n_segmented_pixels 
           << " Cum filled fraction = " << cum_filled_frac << endl;

   } // loop over seed index

// Perform 2D recursive emptying and filling in order to eliminate
// small islands within binary mask:

   cout << "Recursively emptying black islands surrounded by white oceans:" 
        << endl;

   int n_recursion_iters=25;
   binaryimagefunc::binary_reverse(segmentation_mask_twoDarray_ptr);
   recursivefunc::recursive_empty(
      n_recursion_iters,segmentation_mask_twoDarray_ptr,false);
   binaryimagefunc::binary_reverse(segmentation_mask_twoDarray_ptr);

// Recolor flooded pixels union within *texture_rectangle_ptr:

   texture_rectangle_ptr->import_photo_from_file(image_filename);

   int R,G,B;
   for (int px=0; px<width; px++)
   {
      for (int py=0; py<height; py++)
      {
         texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
         if (segmentation_mask_twoDarray_ptr->get(px,py) > 0.5)
         {
            texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,flood_R,flood_G,flood_B);
         }
      }
   }

// Mark seed locations with colored circles:

   int pixel_radius=5;
   for (unsigned int seed=0; seed < seed_posns.size(); seed++)
   {
      int pu=seed_posns[seed].get(0);
      int pv=seed_posns[seed].get(1);
      texture_rectangle_ptr->fill_circle(
         pu,pv,pixel_radius,colorfunc::darkpurple);
   }
   
   string flooded_image_filename="flooded_image.jpg";
   texture_rectangle_ptr->write_curr_frame(flooded_image_filename);
   banner="Exported flood-filled image to "+flooded_image_filename;
   outputfunc::write_big_banner(banner);

// Export montage displaying raw vs segmented images side-by-side:

   unsigned int max_xdim=500;
   unsigned int max_ydim=500;
   string downsized_image_filename=prefix+".jpg";
   unsigned int new_xdim,new_ydim;
   videofunc::downsize_image(
      image_filename,max_xdim,max_ydim,
      downsized_image_filename,new_xdim,new_ydim);

   string unix_cmd="montageview "+downsized_image_filename+" "+
      flooded_image_filename+" NO_DISPLAY";
   sysfunc::unix_command(unix_cmd);

   string montages_subdir=filefunc::getdirname(image_filename);
   unix_cmd="mv montage*.jpg "+montages_subdir;
   sysfunc::unix_command(unix_cmd);

   filefunc::deletefile(downsized_image_filename);
   banner="Exported montage of raw and flooded images to "+montages_subdir;
   outputfunc::write_big_banner(banner);

   delete texture_rectangle_ptr;
   delete filtered_texture_rectangle_ptr;
   delete segmentation_mask_twoDarray_ptr;
   delete mu_R_twoDarray_ptr;
   delete mu_G_twoDarray_ptr;
   delete mu_B_twoDarray_ptr;
   delete sigma_R_twoDarray_ptr;
   delete sigma_G_twoDarray_ptr;
   delete sigma_B_twoDarray_ptr;

   cout << "At end of program FLOODFILL" << endl;
   outputfunc::print_elapsed_time();
}

