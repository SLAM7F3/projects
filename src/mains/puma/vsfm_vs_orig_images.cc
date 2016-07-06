// ==========================================================================
// Program VSFM_VS_ORIG_IMAGES imports VSFM text file "cameras_v2.txt".
// It exports relationships between original and renamed VSFM image
// filenames to a text file in bundler_IO_subdir.
// ==========================================================================
// Last updated on 12/3/13; 12/10/13; 12/30/13
// ==========================================================================

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

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
   cout.precision(10);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   string reconstructed_camera_posns_filename=
      bundler_IO_subdir+"reconstructed_camera_posns.dat";

/*
   string bundler_IO_subdir;
   cout << "Enter bundler_IO_subdir:" << endl;
   cin >> bundler_IO_subdir;
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string orig_images_subdir=bundler_IO_subdir+"orig_images/";
   filefunc::dircreate(orig_images_subdir);
*/

   string cameras_filename=bundler_IO_subdir+"cameras_v2.txt";
   cout << "cameras_filename = " << cameras_filename << endl;
   string unix_cmd="dos2unix "+cameras_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::ReadInfile(cameras_filename);

   int line_number=0;
   int n_cameras=stringfunc::string_to_number(
      filefunc::text_line[line_number++]);
   cout << "n_cameras = " << n_cameras << endl;

   int image_line_counter=1;
   vector<string> vsfm_filenames,orig_filenames;
   while (line_number < filefunc::text_line.size())
   {
      string curr_line=filefunc::text_line[line_number];
//      cout << image_line_counter << "  " << curr_line << endl;
      if (image_line_counter==1)
      {
         vsfm_filenames.push_back(curr_line);
      }
      else if (image_line_counter==2)
      {
         orig_filenames.push_back(filefunc::getbasename(curr_line,"\\"));
      }

      line_number++;

      image_line_counter++;
      if (image_line_counter%13==0)
      {
         image_line_counter=0;
//         outputfunc::enter_continue_char();
      }

   } // loop over index i labeling VSFM camera filename lines

// Sort orig_filenames so that they're temporally ordered:

   templatefunc::Quicksort(orig_filenames,vsfm_filenames);

// Store relationship between VSFM and original image filenames within
// STL map:

   typedef map<string,string> VSFM_ORIG_FILENAMES_MAP;

// independent string = vsfm image filename
// dependent string = original video filename

   VSFM_ORIG_FILENAMES_MAP vsfm_orig_filenames_map;
   VSFM_ORIG_FILENAMES_MAP::iterator iter;

// Generate text file that connects originally named and VSFM renamed
// image files:

   string output_filename=bundler_IO_subdir+"orig_vs_VSFM_images.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Original filename   VSFM filename " << endl << endl;

   for (int j=0; j<orig_filenames.size(); j++)
   {
      outstream << orig_filenames[j] << "         " << vsfm_filenames[j]
                << endl;
      vsfm_orig_filenames_map[vsfm_filenames[j]]=orig_filenames[j];
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);

/*
// Generate executable script which links VSFM renamed image files to
// originally named image files:

   string link_scriptname=orig_images_subdir+"link_VSFM_to_orig_images";
   filefunc::openfile(link_scriptname,outstream);
   for (int j=0; j<orig_filenames.size(); j++)
   {
      string link_cmd="ln -s ../vsfm_images/"+vsfm_filenames[j]+
         " ./"+orig_filenames[j];
      outstream << link_cmd << endl;
   }
   filefunc::closefile(link_scriptname,outstream);
   filefunc::make_executable(link_scriptname);
   banner="Exported "+link_scriptname;
   outputfunc::write_big_banner(banner);
*/

// Generate new version of reconstructed_camera_posns.dat where camera
// positions are listed as functions of original and not VSFM
// filenames:

   vector<vector<string> > column_values=
      filefunc::ReadInSubstrings(reconstructed_camera_posns_filename);

   orig_filenames.clear();
   vector<string> GPS_posn_strings;
   for (int r=0; r<column_values.size(); r++)
   {
      string vsfm_filename=column_values[r].at(1);
      iter=vsfm_orig_filenames_map.find(vsfm_filename);
      string orig_filename=iter->second;
      orig_filenames.push_back(orig_filename);
      GPS_posn_strings.push_back(
         column_values[r][2]+"  "+column_values[r][3]+"  "+
         column_values[r][4]);
   } // loop over index r labeling rows in reconstructed camera posns file

   templatefunc::Quicksort(orig_filenames,GPS_posn_strings);

   output_filename=bundler_IO_subdir+"orig_reconstructed_camera_posns.dat";
   filefunc::openfile(output_filename,outstream);
   for (int r=0; r<orig_filenames.size(); r++)
   {
      outstream << r << "  " 
                << orig_filenames[r] << "  "
                << GPS_posn_strings[r] << endl;
   }
   filefunc::closefile(output_filename,outstream);   

   banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}
