// ========================================================================
// Program LIST_SUBSAMPLED_IMAGES is a little utility program which we
// wrote to generate a list of subsampled panoramas that serves as
// input to program STABILIZE_PANOS.  It also creates a new
// subdirectory of /data/DIME/bundler which we refer to from here on
// as bundler_IO_subdir.  Output from this and most subsequent
// programs in Peter's DIME processing algorithm chain is exported to
// bundler_IO_subdir.

//			     ./list_subsampled_images

// ========================================================================
// Last updated on 6/21/13; 7/14/13; 7/15/13; 8/8/13; 8/12/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

// On Dunmeyer's Ubuntu 12.4 box:

   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string DIME_subdir="/data/DIME/";
   string MayFieldtest_subdir=DIME_subdir+"panoramas/May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string panos_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "panos_subdir = " << panos_subdir << endl;
   string horizons_subdir=panos_subdir+"horizons/";
   cout << "horizons_subdir = " << horizons_subdir << endl;
   
// Delete any .pgm files in horizons_subdir which may have been left
// over from previous calls to stabilize_panos:

   string substring=".pgm";
   vector<string> pgm_filenames=filefunc::files_in_subdir_matching_substring(
      horizons_subdir,substring);
   for (int i=0; i<pgm_filenames.size(); i++)
   {
      cout << "Deleting pgm file = " << pgm_filenames[i] << endl;
      filefunc::deletefile(pgm_filenames[i]);
   }

// Create appropriate, new subdirectory of /data/DIME/bundler/ which
// we refer to as bundler_IO_subdir:

   string bundler_IO_subdir=DIME_subdir+"bundler/DIME/May2013_Fieldtest/";
   filefunc::dircreate(bundler_IO_subdir);
   bundler_IO_subdir += "05202013/";
   filefunc::dircreate(bundler_IO_subdir);
   bundler_IO_subdir += "Scene"+scene_ID_str+"/";
   filefunc::dircreate(bundler_IO_subdir);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_list_filename=bundler_IO_subdir+"image_list.dat";

   substring="subsampled_uvcorrected_wisp";
   vector<string> image_filenames=filefunc::files_in_subdir_matching_substring(
      horizons_subdir,substring);
   cout << "Number of subsampled UV corrected images in " 
        << horizons_subdir << " = " 
        << image_filenames.size() << endl;

   int n_start=0;
   int n_stop=image_filenames.size()-1;
//   cout << "Enter starting image number:" << endl;
//   cin >> n_start;
//   cout << "Enter stopping image number:" << endl;
//   cin >> n_stop;

   string scriptfilename="subsampled_UVcorrected_images";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   string stabilize_scriptfilename="run_stabilize_panos";
   ofstream stabilizestream;
   filefunc::openfile(stabilize_scriptfilename,stabilizestream);
   stabilizestream << "./stabilize_panos \\" << endl;

   string suffix=".jpg";
   int n_step=1;
//   int n_step=50;
   for (int i=n_start; i<=n_stop; i += n_step)
   {
      string imagenumber_str=stringfunc::integer_to_string(i,5);
      string filename=horizons_subdir+
         "subsampled_uvcorrected_wisp_res0_"+imagenumber_str+suffix
         +" \\";
      string command="--newpass "+filename;
      scriptstream << command << endl;
      stabilizestream << command << endl;
   }
   stabilizestream << "--image_list_filename " << image_list_filename
                   << endl;

   filefunc::closefile(scriptfilename,scriptstream);
   filefunc::closefile(stabilize_scriptfilename,stabilizestream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);

   unix_command="chmod a+x "+stabilize_scriptfilename;
   sysfunc::unix_command(unix_command);

   string banner="Exported subsampled UV corrected image filenames to "
      +scriptfilename;
   outputfunc::write_big_banner(banner);

   banner="Exported stabilize_panos script to "
      +stabilize_scriptfilename;
   outputfunc::write_big_banner(banner);
}
