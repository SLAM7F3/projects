// ==========================================================================
// Program MISSED_MOVERS
// ==========================================================================
// Last updated on 4/11/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string filename="missing_movers.txt";
   filefunc::ReadInfile(filename);

   int recovered_relative_imagenumber,abs_recovered_relative_imagenumber;
   vector<int> abs_relative_imagenumber;

// First push back 103 zeros onto abs_relative_imagenumber vector
// corresponding to movers which are instantaneously in track:

   int n_intrack_movers=103;
   for (int i=0; i<n_intrack_movers; i++)
   {
      abs_relative_imagenumber.push_back(0);
   }

// Next push back "infinite" values corresponding to movers which are
// never tracked:

/*
   int n_nevertracked_movers=2;
   for (int i=0; i<n_nevertracked_movers; i++)
   {
      abs_relative_imagenumber.push_back(100);
   }
*/

// Finally push back absolute values of relative image numbers for when
// missed movers go into track:

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      stringfunc::string_to_two_numbers(
         filefunc::text_line[i],recovered_relative_imagenumber,
         abs_recovered_relative_imagenumber);
      abs_relative_imagenumber.push_back(abs_recovered_relative_imagenumber);
   }
   
//   int n_output_bins=100;
//   int n_output_bins=75;
//   int n_output_bins=65;
   int n_output_bins=55;
   prob_distribution prob(abs_relative_imagenumber,n_output_bins);

   string metafile_name="abs_recovered_rel_imagenumber";
   prob.set_densityfilenamestr(metafile_name+".meta");
   string cumulative_name=metafile_name+"_cum";
   prob.set_cumulativefilenamestr(cumulative_name+".meta");
   prob.set_title("Missed mover recovery histogram");
   prob.set_xlabel("Number of video frames until mover is in track");
   prob.set_freq_histogram(true);
   bool gzip_flag=false;
   prob.write_density_dist(gzip_flag);
   prob.write_cumulative_dist(gzip_flag);
//   prob.writeprobdists();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);
   unixcommandstr="meta_to_pdf "+cumulative_name;
   sysfunc::unix_command(unixcommandstr);

}

