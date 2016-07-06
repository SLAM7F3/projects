// ==========================================================================
// Program LINK_SYMBOL_IMAGES loops over all 9 TOC12 signs.  For each
// one, it generates links to the other 8 signs.  We wrote this
// utility program in order to train the Ng classifier on negative
// examples of symbols to hopefully reduce false alarms.

//				link_symbol_images

// ==========================================================================
// Last updated on 10/4/12; 10/20/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "cluster/akm.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string TOC12_subdir="/home/cho/programs/c++/svn/projects/src/mains/TOC12/";
   string final_signs_subdir=TOC12_subdir+"images/final_signs/";
//   string ppt_signs_subdir=TOC12_subdir+"images/ppt_signs/";
   string not_particular_signs_subdir=TOC12_subdir+
      "images/non_signs/not_particular_final_signs/";

   vector<string> symbol_names;
   symbol_names.push_back("yellow_radiation");
   symbol_names.push_back("orange_biohazard");
   symbol_names.push_back("blue_radiation");
   symbol_names.push_back("blue_water");
   symbol_names.push_back("blue_gas");
   symbol_names.push_back("green_start");
   symbol_names.push_back("red_stop");
   symbol_names.push_back("bw_skull");
   symbol_names.push_back("bw_eat");

   for (int s=0; s<symbol_names.size(); s++)
   {
      string symbol_name=symbol_names[s];
      string banner="Processing symbol = "+symbol_name;
      outputfunc::write_big_banner(banner);

//      cout << endl;
//      cout << "Enter symbol name:" << endl;
//      cout << "  yellow_radiation,orange_biohazard,blue_water" << endl;
//      cout << "  blue_radiation,blue_gas,red_stop" << endl;
//      cout << "  green_start,bw_skull,bw_eat:" << endl;
//      cin >> symbol_name;

      string not_sign_subdir=not_particular_signs_subdir+"not_"+
         symbol_name+"/";
      filefunc::dircreate(not_sign_subdir);

      for (int d=0; d<symbol_names.size(); d++)
      {
         if (d==s) continue;
         
         string different_symbol_name=symbol_names[d];
//         cout << endl;
//         cout << "Enter different symbol name:" << endl;
//         cout << "  yellow_radiation,orange_biohazard,blue_water" << endl;
//         cout << "  blue_radiation,blue_gas,red_stop" << endl;
//         cout << "  green_start,bw_skull,bw_eat:" << endl;
//         cin >> different_symbol_name;
   
         string different_symbols_subdir=
            final_signs_subdir+"synthetic_symbols/"+
            different_symbol_name+"/";        

         cout << "not_sign_subdir = " << not_sign_subdir << endl;
         cout << "different_symbols_subdir = " << different_symbols_subdir
              << endl << endl;

//         int n_images=2000;
         int n_images=3000;
         for (int i=0; i<n_images; i++)
         {
            string different_image_filename=different_symbols_subdir+
               "synthetic_char_"+stringfunc::integer_to_string(i,5)+".png";
            string linked_image_filename=not_sign_subdir+
               different_symbol_name+"_"+stringfunc::integer_to_string(i,4)
               +".png";
            string unix_cmd="ln -s "+different_image_filename+" "+
               linked_image_filename;
//            cout << unix_cmd << endl << endl;
            sysfunc::unix_command(unix_cmd);
         }

      } // loop over index d labeling different symbol names
   
   } // loop over index s labeling symbol names

}

