// ==========================================================================
// Program TANK_SIGNS is searches for a colored checkerboard
// containing red, yellow, cyan and purple cells.
// ==========================================================================
// Last updated on 11/2/12; 11/4/12; 11/5/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "classification/sign_recognizer.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   sign_recognizer* sign_recognizer_ptr=new sign_recognizer();
   sign_recognizer_ptr->initialize_tank_sign_recognition();

   string tank_signs_subdir=sign_recognizer_ptr->get_tank_signs_subdir();
//   string tank_signs_subdir="./images/blimp_imagery/jpg_images/blimp_images/";
   cout << "tank_signs_subdir = " << tank_signs_subdir << endl;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      tank_signs_subdir);

   int i_start=0;
   int i_stop=image_filenames.size()-1;
   for (int i=i_start; i<=i_stop; i++)
   {
      sign_recognizer_ptr->search_for_tank_sign(image_filenames[i]);
   } // loop over index i labeling image filenames

   sign_recognizer_ptr->close_relative_tank_position_file();
}


