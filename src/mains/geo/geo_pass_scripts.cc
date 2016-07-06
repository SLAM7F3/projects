// ==========================================================================
// Program GEO_PASS_SCRIPTS queries the user to enter a particular GEO
// pass (e.g. 20120105_1402).  It first purges any previously-exiting
// bundler_IO_subdir corresponding to the specified GEO pass.
// GEO_PASS_SCRIPTS then unpacks a "minimal" tarball containing just
// raw FLIR video frames plus an excised camera metadata text file.
// Finally, it generates run_generate_peter_inputs,
// run_visualize_FLIR_metadata and run_crop_analog_frames scripts for
// the specified GEO pass.
// ==========================================================================
// Last updated on 8/27/13; 9/11/13; 9/16/13; 11/12/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "messenger/Messenger.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// --------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   cout.precision(12);

// First kill any flowdiag processes that may have been left over from
// a previous GEO demo run:

   sysfunc::kill_process("flowdiags");
   sysfunc::kill_process("display");

   string GEO_pass_string;
   cout << "Enter GEO pass (e.g. 20120105_1402, 20120105_1438, 20110728_0905, 20120105_1443, 20110728_1509)" 
        << endl;
//   cout << "Enter GEO pass (e.g. 20120105_1443, 20110728_0905, 20110728_1509, 20120105_1402, 20120105_1438)" << endl;
   cin >> GEO_pass_string;

   string projects_rootdir = getenv("PROJECTSROOT");
   string mains_dir=projects_rootdir+"/src/mains/";
   string aerosynth_subdir=mains_dir+"aerosynth/";
   string geo_subdir=mains_dir+"geo/";
   string photosynth_subdir=mains_dir+"photosynth/";

   string bundler_subdir=photosynth_subdir+"bundler/";
   string bundler_GEO_subdir=bundler_subdir+"GEO/";
   string bundler_GEOpass_subdir=bundler_GEO_subdir+GEO_pass_string;
   cout << "bundler_GEOpass_subdir = " 
        << bundler_GEOpass_subdir << endl;

// Purge any existing content within bundler_GEOpass_subdir:

   string unix_cmd="/bin/rm -r -f "+bundler_GEOpass_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="tar xvzf "+bundler_GEO_subdir+GEO_pass_string+"_minimal.tgz";
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv "+GEO_pass_string+" "+bundler_GEO_subdir;
   sysfunc::unix_command(unix_cmd);

// Export run_generate_peter_inputs script:

   string filename="RUN_generate_peter_inputs";
   ofstream outstream;
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"generate_peter_inputs ";
   outstream << "GEO/"+GEO_pass_string << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   string banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_visualize_FLIR_metadata script:

   filename="RUN_visualize_FLIR_metadata";
   filefunc::openfile(filename,outstream);
   outstream << "../aerosynth/visualize_FLIR_metadata \\" << endl;
   outstream << "--region_filename dummy.osga \\" << endl;
   outstream << "--start_frame_ID -100 \\" << endl;
   outstream << "--stop_frame_ID -100 \\" << endl;
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_crop_analog_frames script:

   filename="RUN_crop_analog_frames";
   filefunc::openfile(filename,outstream);
   outstream << "./crop_analog_frames \\" << endl;
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_photo_sizes script:

   filename="RUN_photo_sizes";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"photo_sizes ";
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_thumbnails script:

   filename="RUN_thumbnails";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"thumbnails ";
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_mini_convert script:

   filename="RUN_mini_convert";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"mini_convert ";
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_bundler_photos script:

   filename="RUN_bundler_photos";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"bundler_photos ";
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_write_viewbundler script:

   filename="RUN_write_viewbundler_script";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"write_viewbundler_script ";
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_viewbundler script:

   filename="RUN_viewbundler";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"viewbundler \\" << endl;
   outstream << "./bundler/GEO/"+GEO_pass_string+
      "/thresholded_xyz_points.osga  \\" << endl;
   outstream << "./bundler/GEO"+GEO_pass_string+
      "/reconstructed_camera_posns.osga \\" << endl;
   outstream << "--height_colormap 11 \\" << endl;
   outstream << "--image_list_filename ./bundler/GEO/"+GEO_pass_string+
      "/image_list.dat \\" << endl;
   outstream << "--image_sizes_filename ./bundler/GEO/"+GEO_pass_string+
      "/image_sizes.dat \\" << endl;
   outstream << "--initial_mode Manipulate_Fused_Data_Mode " << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_densecloud script:

   filename="RUN_densecloud";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"densecloud \\" << endl;
   outstream << "--region_filename ./bundler/GEO/"+
      GEO_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

   banner="Chant run_GEO_demo in order to start GEO demonstration";
   outputfunc::write_big_banner(banner);
}

