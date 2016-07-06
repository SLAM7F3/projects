// ==========================================================================
// Program PARSE_TXT is a special purpose program which parses Hyrum
// Anderson's text file dump of imagecdf header information.  It
// assumes that the top few lines within the text file have been
// stripped off.  So the first line should read "Image 0:".
// ==========================================================================
// Last updated on 3/30/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "space/spasefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string imagecdf_text_filename="mini.txt";
//   cout << "Enter name of input imagecdf text file:" << endl;
//   cin >> imagecdf_text_filename;

   filefunc::ReadInfile(imagecdf_text_filename);
   int n_images=filefunc::text_line.size()/8;
   cout << "n_images = " << n_images << endl;
   vector<pair<int,int> > image_size;
   vector<twovector> meters_per_pixel,center_shift,translation;
   vector<genmatrix*> R_ptr;

   spasefunc::parse_imagecdf_textdump(
      imagecdf_text_filename,image_size,meters_per_pixel,
      center_shift,translation,R_ptr);

   for (int i=0; i<n_images; i++)
   {
      cout << "image i = " << i << endl;
      cout << "width = " << image_size[i].first 
           << " height = " << image_size[i].second << endl;
      cout << "meters_per_pixel = " << meters_per_pixel[i] << endl;
      cout << "center_shift = " << center_shift[i] << endl;
      cout << "trans = " << translation[i] << endl;
      cout << "R = " << *(R_ptr[i]) << endl;

      genmatrix curr_R(3,3);
      curr_R=*(R_ptr[i]);
      double det;
      curr_R.determinant(det);
      cout << "R.det = " << det << endl;
      cout << "R*Rtrans = " << curr_R*curr_R.transpose() << endl;
   }
   
}
