// ==========================================================================
// Program DRAWPLANES
// ==========================================================================
// Last updated on 4/15/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "connectfuncs.h"
#include "draw3Dfuncs.h"
#include "featurefuncs.h"
#include "general/filefuncs.h"
#include "genfuncs.h"
#include "groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "imagefuncs.h"
#include "ladarfuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "parallelogram.h"
#include "recursivefuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/twoDarray.h"
#include "urbanimage.h"
#include "voronoifuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];
   string logfilename=sysfunc::get_cplusplusrootdir()+
      "alirt/drawplanes.logfile";

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;
   urbanimage cityimage;

   string planes_filename=cityimage.imagedir+"planes.xyzp";   
   
   
   parallelogram rectangle(4,5);
   for (int i=0; i<=20; i++)
   {
      parallelogram working_rectangle=rectangle;
      working_rectangle.translate(myvector(3*(i%4),0,i));

      double annotation_value=i*0.05;
      draw3Dfunc::draw_rectangle_grid(
         working_rectangle,planes_filename,annotation_value);
   }

}


