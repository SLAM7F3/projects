// ==========================================================================
// Program IMAGE_READER is a playground which reads the current set of
// images within a specified subdirectory.  It moves all images except
// the latest one to an archive subdirectory which is time-stamped.
// We wrote this playground program to simulate TANK_SIGNS and
// PG_RECOG taking input from some hardwired subdirectory where TOC12
// PointGrey will be dumped.
// ==========================================================================
// Last updated on 11/4/12; 1/24/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <unistd.h>
#include  <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/sysfuncs.h"
#include "classification/signrecogfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string tanksigns_images_subdir="./images/tank_signs/";
   string input_images_subdir=tanksigns_images_subdir+"camera_images/";
   string archived_images_subdir=
      signrecogfunc::generate_timestamped_archive_subdir(
         tanksigns_images_subdir);


   string latest_filename,next_to_latest_filename;
   while (true)
   {
      sleep(10);

      string next_to_latest_filename=
         signrecogfunc::archive_all_but_latest_image_files(
            input_images_subdir,archived_images_subdir);
      cout << "New name for next-to-latest image = "
           << next_to_latest_filename << endl;
    
   } // infinite while loop
   
}

