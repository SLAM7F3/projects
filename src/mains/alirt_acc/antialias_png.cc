// ==========================================================================
// Program ANTIALIAS_PNG queries the user to enter some PNG image.
// Every non-black pixel within the original image is left untouched.
// But black pixels which are surrounded by 3 (or 2) non-black "corner
// neighbors" are recolored to some fraction of the neighbor values.
// This program effectively smears out jaggy edges in PNG images
// without diminishing edge brightness.  We cooked up this program for
// poster generation purposes.
// ==========================================================================
// Last updated on 8/10/05; 12/2/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/pngfuncs.h"
#include "general/sysfuncs.h"

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

// Read and store contents of PNG image:

   string input_subdir="./";
   string png_filename;
//   cout << "Enter input png filename:" << endl;
//   cin >> png_filename;
   png_filename="new_abstract_streets.png";

   string output_png_filename="antialiased_image.png";
   pngfunc::open_png_file(input_subdir+png_filename);
   pngfunc::parse_png_file();

   pngfunc::allocate_RGB_twoDarrays();
   pngfunc::fill_RGB_twoDarrays();

   vector<double> filter_frac;
//   int nmax_iters=2;	// buildings
   int nmax_iters=3;	// streets
//   int nmax_iters=4;	// streets
   for (int iter=0; iter<nmax_iters; iter++)
   {
      filter_frac.clear();
      switch (iter)
      {
         case 0:
            filter_frac.push_back(0.85);
            filter_frac.push_back(0.75);
            filter_frac.push_back(0.90);
//            filter_frac.push_back(0.75);   // Fracs for bldgs in chunk45-51
//            filter_frac.push_back(0.60);   // for poster
//            filter_frac.push_back(0.00);
            break;
         case 1:
            filter_frac.push_back(0.85);
            filter_frac.push_back(0.75);
            filter_frac.push_back(0.70);
            break;
         case 2:
            filter_frac.push_back(0.85);
            filter_frac.push_back(0.75);
            filter_frac.push_back(0.50);
            break;
         case 3:
            filter_frac.push_back(0.85);
            filter_frac.push_back(0.75);
            filter_frac.push_back(0.30);
            break;
      }
      pngfunc::antialias_RGB_twoDarrays(filter_frac);
   }

   nmax_iters=2;
   for (int iter=0; iter<nmax_iters; iter++)
   {
      pngfunc::smear_RGB_twoDarrays();
   }
   
   pngfunc::transfer_RGB_twoDarrays_to_rowpointers();
   pngfunc::write_output_png_file(output_png_filename);

   pngfunc::delete_RGB_twoDarrays();
}
