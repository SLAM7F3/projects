// ==========================================================================
// Program PARSE_COLOR_TRIPLES


//                     ./parse_color_triples

// ==========================================================================
// Last updated on 1/27/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;


void parse_color_triple(string curr_basename, threevector& curr_XYZ)
{
   cout << "curr_basename = " << curr_basename << endl;
   vector<string> substrings = stringfunc::decompose_string_into_substrings(
      curr_basename, "_");

   double R_foreground = stringfunc::string_to_number(substrings[1]);
   double R_background = stringfunc::string_to_number(substrings[2]);
   double dR = fabs(R_foreground - R_background);

   double G_foreground = stringfunc::string_to_number(substrings[3]);
   double G_background = stringfunc::string_to_number(substrings[4]);
   double dG = fabs(G_foreground - G_background);

   double B_foreground = stringfunc::string_to_number(substrings[5]);
   double B_background = stringfunc::string_to_number(substrings[6]);
   double dB = fabs(B_foreground - B_background);   

   /*
   cout << "R_foreground = " << R_foreground 
        << " R_background = " << R_background << endl;
   cout << "G_foreground = " << G_foreground 
        << " G_background = " << G_background << endl;
   cout << "B_foreground = " << B_foreground 
        << " B_background = " << B_background << endl;
   */

   dR /= 255;
   dG /= 255;
   dB /= 255;

   curr_XYZ=threevector(dR, dG, dB);
}


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   string colored_chars_subdir="./colored_chars/";
   string legible_subdir=colored_chars_subdir+"legible_digits/";
   string unreadable_subdir=colored_chars_subdir+"unreadable_digits/";
   
   vector<string> legible_image_filenames=filefunc::image_files_in_subdir(
      legible_subdir);
   vector<string> unreadable_image_filenames=filefunc::image_files_in_subdir(
      unreadable_subdir);
   
   vector<threevector> XYZ;
   vector<double> P;

// Import legible color triples:

   for(unsigned int i = 0; i < legible_image_filenames.size(); i++)
   {
      string curr_basename=filefunc::getbasename(legible_image_filenames[i]);
      threevector curr_XYZ;
      parse_color_triple(curr_basename, curr_XYZ);
      double curr_P = 1.0;
      XYZ.push_back(curr_XYZ);
      P.push_back(curr_P);

   } // loop over index i labeling legible color triples

// Import unreadable color triples:

   for(unsigned int i = 0; i < unreadable_image_filenames.size(); i++)
   {
      string curr_basename=filefunc::getbasename(
         unreadable_image_filenames[i]);   
      threevector curr_XYZ;
      parse_color_triple(curr_basename, curr_XYZ);
      double curr_P = 0.0;
      XYZ.push_back(curr_XYZ);
      P.push_back(curr_P);
   } 

   string tdp_filename="legible_vs_unreadable.tdp";
   tdpfunc::write_xyzp_data(tdp_filename, &XYZ, &P);
   string banner="Exported "+tdp_filename;
   outputfunc::write_banner(banner);

// Export legible and unreadable color triples to output text file:

   string output_filename="legible_vs_unreadable.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);
   for(unsigned int i = 0; i < XYZ.size(); i++)
   {
      threevector curr_XYZ = XYZ[i];
      outstream << curr_XYZ.get(0) << "  "
                << curr_XYZ.get(1) << "  "
                << curr_XYZ.get(2) << "  "
                << P.at(i) << endl;
   }
   filefunc::closefile(output_filename, outstream);

   banner="Exported "+output_filename;
   outputfunc::write_banner(banner);
}


