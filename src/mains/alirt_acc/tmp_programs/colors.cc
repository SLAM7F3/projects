// ==========================================================================
// Program COLORS
// ==========================================================================
// Last updated on 2/25/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <new>
#include "myconstants.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "genfuncs.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"

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

   double zmin,zmax;
   cout << "Enter zmin (which is to be mapped to purple):" << endl;
   cin >> zmin;
   cout << "Enter zmax (which is to be mapped to red):" << endl;
   cin >> zmax;
//   zmin=0;
//   zmax=40;
   double deltaz=zmax-zmin;
   
   int nsteps_total;
   cout << "Enter total number of color steps:" << endl;
   cin >> nsteps_total;
   int nsteps1=floor(0.2*nsteps_total);
   int nsteps2=floor(0.4*nsteps_total);
   int nsteps3=nsteps_total-nsteps1-nsteps2;
   int ninterpolations=floor(200.0/double(nsteps_total));

   double intensity;
   cout << "Enter intensity (0 - 255)" << endl;
   cin >> intensity;
   int max_intensity=255;
   outputfunc::newline();

// Negative z values map to black:

   cout << -10 << "\t" << 0 << " \t" << 0 << "\t"
        << 0 << "\t" << 2 << endl;
   
// Lowest 20% of z values map from purple to blue:

   double theta_lo=PI/4;
   double theta_hi=0;
   double dtheta=(theta_hi-theta_lo)/nsteps1;
   int red,green,blue;
   double currz;
   for (int n=0; n<nsteps1; n++)
   {
      double theta=theta_lo+n*dtheta;
      red=mathfunc::round(intensity*sin(theta));
      green=0;
      blue=mathfunc::round(intensity*cos(theta));
      currz=zmin+double(n)/double(nsteps_total)*deltaz;
      cout << currz << "\t" << red << " \t" << green << "\t"
           << blue << "\t" << ninterpolations << "\t" 
           << "'" << stringfunc::number_to_string(currz) << "'" << endl;
   }

// Middle 40% of z values map from blue to green:

   theta_lo=0;
   theta_hi=PI/2;
   dtheta=(theta_hi-theta_lo)/nsteps2;
   for (int n=nsteps1; n<nsteps1+nsteps2; n++)
   {
      double theta=theta_lo+(n-nsteps1)*dtheta;
      red=0;
      green=mathfunc::round(intensity*sin(theta));
      blue=mathfunc::round(intensity*cos(theta));
      currz=zmin+double(n)/double(nsteps_total)*deltaz;
      cout << currz << "\t" << red << " \t" << green << "\t"
           << blue << "\t" << ninterpolations << "\t" 
           << "'" << stringfunc::number_to_string(currz) << "'" << endl;
   }

// Top 40% of z values map from green to red:

   theta_lo=0;
   theta_hi=PI/2;
   dtheta=(theta_hi-theta_lo)/nsteps3;
   for (int n=nsteps1+nsteps2; n<=nsteps_total; n++)
   {
      double theta=theta_lo+(n-nsteps1-nsteps2)*dtheta;
      red=mathfunc::round(intensity*sin(theta));
      green=mathfunc::round(intensity*cos(theta));
      blue=0;
      currz=zmin+double(n)/double(nsteps_total)*deltaz;
      cout << currz << "\t" << red << " \t" << green << "\t"
           << blue << "\t" << ninterpolations << "\t" 
           << "'" << stringfunc::number_to_string(currz) << "'" << endl;
   }

// 1000 to "POSITIVEINFINITY" z values map to white:

   cout << 999 << "\t" << red << " \t" << green << "\t"
        << blue << "\t" << 2 << endl;
   cout << 1000 << "\t" << max_intensity << " \t" 
        << max_intensity << "\t"
        << max_intensity << "\t" << 2 << endl;
   
}






