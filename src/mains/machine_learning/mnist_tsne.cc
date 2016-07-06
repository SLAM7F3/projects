// ==========================================================================
// Program MNIST_TSNE is a playground for viewing T-SNE layout for 2500 MNIST
// digits in 2-dimensions.
// ==========================================================================
// Last updated on 11/1/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/constants.h"
#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   string mnist_subdir = "/home/cho/programs/python/tsne_python/";
   string labels_filename = mnist_subdir+"mnist2500_labels.txt";
   string Ycoords_filename = mnist_subdir+"mnist2500_Y.txt";
   
   filefunc::ReadInfile(labels_filename);
   int n_labels = filefunc::text_line.size();
   cout << "n_labels = " << n_labels << endl;

   vector<int> labels;
   for(int i = 0; i < n_labels; i++)
   {
      string curr_label=filefunc::text_line[i];
      labels.push_back(stringfunc::string_to_number(curr_label));
   }

   filefunc::ReadInfile(Ycoords_filename);

   vector<double> X, Y;
   double min_val = POSITIVEINFINITY;
   double max_val = NEGATIVEINFINITY;
   for(int i = 0; i < n_labels; i++)
   {
      vector<double> curr_coords = stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_X = curr_coords[0];
      double curr_Y = curr_coords[1];
      min_val = basic_math::min(min_val, curr_X);
      max_val = basic_math::max(max_val, curr_X); 
      min_val = basic_math::min(min_val, curr_Y);
      max_val = basic_math::max(max_val, curr_Y);
      X.push_back(curr_X);
      Y.push_back(curr_Y);
      cout << "i = " << i << " label = " << labels[i]
           << " X = " << X.back()
           << " Y = " << Y.back() << endl;
   }
   cout << "min_val = " << min_val << " max_val = " << max_val << endl;
   
// Generate metafile output whose markers are colored according to
// MNIST class labels:

   metafile curr_metafile;
   
   string meta_filename="mnist_2500";
   string title="TSNE layout for MNIST 2500";
   string x_label="X";
   string y_label="Y";

   curr_metafile.set_legend_flag(true);
   curr_metafile.set_parameters(
      meta_filename,title,x_label,y_label,
      min_val, max_val, min_val, max_val);

   curr_metafile.openmetafile();
   curr_metafile.write_header();

   for(unsigned int i = 0; i < labels.size(); i++){
     curr_metafile.set_legendlabel(stringfunc::number_to_string(labels[i]));
   }
   curr_metafile.write_markers(labels,X,Y);

   curr_metafile.closemetafile();

   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);
}

