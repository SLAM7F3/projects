// ==========================================================================
// Program GENERATE_INDIVIDUAL_PHOTO_INPUTS expects to find
// "bundle.out" and "list_tmp.txt" sitting within $bundler_subdir=
// ./bundler/individual_photo/.  It generates an images_list.dat file
// containing renamed versions of the input images.  This program then
// creates a packages subdir of $bundler_subdir and generates a
// peter_inputs.pkg file containing names for various input
// parameters.  Finally, GENERATE_INDIVIDUAL_PHOTO_INPUTS writes out
// executable scripts for the remaining programs which must be run in
// order to visualize the n+1st photo as a 3D frustum within the
// MIT2317 bundler point cloud.

//  generate_individual_photo_inputs --bundle_filename ./bundler/individual_photo/bundle.out

// ==========================================================================
// Last updated on 5/21/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

//   string bundle_filename=passes_group.get_bundle_filename();
   string bundle_filename="./bundler/individual_photo/bundle.out";
   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Generate a list of renamed image filenames from BUNDLER output file
// "list_tmp.txt":

   string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
   filefunc::ReadInfile(list_tmp_filename);

// On 12/17/09, we learned the painful and hard way that image
// filenames coming from windows machines may have invisible trailing
// carriage returns.  So we must strip off all white space at the ends
// of image filenames in order to read in photos on our linux disks!

   vector<string> truncated_image_filenames;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      string truncated_image_filename=stringfunc::remove_trailing_whitespace(
         "images/"+filefunc::getbasename(curr_line));
      truncated_image_filenames.push_back(truncated_image_filename);
   }
   
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   ofstream image_filename_stream;
   filefunc::openfile(image_list_filename,image_filename_stream);
   for (unsigned int i=0; i<truncated_image_filenames.size(); i++)
   {
      image_filename_stream << truncated_image_filenames[i] << endl;
   }
   filefunc::closefile(image_list_filename,image_filename_stream);

// Generate a packages subdirectory of bunder_IO_subdir.  Then
// generate peter_inputs.pkg file:

   string packages_subdir=bundler_IO_subdir+"packages/";
   filefunc::dircreate(packages_subdir);
   string peter_inputs_filename=packages_subdir+"peter_inputs.pkg";
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

//   string banner="Enter 'y/n' to convert from bundler to world coordinates";
//   banner += " for the MIT2317 set using ladar-derived transformation:";
//   outputfunc::write_banner(banner);

   char response_char='y';
//   cin >> response_char;
   if (response_char=='y')
   {
      fitted_world_to_bundler_distance_ratio=11.2648221644;
      bundler_translation_X=328141.302781;
      bundler_translation_Y=4692067.27943;
      bundler_translation_Z=18.7822026982;
      global_az=-159.785505829;
      global_el=1.14926469438;
      global_roll=-16.5751038749;
      bundler_rotation_origin_X=328212.210605;
      bundler_rotation_origin_Y=4692025.66432;
      bundler_rotation_origin_Z=36.1552629968;
   }
   
   cout << "=======================================================" << endl;
   cout << "fitted_world_to_bundler_distance_ratio = "
        << fitted_world_to_bundler_distance_ratio << endl;
   cout << "bundler_translation_X = " << bundler_translation_X << endl;
   cout << "bundler_translation_Y = " << bundler_translation_Y << endl;
   cout << "bundler_translation_Z = " << bundler_translation_Z << endl;
   cout << "global_az = " << global_az << endl;
   cout << "global_el = " << global_el << endl;
   cout << "global_roll = " << global_roll << endl;
   cout << "bundler_rotation_origin_X = " 
        << bundler_rotation_origin_X << endl;
   cout << "bundler_rotation_origin_Y = " 
        << bundler_rotation_origin_Y << endl;
   cout << "bundler_rotation_origin_Z = " 
        << bundler_rotation_origin_Z << endl;

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

   peter_inputs_stream << "--image_sizes_filename "
                       << bundler_IO_subdir+"image_sizes.dat"
                       << endl;
   filefunc::closefile(peter_inputs_filename,peter_inputs_stream);

// Generate executable scripts for INDIVIDUAL_PHOTO, PHOTO_SIZES and
// THUMBNAILS main programs:

   ofstream script_stream;
   vector<string> program_filename;
   program_filename.push_back("./individual_photo");
   program_filename.push_back("./photo_sizes");
   program_filename.push_back("./thumbnails");

   for (unsigned int n=0; n<program_filename.size(); n++)
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
}

