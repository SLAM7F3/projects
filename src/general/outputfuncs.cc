// ==========================================================================
// OUTPUTFUNCS stand-alone methods
// ==========================================================================
// Last modified on 11/28/15; 12/1/15; 12/2/15; 5/30/16
// =========================================================================

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace outputfunc
{

// Method enter_continue_char is useful for debugging purposes:

   void enter_continue_char()
   {
      string message=
         "Enter any non-white character followed by carriage return to continue:";
      enter_continue_char(message);
   }

   void enter_continue_char(string message)
   {
      char junkchar;
      cout << message << endl;
      cin >> junkchar;
   }

   string select_logfile_name(
      bool input_param_file,string inputline[],unsigned int& currlinenumber)
      {
         string outputline[1];
         outputline[0]="Enter full pathname for output logfile:";
         string logfilename=stringfunc::mygetstring(
            1,outputline,input_param_file,inputline,currlinenumber);
         return logfilename;
      }

// ---------------------------------------------------------------------
// Method select_output_directory queries the user to enter the name
// of the subdirectory where metafile and text output will be written.
// For demonstration purposes, a numbered demo directory is
// automatically created, and the user is NOT asked to specify the
// output directory.

   string select_output_directory(
      bool public_software,bool pop_open_window,
      bool input_param_file,string inputline[],unsigned int& currlinenumber,
      string basedirname)
      {
         int i=0;
         string imagedir,subdirname,outputline[10];
   
         if (public_software && pop_open_window)
         {
            do
            {
               subdirname="demo"+stringfunc::number_to_string(i);
               imagedir=basedirname+"/"+subdirname+"/";
               i++;
            }
            while(filefunc::direxist(imagedir));
         }
         else if (public_software && (!pop_open_window))
         {
            outputline[0]="Enter full subdirectory pathname";
            outputline[1]="where all output files will be written:";
            subdirname=stringfunc::mygetstring(
               2,outputline,input_param_file,inputline,currlinenumber);
            imagedir=subdirname+"/";
         }
         else
         {
            outputline[0]="Enter name of subdirectory of ./"
               +basedirname+" where all output files will be written:";
            subdirname=stringfunc::mygetstring(
               1,outputline,input_param_file,inputline,currlinenumber);
            imagedir=basedirname+"/"+subdirname+"/";
         }
         filefunc::dircreate(imagedir);
         return imagedir;
      }

// ---------------------------------------------------------------------
   void write_big_banner(string banner_message)
      {
         outputfunc::newline();
         cout << "=======================================================" 
              << endl;
         cout << banner_message << endl;
         cout << "=======================================================" 
              << endl;
         outputfunc::newline();
      }
   
   void write_banner(string banner_message)
      {
         outputfunc::newline();
         cout << banner_message << endl;
         outputfunc::newline();
      }

// ---------------------------------------------------------------------
   void print_filename_and_date(ofstream& datastream,string filenamestr)
      {
         datastream << "story 'Filename = "+filenamestr+"'" << endl;
         datastream << "story '"+timefunc::getcurrdate()+"'" << endl;
         datastream << "storyloc -1.5 6.25 " << endl;
      }

// ---------------------------------------------------------------------
   void write_initial_results_info(ofstream& datastream) 
      {
         datastream << 
            "# ========================================================="
                    << endl;
         datastream << "Automated system results generated on "
                    << timefunc::getcurrdate() << endl;
         datastream << "Workstation = " << sysfunc::get_hostname() << endl;
      }

// ---------------------------------------------------------------------
// Method processing_time returns the total processing time in minutes
// needed to perform some computation:

   double processing_time(const time_t& start_processing_time)
      { 
         time_t stop_processing_time=time(NULL);
         double processing_time=
            (stop_processing_time-start_processing_time)/60.0;
//         cout << "start_processing_time = " << start_processing_time << endl;
//         cout << "stop_processing_time = " << stop_processing_time << endl;
         return processing_time;
      }

// ---------------------------------------------------------------------
// Method report_processing_time_info writes out time required to run
// executable to both screen and summary results file output:

   void report_processing_time_info(
      long start_processing_time,ostream& datastream) 
      {
         datastream << "Time required to finish computation = "
            +stringfunc::number_to_string(processing_time(
               start_processing_time),1)+" minutes" << endl;
         datastream << endl;
      }
   
// ---------------------------------------------------------------------
   void write_program_finished_message() 
      {
         outputfunc::newline();
         cout << "=======================================================" 
              << endl;
         cout << "PROGRAM FINISHED" << endl;
         cout << "=======================================================" 
              << endl;
         outputfunc::newline();
      }

// ==========================================================================
// Script generation methods
// ==========================================================================

// Method generate_view_script takes in an array of nfilenames JPEG
// files.  It creates a script which runs xv on each of these JPEG
// files.

   void generate_view_script(
      int nfilenames,string basename[],string imagedir,string scriptfilename)
      {
         ofstream scriptstream;
         filefunc::openfile(imagedir+scriptfilename,scriptstream);
         for (int i=0; i<nfilenames; i++)
         {
            scriptstream << "xv "+basename[i]+".jpg &" << endl;
         }
         filefunc::closefile(imagedir+scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+imagedir+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method generate_view_scripts takes in a basefilename for some set
// of numbered JPEG images.  It creates a set of scripts which call xv
// for up to 8 images at a time.  This method helps streamline the
// viewing of many images within a movie sequence.

   void generate_view_scripts(
      int number_of_images,string basefilename,string imagedir,
      string scriptfilename)
      {
// In order to avoid overloading the image buffer when displaying
// multiple postscript images using ghostview, we limit the number of 
// of images which are to be displayed per script:

         const int npsfiles_per_script=8;
         int nscripts=static_cast<int>(number_of_images/npsfiles_per_script);

         if (number_of_images%npsfiles_per_script > 0) nscripts++;
      
// Index j labels script number; index i labels image number
         int j=0;
         string script_filename[10];
         ofstream scriptstream;
         for (int i=0; i<number_of_images; i++)
         {
            if (i%npsfiles_per_script==0)
            {
               if (i>0)
               {
                  filefunc::closefile(script_filename[j],scriptstream);
                  j++;
               }
               script_filename[j]=imagedir+scriptfilename+
                  stringfunc::number_to_string(j);
               filefunc::openfile(script_filename[j],scriptstream);
            }

// XELIAS numbers images starting from 1.  We therefore add 1 to our 
// image numbers for consistency with XELIAS:

            scriptstream << "xv "+basefilename+
               stringfunc::number_to_string(i+1)+".jpg &" << endl;
         }
         filefunc::closefile(script_filename[j],scriptstream);
   
// Make scripts executable:

         for (int j=0; j<nscripts; j++)
         {
            string unixcommandstr="chmod a+x "+script_filename[j];
            sysfunc::unix_command(unixcommandstr);
         }
      }

// ---------------------------------------------------------------------
// Method generate_animation_script writes out a script which can be
// used with the Imagemagick program animate to view a movie of
// several JPEG images strung together.  The base filename of the JPEG
// images as well as the name of the output animation script are
// specified as input parameters.

   void generate_animation_script(
      int number_of_images,string basefilename,string imagedir,
      string scriptfilename,int delay,string suffix,int skip)
      {
         generate_animation_script(0,number_of_images,basefilename,imagedir,
                                   scriptfilename,delay,suffix,skip);
      }
   
   void generate_animation_script(
      int start_image,int stop_image,string basefilename,string imagedir,
      string scriptfilename,int delay,string suffix,int skip)
      {
         const string xstdcmapstr="xstdcmap -best";
         string animatestr="animate -delay "
            +stringfunc::number_to_string(delay)+" ";
         ofstream scriptstream;

// XELIAS numbers images starting from 1.  We therefore add 1 to our 
// image numbers for consistency with XELIAS:
   
         for (int i=start_image; i<stop_image; i += skip)
         {
            if (i%5==0)
            {
               animatestr += " \\\n";
            }
            animatestr += basefilename+stringfunc::integer_to_string(i,4)
//            animatestr += basefilename+stringfunc::integer_to_string(i+1,3)
               +"."+suffix+" ";
         }
   
         scriptfilename=imagedir+scriptfilename;
         filefunc::openfile(scriptfilename,scriptstream);

         cout << "scriptfilename = " << scriptfilename << endl;
         
         scriptstream << xstdcmapstr << endl;
         scriptstream << animatestr << endl;
         filefunc::closefile(scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method generate_conversion_script writes out a script which uses
// with the Imagemagick program convert to transform one image type to
// another.  

   void generate_conversion_script(
      int start_image,int stop_image,string basefilename,
      string input_dir,string output_dir,
      string scriptfilename,string init_suffix,string final_suffix,
      int image_skip,int n_digits,int quality)
      {
         scriptfilename=input_dir+scriptfilename;
         ofstream scriptstream;
         filefunc::openfile(scriptfilename,scriptstream);

         for (int i=start_image; i<stop_image; i += image_skip)
         {
            string convertstr="convert ";
            if (quality > 0)
            {
               convertstr += "-quality "+stringfunc::number_to_string(quality)
                  +" ";
            }
            convertstr += input_dir+basefilename
               +stringfunc::integer_to_string(i,n_digits)+"."+init_suffix+" ";
            convertstr += 
               output_dir+basefilename+
               stringfunc::integer_to_string(i,n_digits)
               +"."+final_suffix;
            scriptstream << convertstr << endl;
         }

         cout << "Conversion script filename = " << scriptfilename << endl;
         
         filefunc::closefile(scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
   void generate_conversion_script(
      int start_image,int stop_image,string basefilename,string imagedir,
      string scriptfilename,string init_suffix,string final_suffix,
      int image_skip,int n_digits,int quality)
      {
         scriptfilename=imagedir+scriptfilename;
         ofstream scriptstream;
         filefunc::openfile(scriptfilename,scriptstream);

         for (int i=start_image; i<stop_image; i += image_skip)
         {
            string convertstr="convert ";
            if (quality > 0)
            {
               convertstr += "-quality "+stringfunc::number_to_string(quality)
                  +" ";
            }
            convertstr += basefilename
               +stringfunc::integer_to_string(i,n_digits)+"."+init_suffix+" ";
            convertstr += 
               basefilename+stringfunc::integer_to_string(i,n_digits)
               +"."+final_suffix;
            scriptstream << convertstr << endl;
         }

         cout << "Conversion script filename = " << scriptfilename << endl;
         
         filefunc::closefile(scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method generate_avi_script writes out a script which used to
// generate AVI movies from concatenation of multiple JPEG images via
// the SGI program dmconvert:

   void generate_avi_script(
      int istart,int number_of_images,int npasses,string imagedir)
      {
         string subdir="";
         generate_avi_script(istart,number_of_images,npasses,imagedir,subdir);
      }

   void generate_avi_script(
      int istart,int number_of_images,int npasses,
      string imagedir,string subdir)
      {
         string dmconvertstr="dmconvert -f avi -n ";
         string scriptfilename;
         ofstream scriptstream;

         if (npasses==0)
         {
            dmconvertstr += "image#.jpg";
         }
         else if (npasses==1)
         {
            dmconvertstr += "singlet#.jpg";
         }
         else if (npasses==2)
         {
            dmconvertstr += "doublet#.jpg";
         }
         else if (npasses==3)
         {
            dmconvertstr += "triplet#.jpg";
         }

// On 10/3/03, Hyrum Anderson told us that the CINEPAK codec
// (compression/decompression) scheme is nearly universal on most
// WINDOWS machines.  And compressing with CINEPAK leads to nearly a
// 10-fold savings in disk space!  So we now include the string
// "comp=qt_cvid" as a default parameter within our avi generation
// scripts.  We also note that increasing the inrate value yields
// movies which play faster.

         dmconvertstr += ",start="+stringfunc::number_to_string(istart+1)
            +",end="+stringfunc::number_to_string(number_of_images)
            +",step=1 -p video,comp=qt_cvid,inrate=2.0 ";

         if (npasses==0)
         {
            dmconvertstr += "image#.jpg";
         }
         else if (npasses==1)
         {
            dmconvertstr += "singlet#.jpg";
         }
         else if (npasses==2)
         {
            dmconvertstr += "doublet#.jpg";
         }
         else if (npasses==3)
         {
            dmconvertstr += "triplet#.jpg";
         }
   
         dmconvertstr += " movie.avi";

         scriptfilename=imagedir+subdir+"generate_avi";
         filefunc::openfile(scriptfilename,scriptstream);
         scriptstream << dmconvertstr << endl;
         filefunc::closefile(scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Boolean method query_viewgraph_mode queries the user whether output
// images should be prepared for viewgraph presentations.

   bool query_viewgraph_mode(
      bool input_param_file,string inputline[],unsigned int& currlinenumber)
      {
         char viewgraph_char;
         string outputline[1];

         outputline[0]=
            "Generate movies for viewgraph presentation? (y/n):";
         viewgraph_char=stringfunc::mygetchar(
            1,outputline,input_param_file,inputline,currlinenumber);
         return (viewgraph_char=='y');
      }

// ---------------------------------------------------------------------
   void generate_crop_script(
      int start_image,int stop_image,string basefilename,string imagedir,
      string scriptfilename,string init_suffix,string final_suffix,
      int image_skip,int n_digits)
      {
         scriptfilename=imagedir+scriptfilename;
         ofstream scriptstream;
         filefunc::openfile(scriptfilename,scriptstream);

         for (int i=start_image; i<stop_image; i += image_skip)
         {
            string convertstr="convert "+basefilename
               +stringfunc::integer_to_string(i,n_digits)+"."+init_suffix+" ";
            convertstr += 
               basefilename+stringfunc::integer_to_string(i,n_digits)
               +"."+final_suffix;
            scriptstream << convertstr << endl;
         }

         cout << "Conversion script filename = " << scriptfilename << endl;
         
         filefunc::closefile(scriptfilename,scriptstream);
   
// Make script executable:

         string unixcommandstr="chmod a+x "+scriptfilename;
         sysfunc::unix_command(unixcommandstr);
      }

// ==========================================================================
// Progress reporting methods
// ==========================================================================

   void print_elapsed_time()
   {
      double elapsed_secs,elapsed_mins,elapsed_hours;
      timefunc::get_elapsed_time(elapsed_secs,elapsed_mins,elapsed_hours);

      cout << "............................................................." 
           << endl;
      cout << "Current time = " << timefunc::getcurrdate() << endl;
      cout << "Elapsed time = " 
           << stringfunc::number_to_string(elapsed_secs,1) << " secs =   "
           << stringfunc::number_to_string(elapsed_mins,2) << " minutes =   " 
           << stringfunc::number_to_string(elapsed_hours,3) << " hours " 
           << endl;
      cout << "............................................................." 
           << endl;
   }

   void print_elapsed_time(ofstream& outstream)
   {
      double elapsed_secs,elapsed_mins,elapsed_hours;
      timefunc::get_elapsed_time(elapsed_secs,elapsed_mins,elapsed_hours);

      outstream << 
         "............................................................." 
           << endl;
      outstream << "Current time = " << timefunc::getcurrdate() << endl;
      outstream << "Elapsed time = " 
           << stringfunc::number_to_string(elapsed_secs,1) << " secs =   "
           << stringfunc::number_to_string(elapsed_mins,2) << " minutes =   " 
           << stringfunc::number_to_string(elapsed_hours,3) << " hours " 
           << endl;
      outstream << 
         "............................................................." 
           << endl;
   }

   void print_remaining_time(double progress_frac)
   {
      double elapsed_secs,elapsed_mins,elapsed_hours;
      timefunc::get_elapsed_time(elapsed_secs,elapsed_mins,elapsed_hours);

      if (progress_frac < 1E-4) return;
      double ratio=(1-progress_frac)/progress_frac;
      double remaining_secs=ratio*elapsed_secs;
      double remaining_mins=ratio*elapsed_mins;
      double remaining_hours=ratio*elapsed_hours;

      cout << "............................................................." 
           << endl;
      cout << "Current time = " << timefunc::getcurrdate() << endl;
      cout << "Estimated remaining time = " 
           << stringfunc::number_to_string(remaining_secs,1) << " secs =   "
           << stringfunc::number_to_string(remaining_mins,2) << " mins =   " 
           << stringfunc::number_to_string(remaining_hours,3) << " hours " 
           << endl;
      cout << "............................................................." 
           << endl;
   }

   void print_elapsed_and_remaining_time(double progress_frac)
   {
      double elapsed_secs,elapsed_mins,elapsed_hours;
      timefunc::get_elapsed_time(elapsed_secs,elapsed_mins,elapsed_hours);

      if (progress_frac < 1E-4) return;
      double ratio=(1-progress_frac)/progress_frac;
      double remaining_secs=ratio*elapsed_secs;
      double remaining_mins=ratio*elapsed_mins;
      double remaining_hours=ratio*elapsed_hours;

      cout << "............................................................." 
           << endl;
      cout << "Current time = " << timefunc::getcurrdate() << endl;
      cout << "Elapsed time = " 
           << stringfunc::number_to_string(elapsed_secs,1) << " secs =   "
           << stringfunc::number_to_string(elapsed_mins,2) << " minutes =   " 
           << stringfunc::number_to_string(elapsed_hours,3) << " hours " 
           << endl;
      cout << "Estimated remaining time = " 
           << stringfunc::number_to_string(remaining_secs,1) << " secs =   "
           << stringfunc::number_to_string(remaining_mins,2) << " mins =   " 
           << stringfunc::number_to_string(remaining_hours,3) << " hours " 
           << endl;
      cout << "............................................................." 
           << endl;
   }

   double update_progress_fraction(int n,int n_progress,int n_max)
   {
      if (n==0) cout << "Progress fraction: " << flush;

      double frac=double(n)/double(n_max);      
      if (n%n_progress==0)
      {
         cout << stringfunc::number_to_string(frac,3) << " " << flush;
         if (n >= n_max-1) cout << endl;
      }
      return frac;
   }

   double update_progress_and_remaining_time(int n, int n_progress, int n_max)
   {
      double frac=double(n)/double(n_max);
      if (n%n_progress==0)
      {
         cout << "Progress fraction: " << stringfunc::number_to_string(frac,3)
              << endl;
         if(n > 0)
         {
            outputfunc::print_elapsed_and_remaining_time(frac);
         }
      }
      return frac;
   }

   double update_progress_and_remaining_time(
      int& counter, double n, int n_progress, double n_max)
   {
      double frac=n/n_max;
      if (counter%n_progress==0)
      {
        cout << "Progress fraction: " << stringfunc::number_to_string(frac,4) << endl;
        if(counter > 0)
        {
          outputfunc::print_elapsed_and_remaining_time(frac);
          counter = 0;
        }
      }
      return frac;
   }

   void display_flow_diagram(
      string flow_diag_subdir,string curr_diag_filename)
   {
      string unix_cmd=
         "display -geometry +0+0 "+flow_diag_subdir+curr_diag_filename+" &";
      sysfunc::unix_command(unix_cmd);
   }

/*
   void display_timed_flow_diagram(
      string flow_diag_subdir,string curr_diag_filename,
      int px_start,int py_start,int elapsed_mins,int elapsed_secs)
   {

      int fontsize=50;
      int npx=2.25*fontsize;
      int npy=2.25*fontsize;
      double angle=0;
      vector<string> text_lines;
      text_lines.push_back("01:29");
      colorfunc::RGB text_rgb(1,1,1);

      string png_filename="output.png";
      double background_greyscale_intensity=0;
      string font_path=
         "/home/cho/programs/c++/svn/projects/src/mains/imagetext/fonts/arial.ttf";
      
      pngfunc::convert_textlines_to_PNG(
         px_start,py_start,npx,npy,angle,
         text_lines,png_filename,text_rgb,
         background_greyscale_intensity,font_path,fontsize);

      string unix_cmd=
         "display -geometry +0+0 "+png_filename+" &";
      sysfunc::unix_command(unix_cmd);
   }
*/
 
   
} // outputfunc namespace
