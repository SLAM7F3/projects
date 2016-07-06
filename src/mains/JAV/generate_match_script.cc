// ========================================================================
// Program GENERATE_MATCH_SCRIPT queries the user to enter
// bundler_IO_subdir.  It then scans the filenames for all images
// within bundler_IO_subdir/images/ .  GENERATE_MATCH_SCRIPT then
// exports an executable script for program MATCH_SUCCESSIVE_FRAMES.

//			./generate_match_script

// ========================================================================
// Last updated on 9/8/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   cout << "Enter subdirectory of photosynth/bundler/ in which images & metadata reside" << endl;
   cout << "(e.g. kermit, MIT2317, Thunderstorm/20110511_flight1)" << endl;
   cout << endl;

   string bundler_subdir;
   cin >> bundler_subdir;
   filefunc::add_trailing_dir_slash(bundler_subdir);
   string bundler_IO_subdir="./bundler/"+bundler_subdir;
   
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_list_filename=bundler_IO_subdir+"image_list.dat";

   string match_scriptname="run_match_frames";
   ofstream outstream;
   filefunc::openfile(match_scriptname,outstream);
   outstream << "./match_successive_frames \\" << endl;
   filefunc::ReadInfile(image_list_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string image_path=bundler_IO_subdir+filefunc::text_line[i];
      outstream << "--newpass "+image_path+" \\" << endl;
   }
   outstream << "--image_list_filename "+image_list_filename << endl;
   filefunc::closefile(match_scriptname,outstream);   

// Make match_frames script executable:

   filefunc::make_executable(match_scriptname);
   string banner="Exported "+match_scriptname;
   outputfunc::write_big_banner(banner);

}


