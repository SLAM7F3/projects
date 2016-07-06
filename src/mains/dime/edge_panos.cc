// ========================================================================
// Program EDGE_PANOS reads in "bundler-style" image_list and
// image_sizes files for set of 360 deg panorama mosaics.  It calls
// ImageMagick's edge command in order to extract high-frequency
// spatial content from each WISP panorama.  The edge panoramas are
// exported to edge_pics_subdir.

//				./edge_panos

// ========================================================================
// Last updated on 7/9/13; 7/11/13; 7/14/13; 8/8/13; 8/12/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
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

//   int scene_ID=18;
   int scene_ID=19;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string edge_pics_subdir=bundler_IO_subdir+"stable_frames/edge_pics/";
   filefunc::dircreate(edge_pics_subdir);

   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";

   timefunc::initialize_timeofday_clock();
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_photos(photogroup_ptr->get_n_photos());
   cout << "n_photos = " << n_photos << endl;

   int n_start=0;
   cout << "Enter starting photo number:" << endl;
//   cin >> n_start;

   int n_stop=n_photos-1;
   cout << "Enter stopping photo_number:" << endl;
//   cin >> n_stop;

   for (int n=n_start; n<=n_stop; n++)
   {
      double progress_frac=double(n-n_start)/double(n_stop-n_start);
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string filename=photo_ptr->get_filename();
      string basename=filefunc::getbasename(filename);
      string prefix=stringfunc::prefix(basename);
      string edge_pic_filename=edge_pics_subdir+"edges_"+prefix+".png";

      string unix_cmd="convert "+filename+" -edge 2 "+edge_pic_filename;
//      cout << unix_cmd << endl;
      string banner="High-pass filtering "+basename;
      outputfunc::write_banner(banner);
      sysfunc::unix_command(unix_cmd);
   } // loop over index n labeling photos

   string n_panos_str=stringfunc::number_to_string(n_stop-n_start+1);
   string banner=n_panos_str+" high-pass filtered panoramas written to "+
      edge_pics_subdir;
   outputfunc::write_banner(banner);

   banner="Finished running program EDGE_PANOS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

