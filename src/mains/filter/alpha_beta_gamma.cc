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
   }

   double alpha,beta,gamma;
   cout << "Enter alpha:" << endl;
   cin >> alpha;
   cout << "Enter beta:" << endl;
   cin >> beta;
   cout << "Enter gamma:" << endl;
   cin >> gamma;

   double arg=sqr(alpha)+4*beta;
   double lambda1=0.5*(alpha+sqrt(arg));
   double lambda2=0.5*(alpha-sqrt(arg));

   cout << "lambda1=" << lambda1 << " lambda2 = " << lambda2 << endl;

   ofstream smoothstream;
   filefunc::openfile("smoothed_firstpeak.meta",smoothstream);
   
   double prev_filtered_z=raw_z[0];
   double prev_filtered_zdot=0;
   double prev_filtered_zdotdot=0;
   const double tau_beta=6;	// secs
   const double tau_gamma=3;	// secs
   for (int n=0; n<nlines; n++)
   {
      double curr_filtered_z,curr_filtered_zdot,curr_filtered_zdotdot,dt=0.25;
      double curr_t=n*dt;
      filterfunc::alphabetagamma_filter(
         raw_z[n],prev_filtered_z,prev_filtered_zdot,prev_filtered_zdotdot,
         curr_filtered_z,curr_filtered_zdot,curr_filtered_zdotdot,
         alpha,beta,gamma,dt);
//         alpha,beta*exp(-curr_t/tau_beta),gamma*exp(-curr_t/tau_gamma),dt);
//      alphabeta_filter(
//         raw_z[n],prev_filtered_z,prev_filtered_zdot,
//         curr_filtered_z,curr_filtered_zdot,alpha,beta,dt);
//      double curr_filtered_z=alpha_filter(raw_z[n],prev_filtered_z,alpha);

      smoothstream << time[n] << "\t\t" << curr_filtered_z << endl;
      prev_filtered_z=curr_filtered_z;
      prev_filtered_zdot=curr_filtered_zdot;
      prev_filtered_zdotdot=curr_filtered_zdotdot;
   }

   filefunc::closefile("smoothed_firstpeak.meta",smoothstream);
}





