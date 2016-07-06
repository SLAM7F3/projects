// ========================================================================
// Program WATCH_FOR_NEW_PHOTO constantly monitors watch_subdir for
// new files.  Whenever a new image file is dropped into this folder,
// this program regenerates two executable scripts which depend upon
// the file's name.  We wrote this little utility program specifically
// for the summer 2010 SIGMA demo.

//				watch_for_new_photo

// ========================================================================
// Last updated on 6/8/10; 6/9/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   string home_dir = getenv("HOME");
   string watch_subdir=home_dir+"/programs/c++/svn/projects/src/mains/photosynth/bundler/individual_photo/images/";
   filefunc::dircreate(watch_subdir);
   bool searchallchildren = true;
   filefunc::purge_files_in_subdir(watch_subdir, searchallchildren);

   string latest_subdir_filename;
   vector<string> current_subdir_filenames,previous_subdir_filenames;
   ofstream outstream;

   while(true)
   {

// Perform ls of watch_subdir:

         string ls_cmd="ls -t "+watch_subdir;
         FILE* fp = popen(ls_cmd.c_str(), "r");
         if (fp == NULL)
         {
            cout << "Could not open virtual file" << endl;
            exit(-1);
         }

         current_subdir_filenames.clear();
         int PATHMAX=2000;
         char path[PATHMAX];
         while (fgets(path, PATHMAX, fp) != NULL)
         {
            string curr_subdir_filename=watch_subdir+string(path);
	// ignores thumbnails directory
	    //if (curr_subdir_filename.find("thumbnails") == string::npos) {
		    curr_subdir_filename=stringfunc::remove_trailing_whitespace(
		       curr_subdir_filename);
		    current_subdir_filenames.push_back(curr_subdir_filename);
	//            cout << subdir_filenames.back() << endl;
	    //}
         }

         int status = pclose(fp);
         if (status == -1) 
         {
            cout << "Error reported by pclose()" << endl;
         } 

         if (current_subdir_filenames.size()==
             previous_subdir_filenames.size()) continue;

         for (int j=0; j<current_subdir_filenames.size(); j++)
         {
            string curr_subdir_filename=current_subdir_filenames[j];
            bool curr_filename_matches_prev_filename_flag=false;
            for (int k=0; k<previous_subdir_filenames.size(); k++)
            {
               if (curr_subdir_filename==previous_subdir_filenames[k])
               {
                  curr_filename_matches_prev_filename_flag=true;
               }
            } // loop over index k labeling previous subdir filenames
            if (!curr_filename_matches_prev_filename_flag)
            {
               latest_subdir_filename=curr_subdir_filename;
            }
         } // loop over index j labeling current subdir filenames
         
         previous_subdir_filenames.clear();
         for (int j=0; j<current_subdir_filenames.size(); j++)
         {
            previous_subdir_filenames.push_back(current_subdir_filenames[j]);
         }
         
         cout << "latest_filename = " << latest_subdir_filename << endl;
         string new_image_filename=watch_subdir+filefunc::getbasename(
            latest_subdir_filename);

         string scriptname="run_graph_script";
         filefunc::openfile(scriptname,outstream);
         string cmd="./run_demo.pl "+new_image_filename;
         outstream << cmd << endl;
         filefunc::closefile(scriptname,outstream);

         string unix_cmd="chmod a+x "+scriptname;
         sysfunc::unix_command(unix_cmd);

         scriptname="run_3Dman_script";
         filefunc::openfile(scriptname,outstream);
         cmd="./run_face_detect.pl "+new_image_filename;
         outstream << cmd << endl;
         filefunc::closefile(scriptname,outstream);

         unix_cmd="chmod a+x "+scriptname;
         sysfunc::unix_command(unix_cmd);

   } // while(true) loop

}

   


