// ========================================================================
// Program GENERATE_WISP_MOVIE reads in a set of WISP panorama jpg
// files from a specified subdirectory.  It converts each
// single-channel greyscale input image into a 3-channel RGB output
// image.  The horizontal pixel size of each output jpg file is
// restricted to be less than 4000.  The resulting downsized, colored
// WISP panoramas may subsequently be converted into a movie via
// mkmpeg4.
// ========================================================================
// Last updated on 2/15/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string DIME_dir="/data_third_disk/DIME/";
   string panos_dir=DIME_dir+"panoramas/jpg_files/";
   vector<string> pano_filenames=filefunc::image_files_in_subdir(panos_dir);

   int width=20000;
   int height=1100;
   int n_images=1;
   int n_channels=1;
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      width,height,n_images,n_channels,NULL);

   texture_rectangle* colored_texture_rectangle_ptr=new texture_rectangle(
      width,height,n_images,3,NULL);
   string blank_filename="blank.jpg";
   colored_texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,128);

   int n_panos=pano_filenames.size();
   for (int i=0; i<n_panos; i++)
   {
      texture_rectangle_ptr->import_photo_from_file(pano_filenames[i]);

      texture_rectangle_ptr->fill_ptwoDarray_from_single_channel_byte_data();
      twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();

      bool randomize_blue_values_flag=false;      
      colored_texture_rectangle_ptr->
         convert_single_twoDarray_to_three_channels(
            ptwoDarray_ptr,randomize_blue_values_flag);
      colored_texture_rectangle_ptr->convert_grey_values_to_hues();
    
      string pano_basename=filefunc::getbasename(pano_filenames[i]);
      string dirname=filefunc::getdirname(pano_filenames[i]);

      string recolored_pano_filename=dirname+"colored_"+pano_basename;
      colored_texture_rectangle_ptr->write_curr_frame(recolored_pano_filename);

      int max_xdim=4000;
      int max_ydim=4000;
      videofunc::downsize_image(recolored_pano_filename,max_xdim,max_ydim);

      string banner="Exported "+recolored_pano_filename;
      outputfunc::write_banner(banner);
   }
   delete texture_rectangle_ptr;
   delete colored_texture_rectangle_ptr;
}
