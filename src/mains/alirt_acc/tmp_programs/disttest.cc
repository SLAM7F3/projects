// ==========================================================================
// Program DISTTEST
// ==========================================================================
// Last updated on 3/10/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/sysfuncs.h"
#include "genfuncs.h"
#include "general/outputfuncs.h"
#include "math/myvector.h"
#include "linesegment.h"

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

   myvector v1(2,2,0);
   myvector v2(1,2,0);
   linesegment l(v1,v2);
   
   while (true)
   {
      double x,y;
      cout << "Enter x:" << endl;
      cin >> x;
      cout << "Enter y:" << endl;
      cin >> y;
      myvector p(x,y);
      
      myvector closest_pnt_on_segment;
      double distance=l.point_to_line_segment_distance(
         p,closest_pnt_on_segment);
      cout << "distance = " << distance << endl;
      cout << "closet_pnt_on_segment = " << closest_pnt_on_segment << endl;
      cout << "=====================" << endl;
      outputfunc::newline();
   }
   

}


