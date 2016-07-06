// ==========================================================================
// Program GISTCOMPARE queries the user to enter the basenames for two
// input images whose GIST descriptors are assumed to have been
// precalculated via program GISTCOMPUTE.  It computes and reports the
// Euclidean distance between the two images' GIST descriptors.
// ==========================================================================
// Last updated on 5/22/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string image_prefix,image2_prefix;
   cout << "Enter image prefix #1:" << endl;
   cin >> image_prefix;

   cout << "Enter image prefix #2:" << endl;
   cin >> image2_prefix;

   string gist_filename=image_prefix+".gist";
   string gist2_filename=image2_prefix+".gist";

   filefunc::ReadInfile(gist_filename);
   vector<double> gist_values=stringfunc::string_to_numbers(
      filefunc::text_line[0]);

   filefunc::ReadInfile(gist2_filename);
   vector<double> gist2_values=stringfunc::string_to_numbers(
      filefunc::text_line[0]);
   
   double delta=0;
   for (int i=0; i<gist_values.size(); i++)
   {
      delta += sqr(gist_values[i]-gist2_values[i]);
   }
   delta = sqrt(delta);
   cout << "Delta = " << delta << endl;
}

