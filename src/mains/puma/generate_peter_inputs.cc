// ==========================================================================
// Program GENERATE_PETER_INPUTS queries the user to enter a
// subdirectory of ./bundler/ in which some set of images and metadata
// reside.  It expects to find "./images/" sitting within
// $bundler_subdir that contains a set of JPEG and/or PNG image files.

// If no "list_tmp.txt" file exists within $bundler_subdir, a version
// of this file is generated. If no "bundle.out" file resides in
// bundler_IO_subdir, GENERATE_PETER_INPUTS creates a trivial version.
// It next generates an images_list.dat file containing renamed
// versions of the input images.  This program then creates a packages
// subdir of bundler_IO_subdir and generates a peter_inputs.pkg file
// containing names for various input parameters.  Finally,
// GENERATE_PETER_INPUTS writes out executable scripts for the
// several programs which must be run in order to produce thumbnails, 
// construct SIFT graphs and visualize reconstructed points clouds and
// frusta in Peter's 3D viewer.

//			generate_peter_inputs

// ==========================================================================
// Last updated on 11/4/13; 12/4/13; 12/30/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
#include "messenger/Messenger.h"
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

   bool demo_flag=false;
   if (demo_flag)
   {
      string broker_URL = "tcp://127.0.0.1:61616";
      string message_queue_channel_name="127.0.0.1";
      string message_sender_ID="MESSAGE_SENDER";
      bool include_sender_and_timestamp_info_flag=false;
      Messenger m(broker_URL,message_queue_channel_name,message_sender_ID,
                  include_sender_and_timestamp_info_flag);

      string command="DISPLAY_NEXT_FLOW_DIAGRAM";
      m.sendTextMessage(command);
   }

// For summer 2013 GEO demo purposes, enable bundler subdir to be
// passed as a command-line argument:

   string bundler_subdir;
   vector<string> param_lines;
   if (filefunc::parameter_input(argc,argv,param_lines))
   {
      bundler_subdir=param_lines[0];
//      cout << "bundler_subdir = " << bundler_subdir << endl;
   }
   else
   {
      cout << "Enter subdirectory of puma/bundler/ in which images & metadata reside" << endl;
      cout << "(e.g. Puma/Sep13_2013_YPG, Puma/Bryce/DataCollect_3, ...)" 
           << endl;
      cout << endl;

      cin >> bundler_subdir;
   }
   filefunc::add_trailing_dir_slash(bundler_subdir);

   string bundler_IO_subdir="./bundler/"+bundler_subdir;
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string images_subdir=bundler_IO_subdir+"images/";

   string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
   if (!filefunc::fileexist(list_tmp_filename))
   {
      list_tmp_filename=bundlerfunc::generate_list_tmp_file(bundler_IO_subdir);
   }
//   cout << " bundle_filename = " << bundle_filename << endl;


   filefunc::ReadInfile(list_tmp_filename);

   string bundle_filename="./bundler/"+bundler_subdir+"bundle.out";
   if (!filefunc::fileexist(bundle_filename))
   {
      bundlerfunc::generate_trivial_bundle_dot_out_file(list_tmp_filename);
   }
//   cout << " bundle_filename = " << bundle_filename << endl;

// On 12/17/09, we learned the painful and hard way that image
// filenames coming from windows machines may have invisible trailing
// carriage returns.  So we must strip off all white space at the ends
// of image filenames in order to read in photos on our linux disks!

   vector<string> truncated_image_filenames;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      string truncated_image_filename=stringfunc::remove_trailing_whitespace(
         "images/"+filefunc::getbasename(curr_line));
      truncated_image_filenames.push_back(truncated_image_filename);
   }
   
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   ofstream image_filename_stream;
   filefunc::openfile(image_list_filename,image_filename_stream);
   for (int i=0; i<truncated_image_filenames.size(); i++)
   {
      image_filename_stream << truncated_image_filenames[i] << endl;
   }
   filefunc::closefile(image_list_filename,image_filename_stream);

// Generate a packages subdirectory of bunder_IO_subdir.  Then
// generate peter_inputs.pkg file:

   string packages_subdir=bundler_IO_subdir+"packages/";
   filefunc::dircreate(packages_subdir);
   string peter_inputs_filename=packages_subdir+"peter_inputs.pkg";
   string peter_inputs_orig_filename=packages_subdir+"peter_inputs_orig.pkg";
   ofstream peter_inputs_stream;
   filefunc::openfile(peter_inputs_filename,peter_inputs_stream);
   peter_inputs_stream << "--bundle_filename "+bundle_filename << endl;
   peter_inputs_stream << "--image_list_filename "+image_list_filename 
                       << endl;

   double fitted_world_to_bundler_distance_ratio=1;
   double bundler_translation_X=0;
   double bundler_translation_Y=0;
   double bundler_translation_Z=0;
   double global_az=0;
   double global_el=0;
   double global_roll=0;
   double bundler_rotation_origin_X=0;
   double bundler_rotation_origin_Y=0;
   double bundler_rotation_origin_Z=0;

   peter_inputs_stream 
      << "--fitted_world_to_bundler_distance_ratio " 
      << stringfunc::number_to_string(fitted_world_to_bundler_distance_ratio)
      << endl;
   peter_inputs_stream 
      << "--bundler_translation_X "
      << stringfunc::number_to_string(bundler_translation_X)
      << endl;
   peter_inputs_stream 
      << "--bundler_translation_Y "
      << stringfunc::number_to_string(bundler_translation_Y)
      << endl;
   peter_inputs_stream 
      << "--bundler_translation_Z "
      << stringfunc::number_to_string(bundler_translation_Z)
      << endl;

   peter_inputs_stream 
      << "--global_az " 
      << stringfunc::number_to_string(global_az)
      << endl;
   peter_inputs_stream 
      << "--global_el " 
      << stringfunc::number_to_string(global_el)
      << endl;
   peter_inputs_stream 
      << "--global_roll " 
      << stringfunc::number_to_string(global_roll)
      << endl;

   peter_inputs_stream 
      << "--bundler_rotation_origin_X " 
      << stringfunc::number_to_string(bundler_rotation_origin_X)
      << endl;
   peter_inputs_stream 
      << "--bundler_rotation_origin_Y " 
      << stringfunc::number_to_string(bundler_rotation_origin_Y)
      << endl;
   peter_inputs_stream 
      << "--bundler_rotation_origin_Z " 
      << stringfunc::number_to_string(bundler_rotation_origin_Z)
      << endl;

   peter_inputs_stream << "--photoids_xyzids_filename "
                       << bundler_IO_subdir+"photoids_xyzids.dat"
                       << endl;
   peter_inputs_stream << "--image_sizes_filename "
                       << bundler_IO_subdir+"image_sizes.dat"
                       << endl;
   filefunc::closefile(peter_inputs_filename,peter_inputs_stream);
   string unix_cmd="cp "+peter_inputs_filename+" "+peter_inputs_orig_filename;
   sysfunc::unix_command(unix_cmd);

// Generate executable scripts for BUNDLER_CONVERT, PHOTO_SIZES,
// THUMBNAILS, BUNDLER_PHOTOS and WRITE_VIEWBUNDLER_SCRIPT main
// programs:

   ofstream script_stream;
   vector<string> program_filename;
   program_filename.push_back("./bundler_photos");
   program_filename.push_back("./densecloud");
   program_filename.push_back("./gps_waypoints");
   program_filename.push_back("./gpsfit");
   program_filename.push_back("./mini_convert");
   program_filename.push_back("./mosaic");
   program_filename.push_back("./photo_sizes");
   program_filename.push_back("./parse_puma_metadata");
   program_filename.push_back("./ply2tdp");
   program_filename.push_back("./thumbnails");
   program_filename.push_back("./undistort");
   program_filename.push_back("./vsfm_vs_orig_images");
   program_filename.push_back("./write_viewbundler_script");

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

// Generate script for mains/aerosynth/visualize_FLIR_metadata with
// default sentinel values set for starting and stopping frame IDs:

   program_filename.clear();
//   program_filename.push_back("../aerosynth/visualize_FLIR_metadata");

   for (int n=0; n<program_filename.size(); n++)
   {
      string script_filename=
         "./run_"+filefunc::getbasename(program_filename[n]);
      filefunc::openfile(script_filename,script_stream);   
      script_stream << program_filename[n] << " \\" << endl;
      script_stream << " --region_filename dummy.osga \\" 
                    << endl;
      script_stream << " --start_frame_ID -100 \\" << endl;
      script_stream << " --stop_frame_ID -100 \\" << endl;
      script_stream << " --region_filename " << peter_inputs_filename << endl;

      filefunc::closefile(script_filename,script_stream);   

      string unix_command="chmod a+x "+script_filename;
      sysfunc::unix_command(unix_command);
   } // loop over index n labeling program filenames

// Generate scripts which have --GIS_layer arguments:

   program_filename.clear();

   for (int n=0; n<program_filename.size(); n++)
   {
      string script_filename=
         "./run_"+filefunc::getbasename(program_filename[n]);
      filefunc::openfile(script_filename,script_stream);   
      script_stream << program_filename[n]+" \\" << endl;
      script_stream << "--region_filename "
                    << peter_inputs_filename << " \\" << endl;
      script_stream << "--GIS_layer ./packages/imagery_metadata.pkg " << endl;
      filefunc::closefile(script_filename,script_stream);   
      string unix_command="chmod a+x "+script_filename;
      sysfunc::unix_command(unix_command);
   } // loop over index n labeling program filenames

// Move script files particularly useful for reconstructed aerial
// video frame georegistration into mains/aerosynth:

   vector<string> script_filenames;
   script_filenames.push_back("run_ply2tdp");

   for (int n=0; n<script_filenames.size(); n++)
   {
      string curr_script_filename=script_filenames[n];
      unix_cmd="mv "+curr_script_filename+" ../aerosynth/";
      sysfunc::unix_command(unix_cmd);
   }

   string banner="Executable scripts written to ./";
   outputfunc::write_big_banner(banner);
}

