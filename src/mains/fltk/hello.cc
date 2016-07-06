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
#include "math/genmatrix.h"
#include "numerical/param_range.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

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
   Fl_Window *window = new Fl_Window(300,180);
   Fl_Box *box = new Fl_Box(20,40,260,100,"Hello, World!");

   box->box(FL_UP_BOX);
   box->labelsize(36);
   box->labelfont(FL_BOLD+FL_ITALIC);
   box->labeltype(FL_SHADOW_LABEL);
   window->end();
   window->show(argc, argv);
   return Fl::run();

}

   
