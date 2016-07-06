// ==========================================================================
// Program TANKSIGN_PROCESSOR executes an infinite while loop.
// It constantly imports the next-to-latest image file residing within
// input_images_subdir.  It moves all other images in
// input_images_subdir to a time-stamped archive subdirectory.
// TANKSIGN_PROCESSOR generates text file output containing tank
// position relative to the instantaneous camera versus local computer
// time.
// ==========================================================================
// Last updated on 11/4/12; 11/5/12; 11/6/12
// ==========================================================================

#include  <iostream>
#include  <string>

#include "general/filefuncs.h"
#include "classification/sign_recognizer.h"
#include "classification/signrecogfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
   sign_recognizer* sign_recognizer_ptr=new sign_recognizer();
   sign_recognizer_ptr->initialize_tank_sign_recognition();

   string tank_signs_subdir="./images/tank_signs/";
   string archived_images_subdir=
      signrecogfunc::generate_timestamped_archive_subdir(tank_signs_subdir);

// Infinite while loop starts here:

   while (true)
   {
      string input_images_subdir="./images/incoming_PointGrey_images/";
      string next_to_latest_image_filename=
         signrecogfunc::archive_all_but_latest_image_files(
            input_images_subdir,archived_images_subdir);
      string image_filename=next_to_latest_image_filename;
      if (image_filename.size()==0) continue;

      if (sign_recognizer_ptr->search_for_tank_sign(image_filename))
      {
         string JSON_output_string=sign_recognizer_ptr->get_JSON_string();
         cout << "JSON_output_string = " << JSON_output_string << endl;
	 // publish(JSON_output_string)
      }

   } // end of infinite while loop 

   sign_recognizer_ptr->close_relative_tank_position_file();
}


