// =========================================================================
// Header file for stand-alone output functions.
// =========================================================================
// Last modified on 4/5/14; 11/28/15; 12/1/15; 12/2/15
// =========================================================================

#ifndef OUTPUTFUNCS_H
#define OUTPUTFUNCS_H

#include <fstream>
#include <iostream>
#include <string>
#include <time.h>

namespace outputfunc
{
   void newline();
   void enter_continue_char();
   void enter_continue_char(std::string message);

   std::string select_logfile_name(
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);
   std::string select_output_directory(
      bool public_software,bool pop_open_window,
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber,std::string basedirname);
   void write_big_banner(std::string message);
   void write_banner(std::string message);
   void print_filename_and_date(
      std::ofstream& datastream,std::string filenamestr);
   void write_initial_results_info(std::ofstream& datastream);
   double processing_time(const time_t& start_processing_time);
   void report_processing_time_info(
      long start_processing_time,std::ostream& datastream);
   void write_program_finished_message();

// Script generation methods

   void generate_view_script(
      int nfilenames,std::string basename[],std::string imagedir,
      std::string scriptfilename);
   void generate_view_scripts(
      int number_of_images,std::string basefilename,std::string imagedir,
      std::string scriptfilename);
   void generate_animation_script(
      int number_of_images,std::string basefilename,std::string imagedir,
      std::string scriptfilename,int delay=10,std::string suffix="jpg",
      int skip=1);
   void generate_animation_script(
      int start_image,int stop_image,std::string basefilename,
      std::string imagedir,std::string scriptfilename,int delay=10,
      std::string suffix="jpg",int skip=1);

   void generate_conversion_script(
      int start_image,int stop_image,std::string basefilename,
      std::string input_dir,std::string output_dir,std::string scriptfilename,
      std::string init_suffix,std::string final_suffix,
      int image_skip,int n_digits,int quality=-1);
   void generate_conversion_script(
      int start_image,int stop_image,
      std::string basefilename,std::string imagedir,
      std::string scriptfilename,std::string init_suffix,
      std::string final_suffix,int image_skip=1,int n_digits=4,int quality=-1);

   void generate_avi_script(
      int istart,int number_of_images,int npasses,std::string imagedir);
   void generate_avi_script(
      int istart,int number_of_images,int npasses,
      std::string imagedir,std::string subdir);
   bool query_viewgraph_mode(
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);

// Progress reporting methods

   void print_elapsed_time();
   void print_elapsed_time(std::ofstream& outstream);
   void print_remaining_time(double progress_frac);
   void print_elapsed_and_remaining_time(double progress_frac);

   double update_progress_fraction(int n,int n_progress,int n_max);
   double update_progress_and_remaining_time(
      int& counter, double n, int n_progress, double n_max);
   double update_progress_and_remaining_time(int n, int n_progress, int n_max);
   void display_flow_diagram(
      std::string flow_diag_subdir,std::string curr_diag_filename);
   void display_timed_flow_diagram(
      std::string flow_diag_subdir,std::string curr_diag_filename,
      int px,int py,int elapsed_mins,int elapsed_secs);
}

// ==========================================================================
// Inlined methods
// ==========================================================================

inline void outputfunc::newline()
{
   std::cout << std::endl;
}

#endif // general/outputfuncs.h




