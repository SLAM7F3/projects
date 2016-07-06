// ==========================================================================
// Program GPS_vs_RECON reads in 42 iPhone GPS world-positions along
// with corresponding reconstructed camera world-positions.  This
// program computes the residual distribution between the two assuming
// the GPS positions represent ground truth.  It also populates a
// probability distribution with the residual information and outputs
// a frequency histogram metafile.

//				gps_vs_recon

// adobe prob_density.meta
// convert prob_density.ps prob_density.png
// view prob_density.png

// ==========================================================================
// Last updated on 2/25/10; 3/6/10
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   vector<twovector> gps_posns,reconstructed_posns;

   string filename="iphone_gps_posns.dat";
   filefunc::ReadInfile(filename);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string image_filename=filefunc::getbasename(substrings[0]);
      double easting=stringfunc::string_to_number(substrings[1]);
      double northing=stringfunc::string_to_number(substrings[2]);
      gps_posns.push_back(twovector(easting,northing));
   }
   
   filename="reconstructed_iphone_posns.dat";
   filefunc::ReadInfile(filename);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string image_filename=filefunc::getbasename(substrings[0]);
      double easting=stringfunc::string_to_number(substrings[1]);
      double northing=stringfunc::string_to_number(substrings[2]);
      reconstructed_posns.push_back(twovector(easting,northing));
   }

   vector<double> residuals;
   for (unsigned int i=0; i<gps_posns.size(); i++)
   {
      double curr_residual=(gps_posns[i]-reconstructed_posns[i]).magnitude();
      residuals.push_back(curr_residual);
//      cout << "i = " << i << " residual = " << curr_residual << endl;
   }

   std::sort(residuals.begin(),residuals.end());
   for (unsigned int i=0; i<residuals.size(); i++)
   {
      cout << "i = " << i << " residual = " << residuals[i] << endl;
   }
   double median_residual = mathfunc::median_value(residuals);

   string banner="Median residual = "+stringfunc::number_to_string(
      median_residual);
   outputfunc::write_banner(banner);

   int n_output_bins=200;
   cout << "Enter n_output_bins:" << endl;
   cin >> n_output_bins;
   prob_distribution prob(residuals,n_output_bins);
   cout << "median = " << prob.median() << endl;

   cout << "20 percentile value = " << prob.find_x_corresponding_to_pcum(0.2)
        << endl;
   cout << "40 percentile value = " << prob.find_x_corresponding_to_pcum(0.4)
        << endl;
   cout << "60 percentile value = " << prob.find_x_corresponding_to_pcum(0.6)
        << endl;
   cout << "80 percentile value = " << prob.find_x_corresponding_to_pcum(0.8)
        << endl;

   prob.set_freq_histogram(true);
   bool gzip_flag=false;
   prob.writeprobdists(gzip_flag);
}

/*

i = 0 residual = 2.13847
i = 1 residual = 2.44368
i = 2 residual = 2.47243
i = 3 residual = 3.14802
i = 4 residual = 3.22035
i = 5 residual = 4.65079
i = 6 residual = 5.03525
i = 7 residual = 6.11603
i = 8 residual = 6.24401
i = 9 residual = 6.67553
i = 10 residual = 6.68312
i = 11 residual = 6.8552
i = 12 residual = 7.01727
i = 13 residual = 7.85769
i = 14 residual = 7.97088
i = 15 residual = 8.75722
i = 16 residual = 8.88794
i = 17 residual = 10.4736
i = 18 residual = 10.7031
i = 19 residual = 10.8128
i = 20 residual = 11.1446
i = 21 residual = 12.4038
i = 22 residual = 13.0599
i = 23 residual = 13.2817
i = 24 residual = 13.5143
i = 25 residual = 13.5161
i = 26 residual = 13.9391
i = 27 residual = 19.4507
i = 28 residual = 20.4433
i = 29 residual = 20.5531
i = 30 residual = 21.816
i = 31 residual = 23.0461
i = 32 residual = 26.1973
i = 33 residual = 27.5339
i = 34 residual = 32.1894
i = 35 residual = 32.416
i = 36 residual = 33.2453
i = 37 residual = 33.9396
i = 38 residual = 42.1257
i = 39 residual = 414.599
i = 40 residual = 596.715
i = 41 residual = 598.535

Median residual = 11.7742

*/
