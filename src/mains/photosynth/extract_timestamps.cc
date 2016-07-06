// ========================================================================
// Program EXTRACT_TIMESTAMPS is a little utility which pulls out time
// stamps from the original MIT2317 photos and generates SQL update
// commands for the photo table within the data_network database.

//  extract_timestamps --region_filename ./bundler/MIT2317/packages/peter_inputs.pkg


// ========================================================================
// Last updated on 4/23/10; 4/26/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photograph.h"
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

   string image_files_on_disk_filename="image_files_on_disk.2317";
   filefunc::ReadInfile(image_files_on_disk_filename);
   int n_photos=filefunc::text_line.size();
   cout << "n_photos = " << n_photos << endl;

   string output_filename="update_timestamp.sql";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (int n=0; n<n_photos; n++)
   {
      string curr_image_filename=filefunc::text_line[n];
      cout << curr_image_filename << endl;

      int id=-1;
      photograph photo(id,curr_image_filename);
      photo.set_UTM_zonenumber(19);	// Boston
      photo.parse_Exif_metadata();
      double secs_since_epoch=photo.get_clock().
         secs_elapsed_since_reference_date();
      cout << "secs_since_epoch = " << secs_since_epoch << endl;

      string date_str=photo.get_clock().YYYY_MM_DD_H_M_S();

      string SQL_command="update photo set time_stamp = ";
      SQL_command += "'"+date_str+"' where id="
         +stringfunc::number_to_string(n)+";";
//      cout << "SQL_command = " << SQL_command << endl;
      outstream << SQL_command << endl;
   }

   filefunc::closefile(output_filename,outstream);

/*


// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   filefunc::ReadInfile(image_list_filename);
   int n_photos=int(filefunc::text_line.size());
   cout << "n_photos = " << n_photos << endl;

   string photo_dir=
      "/media/disk/Photos/Files/MIT_field_trip/26_June_2009/Zach1/";

   string output_filename="find_image_files";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   for (int n=0; n<n_photos; n++)
   {
      string curr_line=filefunc::text_line[n];
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(curr_line);
      string filename=filefunc::getbasename(substrings[0]);
      cout << "n = " << n << " filename = " << filename << endl;
      string prefix1=stringfunc::prefix(filename);
      string prefix2=stringfunc::prefix(prefix1);

      string find_cmd="find "+photo_dir+" -name '"+prefix2+".JPG' -print";
      outstream << find_cmd << endl;
   }
   filefunc::closefile(output_filename,outstream);
   string unix_cmd="chmod a+x "+output_filename;
   sysfunc::unix_command(unix_cmd);
   
*/

}
