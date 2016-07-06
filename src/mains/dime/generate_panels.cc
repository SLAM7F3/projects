// ========================================================================
// Program GENERATE_PANELS reads in "bundler-style" image_list and
// image_sizes files for set of 360 deg panorama mosaics.  (The input
// panoramas may have been previously high-pass filtered via program 
// EDGE_PANOS.)  It chops up each input panorama into 10 individual
// panels which should be oriented 36 degrees apart in azimuth.  The
// individual panel images are written to a panels subdirectory of the
// images directory holding all of the input mosaics.  Each panel is
// labeled by the name of its parent panorama and a "pn" descriptor
// where n ranges from 0 to 9.

//			 ./generate_panels

// Note: As of 6/21/13, we revert to working with texture rectangles.
// Their write_curr_frame() method which involves OSG's
// writeImageFile() command appears to run significantly faster than
// ImageMagick's convert -extract command.


// Note: As of 3/29/13, we call ImageMagick's convert -extract command
// in order to pull out panels from full WISP panos.  But this does
// not appear to run significantly faster than our older approach
// using texture rectangles and ImageMagick's crop command.

// ========================================================================
// Last updated on 7/10/13; 7/11/13; 7/14/13; 8/8/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string blank_image_filename="./blank_40Kx2.2K.jpg";
   texture_rectangle* input_texture_rectangle_ptr=
      new texture_rectangle(blank_image_filename,NULL);

   string image_subdir=bundler_IO_subdir+"stable_frames/";
   string highpass_filter_char;
   cout << "Enter 'h' to generate panel images for high-pass filtered panos"
        << endl;
   cin >> highpass_filter_char;
   if (highpass_filter_char=="h")
   {
      image_subdir=bundler_IO_subdir+"stable_frames/edge_pics/";
   }

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      image_subdir);
   int n_images=image_filenames.size();
   cout << "n_images = " << n_images << endl;

   int n_start=0;
//   cout << "Enter starting image number:" << endl;
//   cin >> n_start;

   int n_stop=n_images-1;
//   cout << "Enter stopping image number:" << endl;
//   cin >> n_stop;

   timefunc::initialize_timeofday_clock();

   int xdim=40000;
   int ydim=2200;

   int n_panels=10;
   for (int n=n_start; n<=n_stop; n++)
   {
      double progress_frac=double(n-n_start)/double(n_stop-n_start);
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      string stable_image_filename=image_filenames[n];
      input_texture_rectangle_ptr->import_photo_from_file(
         stable_image_filename);

      cout << endl;
      cout << "n = " << n << " Chopping filename = " << stable_image_filename 
//           << " xdim = " << xdim
//           << " ydim = " << ydim 
           << endl;

      string subdir=filefunc::getdirname(stable_image_filename);
      string basename=filefunc::getbasename(stable_image_filename);
      string panels_subdir=subdir+"panels/";
      filefunc::dircreate(panels_subdir);
      cout << "panels_subdir = " << panels_subdir << endl;

      int p_start=0;
      int p_stop=n_panels-1;
      for (int p=p_start; p<=p_stop; p++)
      {
         int width=double(xdim)/double(n_panels);
         int height=ydim;
         int xoffset=double(p*xdim)/double(n_panels);
         int yoffset=0;

         string basename_prefix=stringfunc::prefix(basename);
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(basename_prefix,"_");
         string basename_suffix="png";

         int n_substrings=substrings.size();
//         for (int s=0; s<n_substrings; s++)
//         {
//            cout << "s = " << s 
//                 << " substrings[s] = " << substrings[s] << endl;
//         }
         
         string output_filename=panels_subdir+substrings[0]
            +"_p"+stringfunc::number_to_string(p);
         for (int s=1; s<n_substrings; s++)
         {
            output_filename += "_"+substrings[s];
         }
         output_filename += "."+basename_suffix;
//         cout << "stable_image_filename = " << stable_image_filename << endl;
//         cout << "output_filename = " << output_filename << endl;

// As of 6/21/13, we assume that if output_filename already exists, it
// does not need to be regenerated:

         if (filefunc::fileexist(output_filename))
         {
            cout << output_filename << " already exists!" << endl;
         }
         else
         {
            int px_start=xoffset;
            int px_stop=xoffset+width-1;
            int py_start=yoffset;
            int py_stop=yoffset+height;
            input_texture_rectangle_ptr->write_curr_frame(
               px_start,px_stop,py_start,py_stop,output_filename);

            cout << "Exported panel "+output_filename << endl;
         }

//         cout << "Exported panel "+stringfunc::number_to_string(p) << endl;
      } // loop over index p labeling panels
   } // loop over index n labeling photos

   delete input_texture_rectangle_ptr;

   string banner="Finished running program GENERATE_PANELS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

