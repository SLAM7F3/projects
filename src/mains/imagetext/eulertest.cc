// ==========================================================================
// Program EULERTEST
// ==========================================================================
// Last updated on 4/28/14
// ==========================================================================

#include <iostream>
#include "image/binaryimagefuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;

int main (int argc, char* argv[])
{
   int width=3;
   int height=3;
   int pfill=1;
   
   twoDarray* twoDarray_ptr=new twoDarray(width,height);
//   twoDarray_ptr->initialize_values(0);
   twoDarray_ptr->initialize_values(pfill);
   twoDarray_ptr->put(1,1,0);

   double euler_number,perimeter,area;
   binaryimagefunc::image_Euler_number_perimeter_area(
     pfill,twoDarray_ptr,euler_number,perimeter,area);
   cout << "euler_number = " << euler_number << endl;
   cout << "perimeter = " << perimeter << endl;
   cout << "area = " << area << endl;
   
   delete twoDarray_ptr;
}
