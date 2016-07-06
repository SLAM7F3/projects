// ==========================================================================
// Program MULTI_WEBCAMS can capture individual stills from one of multiple
// external webcams attached via USB ports to a computer.  It
// generates a time-stamped subdirectory within /data/tech_challenge/
// to hold JPEG frames captured via FFMPEG from a webcam.  FFMPEG
// assigns a unique name to each captured JPEG frame.

//			multi_webcams

// ==========================================================================
// Last updated on 6/15/10; 8/1/10; 8/23/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
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

   Clock clock;
   clock.current_local_time_and_UTC();
   
   bool display_UTC_flag=false;
   string image_subdir="/data/tech_challenge/webcam_images_"
      +clock.YYYY_MM_DD_H_M_S("_","_",display_UTC_flag)+"/";

   string command1="mkdir "+image_subdir;
//   cout << "command1 = " << command1 << endl;
   sysfunc::unix_command(command1);

   string command2a="/bin/rm /data/tech_challenge/webcam_images";
//   cout << "command2a = " << command2a << endl;
   sysfunc::unix_command(command2a);
   string command2="ln -s "+image_subdir+" /data/tech_challenge/webcam_images";
//   cout << "command2 = " << command2 << endl;
   sysfunc::unix_command(command2);

   sysfunc::clearscreen();
   cout << "Enter device name for external webcam (e.g. /dev/video1 ):"
        << endl << endl;
   
   cout << "BEWARE:  If this computer is a netbook, /dev/video0 is the built-in"
        << endl;
   cout << "and MASKED-OFF internal camera!" << endl << endl;

   string video_devicename;
   cin >> video_devicename;

   bool video_device_found_flag=filefunc::chardevexist(video_devicename);
   if (!video_device_found_flag)
   {
      cout << "No webcam detected attached to "+video_devicename << endl
           << endl;
      exit(-1);
   }
   else
   {
      string banner="Webcam detected attached to "+video_devicename;
      outputfunc::write_big_banner(banner);
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
