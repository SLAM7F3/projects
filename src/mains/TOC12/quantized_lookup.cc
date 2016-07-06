// ==========================================================================
// Program QUANTIZED_LOOKUP exports or imports a binary file
// containing 256x256x256=16M bytes. If exporting, this program reads
// in a set of hand-picked RGB triples with assigned quantized color
// names.  It essentially sets up a Voronoi diagram with 3D RGB space
// and maps all 16M RGB triples to their closest Voronoi neighbor.  If
// importing, QUANTIZED_LOOKUP queries the user to enter an RGB triple
// and returns their quantized values.
// ==========================================================================
// Last updated on 9/11/12; 9/12/12; 9/21/12; 10/29/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "video/RGB_analyzer.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();

//   string liberalized_color="";
//   string liberalized_color="yellow"; // TOC12 yellow radiation
   string liberalized_color="blue";	// TOC12 water, blue radiation, gas
//   string liberalized_color="blue2";	// TOC12 gas
//   string liberalized_color="black";	// TOC12 eat,skull
//   string liberalized_color="orange";	// TOC12 biohazard
//   string liberalized_color="red";	// TOC12 stop
//   string liberalized_color="green";	// TOC12 start
   RGB_analyzer_ptr->export_quantized_RGB_lookup_table(
      liberalized_color);

/*
   exit(-1);
   
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(
      liberalized_color);

   int R,G,B;
   int R_quantized,G_quantized,B_quantized;
   while (true)
   {
      cout << "Enter R:" << endl;
      cin >> R;
      cout << "Enter G:" << endl;
      cin >> G;
      cout << "Enter B:" << endl;
      cin >> B;

      string computed_color_name=
         RGB_analyzer_ptr->compute_quantized_color_name(R,G,B);
      cout << "Computed color name = " << computed_color_name << endl;
      string retrieved_color_name=
         RGB_analyzer_ptr->retrieve_quantized_colorname_from_lookup_map(R,G,B);
      cout << "Retrieved color name = " << retrieved_color_name << endl;
   }
*/

   delete RGB_analyzer_ptr;
}

