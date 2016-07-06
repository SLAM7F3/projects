// ==========================================================================
// Program CROPJPEGS was cluged together in order to resize JPEG
// imagery output from screen capture tool XVIDCAP.  In June 04, we
// discovered that the pixel widths and pixel lengths of JPEG images
// must be perfect multiples of 4 if they are to be converted to AVI
// movies.  This program generates a script which can be used to shave
// off a few pixels from each JPEG's width and/or length.
// ==========================================================================
// Last updated on 7/26/04
// ==========================================================================

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ostream;
   using std::ofstream;
   using std::string;

   ofstream filestream;
   string filename="crop_jpegs.script";
   filefunc::openfile(filename,filestream);

   int nfiles;
   cout << "Enter number of input jpeg files:" << endl;
   cin >> nfiles;
   string basefilename;
   cout << "Enter basename for each jpeg file:" << endl;
   cin >> basefilename;

   int wpixels,hpixels;
   cout << "Enter width of each input jpeg file in pixels:" << endl;
   cin >> wpixels;
   cout << "Enter height of each input jpeg file in pixels:" << endl;
   cin >> hpixels;
   int new_wpixels=4*(wpixels/4);
   int new_hpixels=4*(hpixels/4);
   cout << "Width in pixels of each output jpeg file = " << new_wpixels
        << endl;
   cout << "Height in pixels of each output jpeg file = " << new_hpixels
        << endl;

   for (int n=0; n<nfiles; n++)
   {
      string imagenumber=stringfunc::integer_to_string(n,4);
      string frmstr=basefilename+imagenumber+".jpg";
      string cropstr="crop-"+imagenumber+".jpg";
      filestream << "convert -quality 100 -crop "+
         stringfunc::number_to_string(new_wpixels)+"x"+
         stringfunc::number_to_string(new_hpixels)+" "+frmstr+" "+cropstr 
                 << endl;
      filestream << "mv "+cropstr+".0 "+cropstr << endl;
      filestream << "rm "+cropstr+".1" << endl;
      filestream << endl;
   }
   filefunc::closefile(filename,filestream);

   string unixcommandstr="chmod a+x "+filename;
   sysfunc::unix_command(unixcommandstr);
}
