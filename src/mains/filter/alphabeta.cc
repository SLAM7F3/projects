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

// ---------------------------------------------------------------------
// Second order alpha-beta filter:

void alphabeta_filter(
   double curr_raw_x,double prev_filtered_x,double prev_filtered_xdot,
   double& curr_filtered_x,double& curr_filtered_xdot,
   double alpha,double beta,double dt)
{
// Filtered estimate for velocity at present time:   
   curr_filtered_xdot=beta*(curr_raw_x-prev_filtered_x)/dt+
      (1-beta)*prev_filtered_xdot;

// Filtered estimate for position at present time:
   curr_filtered_x=alpha*curr_raw_x+(1-alpha)*(
      prev_filtered_x+dt*prev_filtered_xdot);
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

   double alpha,beta;
   cout << "Enter alpha:" << endl;
   cin >> alpha;
   cout << "Enter beta:" << endl;
   cin >> beta;

   double arg=sqr(alpha)+4*beta;
   double lambda1=0.5*(alpha+sqrt(arg));
   double lambda2=0.5*(alpha-sqrt(arg));

   cout << "lambda1=" << lambda1 << " lambda2 = " << lambda2 << endl;

   ofstream smoothstream;
   filefunc::openfile("smoothed_firstpeak.meta",smoothstream);
   
   double prev_filtered_z=raw_z[0];
   double prev_filtered_zdot=0;
   for (int n=0; n<nlines; n++)
   {
      double curr_filtered_z,curr_filtered_zdot,dt=0.25;
      alphabeta_filter(
         raw_z[n],prev_filtered_z,prev_filtered_zdot,
         curr_filtered_z,curr_filtered_zdot,alpha,beta,dt);
//      double curr_filtered_z=alpha_filter(raw_z[n],prev_filtered_z,alpha);

      smoothstream << time[n] << "\t\t" << curr_filtered_z << endl;
      prev_filtered_z=curr_filtered_z;
      prev_filtered_zdot=curr_filtered_zdot;
   }

   filefunc::closefile("smoothed_firstpeak.meta",smoothstream);
}





