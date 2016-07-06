// ==========================================================================
// Program POLYFIT
// ==========================================================================
// Last updated on 5/7/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/filefuncs.h"
#include "genfuncs.h"
#include "math/mypolynomial.h"
#include "plot/plotfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

   int poly_order=3;
   double coeff[poly_order];
   coeff[0]=1;
   coeff[1]=22;
   coeff[2]=-3.33;
   coeff[3]=4.232;
   mypolynomial poly(poly_order,coeff);
   
   double x;
   while (true)
   {
      cout << "Enter x:" << endl;
      cin >> x;
      cout << "p(x) = " << poly.value(x) << endl;
      cout << "new p(x) = " << poly.power_value(x) << endl;
      outputfunc::newline();
   }
   
}


