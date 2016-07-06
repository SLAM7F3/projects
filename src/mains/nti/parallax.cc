// ==========================================================================
// Program PARALLAX is a special case utility to generate plots for
// the NTI ladar/video fusion analysis.  
// ==========================================================================
// Last updated on 11/8/05; 7/30/06; 12/4/10
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;

// Clewiston persistent surveillance racetrack parameters:

   double H=4698;	// meters
   double phidot=0.019;	 // sec**-1
   double R=4161;	// meters
   
   double b_start=0;
   double b_stop=1000;	// meters
//   double b_stop=16;	// meters
   double db=2;
//   double db=0.1;
   int n_steps=(b_stop-b_start)/db+1;
   double conversion=0.447;	// 1 mile/hour = 0.447 m/sec

// Assumed street parameter values:

   double W=10;	// street width in meters

   double d=2;	// recession of trees from street curb
//   cout << "Enter recession distance d:" << endl;
//   cin >> d;

   double z_tree=0;  // elevation of tree ground height above sea-level 

//   string filename="parallax.meta";
//   string filename="occlusion.meta";
   string filename="false_alarms.meta";
   ofstream outstream;
   filefunc::openfile(filename,outstream);

   const double Lcar=4.8;	// typical car length in meters

// Projected "flagpole" length on ground for which parallax causes
// less than one pixel frame-to-frame misregistration.  Therefore, no
// false alarms are expected for projected lengths less than rstar:

   const double rstar=63.1;	

   for (int n=0; n<n_steps; n++)
   {
      double b=b_start+n*db;
      double v_app=b/(H-b)*R*phidot;
      v_app /= conversion;
//      cout << b << "\t\t" << v_app << endl;
      double delta=(b*R/(H-z_tree-b)-d)/W;

      double false_alarms=(b*R/(H-b) - rstar)/Lcar;
      false_alarms=basic_math::max(0.0,false_alarms);
      
      delta=basic_math::min(1.0,delta);
      delta=basic_math::max(0.0,delta);

//      outstream << b << " \t\t" << v_app << endl;
//      outstream << b << " \t\t" << delta*100 << endl;
      outstream << b << " \t\t" << false_alarms << endl;
   }
   filefunc::closefile(filename,outstream);
  
}
