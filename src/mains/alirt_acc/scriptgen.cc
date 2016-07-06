// ==========================================================================
// Program SCRIPT_GENERATOR queries the user for the name of the
// program to be run on multiple passes of imagery data.  It also asks
// for the full pathname of a directory containing multiple raw data
// files (which may or may not be compressed via the standard UNIX
// compression utility).  Given these two pieces of information,
// SCRIPT_GENERATOR first creates subdirectories within the specified
// directory which are annotated with the name of the program.  All
// output from the program for each pass will be written to these
// subdirectories.  Within these subdirectories, SCRIPT_GENERATOR also
// creates a 2-line file which provides the necessary input for the
// program to run.  Finally, SCRIPT_GENERATOR creates an executable
// script called "Run_program" which the user can simply chant in
// order to start running the program on all of the input imagery
// files within the user specified directory.
// ==========================================================================
// Last updated on 2/25/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <new>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::ostream;
   using std::ofstream;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int max_nlines=500;
   const string lsfilename="/tmp/ls.out";
   
   bool input_param_file;
   unsigned int ninputlines,nlines,currlinenumber=0;

   string inputline[200],outputline[5];
   string *line=new string[max_nlines];
   string *xyz_filename=new string[max_nlines];
   ofstream scriptstream,batchstream;

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
//   clearscreen();
   cout.precision(3);
   cout.setf(ios::showpoint);

   outputfunc::newline();
   outputline[0]="Enter full pathname for logfile:";
   string logfile_name=stringfunc::mygetstring(
      1,outputline,input_param_file,inputline,currlinenumber);

   outputfunc::newline();
   outputline[0]="Enter full pathname of ALIRT program to be run:";
   string program_name=stringfunc::mygetstring(
      1,outputline,input_param_file,inputline,currlinenumber);

   outputfunc::newline();
   outputline[0]="Enter full pathname of subdirectory";
   outputline[1]="containing input imagery files:";
   string subdirname=stringfunc::mygetstring(
      2,outputline,input_param_file,inputline,currlinenumber);
   subdirname += "/";
   string scriptfile=subdirname+"Run_"+filefunc::getbasename(program_name);
   filefunc::openfile(scriptfile,scriptstream);

   bool nice=true;
   string unixcommandstr;
   if (filefunc::fileexist(lsfilename))
   {
      unixcommandstr="rm "+lsfilename;
      sysfunc::unix_command(unixcommandstr);
   }
   unixcommandstr="ls "+subdirname+" > "+lsfilename;
   sysfunc::unix_command(unixcommandstr);

// In order to another user to be able to delete /tmp/ls.out, we need
// to change its permissions so that it's writeable by all users:

   unixcommandstr="chmod a+w "+lsfilename;
   sysfunc::unix_command(unixcommandstr);

   filefunc::ReadInfile(lsfilename,line,nlines);

// Delete file containing ls output after its contents have been read
// into string array line:

   if (filefunc::fileexist(lsfilename))
   {
      unixcommandstr="rm "+lsfilename;
      sysfunc::unix_command(unixcommandstr);
   }

   int j=0;
   for (unsigned int i=0; i<nlines; i++)
   {
      string::size_type xyz_pos=line[i].find("xyz");
      if (xyz_pos != string::npos && 
          !filefunc::direxist(subdirname+line[i]))
      {
         xyz_filename[j]=subdirname+line[i];

         int dot_position = xyz_filename[j].find_first_of(".",0);
         string basename = xyz_filename[j].substr(0,dot_position);
         string output_dirname=xyz_filename[j]+"_results/";
         string output_subdirname=output_dirname
            +filefunc::getbasename(program_name)+"/";

         if (!filefunc::direxist(output_dirname))
            sysfunc::unix_command("mkdir -p "+output_dirname);
         if (!filefunc::direxist(output_subdirname))
            sysfunc::unix_command("mkdir -p "+output_subdirname);

         string input_filename=output_subdirname+line[i]+"_input";

         scriptstream << endl;
         if (nice) scriptstream << "nice ";
         scriptstream << program_name << " " << input_filename
                      << endl;

// Write out individual pass batch files:

         filefunc::openfile(input_filename,batchstream);
         batchstream << logfile_name << endl;
         batchstream << xyz_filename[j] << endl;
         batchstream << output_subdirname << endl;
//         string flight_filename=ladarfunc::public_flight_path_filename(
//            xyz_filename[j]);
//         cout << "flight_filename = " << flight_filename << endl;
//         batchstream << flight_filename << endl;
//         batchstream << "/tmp/junk" << endl;
         filefunc::closefile(input_filename,batchstream);
         j++;
      }
   } // loop over index i
   
   filefunc::closefile(scriptfile,scriptstream);
   sysfunc::unix_command("chmod a+x "+scriptfile);

   delete [] line;
   delete [] xyz_filename;
}




