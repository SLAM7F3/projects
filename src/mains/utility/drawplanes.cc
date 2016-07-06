// ==========================================================================
// Program DRAWPLANES
// ==========================================================================
// Last updated on 1/20/05; 4/23/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "geometry/parallelogram.h"
#include "general/sysfuncs.h"
#include "urban/urbanimage.h"

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
   unsigned int ninputlines;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   urbanimage cityimage;

   string planes_filename=cityimage.get_imagedir()+"planes.xyzp";   
   filefunc::deletefile(planes_filename);
   
   parallelogram rectangle(4,5);
   int nplanes=50;
   int nplanes_per_level=10;
   for (int i=0; i<=nplanes; i++)
   {
      parallelogram working_rectangle=rectangle;
      threevector trans(3*(i%nplanes_per_level),0,i);
      working_rectangle.translate(trans);

      double annotation_value=i/double(nplanes);
      draw3Dfunc::draw_rectangle_grid(
         working_rectangle,planes_filename,annotation_value);
   }

}


