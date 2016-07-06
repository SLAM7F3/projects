 // ========================================================================
// Program JPG2jpg renames "foo.JPG" as "foo.jpg".  We wrote this
// little utility in order to convert Noah's JPG suffix to our jpg
// suffix.

//				JPG2jpg		

// ========================================================================
// Last updated on 3/5/10; 6/29/11; 11/23/11
// ========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{

   string basename;
   cout << "Enter file basename:" << endl;
   cin >> basename;
   int starting_image_number,stopping_image_number;
   cout << "Enter starting image number:" << endl;
   cin >> starting_image_number;
   cout << "Enter stopping image number:" << endl;
   cin >> stopping_image_number;

   string script_filename="mv_JPG_2_jpg";
   ofstream outstream;
   filefunc::openfile(script_filename,outstream);

   string extra_label;
   cout << "Enter extra trailing label for new jpeg files:" << endl;
   cin >> extra_label;
   
   int n_digits=4;
//   int n_digits=5;
   for (int i=starting_image_number; i<=stopping_image_number; i++)
   {
      string curr_filename=basename+stringfunc::integer_to_string(
         i,n_digits)+".JPG";
      string new_filename=basename+stringfunc::integer_to_string(
         i,n_digits)+extra_label+".jpg";
      string unix_command="mv "+curr_filename+" "+new_filename;
      outstream << unix_command << endl;
   }

   filefunc::closefile(script_filename,outstream);


   string unix_command_str="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command_str);
   
   string banner="Wrote script file "+script_filename;
   outputfunc::write_banner(banner);
}
