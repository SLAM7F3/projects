// ==========================================================================
// Program DENSECLOUD executes a series of steps needed in order to
// convert bundler output into a format which can be ingested into
// Furukawa's PMVS multi-view stereo system software.  The densely
// reconstructed point cloud is exported to dense_cloud.osga within
// bundler_IO_subdir.
// ==========================================================================
// Last updated 9/11/13; 12/3/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);
   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string pmvs_subdir=bundler_IO_subdir+"pmvs/";
   filefunc::dircreate(pmvs_subdir);
   string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
   string packages_subdir=bundler_IO_subdir+"packages/";
   string peter_inputs_filename=packages_subdir+"peter_inputs.pkg";

// First generate list.txt from list_tmp.txt where images/
// in list_tmp.txt is replaced by ./ in list.txt:

   string list_filename=bundler_IO_subdir+"list.txt";
   ofstream outstream;
   filefunc::openfile(list_filename,outstream);
   
   filefunc::ReadInfile(list_tmp_filename);
   string separator_chars="/";
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string filename=bundler_IO_subdir+filefunc::text_line[i];
      outstream << filename << endl;
   }
   filefunc::closefile(list_filename,outstream);   

// Call Bundle2PMVS on list.txt:

   string unix_cmd="Bundle2PMVS "+bundler_IO_subdir+"list.txt "+
      bundler_IO_subdir+"bundle.out "+pmvs_subdir;
   sysfunc::unix_command(unix_cmd);

// Call RadialUndistort on list.txt:

   unix_cmd="RadialUndistort "+list_filename+" "+
      bundler_IO_subdir+"bundle.out "+pmvs_subdir;
   sysfunc::unix_command(unix_cmd);

   string visualize_subdir=pmvs_subdir+"visualize/";
   filefunc::dircreate(visualize_subdir);
   string txt_subdir=pmvs_subdir+"txt/";
   filefunc::dircreate(txt_subdir);
   string models_subdir=pmvs_subdir+"models/";
   filefunc::dircreate(models_subdir);

   vector<string> rd_image_filenames=filefunc::image_files_in_subdir(
      pmvs_subdir);
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> text_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,pmvs_subdir);

   int n_images=rd_image_filenames.size();
   for (int i=0; i<n_images; i++)
   {
      string new_image_filename=visualize_subdir+
         stringfunc::integer_to_string(i,8)+".jpg";
      string unix_cmd="mv "+rd_image_filenames[i]+" "+new_image_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="mv "+text_filenames[i]+" "+txt_subdir;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
   
// Run Bundle2VIS:

   unix_cmd="Bundle2Vis "+pmvs_subdir+"bundle.rd.out "+
      pmvs_subdir+"vis.dat";
   sysfunc::unix_command(unix_cmd);

// Run PMVS2:

   unix_cmd="pmvs2 "+pmvs_subdir+" pmvs_options.txt";
   sysfunc::unix_command(unix_cmd);

// Convert ply file exported by PMVS to tdp format:

   outputfunc::print_elapsed_time();
   unix_cmd=
      "/home/cho/programs/c++/svn/projects/src/mains/aerosynth/ply2tdp --region_filename "
      +peter_inputs_filename;
//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   string dense_cloud_filename=bundler_IO_subdir+"dense_cloud.osga";
   unix_cmd="mv "+bundler_IO_subdir+"pmvs_options.txt.osga "+
      dense_cloud_filename;
//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported dense reconstructed cloud to "+
      dense_cloud_filename;
   
   cout << "At end of program DENSECLOUD" << endl;
   outputfunc::print_elapsed_time();
}

