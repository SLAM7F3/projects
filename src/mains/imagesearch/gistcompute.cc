// ==========================================================================
// Program GISTCOMPUTE first queries the user to enter an image.  It
// then generates a PPM thumbnail version of the input image.
// GISTCOMPUTE then calls the LEAR c implementation of Torralba's
// original MATLAB code which computes a 960-dimensional GIST
// descriptor for the thumbnail.  The GIST descriptor is written to an
// output text file.
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

   string image_filename="SDC12830.JPG";
   cout << "Enter image filename:" << endl;
   cin >> image_filename;

   int width,height;
   imagefunc::get_image_width_height(image_filename,width,height);
   double aspect_ratio=double(width)/double(height);

   int new_width,new_height;
   if (width > height)
   {
      new_width=64;
      new_height=new_width/aspect_ratio;
   }
   else
   {
      new_height=64;
      new_width=aspect_ratio*new_height;
   }

   string prefix=stringfunc::prefix(image_filename);
   string ppm_filename=prefix+".ppm";
   string gist_filename=prefix+".gist";

   cout << "Resizing input JPG image and converting to ppm file:" << endl;
   string unix_cmd="convert "+image_filename
      +" -resize "+stringfunc::number_to_string(new_width)+"x"
      +stringfunc::number_to_string(new_height)+" "+ppm_filename;
   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   cout << "Calculating GIST descriptor:" << endl;
   unix_cmd="compute_gist "+ppm_filename+" > "+gist_filename;
   sysfunc::unix_command(unix_cmd);
   
   filefunc::ReadInfile(gist_filename);
   vector<double> gist_values=stringfunc::string_to_numbers(
      filefunc::text_line[0]);
   
   cout << "gist_values.size() = " << gist_values.size() << endl;
   double norm=0;
   for (int i=0; i<gist_values.size(); i++)
   {
      norm += sqr(gist_values[i]);
   }
   norm =sqrt(norm);
   cout << "norm = " << norm << endl;
}

