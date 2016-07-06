// ========================================================================
// Program Wisp360CalAssistant_script is a little utility program
// which we wrote to generate a script to run the G99
// Wisp360CalAssistant binary.  The executable transforms raw WISP
// imagery into output 360 panorama JPG files.

//			./Wisp360CalAssistant_script

// ========================================================================
// Last updated on 7/12/13; 8/8/13; 8/22/13
// ========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   string scriptfilename="run_Wisp360CalAssistant";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);

// Dave Dunemeyer's Ubuntu 12.04 machine:

   string DIME_dir="/data/DIME/";		  
   string wisp_bin_dir=DIME_dir+"Condor_March2013/WispRDC_test/bin/";

   string calib_filename=wisp_bin_dir+"calib.w360 ";
   string banner=
      "Enter name of WISP calibration file (e.g. calib.w360) to import from "+
      wisp_bin_dir+" :";
   outputfunc::write_banner(banner);
   cin >> calib_filename;

   string binary_filename=wisp_bin_dir+"Wisp360CalAssistant ";

   string panos_dir=DIME_dir+"panoramas/";

   string DeerIsland_dir=panos_dir+"Feb2013_DeerIsland/";
   string Rooftop_dir=panos_dir+"Apr2013_LLrooftop/";
   string Fieldtest_dir=panos_dir+"May2013_Fieldtest/";

   banner=
      "Wisp360CalAssistant will process raw WISP data as specified in "
      +calib_filename;
   outputfunc::write_banner(banner);

   cout << endl;
   cout << "Choose resolution for output panoramas:" << endl << endl;
   cout << "0  --> 40K x 2.2K high resolution" << endl;
   cout << "1  --> 20K x 1.1K medium resolution" << endl;
   cout << "2  --> 10K x 0.55K low resolution" << endl;
   int resolution_setting=1;
   cin >> resolution_setting;

   int n_start,n_stop;
   cout << endl << endl;
   cout << "Enter starting panorama frame number:" << endl;
   cin >> n_start;
   cout << "Enter stopping panorama frame number:" << endl;
   cin >> n_stop;

   int greyscale_bit_depth=8;
   cout << endl << endl;
   cout << "Enter 8 or 16 for greyscale bit depth:" << endl;
   cin >> greyscale_bit_depth;

   cout << endl;
//   cout << "Enter raw data set category:" << endl << endl;
//   cout << "1  --> Feb 2013 Deer Island" << endl;
//   cout << "2  --> Apr 2013 LL Rooftop" << endl;
//   cout << "3  --> May 2013 Fieldtest" << endl;
//   cout << "4  --> Enter full path for output panoramas" << endl;

   int dataset_category=4;
//   cin >> dataset_category;

   string output_dir;
   if (dataset_category==1)
   {
      output_dir=DeerIsland_dir;
   }
   else if (dataset_category==2)
   {
      output_dir=Rooftop_dir;
   }
   else if (dataset_category==3)
   {
      output_dir=Fieldtest_dir;
   }
   else if (dataset_category==4)
   {
      cout << "Enter full path for directory where panoramas will be placed:"
           << endl;
      cin >> output_dir;
      filefunc::add_trailing_dir_slash(output_dir);
      filefunc::dircreate(output_dir);
   }
   else
   {
      cout << "Invalid category entered!" << endl;
      exit(-1);
   }

   bool highpass_filter_flag=false;

/*
   cout << endl;
   cout << "Enter 'y'/'n' in order to generate high-pass filtered panoramas:"
        << endl;

   string filter_char;
   cin >> filter_char;
   if (filter_char=="y")
   {
      highpass_filter_flag=true;
   }
//   cout << "highpass_filter_flag = " << highpass_filter_flag;
*/

   for (int i=n_start; i<=n_stop; i++)
   {
      string image_prefix="wisp_res"
         +stringfunc::number_to_string(resolution_setting)
         +"_"+stringfunc::integer_to_string(i,5);
      string pgm_filename=output_dir+image_prefix+".pgm";
      string jpg_filename=output_dir+image_prefix+".jpg";
      string highpass_filename=output_dir+"highpass_"+image_prefix+".jpg";

      string command="echo 'Generating panorama "
         +stringfunc::number_to_string(i)+"'";
      scriptstream << command << endl;

      command=binary_filename+calib_filename;

      if (greyscale_bit_depth==16)
      {
         command += " -PanoramicF "+stringfunc::number_to_string(i);
      }
      else
      {
         command += " -Panoramic "+stringfunc::number_to_string(i);
      }
      command += " "+stringfunc::number_to_string(resolution_setting)+" "
         +pgm_filename;
      scriptstream << command << endl;

      command="convert -quality 100 "+pgm_filename+" "+jpg_filename;
      scriptstream << command << endl;
      command="/bin/rm "+pgm_filename;
      scriptstream << command << endl;
//      command="mv ps_exec_log*.txt "+output_dir;

      if (highpass_filter_flag)
      {
         command="convert "+jpg_filename+" -edge 2 "+highpass_filename;
         scriptstream << command << endl;
      }
      
      command="echo 'Panorama exported to "+jpg_filename+"'";
      scriptstream << command << endl;
   }
   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);

   banner="Exported executable script to "+scriptfilename;
   outputfunc::write_big_banner(banner);
}
