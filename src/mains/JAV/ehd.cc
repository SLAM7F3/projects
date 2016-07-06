// ==========================================================================
// Program EdgeHistogramDescriptor
// ==========================================================================
// Last updated on 9/3/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "video/descriptorfuncs.h"
#include "video/photograph.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string ImageEngine_subdir="/data/ImageEngine/";
//   string image_filename=ImageEngine_subdir+"kermit/kermit000.jpg";
//   string image_filename="./vertical_stripes.jpg";
//   string image_filename="./horizontal_stripes.jpg";
//   string image_filename="./diagonal45_stripes.jpg";
   string image_filename="./diagonal135_stripes.jpg";

   cout << "Enter image filename:" << endl;
   cin >> image_filename;
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   vector<double> edge_histogram=descriptorfunc::compute_edge_histogram(
      image_filename,texture_rectangle_ptr);
   delete texture_rectangle_ptr;

   for (unsigned int e=0; e<edge_histogram.size(); e++)
   {
      cout << "e = " << e << " edge_hist = " << edge_histogram[e]
           << endl;
   }
   

}
