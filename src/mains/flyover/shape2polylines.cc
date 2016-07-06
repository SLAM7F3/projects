// ==========================================================================
// Program SHAPE2POLYLINES

//				./shape2polylines

// ==========================================================================
// Last updated on 8/18/15
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   
//   string input_filename="hwy_280_5k.shape";
   string input_filename="hwy_101.shape";
   filefunc::ReadInfile(input_filename);

   unsigned int i_start = 2;
   for(unsigned int i = i_start; i < filefunc::text_line.size(); i++){
//     cout << filefunc::text_line[i] << endl;
     vector<double> curr_XYZ = stringfunc::string_to_numbers(filefunc::text_line[i]);
     cout << "    " << i - i_start << "  " << curr_XYZ[1] << "  " << curr_XYZ[0] << "  " << curr_XYZ[2] << endl;
   }
   

}

