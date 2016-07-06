// ==========================================================================
// Program PERCENTAGES computes the mean and standard deviation of the
// percentage times vehicles were in track.

// Percentage time vehicles followed by human testers were in track = 

//				78 +/- 17 %


// ==========================================================================
// Last updated on 7/30/07
// ==========================================================================

#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
//   cout.precision(15);

   string filename="percentages.txt";
   if (!filefunc::ReadInfile(filename))
   {
      cout << "Couldn't read in file = " << filename << endl;
      exit(-1);
   }

   vector<double> percentages;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      double curr_percentage=stringfunc::string_to_number(
         filefunc::text_line[i]);
      if (curr_percentage > 0)
      {
         percentages.push_back(curr_percentage);
      }
   }

   templatefunc::printVector(percentages);
   
   cout << "percentages.size() = " << percentages.size() << endl;
   double mean_percentage = mathfunc::mean(percentages);
   double sigma_percentage = mathfunc::std_dev(percentages);

   cout << "mean_percentage = " << mean_percentage << endl;
   cout << "sigma_percentage = " << sigma_percentage << endl;


   int n_output_bins=25;
   prob_distribution prob(percentages,n_output_bins);

   string metafile_name="percentages";
   prob.set_densityfilenamestr(metafile_name+".meta");
   string cumulative_name=metafile_name+"_cum";
   prob.set_cumulativefilenamestr(cumulative_name+".meta");
   prob.set_title("Percentage time vehicles are in track");
   prob.set_xlabel("Percentage time vehicle in track");
   prob.set_freq_histogram(true);
   prob.set_xtic(10);	
   bool gzip_flag=false;
   prob.write_density_dist(gzip_flag);
   prob.write_cumulative_dist(gzip_flag);
//   prob.writeprobdists();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);
   unixcommandstr="meta_to_pdf "+cumulative_name;
   sysfunc::unix_command(unixcommandstr);

}

