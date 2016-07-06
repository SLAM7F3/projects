// ==========================================================================
// Program GENERATE_SCRIPTS queries the user to enter a subdirectory
// of ./bundler/ in which some set of tripod images and bundler output
// reside.  It exports executable scripts for several programs that
// must be run in order to georegister the tripod cameras, populate
// database tables and reconstruct 3D point clouds from calibrated
// 2D tripod camera stills.

//	         	    generate_scripts

// ==========================================================================
// Last updated on 9/18/12; 9/27/12; 1/7/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

   cout << "Enter subdirectory of photosynth/bundler/ in which images & metadata reside" << endl;
   cout << "(e.g. plume/Nov_2011/Day2B)" << endl;
   cout << endl;
   string bundler_subdir;
   cin >> bundler_subdir;
   filefunc::add_trailing_dir_slash(bundler_subdir);

   string bundler_IO_subdir="./bundler/"+bundler_subdir;
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string images_subdir=bundler_IO_subdir+"images/";
   cout << "images_subdir = " << images_subdir << endl;

   string packages_subdir=bundler_IO_subdir+"packages/";
   string peter_inputs_filename=packages_subdir+"peter_inputs.pkg";

// As of Sep 2012, we assume that fixed tripod image names start with
// their camera numbers (e.g. 17_6268.rd.jpg or
// 18_IMG_0728_DAY4D.JPG).  So we search through the images
// subdirectory and count the number of fixed tripod image files which
// start with an integer:

   int n_tripod_images=0;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   for (int i=0; i<image_filenames.size(); i++)
   {
      string image_filename=filefunc::getbasename(image_filenames[i]);
//      cout << "image_filename = " << image_filename << endl;
      string separator_chars="_";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         image_filename,separator_chars);
      if (stringfunc::is_number(substrings[0])) n_tripod_images++;
   }
   cout << "n_tripod_images = " << n_tripod_images << endl;
   
// Generate executable scripts for several main programs:

   ofstream script_stream;
   vector<string> program_filename;
   program_filename.push_back("./survey_camera_posns");
   program_filename.push_back("./georegister_cameras");
   program_filename.push_back("./thumbnails");
   program_filename.push_back("../photosynth/bundler_photos");

   for (int n=0; n<program_filename.size(); n++)
   {
      string script_filename=
         "./run_"+filefunc::getbasename(program_filename[n]);
      filefunc::openfile(script_filename,script_stream);   
      script_stream << program_filename[n]+" --region_filename "
                    << peter_inputs_filename << endl;
      filefunc::closefile(script_filename,script_stream);   

      string unix_command="chmod a+x "+script_filename;
      sysfunc::unix_command(unix_command);
   } // loop over index n labeling program filenames

// Specialize run_select_fixed_tripods script:

   string script_filename="./run_select_fixed_tripods";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./select_fixed_tripods \\" << endl;
   script_stream << bundler_IO_subdir+"thresholded_xyz_points.osga \\"
                 << endl;
   script_stream << "--image_list_filename "+bundler_IO_subdir+
      "image_list.dat \\" << endl;
   script_stream << "--image_sizes_filename "+bundler_IO_subdir+
      "image_sizes.dat \\" << endl;
   script_stream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;
   filefunc::closefile(script_filename,script_stream);
   string unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

// Specialize run_archive_video_camera_params and
// run_archive_SLR_camera_params scripts:

   script_filename="./run_archive_video_camera_params";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./archive_camera_params \\" << endl;
   for (int t=1; t<=10; t++)
   {
      script_stream << "--region_filename "+packages_subdir+
         "video_tripod_"+stringfunc::integer_to_string(t,2)+".pkg \\" << endl;
   }
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg \\" << endl;
   script_stream << "--image_list_filename "+bundler_IO_subdir+
      "image_list.dat" << endl;

   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);


   script_filename="./run_archive_SLR_camera_params";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./archive_camera_params \\" << endl;
   for (int t=17; t<=26; t++)
   {
      script_stream << "--region_filename "+packages_subdir+
         "tripod_"+stringfunc::number_to_string(t)+".pkg \\" << endl;
   }
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg \\" << endl;
   script_stream << "--image_list_filename "+bundler_IO_subdir+
      "image_list.dat" << endl;

   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

// Specialize run_loadphotos script:

   script_filename="./run_load_photo_metadata";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./load_photo_metadata \\" << endl;
   script_stream << "--region_filename " << peter_inputs_filename 
                 << " \\" << endl;
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

// Specialized scripts which need package inputs info:

   vector<string> special_mains;
   special_mains.push_back("dbvisibility");
   special_mains.push_back("dbvolume");
   special_mains.push_back("calc_volumes");
   special_mains.push_back("interp_missing_slices");
   special_mains.push_back("generate_view3Dmovie_script");

   for (int s=0; s<special_mains.size(); s++)
   {
      script_filename="./run_"+special_mains[s];
      filefunc::openfile(script_filename,script_stream);
      string executable="./"+special_mains[s];
      script_stream << executable+" \\" << endl;
      script_stream << "--region_filename " << peter_inputs_filename 
                    << " \\" << endl;
      script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
      filefunc::closefile(script_filename,script_stream);
      unix_command="chmod a+x "+script_filename;
      sysfunc::unix_command(unix_command);
   }
   
/*
// Specialize run_dbvisibility script:

   script_filename="./run_dbvisibility";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./dbvisibility \\" << endl;
   script_stream << "--region_filename " << peter_inputs_filename 
                 << " \\" << endl;
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

// Specialize run_dbvolume script:

   script_filename="./run_dbvolume";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./dbvolume \\" << endl;
   script_stream << "--region_filename " << peter_inputs_filename 
                 << " \\" << endl;
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

// Specialize run_calc_volumes script:

   script_filename="./run_calc_volumes";
   filefunc::openfile(script_filename,script_stream);
   script_stream << "./calc_volumes \\" << endl;
   script_stream << "--region_filename " << peter_inputs_filename 
                 << " \\" << endl;
   script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
   filefunc::closefile(script_filename,script_stream);
   unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);
*/

   program_filename.clear();
   program_filename.push_back("./temporally_filter_masks");
   program_filename.push_back("./generate_montage_AVIs");
   
   for (int n=0; n<program_filename.size(); n++)
   {
      string script_filename=
         "./run_"+filefunc::getbasename(program_filename[n]);
      filefunc::openfile(script_filename,script_stream);   
      script_stream << program_filename[n]+" \\" << endl;
      script_stream << "--GIS_layer ./packages/plume_metadata.pkg " << endl;
      filefunc::closefile(script_filename,script_stream);   

      string unix_command="chmod a+x "+script_filename;
      sysfunc::unix_command(unix_command);
   } // loop over index n labeling program filenames


   string banner="Executable scripts written to ./";
   outputfunc::write_big_banner(banner);
}

