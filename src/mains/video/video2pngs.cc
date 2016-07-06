 // ========================================================================
// Program VIDEO2PNGS calls ffmpeg to decompose an input movie file
// into constituent video frames as PNG files.

//				video2pngs

// ========================================================================
// Last updated on 7/11/10
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
   string moviefilename;
   cout << "Enter input movie filename:" << endl;
   cin >> moviefilename;

   int fps;
   cout << "Enter movie's rate in frames per second:" << endl;
   cin >> fps;

   string framesdir="./frames/";
   filefunc::dircreate(framesdir);
   string base_output_filename="image-";

   string unix_command="ffmpeg -i "+moviefilename+" -r "+
      stringfunc::number_to_string(fps)+" "+framesdir+base_output_filename
      +"%05d.png";
   cout << "unix_command = " << unix_command << endl;
   sysfunc::unix_command(unix_command);
}
