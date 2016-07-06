// ==========================================================================
// Program SCINTILLATE reads in 3D averaged and instantaneous ISAR
// XYZP files.  For each voxel, this program computes the fraction
// f=(I/(I+S))^alpha where I [S] denotes the instantaneous [static]
// RCS values and alpha denotes some power ranging between 1 and 2.
// The fused RCS value is taken to equal a linear combination of the
// instanteous and static values weighted by fraction f.  Fused output
// is written to ./xyzp_files/fused_instant_composite_nn.xyzp.

// Program scintillate does not take in any command line arguments.

// ==========================================================================
// Last updated on 4/6/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string subdir="./xyzp_files/";
   string composite_xyzp_filename="AK_isar_probs.xyzp";

// First read in time-averaged composite ISAR image:

   vector<fourvector>* xyzp_composite_ptr=xyzpfunc::read_xyzp_float_data(
      composite_xyzp_filename);

   int starting_imagenumber=0;
   //   cout << "Enter starting imagenumber to be fused:" << endl;
   //cin >> starting_imagenumber;
   //   int stopping_imagenumber=9;
   int stopping_imagenumber=48;
   //   cout << "Enter stopping imagenumber to be fused:" << endl;
//   cin >> stopping_imagenumber;
   int imagenumber_skip=1;
//   cout << "Enter imagenumber skip:" << endl;
//   cin >> imagenumber_skip;

/*
   cout << "Scintillation fraction = 0 	 -->   p_instant << p_static" << endl;
   cout << "Scintillation fraction = 0.5 -->   p_instant = p_static" << endl;
   cout << "Scintillation fraction = 1 	--->   p_instant >> p_static" << endl;
   double scintillation_frac=0.5;
   cout << "Enter scintillation fraction:" << endl;
   cin >> scintillation_frac;
*/
 
   for (int imagenumber=starting_imagenumber; imagenumber <=
           stopping_imagenumber; imagenumber += imagenumber_skip)
   {
      const int ndigits=2;
      string number_str=stringfunc::integer_to_string(imagenumber,ndigits);

      string banner="Fusing static and instantaneous image "+number_str;
      outputfunc::write_big_banner(banner);

      string instantaneous_xyzp_filename=subdir+
         "avgd_instant_sunny_"+number_str+".xyzp";
      vector<fourvector>* xyzp_instant_ptr=xyzpfunc::read_xyzp_float_data(
         instantaneous_xyzp_filename);

      for (unsigned int i=0; i<xyzp_instant_ptr->size(); i++)
      {
         fourvector static_xyzp( xyzp_composite_ptr->at(i) );
         fourvector instantaneous_xyzp( xyzp_instant_ptr->at(i) );
         
         double p_static=static_xyzp.get(3);
         double p_instant=instantaneous_xyzp.get(3);

         // alpha	area in instant-static plane where frac=I/(I+S) > 0.5

         //  1			0.5
	 //  1.2		0.392
	 //  1.3		0.354
	 //  1.4		0.322
	 //  1.5		0.295
	 //  2.0		0.21

//         const double alpha=1.3;
         const double alpha=1.0;
         double frac=pow( (p_instant/(p_instant+p_static)) , alpha );

         double p_fuse;
         if (p_static < 0)
         {
            p_fuse=0;
         }
         else if (p_instant < 0)
         {
            p_fuse=p_static;
         }
         else
           {
             p_fuse=frac*p_instant+(1-frac)*p_static;
           }

         xyzp_instant_ptr->at(i).put(3,p_fuse);
      } // loop over index i labeling points in cloud

      string fused_xyzp_filename=subdir+"fused_instant_composite_"+number_str+
         ".xyzp";
      filefunc::deletefile(fused_xyzp_filename);
      xyzpfunc::write_xyzp_data(
         fused_xyzp_filename,xyzp_instant_ptr,false);
      delete xyzp_instant_ptr;

   } // loop over imagenumber index
}
