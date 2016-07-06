// ==========================================================================
// Program HELLO
// ==========================================================================
// Last updated on 7/3/06
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_JPEG_Image.H>

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{

//   fl_register_images();                       // initialize image lib
   Fl_Window     win(720,486);                 // make a window
   Fl_Box        box(10,10,720-20,486-20);     // widget that will contain image
   Fl_JPEG_Image jpg("smallGlobe.jpg");      // load jpeg image into ram
   box.image(jpg);                             // attach jpg image to box
   win.show();
   return(Fl::run());
}

   
