// ==========================================================================
// Program ALPHA
// ==========================================================================
// Last updated on 3/29/04
// ==========================================================================

#include <fstream>
#include <iostream>
#include <new>
#include "math/basic_math.h"
#include "filter/filterfuncs.h"
#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "myconstants.h"
#include "plot/plotfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/waveform.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
      
double alpha_filter(double curr_raw_x,double prev_filtered_x,double alpha)
{
   return alpha*curr_raw_x+(1-alpha)*prev_filtered_x;
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
  
   string filenamestr="firstpeak.dat";
   string line[200];
   int nlines;
   filefunc::ReadInfile(filenamestr,line,nlines);

   double time[nlines],raw_z[nlines],smoothed_z[nlines];
   for (int n=0; n<nlines; n++)
   {
      stringfunc::string_to_two_numbers(line[n],time[n],raw_z[n]);
      cout << "n = " << n << " time = " << time[n] << " raw_z = " << raw_z[n]
           << endl;
   }

   double alpha;
   cout << "Enter alpha:" << endl;
   cin >> alpha;


   ofstream smoothstream;
   filefunc::openfile("smoothed_firstpeak.meta",smoothstream);
   
   double prev_filtered_z=raw_z[0];
   for (int n=0; n<nlines; n++)
   {
      double curr_filtered_z=alpha_filter(raw_z[n],prev_filtered_z,alpha);
//      cout << time[n] << "\t\t" << curr_filtered_z << endl;
      smoothstream << time[n] << "\t\t" << curr_filtered_z << endl;
      prev_filtered_z=curr_filtered_z;
   }

   filefunc::closefile("smoothed_firstpeak.meta",smoothstream);
}





