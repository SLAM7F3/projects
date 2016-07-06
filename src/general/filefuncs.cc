// ==========================================================================
// File methods definitions
// ==========================================================================
// Last updated on 6/30/14; 11/2/15; 3/3/16; 3/6/16
// ==========================================================================

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <glob.h>	// Needed for wild * char pattern matching
#include <sstream>
#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <sys/stat.h>   // Needed for stat() Unix system call
#include <time.h>	// Needed for ctime() Unix system call
#include <unistd.h>     // Needed for stat() Unix system call
#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::bad_alloc;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::fstream;
using std::ios;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

namespace filefunc
{

// Vector of strings "member object" to hold text information read in
// from ascii files:

   vector<string> text_line;

// ==========================================================================
// File opening/closing methods
// ==========================================================================

// Method Openfile takes in a filename string along with an
// ofstream and opens up a filestream:

   bool openfile(string filenamestr,ifstream& filestream)
      {
//         cout << "inside filefunc::openfile(), filename = "
//              << filenamestr << endl;

// Ifstream constructor must take a C-style char* string argument
// rather than a C++ string class object:

         filestream.open(filenamestr.c_str(),ios::in);

         bool file_opened=true;
         if (!filestream)
         {
            cerr << "Cannot open " << filenamestr << " for input" << endl;
            file_opened=false;
         }
         return file_opened;
      }

// ---------------------------------------------------------------------
   bool openfile(string filenamestr,ofstream& filestream)
      {

         filestream.open(filenamestr.c_str(),ios::out);

         bool file_opened=true;
         if (!filestream)
         {
            cerr << "Method filefunc::openfile(string,ofstream) cannot open " 
                 << filenamestr << " for output" << endl;
            file_opened=false;
         }
         return file_opened;
      }

// ---------------------------------------------------------------------
// This overloaded version of method Openfile takes in a filename
// string and opens up a filestream for joint input and output:

   bool openfile(string filenamestr,fstream& filestream)
      {
         filestream.open(filenamestr.c_str(),ios::in | ios::out);

         bool file_opened=true;
         if (!filestream)
         {
            cerr << "Method filefunc::openfile() cannot open " << filenamestr 
                 << " for input & output" << endl;
            file_opened=false;
         }
         return file_opened;
      }

// ---------------------------------------------------------------------
   void closefile(string filenamestr,ifstream& filestream)
      {
         filestream.close();
      }

   void closefile(string filenamestr,ofstream& filestream)
      {
         filestream.close();
      }

   void closefile(string filenamestr,fstream& filestream)
      {
         filestream.close();
      }

// ---------------------------------------------------------------------
// Method appendfile takes in a filename string along with an
// ofstream and opens up an already existing file for data appending.
// If the file corresponding to filename does not already exist, then
// it is simply opened for data writing rather than data appending:

   bool appendfile(string filenamestr,ofstream& filestream)
      {
         if (!fileexist(filenamestr))
         {
            openfile(filenamestr,filestream);
         }
         else
         {
            filestream.open(filenamestr.c_str(),ios::app);
         }
   
         if (!filestream)
         {
            cerr << "Cannot open " << filenamestr 
                 << " for output inside filefunc::appendfile()" << endl;
            return false;
         }
         else
         {
            return true;
         }
      }

// ---------------------------------------------------------------------
// Method open_binaryfile takes in a filename string along with an
// ofstream and tries to open up a filestream.  If the binary file is
// successfully opened, this boolean method and returns true.

   bool open_binaryfile(string filenamestr,ofstream& filestream)
      {
         const int iter_max=100;
         int iter=0;
         do 
         {
            filestream.open(filenamestr.c_str(),
                            ios::out|ios::binary|ios::trunc);
            string unixcommandstr="sleep 0.1";
            sysfunc::unix_command(unixcommandstr);
            iter++;
         }
         while (!filestream && iter < iter_max);
         
         if (!filestream)
         {
            cerr << "Cannot open " << filenamestr 
                 << " inside method filefunc::open_binaryfile()" << endl;
            return false;
         }
         else
         {
            return true;
         }
      }

// ---------------------------------------------------------------------
// These overloaded versions of open_binaryfile take in a filename
// string along with an ifstream and tries to open up a filestream.
// If the binary file is successfully opened, these boolean methods
// return true.

   bool open_binaryfile(string filenamestr,ifstream& filestream)
      {
         long long nbytes;
         return open_binaryfile(filenamestr,filestream,nbytes);
      }

   bool open_binaryfile(string filenamestr,ifstream& filestream,
   long long& nbytes)
      {

// Ifstream constructor must take a C-style char* string argument
// rather than a C++ string class object.

// File opening modes:

// ios::in	open file for reading
// ios::out	open file for writing
// ios::ate	initial position = end of file
// ios::app	every output is appended to end of file
// ios::trunc	erase file if it previously existed
// ios::binary	binary mode

// Open binary file and leave initial posn at end of file for purposes
// of counting file's total number of bytes:

         filestream.open(filenamestr.c_str(),ios::in|ios::binary|ios::ate);

         if (!filestream)
         {
            cerr << "Cannot open " << filenamestr 
                 << " inside filefunc::open_binaryfile()" << endl;
            return false;
         }
         else
         {
            nbytes=filestream.tellg();
//            cout << "Binary input file contains " << nbytes 
//                 << " bytes" << endl;
            filestream.seekg(0,ios::beg);
            return true;
         }
      }

   long long size_of_file_in_bytes(string filenamestr)
      {
//         cout << "inside filefunc::size_of_file_in_bytes()" << endl;
         long long nbytes=-1;
         ifstream filestream;
         if (open_binaryfile(filenamestr,filestream,nbytes))
         {
            closefile(filenamestr,filestream);
         }
//         cout << "nbytes = " << nbytes << endl;
         return nbytes;
      }

// ---------------------------------------------------------------------
   void make_executable(string filename)
   {
      string unix_cmd="chmod a+x "+filename;
      sysfunc::unix_command(unix_cmd);
   }

// ==========================================================================
// Subdirectory methods
// ==========================================================================

// Method dircreate checks whether output subdirectory dirnamestr
// exists.  If not, it creates it...

   void dircreate(string dirnamestr)
   {
      if(!direxist(dirnamestr))
      {
//         cout << "inside filefunc::dircreate()" << endl;
//         cout << "dirnamestr = " << dirnamestr << endl;
         mode_t mode(0777);
         mkdirp(dirnamestr, mode);
      }
   }
   
/*
   void dircreate(string dirnamestr)
      {
         cout << "inside filefunc::dircreate()" << endl;
         if(!direxist(dirnamestr))
         {
            cout << "Creating " << dirnamestr << endl;
            string unixcommandstr="mkdir -p "+dirnamestr;
            cout << "unix cmd = " << unixcommandstr << endl;
            sysfunc::unix_command(unixcommandstr);
            usleep(100);
         }
         cout << "at end of filefunc::dircreate()" << endl;
      }
*/

// ---------------------------------------------------------------------
// Method mkdirp generates a directory tree.  It was adapted in Mar 2016
// from http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux

   bool mkdirp(string path, mode_t mode)
   {
      // const cast for hack
      const char* path_cstr = path.c_str();
      char* p = const_cast<char*>(path_cstr);

      // Do mkdir for each slash until end of string or error
      while (*p != '\0') {
         // Skip first character
         p++;

         // Find first slash or end
         while(*p != '\0' && *p != '/') p++;

         // Remember value from p
         char v = *p;

         // Write end of string at p
         *p = '\0';

         // Create folder from path to '\0' inserted at p
         if(mkdir(path_cstr, mode) == -1 && errno != EEXIST) {
            *p = v;
            return false;
         }

         // Restore path to its former glory
         *p = v;
      }
      return true;
   }

// ---------------------------------------------------------------------
// We stole method direxist from Iva Mooney's "xutil.c" program.
// Chant "man 2 stat" in order to find out more options for the stat
// command.

   bool direxist(string dirnamestr)
      {
//         cout << "inside filefunc::direxist(), dirnamestr = "
//              << dirnamestr << endl;
         struct stat buffer;
         const char *dirname=dirnamestr.c_str();

         if (stat(dirname,&buffer))
            return(false);

         return(S_ISDIR(buffer.st_mode));
      }

// ---------------------------------------------------------------------
// Method add_trailing_dir_slash ensures the input directory name has
// a trailing slash.  

   void add_trailing_dir_slash(string& dirname)
      {
         const char DIRECTORY_SEPARATOR('/');
         if ( dirname.length() > 0 ) 
         {
            if ( dirname[dirname.length()-1] != DIRECTORY_SEPARATOR )
               dirname += DIRECTORY_SEPARATOR;
         }
      }

// ---------------------------------------------------------------------
// Method files_in_subdir() takes in a subdirectory and returns an STL
// vector containing the names of all files within that subdirectory.

   vector<string> files_in_subdir(
      string subdir,bool search_all_children_dirs_flag)
      {
//         cout << "inside filefunc::files_in_subdir()" << endl;
         
         if (search_all_children_dirs_flag)
         {
            return files_in_subdir_and_children_dirs(subdir);
         }
         else
         {
            return files_in_just_subdir(subdir);
         }
      }
   
// ---------------------------------------------------------------------
// Method files_in_subdir_and_children_dirs() takes in a subdirectory
// and returns an STL vector containing the names of all files within
// that subdirectory and all its recursive children subdirectories.

   vector<string> files_in_subdir_and_children_dirs(string subdir)
      {
//         cout << "inside filefunc::files_in_subdir_and_children_dirs()"
//              << endl;
         
         string tmp_filename=generate_tmpfilename();
         string unix_command="find -L "+subdir+" -name \"*\" -print > "
            +tmp_filename;
//          cout << "unix_cmd = " << unix_command << endl;
         
         sysfunc::unix_command(unix_command);

         ReadInfile(tmp_filename);
         deletefile(tmp_filename);

         return text_line;
      }

// ---------------------------------------------------------------------
// Method files_in_just_subdir() takes in a subdirectory and performs
// a Unix listing.  This method returns an STL vector containing names
// for all files within the specified subdirectory NOT including any
// of its children directories.

   vector<string> files_in_just_subdir(string subdir,string ls_args)
   {
//         cout << "inside filefunc::files_in_just_subdir()" << endl;
         string ls_cmd="ls "+ls_args+subdir;
//         cout << "ls_cmd = " << ls_cmd << endl;
         
         FILE* fp=NULL;
         int counter=0;
         while (fp==NULL && counter < 10)
         {
            fp = popen(ls_cmd.c_str(), "r");
//            cout << "In filefuncs::files_in_just_subdir(), ls_cmd = " 
//                 << ls_cmd << endl;
            counter++;
         }

         if (fp == NULL)
         {
            return alternate_files_in_just_subdir(subdir);
         }

         int PATHMAX=500;
//         int PATHMAX=2000;
         char path[PATHMAX];
         vector<string> subdir_filenames;
         while (fgets(path, PATHMAX, fp) != NULL)
         {
            string curr_subdir_filename=subdir+string(path);
            curr_subdir_filename=stringfunc::remove_trailing_whitespace(
               curr_subdir_filename);
            subdir_filenames.push_back(curr_subdir_filename);
         }

         int status = pclose(fp);
         if (status == -1) 
         {
            cout << "Error in filefunc::files_in_just_subdir() reported by pclose()" << endl;
         } 

         return subdir_filenames;
    }

// ---------------------------------------------------------------------
   vector<string> alternate_files_in_just_subdir(string subdir)
   {
//      cout << "inside filefunc::alternate_files_in_just_subdir()" << endl;
//      string tmp_filename=generate_tmpfilename();
      string tmp_filename="./filenames.dat";
      string unix_command="find "+subdir+" -maxdepth 1 -name \"*\" -print > "
         +tmp_filename;

      deletefile(tmp_filename);
      int counter=0;
      while (!filefunc::fileexist(tmp_filename) && counter < 10)
      {
         cout << "Sleeping in filefunc::alternate_files_in_just_subdir()" 
              << endl;
         usleep(100);
         cout << "unix_command = " << unix_command << endl;
         int status=sysfunc::unix_command(unix_command);
         cout << "status = " << status << endl;
         counter++;
      }

      long file_size=0;
      counter=0;
      while (file_size==0 && counter < 10)
      {
         file_size=filefunc::size_of_file_in_bytes(tmp_filename);
         cout << "Sleeping in filefunc::alternative_files_in_just_subdir()" 
              << endl;
         usleep(100);
         counter++;
      }

      ReadInfile(tmp_filename);

// On 9/18/12, we discovered the hard way that find command results
// are NOT generally ordered alphabetically.  So we explicitly perform
// a sort on STL vector text_line:

      std::sort(text_line.begin(),text_line.end());

//      cout << "text_line.size() = " << text_line.size() << endl;

      deletefile(tmp_filename);
      return text_line;
   }

// ---------------------------------------------------------------------
// Method files_in_subdir_matching_specified_suffixes()

   vector<string> files_in_subdir_matching_specified_suffixes(
      vector<string> allowed_suffixes,string subdir,
      bool search_all_children_dirs_flag)
      {
//         cout << "inside files_in_subdir_matching_specified_suffixes() "
//              << endl;

         vector<string> subdir_filenames=files_in_subdir(
            subdir,search_all_children_dirs_flag);
         vector<string> subdir_filenames_matching_suffixes;

//         cout << "subdir_filenames.size = "
//              << subdir_filenames.size() << endl;
         
         for (unsigned int i=0; i<subdir_filenames.size(); i++)
         {
            string curr_suffix=stringfunc::suffix(subdir_filenames[i]);
            for (unsigned int j=0; j<allowed_suffixes.size(); j++)
            {
               if (curr_suffix==allowed_suffixes[j])
               {
                  subdir_filenames_matching_suffixes.push_back(
                     subdir_filenames[i]);
                  break;
               }
            } // loop over index j labeling allowed suffixes
         } // loop over indx i labeling all subdir filenames
         return subdir_filenames_matching_suffixes;
      }

// ---------------------------------------------------------------------
// Method image_files_in_subdir() searches the specified subdirectory
// for files ending with common image suffixes.  It returns an STL
// vector with such files' names.

   vector<string> image_files_in_subdir(
      string subdir,bool search_all_children_dirs_flag)
      {
//         cout << "inside filefunc::image_files_in_subdir()" << endl;
         
         vector<string> allowed_suffixes;
         allowed_suffixes.push_back("jpg");
         allowed_suffixes.push_back("JPG");
         allowed_suffixes.push_back("jpeg");
         allowed_suffixes.push_back("JPEG");
         allowed_suffixes.push_back("png");
         allowed_suffixes.push_back("PNG");
         allowed_suffixes.push_back("rgb");
         allowed_suffixes.push_back("tif");
         allowed_suffixes.push_back("ppm");
         allowed_suffixes.push_back("dng");

         return filefunc::files_in_subdir_matching_specified_suffixes(
            allowed_suffixes,subdir,search_all_children_dirs_flag);
      }

// ---------------------------------------------------------------------
   vector<string> image_files_in_multi_subdirs(
      vector<string> subdirs, bool search_all_children_dirs_flag)
   {
      vector<string> all_image_filenames;
      for(unsigned int i = 0; i < subdirs.size(); i++)
      {
         vector<string> curr_image_filenames=image_files_in_subdir(
            subdirs[i],search_all_children_dirs_flag);
         for(unsigned int j = 0; j < curr_image_filenames.size(); j++)
         {
            all_image_filenames.push_back(curr_image_filenames[j]);
         }
      } // loop over index i labeling subdirs
      return all_image_filenames;
   }

// ---------------------------------------------------------------------
// Method latest_files_in_subdir() takes in a subdirectory and performs
// a Unix listing sorted according to access time.  This method
// returns the names of the latest and next-to-latest accessed files
// within the subdirectory.

   void latest_files_in_subdir(
      const string subdir,string& latest_filename,
      string& next_to_latest_filename)
      {
         string ls_cmd="ls -t "+subdir;
         FILE* fp = popen(ls_cmd.c_str(), "r");
         if (fp == NULL)
         {
            cout << "Could not open virtual file in filefunc::latest_file_in_subdir()" << endl;
            outputfunc::enter_continue_char();
            return;
         }

         vector<string> subdir_filenames;
         int PATHMAX=2000;
         char path[PATHMAX];
         while (fgets(path, PATHMAX, fp) != NULL)
         {
            string curr_subdir_filename=subdir+string(path);
            curr_subdir_filename=stringfunc::remove_trailing_whitespace(
               curr_subdir_filename);
            subdir_filenames.push_back(curr_subdir_filename);
//            cout << subdir_filenames.back() << endl;
         }

         int status = pclose(fp);
         if (status == -1) 
         {
            cout << "Error in filefunc::latest_file_in_subdir() reported by pclose()" << endl;
         } 

         if (subdir_filenames.size()==0)
         {
            latest_filename=next_to_latest_filename="";
         }
         else if (subdir_filenames.size()==1)
         {
            latest_filename=subdir_filenames[0];
            next_to_latest_filename="";
         }
         else
         {
            latest_filename=subdir_filenames[0];
            next_to_latest_filename=subdir_filenames[1];
         }
      }

// ---------------------------------------------------------------------
// Boolean method get_latest_filenames_in_subdir() returns false if
// neither the latest nor next-to-latest file within the specified
// input subdirectory has changed.  Otherwise, this method returns
// modified filename values within output strings latest_filename
// and/or next_to_latest_filename.  This method is a higher-level and
// more user-friendly version of latest_filenames_in_subdir().

      string prev_latest_filename,prev_next_to_latest_filename;   

      bool get_latest_files_in_subdir(
         string subdir,string& latest_filename,
         string& next_to_latest_filename)
         {
            bool files_changed_flag=false;

            filefunc::latest_files_in_subdir(
               subdir,latest_filename,next_to_latest_filename);
            if (latest_filename != prev_latest_filename)
            {
               prev_latest_filename=latest_filename;
               files_changed_flag=true;
            }
            
            if (next_to_latest_filename != prev_next_to_latest_filename)
            {
               prev_next_to_latest_filename=next_to_latest_filename;
               files_changed_flag=true;
            }
            return files_changed_flag;
         }
   
// ---------------------------------------------------------------------
// Method files_in_subdir_matching_substring() scans through all files
// within the specified input subdirectory.  It returns an STL vector
// of those filenames which at least partially match the input
// substring.

   vector<string> files_in_subdir_matching_substring(
      string subdir,string substring,bool search_all_children_dirs_flag)
      {
//         cout << "inside filefunc::files_in_subdir_matching_substring()" 
//              << endl;
//         cout << "subdir = " << subdir << "  substring = " << substring
//              << endl;

         vector<string> filenames=
            files_in_subdir(subdir,search_all_children_dirs_flag);

         vector<string> filenames_with_matching_substring;
         for (unsigned int i=0; i<filenames.size(); i++)
         {
            int substring_posn=stringfunc::first_substring_location(
               filenames[i],substring);
//            cout << "i = " << i << " substring_posn = " 
//                 << substring_posn << endl;
            if (substring_posn > -1)
            {
               filenames_with_matching_substring.push_back(filenames[i]);
//               cout << "filename = " << filenames[i]
//                    << " substring posn = " << substring_posn << endl;
            }
         }
//         cout << "filenames_with_matching_substring.size() = "
//              << filenames_with_matching_substring.size() << endl;
//         outputfunc::enter_continue_char();
         
         return filenames_with_matching_substring;
      }

// ---------------------------------------------------------------------
   vector<string> file_basenames_in_subdir_matching_substring(
      string subdir,string substring,bool search_all_children_dirs_flag)
      {
//         cout << "inside filefunc::file_basenames_in_subdir_matching_substring()" 
//              << endl;
//         cout << "subdir = " << subdir << "  substring = " << substring
//              << endl;

         vector<string> filenames=
            files_in_subdir(subdir,search_all_children_dirs_flag);

         vector<string> filenames_with_matching_substring;
         for (unsigned int i=0; i<filenames.size(); i++)
         {
            string basename=filefunc::getbasename(filenames[i]);
//            cout << "i = " << i << " basename = " << basename << endl;
            
            int substring_posn=stringfunc::first_substring_location(
               basename,substring);
            if (substring_posn > -1)
            {
               filenames_with_matching_substring.push_back(filenames[i]);
//               cout << "filename = " << filenames[i]
//                    << " substring posn = " << substring_posn << endl;
            }
         }
         return filenames_with_matching_substring;
      }

// ---------------------------------------------------------------------
// Method files_sorted_by_bytesize() takes in a subdirectory
// and performs a Unix listing sorted according to file byte size.
// This method returns results sorted from smallest to largest file.

   vector<string> files_sorted_by_bytesize(string subdir)
   {
//      cout << "inside filefunc::files_sorted_by_bytesize()" << endl;
      string ls_args=" -Sr ";
      string tmp_filename="/tmp/files.dat";
      string unix_cmd="ls -Sr "+subdir+ " > "+tmp_filename;
      sysfunc::unix_command(unix_cmd);

      vector<string> output_files;
      filefunc::ReadInfile(tmp_filename);
      filefunc::deletefile(tmp_filename);

      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         output_files.push_back(filefunc::text_line[i]);
      }
//      cout << "output_files.size() = " << output_files.size() << endl;
      return output_files;
   }

// ---------------------------------------------------------------------
// Method purge_files_in_subdir() loops over all files within the
// input subdirectory whose names have size greater than 1 with
// nontrivial and non-matching prefixes and suffixes.  It deletes all
// such files from the subdirectory.  This method should obviously be
// used with care!

   void purge_files_in_subdir(string subdir,bool search_all_children_dirs_flag)
      {
         cout << "inside filefunc::purge_files_in_subdir()" << endl;
         
         vector<string> filenames=files_in_subdir(
            subdir,search_all_children_dirs_flag);
         for (unsigned int i=0; i<filenames.size(); i++)
         {
            string prefix=stringfunc::prefix(filenames[i]);
            string suffix=stringfunc::suffix(filenames[i]);
            
            if (filenames[i].size() > 1 
                && prefix.size() > 0 && suffix.size() > 0 
                && prefix != suffix)
            {
               cout << "Deleting "+filenames[i] << endl;
               deletefile(filenames[i]);
//         cout << "prefix = " << prefix << " suffix = " << suffix << endl;
            }
         }
      }

   void purge_files_with_suffix_in_subdir(
      string subdir,string file_suffix,bool search_all_children_dirs_flag)
      {
//         cout << "inside filefunc::purge_files_with_suffix_in_subdir()" 
//              << endl;
         
         vector<string> filenames=files_in_subdir(
            subdir,search_all_children_dirs_flag);
         for (unsigned int i=0; i<filenames.size(); i++)
         {
            string prefix=stringfunc::prefix(filenames[i]);
            string suffix=stringfunc::suffix(filenames[i]);
            
            if (filenames[i].size() > 1 
                && prefix.size() > 0 && suffix.size() > 0 
                && prefix != suffix && suffix==file_suffix)
            {
               cout << "Deleting "+filenames[i] << endl;
               deletefile(filenames[i]);
//               cout << "prefix = " << prefix << " suffix = " << suffix 
//                    << endl;
            }
         }
      }

// ==========================================================================

   bool deletefile(string filenamestr)
      {
//         cout << "inside filefunc::deletefile(), filename = "
//              << filenamestr << endl;
//         cout << "fileexist(filename) = " << fileexist(filenamestr)
//              << endl;
         bool file_deleted_flag=false;
         if (fileexist(filenamestr))
         {
            string unixcommandstr="rm "+filenamestr;
//            cout << "unixcommandstr = " << unixcommandstr << endl;
            int status=sysfunc::unix_command(unixcommandstr);
            if (status != 127 && status != -1)
            {
               file_deleted_flag=true;
            }
         }
         else
         {
            file_deleted_flag=true;
         }
         
         return file_deleted_flag;
      }

// ---------------------------------------------------------------------
// Method fileexist is a virtual carbon copy of method direxist:

   bool fileexist(string filenamestr)
      {
//         cout << "inside filefunc::fileexist() " << endl;
         struct stat buffer;
         const char *file=filenamestr.c_str();

         if (stat(file,&buffer)) return false;         
         return(S_ISREG(buffer.st_mode));
      }

// Methop chardevexist returns true if the input character device
// (e.g. /dev/ttyUSB0) exists:

   bool chardevexist(string chardevstr)
      {
         struct stat buffer;
         const char *chardev=chardevstr.c_str();

         if (stat(chardev,&buffer)) return false;

         return(S_ISCHR(buffer.st_mode));
      }

// ---------------------------------------------------------------------
// Method files_matching_pattern takes in a string containing a wild *
// character.  It calls the POSIX glob function which returns all
// files matching the pattern.  An STL vector of the matching
// filenames is returned by this method.

   vector<string> files_matching_pattern(string pattern_to_match)
      {
         glob_t globbuf;

// Follow Ross Anderson's suggestion to initialize globbuf structure
// to zero values:

         memset(&globbuf,0,sizeof(glob_t)); 

         globbuf.gl_offs=0;
         int glob_return_value=
            glob(pattern_to_match.c_str(), GLOB_DOOFFS, NULL, &globbuf);

// Print error message if glob return value != 0:

         if (glob_return_value != 0)
         {
            string banner="Error inside filefunc::files_matching_pattern()";
            outputfunc::write_big_banner(banner);
            cout << "pattern_to_match = " << pattern_to_match << endl;
            cout << "glob return value = " << glob_return_value << endl;
            cout << "GLOB_NOSPACE = " << GLOB_NOSPACE << endl;
            cout << "GLOB_ABORTED = " << GLOB_ABORTED << endl;
            cout << "GLOB_NOMATCH = " << GLOB_NOMATCH << endl;
         }
         
         vector<string> matching_filenames;
         for (unsigned int i=0; i<globbuf.gl_pathc; i++)
         {
            matching_filenames.push_back(string(globbuf.gl_pathv[i]));
//            cout << "i = " << i 
//                 << " filename = " << matching_filenames.back() << endl;
         }

// Free dynamically allocated memory within globbuf object before it
// goes out of scope:

         globfree(&globbuf);
         return matching_filenames;
      }

// ---------------------------------------------------------------------
// Method generate_tmpfilename executes a C library call to mktemp()
// which returns a random temporary filename that is supposed to be
// guaranteed to not already exist.  We preprend /tmp to the temporary
// file name.

   string generate_tmpfilename()
      {
         string template_str="/tmp/tmp_XXXXXX";
         mkstemp(const_cast<char*>(template_str.c_str()));
//         cout << "template_str = " << template_str << endl;
         return template_str;
      }

// ---------------------------------------------------------------------
// Method getbasename() takes in a C++ string which contains the
// complete pathname for a file and strips off the leading directory
// information.  The base part of the filename is returned as a C++
// string.

   string getbasename(string filename)
   {
      string slash_char="/";
      return getbasename(filename,slash_char);
   }

   string getbasename(string filename,string slash_char)
   {
      unsigned slashpos=filename.find_last_of(slash_char.c_str());
//      unsigned slashpos=filename.find_last_of("/");

// If incoming filename is terminated with a "/" character, then
// search for next-to-last instance of "/" within the filename string:

         if (slashpos==filename.length()-1)
         {
//            slashpos=filename.substr(0,filename.length()-1).find_last_of("/");
            slashpos=filename.substr(0,filename.length()-1).find_last_of(
               slash_char.c_str());
         }
         string basename=filename.substr(slashpos+1,filename.length());
         return basename;
      }

// ---------------------------------------------------------------------
// Method getdirname() takes in a C++ string which contains the
// complete pathname for a file and returns the leading directory
// information:

   string getdirname(string filename)
      {
         unsigned slashpos=filename.find_last_of("/");

// If incoming filename is terminated with a "/" character, then
// search for next-to-last instance of "/" within the filename string:

         if (slashpos==filename.length()-1)
         {
            slashpos=filename.substr(0,filename.length()-1).find_last_of("/");
         }

         string dirname=filename.substr(0,slashpos+1);
         return dirname;
      }

// ---------------------------------------------------------------------
   string getprefix(string filename)
   {
      string basename=getbasename(filename);
      return stringfunc::prefix(basename);
   }
   
// ---------------------------------------------------------------------
// Method replace_suffix() takes in a C++ string which contains the
// complete pathname for some file.  It replaces the input file's
// suffix with the specified new suffix.

   string replace_suffix(string input_filename,string suffix)
      {
         string prefix=stringfunc::prefix(input_filename);
         string output_filename=prefix+"."+suffix;
         return output_filename;
      }
   
// ---------------------------------------------------------------------
// Getsingleline is an ugly function which reads an entire line of
// input text from stdin including white spaces.  This line is
// returned as a C++ string object.  For reasons which we do not
// understand as of 11/18/99, we need to call cin.getline twice if
// this routine is used after any standard "cin >> someobject" call.
// It appears that a newline character is left over within the stdin
// buffer which is effectively flushed by the first cin.getline call.
// The second call then actually reads in the user's input line of
// text.  Needless to say, this routine is ugly due to our present
// ignorance about C++ I/O:

   string getsingleline()
      {
         string buffer;
         getline(std::cin,buffer);
         return buffer;
      }

// ---------------------------------------------------------------------
// Create or extract bzipped2 file.  Overwrite any existing gzipped
// file by the same name:

   void bzip2_file(string filename)
      {
         string unixcommandstr="bzip2 -f "+filename;
         sysfunc::unix_command(unixcommandstr);
      }

   void bunzip2_file(string filename)
      {
         if (stringfunc::suffix(filename)=="bz2")
         {
            string unixcommandstr="bunzip2 -f "+filename;
            sysfunc::unix_command(unixcommandstr);
         }
      }

// ---------------------------------------------------------------------
// Create or extract gzipped file.  Overwrite any existing gzipped
// file by the same name:

   void gzip_file(string filename)
      {
         string unixcommandstr="gzip -f "+filename;
         sysfunc::unix_command(unixcommandstr);
      }

   void gzip_file_if_gunzipped(string filename)
      {
         if (fileexist(filename) && !fileexist(filename+".gz")) 
            gzip_file(filename);         
      }

   void gunzip_file(string filename)
      {
         string unixcommandstr="gunzip -f "+filename;
         sysfunc::unix_command(unixcommandstr);
      }

   void gunzip_file_if_gzipped(string filename)
      {
         if (fileexist(filename+".gz") && !fileexist(filename)) 
            gunzip_file(filename);         
      }

// ---------------------------------------------------------------------
// Method parameter_input determines whether any arguments were passed
// at the command line used to invoke a main program.  If so, it
// returns an STL vector containing strings corresponding to these
// input parameters.

   bool parameter_input(unsigned int argc,char *argv[],vector<string>& param)
      {
         bool input_param_file=false;

         if (argc>=2)
         {
            input_param_file=true;
            for (unsigned int i=0; i<argc-1; i++)
            {
               param.push_back(argv[i+1]);
            }
         }
         return input_param_file;
      }

// ---------------------------------------------------------------------
// Method parameter_input determines whether any arguments were
// passed at the command line used to invoke a main program.  If so,
// it interprets a single input string argument as the name of a file
// containing input parameters.  If more than one invocation argument
// was entered, this method exits with an error message.

   void parameter_input(unsigned int argc,char *argv[],bool &input_param_file,
                        string line[],unsigned int& nlines)
      {
         string line_with_comments[1000];

         if (argc==1)
         {
            input_param_file=false;
         }
         else if (argc>=2)
         {
            input_param_file=true;
            string inputfilename=argv[1];
            ReadInfile(inputfilename,line_with_comments,nlines);
            
// First eliminate all comments from input file:

            int linenumber=0;
            for (unsigned int i=0; i<nlines; i++)
            {
               string cleanedline=stringfunc::comment_trunc(
                  line_with_comments[i]);

// Eliminate blank lines:

               if (!cleanedline.empty())
               {
                  line[linenumber++]=cleanedline;
               }
            }
            nlines=linenumber;
         }
         //   else
         //   {
         //      cout << "Error in invocation command line" << endl;
         //      exit(-1);
         //   }
         cout << endl;
      }

// ---------------------------------------------------------------------
   void parameter_input(bool &input_param_file,string line[],unsigned int& nlines)
      {
         char inputchar;
         cout << endl;
         cout << "Enter 'k' to input parameters from keyboard or " << endl;
         cout << " or 'f' to input parameters from file" << endl;
         cin >> inputchar;

         input_param_file=false;
         if (inputchar=='f')
         {
            string inputfilename,line_with_comments[1000];
            cout << endl;
            input_param_file=true;
            cout << "Enter name of parameter file" << endl;
            cin >> inputfilename;
            ReadInfile(inputfilename,line_with_comments,nlines);

            int linenumber=0;
            for (unsigned int i=0; i <nlines; i++)
            {
               string cleanedline=stringfunc::comment_trunc(
                  line_with_comments[i]);

// Eliminate blank lines:

               if (!cleanedline.empty())
               {
                  line[linenumber++]=cleanedline;
               }
            }
            nlines=linenumber;
         }
         cout << endl;
      }

// ---------------------------------------------------------------------
   void parameter_input(string inputfilename,vector<string>& line)
      {
         vector<string> line_with_comments;
         line_with_comments.reserve(1000);
         ReadInfile(inputfilename,line_with_comments);

// Eliminate all comments from input file:

         for (unsigned int i=0; i <line_with_comments.size(); i++)
         {
            string cleanedline=stringfunc::comment_trunc(
               line_with_comments[i]);

// Eliminate blank lines from output file:

            if (!cleanedline.empty())
            {
               line.push_back(cleanedline);
            }
         } // loop over index i 
      }
   


// ------------------------------------------------------------------------
   bool get_file_contents(string filename,string& contents)
   {
//      cout << "inside filefunc::get_file_contents()" << endl;
      ifstream in(filename.c_str(), ios::in | ios::binary);
      if (in)
      {
         in.seekg(0, ios::end);
         contents.resize(in.tellg());
         in.seekg(0, ios::beg);
         in.read(&contents[0], contents.size());
         in.close();
         return true;
      }
      return false;
   }

// ----------------------------------------------------------------------
   void tokenize_string(string input_str,vector<string>& lines)
   {
//      cout << "inside filefunc::tokenize_string()" << endl;
      char* str=const_cast<char *>(input_str.c_str());
      char* pch = strtok (str,"\n");
      
      lines.clear();
      while (pch != NULL)
      {
         lines.push_back(string(pch));
         pch = strtok (NULL, "\n");
      }
   }

// ---------------------------------------------------------------------
// Method ReadInfile opens up a file with name specified by string
// variable filenamestr, reads in its contents into array line,
// returns the number of lines within the file within integer variable
// nlines, and then closes the input file.  If the requested file was
// not successfully read, this boolean method returns false.

   bool ReadInfile(string filenamestr,vector<string>& lines)
      {
//         cout << "inside filefunc::ReadInfile(filenamestr,line)" << endl;

         string file_contents;
         if (!get_file_contents(filenamestr,file_contents))
         {
            cout << filenamestr 
                 << " not successfully opened by method ReadInfile!" 
                 << endl;
            return false;
         }

         filefunc::tokenize_string(file_contents,lines);
         return true;
      }

   bool ReadInfile(string filenamestr,string line[],unsigned int &nlines)
      {
         ifstream currfile;
         bool file_successfully_opened=openfile(filenamestr,currfile);

         if (!file_successfully_opened)
         {
            cout << filenamestr 
                 << " not successfully opened by method ReadInfile!" 
                 << endl;
         }
         else
         {
            nlines=0;
            while (getline(currfile,line[nlines],'\n'))
            {
               nlines++;
            }
         }
         closefile(filenamestr,currfile);
   
         return file_successfully_opened;
      }

// ---------------------------------------------------------------------
// This next overloaded version of method ReadInfile stores input
// ascii line information within "member object" text_line.  It also
// strips away any comments from the text lines.

   bool ReadInfile(string filenamestr,bool strip_comments_flag)
      {
         text_line.clear();
         bool file_OK_flag=ReadInfile(filenamestr,text_line);
         if (file_OK_flag && strip_comments_flag)
         {
            stringfunc::comment_strip(text_line);
         }
         return file_OK_flag;
      }

// ---------------------------------------------------------------------
// This overloaded version of ReadInfile imports the contents of the
// specified input text file into output stringstream ss.  Lines 0
// through line_start are ignored.  We wrote this method in order to
// skip over comment lines in data files containing rows of numbers.

   void ReadInfile(string filename,int line_start,stringstream& ss)
   {
//      cout << "inside filefunc::ReadInfile(), filename = " 
//           << filename << endl;
      ifstream fin(filename.c_str());
      stringstream orig_ss;
      orig_ss << fin.rdbuf();

      int line_counter=0;
      string curr_line;

      while (line_counter < line_start)
      {
         std::getline(orig_ss,curr_line);
         line_counter++;
      }

      while (std::getline(orig_ss,curr_line))
      {
         ss << curr_line << endl;
      }
   }

// ------------------------------------------------------------------------
// Method ReadInStrings() takes in the name for some text file.
// It fills and returns an STL vector of strings containing the
// contents of the input text file.

   vector<string> ReadInStrings(string filename)
   {
      ifstream fin(filename.c_str());
      stringstream ss;
      ss << fin.rdbuf();

      string curr_string;
      vector<string> strings;
      while (true)
      {
         ss >> curr_string;
         strings.push_back(curr_string);
         if (!ss) break;
      }
      strings.pop_back();
      return strings;
   }

// ------------------------------------------------------------------------
// Method ReadInSubstrings() takes in the name for some text file
// which we assume corresponds to an input table of strings.  It
// fills and returns an STL vector of vector of strings containing the
// contents of the input text file.

   vector<vector<string> > ReadInSubstrings(string filename)
   {
      vector< vector<string> > all_substrings;
      if (ReadInfile(filename))
      {
 	 unsigned int n_lines=filefunc::text_line.size();
         all_substrings.reserve(n_lines);

         for (unsigned int i=0; i<n_lines; i++)
         {
            vector<string> curr_substrings(
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[i]));
            all_substrings.push_back(curr_substrings);
         }
      } // ReadInfile conditional
      
      return all_substrings;
   }

   void ReadInSubstrings(
      string filename,vector<vector<string> >& all_substrings)
   {
      if (ReadInfile(filename))
      {
         unsigned int n_lines=filefunc::text_line.size();
         all_substrings.reserve(n_lines);

         for (unsigned int i=0; i<n_lines; i++)
         {
            vector<string> curr_substrings(
               stringfunc::decompose_string_into_substrings(
                  filefunc::text_line[i]));
            all_substrings.push_back(curr_substrings);
         }
      } // ReadInfile conditional
   }

// ------------------------------------------------------------------------
// Method ReadInNumbers() takes in the name for some text file
// which we assume contains only numbers.  It fills and returns an STL
// vector of doubles containing the contents of the input text file.

   vector<double> ReadInNumbers(string filename)
   {
      ifstream fin(filename.c_str());
      stringstream ss;
      ss << fin.rdbuf();

      return ReadInNumbers(ss);
   }

   vector<double> ReadInNumbers(stringstream& ss)
   {
      double curr_number=-1;
      vector<double> numbers;
      while (true)
      {
         ss >> curr_number;
         numbers.push_back(curr_number);
         if (!ss) break;
      }
      numbers.pop_back();
      return numbers;
   }

// ---------------------------------------------------------------------
   vector< vector<double> > ReadInRowNumbers(
      unsigned int line_start,unsigned int n_numbers_per_row,string filename)
   {
//      cout << "inside filefunc::ReadInRowNumbers()" << endl;

      stringstream ss;
      filefunc::ReadInfile(filename,line_start,ss);
      vector<double> numbers=ReadInNumbers(ss);

      unsigned int counter=0;
      unsigned int n_numbers=numbers.size();
//      cout << "n_numbers = " << n_numbers << endl;
      unsigned int n_rows=n_numbers/n_numbers_per_row;
//      cout << "n_rows = " << n_rows << endl;

      vector<vector<double> > RowNumbers;
      RowNumbers.reserve(n_rows);
      vector<double> curr_row;
      curr_row.reserve(n_numbers_per_row);

      for (unsigned int r=0; r<n_rows; r++)
      {
         curr_row.clear();
         for (unsigned int c=0; c<n_numbers_per_row; c++)
         {
            curr_row.push_back(numbers[counter]);
            counter++;
         }
         RowNumbers.push_back(curr_row);
      } // loop over index r labeling rows
      return RowNumbers;
   }

// ---------------------------------------------------------------------
   vector< vector<double> > ReadInRowNumbers(string filename)
   {
//      cout << "inside filefunc::ReadInRowNumbers()" << endl;
      
      vector< vector<double > > row_numbers;
      bool flag=ReadInfile(filename);
      if (!flag) return row_numbers;
      
      unsigned int n_lines=filefunc::text_line.size();
      row_numbers.reserve(n_lines);
      for (unsigned int i=0; i<n_lines; i++)
      {
//         cout << "i = " << i 
//              << " filefunc::text_line[i] = "
//              << filefunc::text_line[i] << endl;
         
         vector<double> curr_row_numbers=
            stringfunc::string_to_numbers(filefunc::text_line[i]);
//         cout << "curr_row_numbers.size() = "
//              << curr_row_numbers.size() << endl;
         
         row_numbers.push_back(curr_row_numbers);
      }
      return row_numbers;
   }

// ---------------------------------------------------------------------
// Method resolve_liked_filename was written by James Wanken.  It
// tries to resolve any symbolic links in the input pathname. 

   string resolve_linked_filename(const string& pathname)
      {
//         cout << "inside filefunc::resolve_linked_filename()" << endl;
         const int MAXIMUM_PATH=500;
   
         int code;
         char buf[MAXIMUM_PATH];
         string resolvedName(pathname);

// Check for links to links to ...
    
         while ((code=readlink(resolvedName.c_str(),buf,sizeof(buf))) > 0)
         {

// Make sure the returned name is NULL terminated.  Readlink "may" not
// terminate it on some systems:

            buf[code] = '\0';
            resolvedName = buf;
         }
         return resolvedName;
      }

// In this overloaded version of resolve_linked_filename, the number
// of recursive link levels is returned in integer nlevel.  The names
// associated with each recursive level are returned in string array
// resolvedName.

   void resolve_linked_filename(
      const string& pathname,int& nlevel,string resolvedName[])
      {
         const int MAXIMUM_PATH=500;
   
         int code;
         char buf[MAXIMUM_PATH];
         resolvedName[0]=pathname;

// Check for links to links to ...
    
         nlevel=1;
         while ((code=readlink(resolvedName[nlevel-1].c_str(),
         buf,sizeof(buf))) > 0)
         {

// Make sure the returned name is NULL terminated.  Readlink "may" not
// terminate it on some systems:

            buf[code] = '\0';
            resolvedName[nlevel] = buf;
            nlevel++;
         }
      }

// ---------------------------------------------------------------------
// We adapted method symboliclink_exist from method
// direxist().  It calls both the stat and lstat functions.  The
// latter returns identical information as the former for all files
// other than symbolic links.  If the input file is a link, lstat
// returns info about the source file rather than the link itself.  We
// simply compare the byte sizes returned by the stat and lstat
// commands.  If they do not agree, we conclude that the input file
// must be a link.  Chant "man 2 stat" in order to find out more
// options for the stat command.

   bool symboliclink_exist(string linknamestr)
      {
         struct stat buffer1;
         struct stat buffer2;
         const char *linkname=linknamestr.c_str();

         if (stat(linkname,&buffer1)) return(false);
         if (lstat(linkname,&buffer2)) return(false);

         return (!(buffer1.st_size==buffer2.st_size));
      }

// ---------------------------------------------------------------------
// Method assign_file_epoch_time() imports the path for some input
// file.  If the file exists, this method returns the minimum of its
// access, modification and change times as its best estimate for the
// file's creation time.  Otherwise, this method return -1.

   double assign_file_epoch_time(string filenamestr)
   {
      struct stat buffer;
      const char *filename=filenamestr.c_str();

      double assigned_file_epoch=-1;
      if (stat(filename,&buffer))
      {
      }
      else
      {
         double access_time=buffer.st_atim.tv_sec;
         double modification_time=buffer.st_mtim.tv_sec;
         double change_time=buffer.st_ctim.tv_sec;

         assigned_file_epoch=modification_time;

         if (access_time <= modification_time && access_time <= change_time) 
            assigned_file_epoch=access_time;

         if (change_time <= access_time && change_time <= modification_time) 
            assigned_file_epoch=change_time;
         
//         cout << "access_time = " << access_time << endl;
//         cout << "modification_time = " << modification_time << endl;
//         cout << "change_time = " << change_time << endl;
      }
//      cout << "assigned_file_epoch = " << assigned_file_epoch << endl;

      return assigned_file_epoch;
   }

// ==========================================================================
// Metafile methods
// ==========================================================================

// Method meta_to_adobe calls Iva Mooney's "adobe" command to
// convert meta files into adobe output.  If the input filename has no
// ".meta" suffix, then such a ".meta" suffix is added to the
// metafile's name:

   void meta_to_adobe(string filename)
      {
         string basename,metafilename;

// Strip off ".meta" suffix from filename:

         string dirname=filefunc::getdirname(filename);
         unsigned int dotmeta_posn=
            filefunc::getbasename(filename).rfind(".meta");

         if (dotmeta_posn==string::npos || dotmeta_posn <= 0 ||
             dotmeta_posn > filefunc::getbasename(filename).length())
         {
            basename=filefunc::getbasename(filename).substr(0,dotmeta_posn);
            metafilename=filename;
         }
         else
         {
            basename=filefunc::getbasename(filename);
            metafilename=filename+".meta";
         }
         string stripped_filename=dirname+basename;

         string unixcommandstr="adobe -o "+metafilename;
         sysfunc::unix_command(unixcommandstr);

// Rename ".adb" adobe filename suffix as ".ai":

         if (filefunc::fileexist(stripped_filename+".adb"))
         {
            unixcommandstr="mv "+stripped_filename+".adb "
               +stripped_filename+".ai";
            sysfunc::unix_command(unixcommandstr);
         }
      }

// ---------------------------------------------------------------------
// Method meta_to_gif first generates a JPEG file from a
// meta file.  This method first strips off any ".meta" or other
// suffix if it exists from the input filename.  It next calls the
// ImageMagick utility convert to generate a GIF file from the JPEG
// file.  Finally, the intermediate JPEG file is deleted.  On 1/8/01,
// we observed that GIF images are generally much larger than
// corresponding JPEG images.  So we should use this utility only to
// create animated GIF sequences and NOT for data compressions
// purposes!

   void meta_to_gif(string filename)
      {
// Strip off ".meta" suffix from filename:

         string dirname=filefunc::getdirname(filename);
         unsigned dotmeta_posn=filefunc::getbasename(filename).rfind(".meta");

         string basename;
         if (dotmeta_posn==string::npos || dotmeta_posn <= 0 ||
             dotmeta_posn > filefunc::getbasename(filename).length())
         {
            basename=filefunc::getbasename(filename);
         }
         else
         {
            basename=filefunc::getbasename(filename).substr(0,dotmeta_posn);
         }
         string stripped_filename=dirname+basename;

         meta_to_jpeg(stripped_filename);
         jpeg_to_gif(stripped_filename);
         string unixcommandstr="/bin/rm -f "+stripped_filename+".jpg";
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method meta_to_jpeg first generates a postscript file from a
// meta file.  This method first strips off any ".meta" or other
// suffix if it exists from the input filename.  It next calls the
// ImageMagick utility convert to generate a JPEG file from the
// postscript file.  Finally, the intermediate postscript file is
// deleted if the JPEG file was successfully generated.  This
// method should hopefully help free up disk space, for postscript
// files are generally much larger in size than their corresponding
// JPEG analogues.

   void meta_to_jpeg(string filename)
      {
// Strip off ".meta" suffix from filename:

         string dirname=filefunc::getdirname(filename);
         unsigned dotmeta_posn=filefunc::getbasename(filename).rfind(".meta");

         string basename;
         if (dotmeta_posn==string::npos || dotmeta_posn <= 0 ||
             dotmeta_posn > filefunc::getbasename(filename).length())
         {
            basename=filefunc::getbasename(filename);
         }
         else
         {
            basename=filefunc::getbasename(filename).substr(0,dotmeta_posn);
         }
         string stripped_filename=dirname+basename;

         meta_to_ps(stripped_filename);
         ps_to_jpeg(stripped_filename);

         if (filefunc::fileexist(stripped_filename+".jpg"))
         {
            string unixcommandstr="/bin/rm -f "+stripped_filename+".ps";
            sysfunc::unix_command(unixcommandstr);
         }
      }

// ---------------------------------------------------------------------
// Method meta_to_ps calls Iva Mooney's "adobe -ps" to convert
// meta files into postscript output.  If the input filename has no
// ".meta" suffix, then such a ".meta" suffix is added to the
// metafile's name:

   void meta_to_ps(string filename)
      {
         string basename,metafilename;
         unsigned int dotmeta_posn=
            filefunc::getbasename(filename).rfind(".meta");

         if (dotmeta_posn==string::npos || dotmeta_posn <= 0 ||
             dotmeta_posn > filefunc::getbasename(filename).length())
         {
            basename=filename;
            metafilename=filename+".meta";
         }
         else
         {
            basename=filename.substr(0,dotmeta_posn);
            metafilename=filename;
         }

// In January 2002, we discovered that core dumps were being generated
// as a result of metafiles not being opened.  We suspected that this
// was happening as a result of calls to this meta_to_ps method
// taking place before prior calls to metafile generation methods
// has finished.  In order to make sure that sufficient time has been
// allocated for metafiles to be fully written out to file, we first
// check whether the metafile exists.  If not, go to sleep while the
// metafile is being written:

         const int max_sleeping_time=20;	// secs
         int sleepcount=0;
         if (!filefunc::fileexist(metafilename) 
             && sleepcount < max_sleeping_time)
         {
            cout << "Asleep in meta_to_ps while waiting for meta file" 
                 << endl;
            sleep(1);
            sleepcount++;
         }

         string unixcommandstr="adobe -ps -o "+metafilename;
         sysfunc::unix_command(unixcommandstr);

         sleepcount=0;
         while (!filefunc::fileexist(basename+".ps") 
                && sleepcount < max_sleeping_time)
         {
            cout << "Asleep in meta_to_ps while waiting for postscript file" 
                 << endl;
            sleep(1);
            sleepcount++;
         }
         outputfunc::newline();
      }

// ==========================================================================
// JPEG methods
// ==========================================================================

// Method jpeg_to_gif calls the Imagemagick routine convert to
// transform a JPEG image into a gif image.  On 1/8/01, we found that
// converting meta files into GIF images via the sequence meta ->
// postscript -> JPEG -> GIF appears to yield final GIF images which
// are more compact than those produced via the shorter chain meta ->
// postscript -> GIF.  (Of course, we're producing JPEG images at only
// a 90% quality level.)  Moreover, GIF images appear to be generated
// faster if we follow the first route rather than the second.

   void jpeg_to_gif(string filename)
      {
// Introduce a delay if JPEG images have not yet had sufficient time
// to be generated before JPEG_TO_GIF has been called:

         string jpegfilename=filename+".jpg";
         while (!filefunc::fileexist(jpegfilename))
         {
            usleep(500);
         }
         string unixcommandstr="convert "+jpegfilename+" "+filename+".gif";
         sysfunc::unix_command(unixcommandstr);
      }

   void jpeg_to_png(string filename)
      {
// Introduce a delay if JPEG images have not yet had sufficient time
// to be generated before JPEG_TO_GIF has been called:

         string jpegfilename=filename+".jpg";
         while (!filefunc::fileexist(jpegfilename))
         {
            usleep(500);
         }
         string unixcommandstr="convert "+jpegfilename+" "+filename+".png";
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method ps_to_jpeg calls the Imagemagick routine convert to
// transform a postscript image into a jpeg image.  It also rotates
// images so that they appear in "landscape" rather than "portrait"
// mode:

   void ps_to_jpeg(string filename)
      {
         const int max_sleeping_time=20;	// secs
   
//   string unixcommandstr="convert -geometry 150% "+file_basename_str+".ps "
//      +file_basename_str+".jpg";

// On 10/18/00, we discovered that movies generated with 150% size
// enhancement bring the penguin laptop to its knees!

         string unixcommandstr="convert -rotate 90 -quality 90 "
            +filename+".ps "+filename+".jpg";
         sysfunc::unix_command(unixcommandstr);

// Strip off ".0" suffix from JPEG filenames:

         if (filefunc::fileexist(filename+".jpg.0"))
         {
            unixcommandstr="mv "+filename+".jpg.0 "+filename+".jpg";
            sysfunc::unix_command(unixcommandstr);
         }
   
// For reasons which we do not understand as of October, 2000, utility
// convert frequently generates two JPEG files which it distinguishes
// with a ".0" and ".1" suffix.  The second ".1" file does not seem to
// serve any useful purpose.  We therefore delete it as soon as it is
// created in order to avoid wasting diskspace:

         if (filefunc::fileexist(filename+".jpg.1"))
         {
            unixcommandstr="rm "+filename+".jpg.1";
            sysfunc::unix_command(unixcommandstr);
         }

         int sleepcount=0;
         while (!filefunc::fileexist(filename+".jpg") 
                && sleepcount < max_sleeping_time)
         {
            cout << "Asleep in ps_to_jpeg while waiting for jpeg file" 
                 << endl;
            sleep(1);
            sleepcount++;
         }
      }

// ==========================================================================
// Binary data importing/exporting methods
// ==========================================================================

// Methods LoadUnsignedChars() and LoadFloats() are based upon
// Laura Brattain's original codes.  They take in names for files
// assumed to contain just unsigned chars or floats.
// ReadUnsignedChars() and ReadFloats() returns dynamically
// instantiated arrays containing the imported binary data.

   unsigned char* LoadUnsignedChars(const char* filename, size_t size)
   {
//      cout << "inside filefunc::LoadUnsignedChars()" << endl;
//      cout << "size = " << size << endl;
      
      FILE *fp = fopen(filename, "rb");

      if (!fp) {
         printf("Error opening file '%s'\n", filename);
         return NULL;
      }

      unsigned char* data = new unsigned char[size];
      size_t read = fread(data, 1, size, fp);
      fclose(fp);

      if(read == 0)
      {
         delete [] data;
         return NULL;
      }
      else
      {
	return data;
      }
   }

// ------------------------------------------------------------------------
   unsigned char* ReadUnsignedChars(string& filename,size_t size)
   {
      return LoadUnsignedChars(filename.c_str(),size);
   }

// ------------------------------------------------------------------------
// Method ExportUnsignedChars() writes n_bytes=size unsigned chars
// from array data[] to the output binary file specified by filename.

   void ExportUnsignedChars(
      unsigned char* data,string& filename, size_t size)
   {
      ExportUnsignedChars(data,filename.c_str(),size);
   }
   
   void ExportUnsignedChars(
      unsigned char* data,const char* filename, size_t size)
   {
//      cout << "inside filefunc::ExportUnsignedChars()" << endl;
//      cout << "size = " << size << endl;
      
      FILE *fp = fopen(filename, "wb");

      if (!fp) 
      {
         printf("Error opening file '%s'\n", filename);
      }
      else
      {
         fwrite(data, 1, size, fp);
         fclose(fp);
      }
   }

// ------------------------------------------------------------------------
// This overloaded version of LoadUnsignedChars() takes in a binary
// file containing n_total_features*D bytes.  It reads every
// n_feature_skip'th row of D bytes from the specified binary file
// into a dynamically instantiated unsigned char* array.  

// We wrote this method in April 2012 in order to import a subsampled
// set of SIFT feature descriptors (with D=128) for image vocabulary
// generation.  We don't have enough RAM to hold all descriptors in
// memory at once.  So this method allows us to read in a downsampled
// set of SIFT descriptors into RAM.

   unsigned char* LoadUnsignedChars(
      const char* filename, int n_total_features,int n_feature_skip,size_t D)
   {
//      cout << "inside filefunc::LoadUnsignedChars()" << endl;
      
      FILE *fp = fopen(filename, "rb");

      if (!fp) 
      {
         cout << "Error opening binary file " << filename << endl;
         return NULL;
      }

//      cout << "n_total_features = " << n_total_features << endl;
//      cout << "n_feature_skip = " << n_feature_skip << endl;
      unsigned int n_features_to_import=n_total_features/n_feature_skip;
//      cout << "n_features_to_import = " << n_features_to_import << endl;
      size_t n_chars_to_import=n_features_to_import*D;
//      cout << "n_chars_to_import = " << n_chars_to_import << endl;

      unsigned char* data = new unsigned char[n_chars_to_import];
      
      for (unsigned int f=0; f<n_features_to_import; f++)
      {
         if (f%1000==0) cout << f << " " << flush;
         
         int output_offset=f*D;
//         size_t read = 
	 fread(data+output_offset, 1, D, fp);

         if (f < n_features_to_import-1)
         {
            long input_offset=(n_feature_skip-1)*D;
//            int status=
               fseek(fp,input_offset, SEEK_CUR);
         }

      } // loop over index f labeling imported features
      cout << endl;
      fclose(fp);

      return data;
   }

// ------------------------------------------------------------------------
   unsigned char* ReadUnsignedChars(
      std::string& filename,int n_total_features,int n_feature_skip,size_t D)
   {
      return LoadUnsignedChars(
         filename.c_str(),n_total_features,n_feature_skip,D);
   }

// ------------------------------------------------------------------------
   char* LoadChars(const char* filename, size_t size)
   {
      FILE *fp = fopen(filename, "rb");

      if (!fp) {
         printf("Error opening file '%s'\n", filename);
         return NULL;
      }

      char* data = new char[size];
      size_t read = fread(data, 1, size, fp);
      fclose(fp);

      if(read == 0)
      {
         delete [] data;
         return NULL;
      }
      else
      {
	return data;
      }
   }

   char* ReadChars(string& filename,size_t size)
   {
      return LoadChars(filename.c_str(),size);
   }

// ------------------------------------------------------------------------
// Method export_floats() exports STL vector float_values to a binary
// output file.  

   void export_floats(string& output_filename,vector<float>& float_values)
   {
      ofstream binary_stream;
      filefunc::open_binaryfile(output_filename,binary_stream);
      for (unsigned int i=0; i<float_values.size(); i++)
      {
         filefunc::writeobject(binary_stream,float_values[i]);
      }
      filefunc::closefile(output_filename,binary_stream);
   }

   void ExportFloats(
      float* data,size_t n_floats,string& filename)
   {
      FILE* fp = fopen(filename.c_str(), "wb");

      if (!fp) 
      {
         cout << "Error opening output binary file " << filename << endl;
      }
      else
      {
         fwrite(data, sizeof(float), n_floats, fp);
         fclose(fp);
      }
   }
   
   
// ------------------------------------------------------------------------
// Load a file of floats

   float* LoadFloats(const char* filename, size_t size)
   {
      FILE *fp = fopen(filename, "rb");

      if (!fp) 
      {
         printf("Error opening file '%s'\n", filename);
         return NULL;
      }

      float* data = new float[size];
      size_t read = fread(data, sizeof(float), size, fp);
      fclose(fp);

      if(read == 0)
      {
         delete [] data;
         return NULL;
      }
      else
      {
	return data;
      }
   }

   float* ReadFloats(string& filename,size_t size)
   {
      return LoadFloats(filename.c_str(),size);
   }

// ==========================================================================
// Byte swapping methods to convert between big and little endian integers
// ==========================================================================
   
// Methods copied from
// http://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func

//! Byte swap unsigned short
   
   uint16_t swap_uint16( uint16_t val ) 
   {
     return (val << 8) | (val >> 8 );
   }

//! Byte swap short
   int16_t swap_int16( int16_t val ) 
   {
     return (val << 8) | ((val >> 8) & 0xFF);
   }

//! Byte swap unsigned int
   uint32_t swap_uint32( uint32_t val )
   {
     val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
     return (val << 16) | (val >> 16);
   }

//! Byte swap int
   int32_t swap_int32( int32_t val )
   {
     val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
     return (val << 16) | ((val >> 16) & 0xFFFF);
   }
   
   uint64_t swap_uint64( uint64_t val )
   {
     val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
     val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
     return (val << 32) | (val >> 32);
   }

   int64_t swap_int64( int64_t val )
   {
     val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
     val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
     return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
   }

} // filefunc namespace


