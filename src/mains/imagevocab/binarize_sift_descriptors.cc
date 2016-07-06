// ==========================================================================
// Program BINARIZE_SIFT_DESCRIPTORS imports all sift keyfiles from
// some specified subdirectory.  All 128-dimensional descriptors from
// a key file are converted into unsigned char bytes.  They are
// exported to a binary file whose byte size = 128*n_features in key
// file.  The original text key file is then deleted, and the binary
// file is compressed.  Empirically, we observe that this binarization
// procedure cuts down disk space usage by at least a factor of 4.


// 			  ./binarize_sift_descriptors

// ==========================================================================
// Last updated on 8/16/13; 8/17/13
// ==========================================================================

#include  <algorithm>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "video/image_matcher.h"
#include "datastructures/map_unionfind.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   photogroup* photogroup_ptr=new photogroup;

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);

   bool Lowe_SIFT_flag=true;
   string sift_keys_subdir="/data2/sift_keys/";
   string output_sift_keys_subdir="/data/sift_keyfiles/";
   string descriptors_subdir=output_sift_keys_subdir+"binary_descriptors/";
   filefunc::dircreate(descriptors_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("key");
   vector<string> all_key_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,sift_keys_subdir);

   cout << "all_key_filenames.size() = " << all_key_filenames.size() << endl;
   for (unsigned int i=0; i<all_key_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(i,100,all_key_filenames.size());
      string key_filename=all_key_filenames[i];

      string basename=filefunc::getbasename(key_filename);
      string prefix=stringfunc::prefix(basename);
      string descriptors_filename=descriptors_subdir+prefix+".descriptors";
      SIFT.parse_Lowe_descriptors(
         Lowe_SIFT_flag,key_filename,descriptors_filename);
      
// As of 4/19/12, we use LZOP rather than GZIP to compress raw HDF5
// binary files.  LZOP does not compress as well as GZIP.  But it runs
// appreciably faster:

//      string unix_cmd="gzip "+descriptors_filename;
      string unix_cmd="lzop -U "+descriptors_filename;
      sysfunc::unix_command(unix_cmd);

// Delete original SIFT keyfile after its descriptors have been
// extracted and compressed:

      unix_cmd ="/bin/rm "+key_filename;
      sysfunc::unix_command(unix_cmd);

//      string banner="Exported SIFT descriptors to binary file "+
//         descriptors_filename;
//      outputfunc::write_banner(banner);
   }
   cout << endl;

   delete photogroup_ptr;

   cout << "At end of program BINARIZE_SIFT_DESCRIPTORS" << endl;
   outputfunc::print_elapsed_time();
}

