// ==========================================================================
// Program CAPTURE_WEBCAM_IMAGES is meant to be used for capturing
// individual stills from a SINGLE external webcam.  It generates a
// time-stamped subdirectory within /data/tech_challenge/ or on an
// mounted SD card subdirectory to hold JPEG frames captured via
// FFMPEG from a webcam.  FFMPEG assigns a unique name to each
// captured JPEG frame.

//			capture_webcam_images

// ==========================================================================
// Last updated on 8/1/10; 8/22/10; 8/27/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   sysfunc::clearscreen();
   cout << endl;
   
   string base_subdir;

// Search for a mounted SD card: (e.g. /media/CANON_DC or
// /media/34E9-289E).  If one exists, write output JPG files to its
// mount point.  Otherwise, write JPG files to subdir of
// /data/tech_challenge/ :

   vector<string> media_files=filefunc::files_in_subdir("/media/");
   for (int i=0; i<media_files.size(); i++)
   {
//      cout << "i = " << i
//           << " media file = " << media_files[i] << endl;
      string curr_subdir_name=filefunc::getbasename(media_files[i]);
//      cout << " curr_subdir_name = " << curr_subdir_name << endl;

      if (curr_subdir_name=="CANON_DC")
      {
         base_subdir=media_files[i]+"/";
         break;
      }
      else
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               curr_subdir_name,"-_");
         if (stringfunc::is_number(substrings[0]))
         {
            base_subdir=media_files[i]+"/";
            break;
         }
         else
         {
            if (media_files.size()==1)
            {
               base_subdir=media_files[i]+"/";
            }
         }
      }
   } // loop over index i labeling subdirs & files within /media

   bool SDcard_inserted_flag=false;
   if (base_subdir.size() > 0)
   {
      cout << "There is *probably* an inserted SD card mounted to "
           << base_subdir << endl;
      SDcard_inserted_flag=true;
   }
   else
   {
      base_subdir="/data/tech_challenge/";
      cout << "No inserted SD card detected..." << endl;
   }

   Clock clock;
   clock.current_local_time_and_UTC();

   
   bool display_UTC_flag=false;
   string image_subdir=base_subdir+"webcam_images_"
      +clock.YYYY_MM_DD_H_M_S("_","_",display_UTC_flag)+"/";

   cout << endl;
   cout << "Webcam images will be written as JPG files to " 
        << image_subdir << endl << endl;

   string command1="mkdir "+image_subdir;
//   cout << "command1 = " << command1 << endl;
   sysfunc::unix_command(command1);

   if (!SDcard_inserted_flag)
   {
      string command2a="/bin/rm "+base_subdir+"webcam_images";
//   cout << "command2a = " << command2a << endl;
      sysfunc::unix_command(command2a);
      string command2="ln -s "+image_subdir+" "+base_subdir+"webcam_images";
//   cout << "command2 = " << command2 << endl;
      sysfunc::unix_command(command2);
   }
   
// Search for webcam attached to /dev/videoN where N=0,1,2,...  To
// avoid attaching onto built-in netbook webcam, decrement N:

   vector<string> candidate_video_devicenames;
   candidate_video_devicenames.push_back("/dev/video4");
   candidate_video_devicenames.push_back("/dev/video3");
   candidate_video_devicenames.push_back("/dev/video2");
   candidate_video_devicenames.push_back("/dev/video1");
   candidate_video_devicenames.push_back("/dev/video0");

   bool video_device_found_flag=false;   
   string video_devicename;
   for (int i=0; i<candidate_video_devicenames.size(); i++)
   {
      string curr_video_devicename=candidate_video_devicenames[i];
      if (filefunc::chardevexist(curr_video_devicename))
      {
         video_devicename=curr_video_devicename;
         video_device_found_flag=true;
         break;
      }
   }
    
   if (!video_device_found_flag)
   {
      cout << "No webcam detected attached to /dev/videoN!" << endl;
      exit(-1);
   }
   else
   {
      string banner="Webcam detected attached to "+video_devicename;
      outputfunc::write_big_banner(banner);

      cout << "BEWARE:  If this computer is a netbook, /dev/video0 may be the built-in"
           << endl;
      cout << "and MASKED-OFF internal camera!" << endl << endl;

      cout << "After hitting return, you should start to see incrementing webcame frame" << endl;
      cout << "numbers being written to the output disk (e.g. frame=3, frame=4, frame=5, etc)." <<endl;
      cout << "If instead you see the image counter stalled on frame=1, kill this program" << endl;
      cout << "by entering control-c and restart it again.  You may need to restart" << endl;
      cout << "the program multiple times before webcam imagery will be successfully" << endl;
      cout << "recorded to disk..." << endl << endl;
      
      outputfunc::enter_continue_char();
   }

// According to a webpage, the Logitech Tessar 2.0/3.7 webcam is
// supposed to support 960x720 images at 30 fps:

   string command3="ffmpeg  -f video4linux2 -r 15 ";
   command3 += "-i "+video_devicename+" -s 960x720  -r 2 ";
//   string command3=
//      "ffmpeg  -f video4linux2 -r 15 -i /dev/video0 -s 800x600  -r 2 ";
   command3 += image_subdir+"image-%05d.jpg";
   cout << "command3 = " << command3 << endl;
   sysfunc::unix_command(command3);

}
