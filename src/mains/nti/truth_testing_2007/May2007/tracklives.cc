// ==========================================================================
// Program TRACKLIVES computes the mean and standard deviation of the
// tracklet lifetimes measured in seconds.

// tracklet_lifetimes.size() = 230

// Average tracklet lifetime = 24.5913 +/- 32.4776 secs

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

   string filename="tracklives.txt";
   if (!filefunc::ReadInfile(filename))
   {
      cout << "Couldn't read in file = " << filename << endl;
      exit(-1);
   }

   vector<double> tracklet_lifetimes;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      double curr_tracklet_lifetime=0.5*stringfunc::string_to_number(
         filefunc::text_line[i]);
      if (curr_tracklet_lifetime > 0)
      {
         tracklet_lifetimes.push_back(curr_tracklet_lifetime);
      }
   }

   templatefunc::printVector(tracklet_lifetimes);
   
   cout << "tracklet_lifetimes.size() = " << tracklet_lifetimes.size() 
        << endl;
   double mean_tracklet_lifetime = mathfunc::mean(tracklet_lifetimes);
		// secs
   double sigma_tracklet_lifetime = mathfunc::std_dev(tracklet_lifetimes);
		// secs

   cout << "Average tracklet lifetime = "
        << mean_tracklet_lifetime << " +/- "
        << sigma_tracklet_lifetime << " secs" << endl;

   int n_output_bins=50;
   prob_distribution prob(tracklet_lifetimes,n_output_bins);

   string metafile_name="tracklet_lifetimes";
   prob.set_densityfilenamestr(metafile_name+".meta");
   string cumulative_name=metafile_name+"_cum";
   prob.set_cumulativefilenamestr(cumulative_name+".meta");
   prob.set_title("Tracklet lifetime histogram");
   prob.set_xlabel("Vehicle tracklet lifetime (secs)");
   prob.set_freq_histogram(true);
   prob.set_xtic(25);	// secs
   bool gzip_flag=false;
   prob.write_density_dist(gzip_flag);
   prob.write_cumulative_dist(gzip_flag);
//   prob.writeprobdists();

   string unixcommandstr="meta_to_pdf "+metafile_name;
   sysfunc::unix_command(unixcommandstr);
   unixcommandstr="meta_to_pdf "+cumulative_name;
   sysfunc::unix_command(unixcommandstr);

   
}

