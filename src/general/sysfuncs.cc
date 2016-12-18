// ==========================================================================
// Some useful system functions
// ==========================================================================
// Last updated on 1/18/10; 7/23/13; 10/4/13; 4/4/14
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <new> 		// Needed for bad_alloc
#include <sys/stat.h>   // Needed for stat() Unix system call
#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <time.h>	// Needed for ctime() Unix system call
#include <unistd.h>     // Needed for usleep() Unix system call
#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::bad_alloc;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::ios;
using std::string;
using std::vector;

namespace sysfunc
{

   void clearscreen()
      {
         string unixcommandstr="clear";
         unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method get_cgibindir returns the location of the apache web
// server's cgi-bin directory.

   string get_cgibindir()
      {
         string cgibindir="/home/httpd/cgi-bin/";
         return cgibindir;
      }

// ---------------------------------------------------------------------
// Method get_projectsrootdir reads in the value of the projects root
// directory stored within environmental variable PROJECTSROOTDIR.
// The returned string is terminated with a trailing "/" if the
// environmental variable is not already so terminated...

   string get_projectsrootdir()
      {
         string projectsrootdir=get_environmental_variable(
            "PROJECTSROOT");
         string lastchar=projectsrootdir.substr(
            projectsrootdir.length()-1,1);
         if (lastchar != "/") projectsrootdir += "/";
         return projectsrootdir;
      }

// ---------------------------------------------------------------------
// Method get_environmental_variable takes in a C++ string which
// contains the name of some environmental variable and returns its
// contents as a string:

   string get_environmental_variable(string env_var_name)
      {
         char *env_var_value_ptr=getenv(env_var_name.c_str());
         if (env_var_value_ptr != NULL)
         {

// We learned from Peter Buchak on 10/1/02 that we can construct a C++
// string from a C-style char* via the following syntax:

            string env_var_value(env_var_value_ptr);
            return env_var_value;
         }
         else
         {
            return "";
         }
      }

// ---------------------------------------------------------------------
// Method get_hostname() returns a C++ string containing the output
// from the system gethostname function:

   string get_hostname()
      {
         const int MAX_NAMELENGTH=100;
         char hostname_cstr[MAX_NAMELENGTH];
         size_t len=MAX_NAMELENGTH;
         string hostnamestring="";

         int err=gethostname(hostname_cstr,len);
         if (err==-1)
         {
            hostnamestring="Call to gethostname failed!";
         }
         else
         {
            hostnamestring=hostname_cstr;
            if (hostnamestring=="grp93.tin.hanscom.af.smil.mil")
            {
               hostnamestring="group93.tin.hanscom.af.smil.mil";
            }
         }
         return hostnamestring;
      }

// ---------------------------------------------------------------------
// Method get_loginname() returns a C++ string containing the output
// from the system cuserid() function.

   string get_loginname()
      {
//         cout << "inside sysfunc::get_loginnname()" << endl;
         char* buffer=new char[255];
         buffer=cuserid(NULL);
         string loginname(buffer);
//         cout << "loginname = " << loginname << endl;
         return loginname;
      }
   
// ---------------------------------------------------------------------
// Method my_get_pid takes in the name of some process and returns
// npids process id numbers within integer array pid that correspond
// to this process name.  On 3/17/03, we discovered (with lots of help
// from Vadim G!) that get_pid() is an SGI macro.  So we had to rename
// our get_pid() method in order to avoid a name clash!

   vector<int> my_get_pid(string processname)
      {
//         cout << "inside sysfunc::my_get_pid(), processname = "
//              << processname << endl;
         string unixcommandstr;
   
         string tmpfilename="ps.tmp";
         if (filefunc::fileexist(tmpfilename))
         {
            unixcommandstr="/bin/rm -f "+tmpfilename;
            unix_command(unixcommandstr);
         }
   
         unixcommandstr="ps -C "+processname+" --no-headers >> "+tmpfilename;
//         cout << "unix_cmd = " << unixcommandstr << endl;
         unix_command(unixcommandstr);
   
         filefunc::ReadInfile(tmpfilename);

         vector<int> process_IDs;
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            vector<string> pidstr=
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[i]);
            process_IDs.push_back(
               basic_math::round(stringfunc::string_to_number(pidstr[0])));
         }

         unixcommandstr="/bin/rm -f "+tmpfilename;
         unix_command(unixcommandstr);

         return process_IDs;
      }

// ---------------------------------------------------------------------
// Method get_process_name takes in the process ID of some job and
// returns its name as a string output:

   string get_process_name(int pid)
      {
         const string tmpfilename="ps.tmp";
         string unixcommandstr;
         if (filefunc::fileexist(tmpfilename))
         {
            unixcommandstr="/bin/rm -f "+tmpfilename;
            unix_command(unixcommandstr);
         }
   
         unixcommandstr="ps -p "+stringfunc::number_to_string(pid)
            +" --no-headers >> "+tmpfilename;
         unix_command(unixcommandstr);
   
         unsigned int nlines;
         string line[10];
         filefunc::ReadInfile(tmpfilename,line,nlines);

         unixcommandstr="/bin/rm -f "+tmpfilename;
         unix_command(unixcommandstr);

         vector<string> pidstr=
            stringfunc::decompose_string_into_substrings(line[0]);
         return pidstr[3];
      }

   string get_process_name()
      {
         pid_t currpid=getpid();
         return get_process_name(currpid);
      }

// ---------------------------------------------------------------------
// Method import_root_scrn captures the current contents of the
// root window using the ImageMagick import utility and sends it to
// the output jpeg file "root_i.jpg" where quasi random integer i is
// related to the current time.

   void import_root_scrn()
      {
         time_t currtime=time(NULL);
         int i=currtime%100;
   
         string filename="root_"+stringfunc::number_to_string(i)+".jpg";
         string unixcommandstr="import -window root "+filename;
         sleep(5);
         unix_command(unixcommandstr);
      }

// -------------------------------------------------------------------------- 
   void kill_process(string processname)
      {
         int calling_pid=getpid();
         string unixcommandstr;
         unsigned int nprocesses_to_be_killed_first;

// First kill all processes corresponding to processname EXCLUDING the
// calling process if it shares that process name:

         do
         {
            vector<int> pids=sysfunc::my_get_pid(processname);

            unsigned int npids=pids.size();
            nprocesses_to_be_killed_first=npids;
            for (unsigned i=0; i<npids; i++)
            {
               if (pids[i]==calling_pid)
               {
                  nprocesses_to_be_killed_first=npids-1;
               }
            }
      
            for (unsigned i=0; i<npids; i++)
            {
               if (pids[i] != calling_pid)
               {
                  unixcommandstr="kill -9 "+stringfunc::number_to_string(
                     pids[i]);
                  unix_command(unixcommandstr);
               }
            }
         }
         while (nprocesses_to_be_killed_first > 0);

// Now kill calling process if its name matches processname:

         unsigned int npids=0;
         do
         {
            vector<int> pids=sysfunc::my_get_pid(processname);
            npids=pids.size();
            for (unsigned int i=0; i<npids; i++)
            {
               unixcommandstr="kill -9 "+stringfunc::number_to_string(pids[i]);
               unix_command(unixcommandstr);
            }
         }
         while (npids > 0);
      }

// ---------------------------------------------------------------------
// Be sure to include the line "set_new_handler(out_of_memory)" at the
// top of every main program in order to be warned if the system runs
// out of memory while trying to dynamically allocate memory space.

   void out_of_memory()
      {
         outputfunc::newline();
         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
         cerr << "Operator new failed: System out of memory !" << endl;
         outputfunc::newline();
         cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
         outputfunc::newline();
         throw bad_alloc();
      }

// ---------------------------------------------------------------------
   void run_display(string filename)
      {
         sysfunc::run_display(filename,0,0);
      }

   void run_display(string filename,int x_origin,int y_origin)
      {
         string unixcommandstr;
   
         if (x_origin < 0 && y_origin < 0)
         {
            unixcommandstr="display -update 1 -geometry -"+
               stringfunc::number_to_string(abs(x_origin))+"-"+
               stringfunc::number_to_string(abs(y_origin))+" "+filename+" &";
         }
         else
         {
            unixcommandstr="display -update 1 -geometry +"+
               stringfunc::number_to_string(x_origin)+"+"
               +stringfunc::number_to_string(y_origin)+" "+filename+" &";
         }
         unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method show_free_memory executes a Unix system call which
// displays the amount of free memory in megabytes:

   void show_free_memory()
      {
         sleep(2);
//         string unixcommandstr="free -b";
         string unixcommandstr="free -m";
         unix_command(unixcommandstr);
         outputfunc::newline();
         sleep(2);
      }

// ---------------------------------------------------------------------
// Method unix_compress applies the standard Unix compression
// utility to the input file.  This method resolves symbolic links
// and compresses the resolved filename and not the link itself.
// After compressing the resolved file, it deletes all dangling links
// and generates a new soft link chain to the compressed target file.

   void unix_compress(const string& filename)
      {
         bool link_exists=filefunc::symboliclink_exist(filename);

         const int MAX_NLEVELS=10;
         int nlevels;
         string resolved_filename[MAX_NLEVELS];
         filefunc::resolve_linked_filename(
            filename,nlevels,resolved_filename);

         string unixcommandstr="compress "+resolved_filename[nlevels-1];
         unix_command(unixcommandstr);

         if (link_exists)
         {
            for (int n=0; n<nlevels-1; n++)
            {
               unixcommandstr="rm -f "+resolved_filename[n];
               unix_command(unixcommandstr);
            }
            for (int n=nlevels-2; n>=0; n--)
            {
               string linksource=resolved_filename[n+1]+".Z";
               string linktarget=resolved_filename[n]+".Z";
               unixcommandstr="ln -s "+linksource+" "+linktarget;
               unix_command(unixcommandstr);
            }
         }
      }

// Method unix_uncompress takes in the name of a file which has
// been compressed using the standard Unix compression utility.  The
// filename may or may not contain a ".Z" suffix.  If such a
// compressed file exists, this method uncompresses it and returns
// true.  This method follows symbolic links and uncompresses the
// resolved filename at the end of the link chain and not the links
// themselves.  After uncompressing the resolved file, it deletes all
// dangling links and generates new soft links to the uncompressed
// target file.

   bool unix_uncompress(const string& filename,string& prefix)
      {
         bool link_exists=filefunc::symboliclink_exist(filename);

         const int MAX_NLEVELS=10;
         int nlevels;
         string resolved_filename[MAX_NLEVELS];
         filefunc::resolve_linked_filename(
            filename,nlevels,resolved_filename);
   
         string substring=resolved_filename[nlevels-1].
            substr(resolved_filename[nlevels-1].length()-2,2);

         string resolved_prefix;
         if (substring==".Z")
         {
            prefix=filename.substr(0,filename.length()-2);
            resolved_prefix=resolved_filename[nlevels-1].substr(
               0,resolved_filename[nlevels-1].length()-2);
         }
         else
         {
            prefix=filename;
            resolved_prefix=resolved_filename[nlevels-1];
         }

         bool unix_compressed_file=false;
         string unixcommandstr;
         if (filefunc::fileexist(resolved_prefix+".Z"))
         {
            unix_compressed_file=true;
            unixcommandstr="uncompress "+resolved_prefix+".Z";
            unix_command(unixcommandstr);
         }
   
// Soft links to compressed files are now dangling at this point.
// Delete them and then sequentially generate new soft links to the
// uncompressed file...

         if (link_exists && unix_compressed_file)
         {
            for (int n=0; n<nlevels-1; n++)
            {
               unixcommandstr="rm -f "+resolved_filename[n];
               unix_command(unixcommandstr);
            }
            string basename=filefunc::getbasename(resolved_prefix);
            for (int n=nlevels-2; n>=0; n--)
            {
               string dirname=filefunc::getdirname(resolved_filename[n]);
               string linksource=filefunc::getdirname(
                  resolved_filename[n+1])+basename;
               string linktarget=filefunc::getdirname(
                  resolved_filename[n])+basename;
               unixcommandstr="ln -s "+linksource+" "+linktarget;
               unix_command(unixcommandstr);
            }
         }
         return unix_compressed_file;
      }

// ---------------------------------------------------------------------
// Method unix_command takes in a C++ string which should contain a
// valid unix command and then executes it via a call to system.  The
// value returned in variable status is 127 or -1 if the command
// fails.  Otherwise, status equals the return code.

   int unix_command(string unixcommandstr)
      {
         int status=system(unixcommandstr.c_str());
         return status;
      }

// same as above but returns output from command instead of 
// status integer
   string unix_command_output(string unixcommandstr)
      {
         string data;
         FILE * stream;
         const int max_buffer = 256;
         char buffer[max_buffer];
         unixcommandstr.append(" 2>&1");

         stream = popen(unixcommandstr.c_str(), "r");
         if (stream) {
            while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
            pclose(stream);
         } 
         return data;
       }





// ==========================================================================
// 2nd xterm methods
// ==========================================================================

// Method get_tty_dir performs a linux tty system call to determine
// the full path of the directory where dynamic xterminal links are
// stored.
   
   string get_tty_dir()
      {
         string tmpfilename=filefunc::generate_tmpfilename();
         string unixcommandstr="tty > "+tmpfilename;
         unix_command(unixcommandstr);

         vector<string> line;
         if (!filefunc::ReadInfile(tmpfilename,line))
         {
            cout << "Error in sysfunc::get_tty_dir() !" << endl;
	    return "";
         }
         else
         {
            filefunc::deletefile(tmpfilename);
            return filefunc::getdirname(line[0]);
         }
      }

// ---------------------------------------------------------------------
// Method most_recent_xterm_filename returns the full path for the
// device which corresponds to the most recently created (not
// necessarily modified) xterm:

   string most_recent_xterm_filename()
      {
         string tty_dir=get_tty_dir();
         string tmpfilename=filefunc::generate_tmpfilename();
         string unixcommandstr="ls -ct -1 "+tty_dir+"> "+tmpfilename;
         unix_command(unixcommandstr);

         vector<string> line;
         if (!filefunc::ReadInfile(tmpfilename,line))
         {
            cout << "Error in sysfunc::most_recent_xterm() !" << endl;
	    return "";
         }
         else
         {
//            filefunc::deletefile(tmpfilename);
            string xterm_number=line[0];
            return tty_dir+xterm_number;
         }
      }

// ---------------------------------------------------------------------
// Method launch_new_xterm opens up a new xterminal and returns its
// full device name within the output string.

   string launch_new_xterm(
      string window_title,string fontname,string geometry_coords,
      string fg_color,string bg_color,bool run_in_background_flag)
      {
         string core_command="/usr/X11R6/bin/xterm ";
         string title_option="-title \""+window_title+"\" ";
         string font_option="-fn "+fontname+" ";
         string geometry_option="-geometry "+geometry_coords+" ";
         string line_option="-sb -sl 1000 ";
         string color_option="-fg "+fg_color+" -bg "+bg_color+" -cr red ";
         string unixcommandstr=core_command+title_option+font_option+
            geometry_option+line_option+color_option;
         if (run_in_background_flag)
         {
            unixcommandstr += "&";
         }
//         cout << "unixcommandstr = " << unixcommandstr << endl;
         unix_command(unixcommandstr);

// On 7/10/05, we found that we need to give the xterm command some
// time to complete before we attempt to connect a stream to the
// device filename associated with the new xterm.  So we execute a
// sleep command for a fraction of a second:

//         sleep(1);
//         usleep(500000);
         usleep(100000);

         return most_recent_xterm_filename();
      }

// ---------------------------------------------------------------------
// Method clear_xterm clears the xterm associated with input file
// tty_devicename.

   void clear_xterm(string tty_devicename)
      {
         string unixcommandstr="tput clear > "+tty_devicename;
         unix_command(unixcommandstr);
      }
   
} // sysfunc namespace
