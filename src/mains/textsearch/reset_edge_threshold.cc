// ========================================================================
// Program RESET_EDGE_THRESHOLD imports the edge list output by
// program DOCRELNS.  It queries the user to enter a more strict edge
// weight threshold and exports a new version of the edge list.

//			reset_edge_threshold

// ========================================================================
// Last updated on 12/15/12; 12/25/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
   string astro_subdir=arXiv_subdir+"astro/";
   string reuters_subdir=arXiv_subdir+"reuters/export/";
//   string text_subdir=astro_subdir+"txt/";
//   string text_subdir=reuters_subdir+"text/5K/";
   string text_subdir=reuters_subdir+"text/43K/";
   string edgelist_filename=text_subdir+"docs_edgelist.dat";

   filefunc::ReadInfile(edgelist_filename);

   int edge_weight_threshold,min_edge_weight_threshold=50;
   cout << "Enter new edge weight threshold:" << endl;
   cin >> edge_weight_threshold;

   if (edge_weight_threshold <= min_edge_weight_threshold) exit(-1);
   
   ofstream outstream;
   string output_filename=text_subdir+"docs_edgelist_"+
      stringfunc::number_to_string(edge_weight_threshold)+".dat";
   filefunc::openfile(output_filename,outstream);
   outstream << "# Edge weight threshold = " << edge_weight_threshold 
             << endl;
   outstream << "# NodeID  NodeID'  Edge weight" << endl << endl;

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      vector<double> column_values=stringfunc::string_to_numbers(curr_line);
      if (column_values[2] < edge_weight_threshold) continue;
      outstream << curr_line << endl;
   }
   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}

