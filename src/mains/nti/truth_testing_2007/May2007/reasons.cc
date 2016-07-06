// ==========================================================================
// Program REASONS tabulates the percentage frequency for track
// termination reasons from the May 2007 truth testing results.

/*

Reason n = 1 frequency = 12 percentage = 4.47761
Reason n = 2 frequency = 1 percentage = 0.373134
Reason n = 3 frequency = 0 percentage = 0
Reason n = 4 frequency = 28 percentage = 10.4478
Reason n = 5 frequency = 43 percentage = 16.0448
Reason n = 6 frequency = 12 percentage = 4.47761
Reason n = 7 frequency = 29 percentage = 10.8209
Reason n = 8 frequency = 25 percentage = 9.32836
Reason n = 9 frequency = 2 percentage = 0.746269
Reason n = 10 frequency = 7 percentage = 2.61194
Reason n = 11 frequency = 1 percentage = 0.373134
Reason n = 12 frequency = 20 percentage = 7.46269
Reason n = 13 frequency = 47 percentage = 17.5373
Reason n = 14 frequency = 10 percentage = 3.73134
Reason n = 15 frequency = 3 percentage = 1.1194
Reason n = 16 frequency = 10 percentage = 3.73134
Reason n = 17 frequency = 18 percentage = 6.71642
frequency_sum = 268
percentage_sum = 100

*/

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

   string filename="reasons.txt";
   if (!filefunc::ReadInfile(filename))
   {
      cout << "Couldn't read in file = " << filename << endl;
      exit(-1);
   }

   vector<int> reasons;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << filefunc::text_line[i] << endl;
      int curr_reason=stringfunc::string_to_integer(
         filefunc::text_line[i]);
      if (curr_reason > 0)
      {
         reasons.push_back(curr_reason);
      }
   }

   templatefunc::printVector(reasons);
   cout << "reasons.size() = " << reasons.size() << endl;
   
   vector<int> reason_frequency;
   const int n_reasons=18;
   for (int n=0; n<n_reasons; n++)
   {
      reason_frequency.push_back(0);
   }

   for (int i=0; i<reasons.size(); i++)
   {
      int curr_reason=reasons[i];
      reason_frequency[curr_reason]++;
   }

   int frequency_sum=0;
   double percentage_sum=0;
   for (int n=1; n<n_reasons; n++)
   {
      double reason_percentage=100*reason_frequency[n]/double(reasons.size());
      cout << "Reason n = " << n
           << " frequency = " << reason_frequency[n] 
           << " percentage = " << reason_percentage 
           << endl;
      frequency_sum += reason_frequency[n];
      percentage_sum += reason_percentage;
   }

   cout << "frequency_sum = " << frequency_sum << endl;
   cout << "percentage_sum = " << percentage_sum << endl;
   
}

