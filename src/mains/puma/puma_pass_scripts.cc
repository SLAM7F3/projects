// ==========================================================================
// Program PUMA_PASS_SCRIPTS queries the user to enter a particular PUMA
// pass.  It first purges any previously-exiting
// bundler_IO_subdir corresponding to the specified PUMA pass.
// PUMA_PASS_SCRIPTS then unpacks a "minimal" tarball containing just
// raw FLIR video frames plus an excised camera metadata text file.
// Finally, it generates run_generate_peter_inputs,
// run_visualize_FLIR_metadata and run_crop_analog_frames scripts for
// the specified PUMA pass.
// ==========================================================================
// Last updated on 9/11/13; 9/16/13; 12/4/13
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
// a previous Puma demo run:

   sysfunc::kill_process("flowdiags");
   sysfunc::kill_process("display");

   string Puma_pass_string="May30_2013/Day1_flt2/VSFM_subset/";
//   cout << "Enter Puma pass (e.g. May30_2013/Day1_flt2/VSFM_subset/" << endl;
//   cin >> Puma_pass_string;

   string mains_dir="~/programs/c++/svn/projects/src/mains/";
   string aerosynth_subdir=mains_dir+"aerosynth/";
   string puma_subdir=mains_dir+"puma/";
   string photosynth_subdir=mains_dir+"photosynth/";

// Export run_generate_peter_inputs script:

   string filename="RUN_generate_peter_inputs";
   ofstream outstream;
   filefunc::openfile(filename,outstream);
   outstream << puma_subdir+"generate_peter_inputs ";
   outstream << "Puma/"+Puma_pass_string << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   string banner="Exported "+filename;
   outputfunc::write_banner(banner);

/*
// Export run_photo_sizes script:

   filename="RUN_photo_sizes";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"photo_sizes ";
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_thumbnails script:

   filename="RUN_thumbnails";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"thumbnails ";
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_mini_convert script:

   filename="RUN_mini_convert";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"mini_convert ";
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);


// Export run_bundler_photos script:

   filename="RUN_bundler_photos";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"bundler_photos ";
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

// Export run_write_viewbundler script:

   filename="RUN_write_viewbundler_script";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"write_viewbundler_script ";
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);
   
// Export run_viewbundler script:

   filename="RUN_viewbundler";
   filefunc::openfile(filename,outstream);
   outstream << photosynth_subdir+"viewbundler \\" << endl;
   outstream << "./bundler/Puma/"+Puma_pass_string+
      "/thresholded_xyz_points.osga  \\" << endl;
   outstream << "./bundler/Puma"+Puma_pass_string+
      "/reconstructed_camera_posns.osga \\" << endl;
   outstream << "--height_colormap 11 \\" << endl;
   outstream << "--image_list_filename ./bundler/Puma/"+Puma_pass_string+
      "/image_list.dat \\" << endl;
   outstream << "--image_sizes_filename ./bundler/Puma/"+Puma_pass_string+
      "/image_sizes.dat \\" << endl;
   outstream << "--initial_mode Manipulate_Fused_Data_Mode " << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);
*/

// Export run_densecloud script:

   filename="RUN_densecloud";
   filefunc::openfile(filename,outstream);
   outstream << puma_subdir+"densecloud \\" << endl;
   outstream << "--region_filename ./bundler/Puma/"+
      Puma_pass_string+"/packages/peter_inputs.pkg" << endl;
   filefunc::closefile(filename,outstream);
   filefunc::make_executable(filename);
   banner="Exported "+filename;
   outputfunc::write_banner(banner);

   banner="Chant run_Puma_demo in order to start Puma demonstration";
   outputfunc::write_big_banner(banner);
}

