// ==========================================================================
// General system methods namespace
// ==========================================================================
// Last updated on 1/18/10; 7/23/13; 10/4/13; 4/4/14
// ==========================================================================

#ifndef SYSFUNCS_H
#define SYSFUNCS_H

#include <string>
#include <vector>

namespace sysfunc
{
   void clearscreen();
   std::string get_cgibindir();
   std::string get_projectsrootdir();
   std::string get_environmental_variable(std::string env_var_name);
   std::string get_hostname();
   std::string get_loginname();
   std::vector<int> my_get_pid(std::string processname);
   std::string get_process_name();
   std::string get_process_name(int pid);
   void import_root_scrn();
   void kill_process(std::string processname);
   void out_of_memory();
   void run_display(std::string filename);
   void run_display(std::string filename,int x_origin,int y_origin);

   void show_free_memory();
   void unix_compress(const std::string& filename);
   bool unix_uncompress(const std::string& filename,std::string& prefix);
   int unix_command(std::string unixcommandstr);
   std::string unix_command_output(std::string unixcommandstr);

// 2nd xterm methods:

   std::string get_tty_dir();
   std::string most_recent_xterm_filename();
   std::string launch_new_xterm(
      std::string window_title="xterm",std::string fontname="10x20",
      std::string geometry_coords="80x25+0+0",
      std::string fg_color="white",std::string bg_color="black",
      bool run_in_background=false);
   void clear_xterm(std::string tty_devicename);
}

#endif  // general/sysfuncs.h


