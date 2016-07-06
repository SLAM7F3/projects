// ========================================================================
// Program CREATE_KAKADU_SCRIPT is a little utility program which we
// wrote to generate an executable script that calls KDU_EXPAND 
// for a set of Bluegrass video images.
// ========================================================================
// Last updated on 1/3/08; 4/24/08; 4/25/08; 7/17/08
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
   string scriptfilename="run_kdu_expand";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   int n_start=50;
   int n_stop=5434;
//   int n_start=2700;
//   int n_stop=2705;
//   int n_stop=3000;
//   int n_stop=3999;

//   int n_start=2759;	// 11:35 am on Sept 27 2007 in AR #1, chunk #2
//   int n_stop=3825;	// 11:45 am on Sept 27 2007 in AR #1, chunk #2
   
   vector<string> command;
//   string kakadu_subdir=
//      "/home/cho/bluegrass/kakadu/v5_2_6-00155N/bin/Linux-x86-gcc/";
//   string kakadu_subdir=
//      "/home/cho/bluegrass/kakadu/v5_2_6-00155N/bin/Linux-x86-gcc/";
   string kakadu_subdir=
      "/raid0/data/processedData/2007-09-27/20070927_CH_n1_002/tif_files/";

   command.push_back(kakadu_subdir+"kdu_expand \\");
//   string data_subdir="/data3/video/Lubbock/constant_hawk/AR1_002/";
   string input_data_subdir=
//      "/data/video/2007/Lubbock/constant_hawk/AR1_002/jp2_files/";
//      "/data3/video/Lubbock/constant_hawk/AR1_002/jp2_files/";
      "/raid0/data/processedData/2007-09-27/20070927_CH_n1_002/";
   string output_data_subdir=
//      "/data/video/2007/Lubbock/constant_hawk/AR1_002/jp2_files/";
//      "/data3/video/Lubbock/constant_hawk/AR1_002/jp2_files/";
      "/raid0/data/processedData/2007-09-27/20070927_CH_n1_002/tif_files/";

   for (int n=n_start; n<=n_stop; n++)
   {
//      string filename_prefix="BG27sep07n00nndm-000_phase3_compressed";
//      string filename_prefix="BG27sep07n1mndm-002_phase3_compressed";
//      string filename_prefix="20070927_BGn1_002_";
      string filename_prefix="20070927_CH_n1_002_";
      string file_number=stringfunc::integer_to_string(n,6);
      string filename=filename_prefix+file_number+".jp2";
      string input_command="-i "+input_data_subdir+filename+" \\";
      string output_filename="ch_"+file_number+".tif";
      string output_command="-o "+output_data_subdir+output_filename;
//      string command0d=" -reduce 0";
      string reduce_command=" -reduce 1";
//      string command0e=" -region '{0.2,0.2},{0.6,0.6}'";
//      string command0d=" -reduce 2";

//      double start_frac_from_left=0.45;
//      double start_frac_from_top=0.1;
//      double original_width_extent_frac=0.4;
//      double original_height_extent_frac=0.5;

      double start_frac_from_left=0.15;
      double start_frac_from_top=0.15;
      double original_width_extent_frac=0.7;
      double original_height_extent_frac=0.7;

      string region_command=" -region '{" +
         stringfunc::number_to_string(start_frac_from_top)+","+
         stringfunc::number_to_string(start_frac_from_left)+"},{"+
         stringfunc::number_to_string(original_height_extent_frac)+","+
         stringfunc::number_to_string(original_width_extent_frac)+"}'";
         
//      string command0e=" -region '{0.0,0.0},{0.2,0.4}'";
//      string command0e=" -region '{0.35,0.2},{0.5,0.4}'";
//      string command0e=" -region '{0.0,0.2},{0.0,0.4}'";

      string subsampled_filename="subsampled_"+output_filename;
//      string command0f="convert -scale 2800x2800+0+0 "

//      string convert_command="convert -scale 2700x2700+0+0 "
//         +output_data_subdir+output_filename+" "
//         +output_data_subdir+output_filename;
//         +data_subdir+scaled_filename+" "
//         +data_subdir+scaled_filename;

      string subsample_command="gdal_translate -outsize 2700 2700 "
         +output_data_subdir+output_filename+" "
         +output_data_subdir+subsampled_filename;

      string mv_command="mv "+
         output_data_subdir+subsampled_filename+" "
         +output_data_subdir+output_filename;

      scriptstream << command[0] << endl;
      scriptstream << input_command << endl;
      scriptstream << output_command
                   << reduce_command
                   << region_command
                   << endl;
      scriptstream << subsample_command << endl;
      scriptstream << mv_command << endl;
   }

   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
