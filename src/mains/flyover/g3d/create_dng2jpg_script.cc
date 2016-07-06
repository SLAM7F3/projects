// ========================================================================
// Program CREATE_DNG2JPG_SCRIPT
// ========================================================================
// Last updated on 6/30/14
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string dng_subdir="/hypersonic2/j125/collection/20140305/dng/";

   vector<string> unplanned_subdirs;
   unplanned_subdirs.push_back("unplanned-20140305_190119/");
   unplanned_subdirs.push_back("unplanned-20140305_191933/");
   unplanned_subdirs.push_back("unplanned-20140305_191948/");
   unplanned_subdirs.push_back("unplanned-20140305_192001/");
   unplanned_subdirs.push_back("unplanned-20140305_202158/");

   string script_filename="run_dng_to_jpg";
   ofstream outstream;
   filefunc::openfile(script_filename,outstream);

   for (unsigned int u=0; u<unplanned_subdirs.size(); u++)
   {
      string curr_unplanned_subdir=dng_subdir+unplanned_subdirs[u];
      cout << "u = " << u 
           << " subdir = " << curr_unplanned_subdir
           << endl;

      vector<string> subdir_filenames=
         filefunc::files_in_subdir(curr_unplanned_subdir);
      for (unsigned s=0; s<subdir_filenames.size(); s++)
      {
         string curr_basename=filefunc::getbasename(subdir_filenames[s]);
         string substr=curr_basename.substr(0,2);
         if (substr != "02") continue;
         string subsubdir = curr_unplanned_subdir+curr_basename+"/";
//         cout << "s = " << s << " subsubdir = " << subsubdir << endl;
         
         vector<string> subsubdir_filenames=
            filefunc::files_in_subdir(subsubdir);
         for (unsigned t=0; t<subsubdir_filenames.size(); t++)
         {
            string curr_basename=filefunc::getbasename(subsubdir_filenames[t]);
            string subsubsubdir=subsubdir+curr_basename+"/";
            string output_jpgdir=subsubsubdir+"jpg/";
            filefunc::dircreate(output_jpgdir);

//            cout << "t = " << t << " subsubsubdir = " << subsubsubdir 
//                 << endl;
            vector<string> dng_filenames=filefunc::image_files_in_subdir(
               subsubsubdir);
//            cout << "dng_filenames.size() = " << dng_filenames.size() << endl;

            string unix_cmd1=
               "/home/pcho/sputnik/pwin/build/linux64/pwin -v -nowin -do1 'c3 batch_convert \\";
            string unix_cmd2 = " -src "+subsubsubdir+" \\";
            string unix_cmd3 = " -dst "+output_jpgdir+"'";
            outstream << unix_cmd1 << endl;
            outstream << unix_cmd2 << endl;
            outstream << unix_cmd3 << endl;
            outstream << endl;

         } // loop over index t labeling subsubdirs of unplanned subdirs
      } // loop over index s labeling subdirs of unplanned subdirs
   } // loop over index u labeling unplanned subdirs
   
   filefunc::closefile(script_filename,outstream);   
   filefunc::make_executable(script_filename);

   string banner="Exported "+script_filename;
   outputfunc::write_big_banner(banner);

/*   
-src /hypersonic2/j125/collection/20140305/dng/unplanned-20140305_190119/02-2645D-07041/0k \
-dst /hypersonic2/j125/collection/20140305/dng/unplanned-20140305_190119/02-2645D-07041/0k/jpg'
*/


}
