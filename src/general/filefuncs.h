// ==========================================================================
// Header file for some useful file manipulation functions
// ==========================================================================
// Last updated on 12/27/13; 11/2/15; 3/3/16; 3/6/16
// ==========================================================================

#ifndef FILEFUNCS_H
#define FILEFUNCS_H

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>

namespace filefunc
{

   extern std::vector<std::string> text_line;

// File opening/closing methods:

   bool openfile(std::string filenamestr,std::ifstream& filestream);
   bool openfile(std::string filenamestr,std::ofstream& filestream);
   bool openfile(std::string filenamestr,std::fstream& filestream);
   void closefile(std::string filenamestr,std::ifstream& filestream);
   void closefile(std::string filenamestr,std::ofstream& filestream);
   void closefile(std::string filenamestr,std::fstream& filestream);
   bool appendfile(std::string filenamestr,std::ofstream& filestream);
   bool open_binaryfile(std::string filenamestr,std::ofstream& filestream);
   bool open_binaryfile(std::string filenamestr,std::ifstream& filestream);
   bool open_binaryfile(
      std::string filenamestr,std::ifstream& filestream,long long& nbytes);
   long long size_of_file_in_bytes(std::string filenamestr);
   void make_executable(std::string filenamestr);

// Subdirectory methods:

   void dircreate(std::string dirnamestr);
   bool mkdirp(std::string dirnamestr, mode_t mode);
   bool direxist(std::string dirnamestr);
   void add_trailing_dir_slash(std::string& dirnamestr);

   std::vector<std::string> files_in_subdir(
      std::string subdir,bool search_all_children_dirs_flag=false);
   std::vector<std::string> files_in_subdir_and_children_dirs(
      std::string subdir);
   std::vector<std::string> files_in_just_subdir(
      std::string subdir,std::string ls_args="");
   std::vector<std::string> alternate_files_in_just_subdir(std::string subdir);
   std::vector<std::string> files_in_subdir_matching_specified_suffixes(
      std::vector<std::string> allowed_suffixes,std::string subdir,
      bool search_all_children_dirs_flag=false);
   std::vector<std::string> image_files_in_subdir(
      std::string subdir,bool search_all_children_dirs_flag=false);
   std::vector<std::string> image_files_in_multi_subdirs(
      std::vector<std::string> subdirs, bool search_all_children_dirs_flag
      =false);

   void latest_files_in_subdir(
      const std::string subdir,std::string& latest_filename,
      std::string& next_to_latest_filename);
   bool get_latest_files_in_subdir(
      std::string subdir,std::string& latest_filename,
      std::string& next_to_latest_filename);

   std::vector<std::string> files_in_subdir_matching_substring(
      std::string subdir,std::string substring,
      bool search_all_children_dirs_flag=false);
   std::vector<std::string> file_basenames_in_subdir_matching_substring(
      std::string subdir,std::string substring,
      bool search_all_children_dirs_flag=false);

   std::vector<std::string> files_sorted_by_bytesize(std::string subdir);

   void purge_files_in_subdir(
      std::string subdir,bool search_all_children_dirs_flag=false);
   void purge_files_with_suffix_in_subdir(
      std::string subdir,std::string file_suffix,
      bool search_all_children_dirs_flag=false);

   bool deletefile(std::string filenamestr);
   bool fileexist(std::string filenamestr);
   bool chardevexist(std::string chardevstr);
   std::vector<std::string> files_matching_pattern(
      std::string pattern_to_match);
   std::string generate_tmpfilename();
   std::string getbasename(std::string filename);
   std::string getbasename(std::string filename,std::string slash_char);
   std::string getdirname(std::string filename);
   std::string getprefix(std::string filename);
   std::string replace_suffix(std::string input_filename,std::string suffix);
   std::string get_pwd();
   std::string getsingleline();

   void bzip2_file(std::string filename);
   void bunzip2_file(std::string filename);
   void gzip_file(std::string filename);
   void gzip_file_if_gunzipped(std::string filename);
   void gunzip_file(std::string filename);
   void gunzip_file_if_gzipped(std::string filename);

   bool parameter_input(unsigned int argc,char *argv[],
                        std::vector<std::string>& param);
   void parameter_input(unsigned int argc,char *argv[],bool &input_param_file,
                        std::string line[],unsigned int& nlines);
   void parameter_input(bool &input_param_file,std::string line[],
                        unsigned int &nlines);
   void parameter_input(std::string inputfilename,
                        std::vector<std::string>& line);

   bool get_file_contents(std::string filename,std::string& contents);
   void tokenize_string(std::string input_str,std::vector<std::string>& lines);
   bool ReadInfile(std::string filenamestr,std::vector<std::string>& line);
   bool ReadInfile(
      std::string filenamestr,std::string line[],unsigned int &nlines);
   bool ReadInfile(std::string filenamestr,bool strip_comments_flag=true);
   void ReadInfile(std::string filename,int line_start,std::stringstream& ss);

   std::vector<std::string> ReadInStrings(std::string filename);
   std::vector<std::vector<std::string> > 
      ReadInSubstrings(std::string filenamestr);
   void ReadInSubstrings(
      std::string filename,std::vector<std::vector<std::string> >& 
      all_substrings);

   std::vector<double> ReadInNumbers(std::string filenamestr);
   std::vector<double> ReadInNumbers(std::stringstream& ss);

   std::vector< std::vector<double> > ReadInRowNumbers(
      unsigned int line_start,unsigned int n_numbers_per_row,
      std::string filename);
   std::vector< std::vector<double> > ReadInRowNumbers(std::string filename);

   std::string resolve_linked_filename(const std::string& pathname);
   void resolve_linked_filename(
      const std::string& pathname,int& nlevel,std::string resolvedName[]);
   bool symboliclink_exist(std::string linknamestr);

   double assign_file_epoch_time(std::string filenamestr);

// Metafile methods:

   void meta_to_adobe(std::string filename);
   void meta_to_gif(std::string filename);
   void meta_to_jpeg(std::string filename);
   void meta_to_ps(std::string filename);

// JPEG methods:

   void jpeg_to_gif(std::string filename);
   void jpeg_to_png(std::string filename);
   void ps_to_jpeg(std::string filename);

// Binary data importing methods:

   unsigned char* LoadUnsignedChars(const char* filename, size_t size);
   unsigned char* ReadUnsignedChars(std::string& filename,size_t size);

   void ExportUnsignedChars(
      unsigned char* data,const char* filename, size_t size);
   void ExportUnsignedChars(
      unsigned char* data,std::string& filename, size_t size);

   unsigned char* LoadUnsignedChars(
      const char* filename,int n_total_features,int n_feature_skip,size_t D);
   unsigned char* ReadUnsignedChars(
      std::string& filename,int n_total_features,int n_feature_skip,size_t D);

   char* LoadChars(const char* filename, size_t size);
   char* ReadChars(std::string& filename,size_t size);

   void export_floats(
      std::string& output_filename,std::vector<float>& float_values);
   void ExportFloats(
      float* data,size_t n_floats,std::string& filename);

   float* LoadFloats(const char* filename, size_t size);
   float* ReadFloats(std::string& filename,size_t size);

// Byte swapping methods to convert between big and little endian integers:

   uint16_t swap_uint16( uint16_t val );
   int16_t swap_int16( int16_t val ) ;
   uint32_t swap_uint32( uint32_t val );
   int32_t swap_int32( int32_t val );
   uint64_t swap_uint64( uint64_t val );
   int64_t swap_int64( int64_t val );

// ==========================================================================
// Inlined methods
// ==========================================================================

// Method get_pwd returns the present working directory name:

   inline std::string get_pwd()
      {
         std::string pwd=getenv("PWD");
         return pwd+"/";
      }

// Reading/writing objects from/to binary files:

   template <class T> inline void readobject(
      std::ifstream& binarystream,T& currobj)
      {
         binarystream.read((char *) &currobj,sizeof(currobj)); 
      }

   template <class T> inline void writeobject(
      std::ofstream& binarystream,T& currobj)
      {
         binarystream.write((char *) &currobj,sizeof(currobj));
      }

}

#endif  // general/filefuncs.h





