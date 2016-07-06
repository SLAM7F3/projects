// ==========================================================================
// Program SIZE_OF_TYPES
// ==========================================================================
// Last updated on 3/18/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
  
   cout << "sizeof(bool) = " << sizeof(bool) << endl;
   cout << "sizeof(int) = " << sizeof(int) << endl;
   cout << "sizeof(unsigned int) = " << sizeof(unsigned int) << endl;
   cout << "sizeof(size_t) = " << sizeof(size_t) << endl;
   cout << "sizeof(unsigned short) = " << sizeof(unsigned short) << endl;
   cout << "sizeof(short int) = " << sizeof(short int) << endl;
   cout << "sizeof(long) = " << sizeof(long) << endl;
   cout << "sizeof(float) = " << sizeof(float) << endl;
   cout << "sizeof(double) = " << sizeof(double) << endl;
   

}


