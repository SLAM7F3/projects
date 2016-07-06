// ==========================================================================
// Program GENERATE_TSNE_INPUT imports 2.5K MNIST digit thumbnails in
// text form from the python subdirectory accompanying downloaded
// T-SNE software.  It also imports the thumbnails' corresponding
// digit labels.  This program reconstructs each of the MNIST
// thumbnails and exports them as jpg files.  It also generates a
// binary data file according to the format specified in "User's Guide
// for t-SNE Software".  The binary data file becomes input to the
// t-SNE C++ binary which cleverly projects high-dimensional feature
// vectors down to 2 or 3 dimensions for visualization purposes.

// ==========================================================================
// Last updated on 11/2/15
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
#include "video/texture_rectangle.h"

using std::ofstream;
using std::string;
using std::vector;

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   int xdim = 28;
   int ydim = 28;
   int n_images = 1;
   int n_channels = 1;
   int n_feature_dims = xdim * ydim;
   double perplexity = 30;
   int n_reduced_dims = 2;

// Theta represents a trade-off parameter between speed and accuracy.
// Theta = 0 corresponds to standard, slow t-SNE, while theta = 1
// makes very crude approximations. Appropriate values for theta are
// between 0.1 and 0.7 (default = 0.5).

   double theta = 0.5;

   string mnist_text_input_subdir = "/home/pcho/programs/python/tsne_python/";
   string input_text_filename = mnist_text_input_subdir+"mnist2500_X.txt";

   string mnist_subdir = "./mnist_2500/";
   string labels_filename = mnist_subdir+"mnist2500_labels.txt";
   string digits_subdir = mnist_subdir + "mnist_digits/";
   filefunc::dircreate(digits_subdir);

   string blank_filename="blank.jpg";
   double grey_level = 0.5;
   texture_rectangle* curr_texture_rectangle_ptr = 
      new texture_rectangle(xdim, ydim, n_images, n_channels, NULL);
   curr_texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
   curr_texture_rectangle_ptr->generate_blank_image_file(
      xdim, ydim, blank_filename, grey_level);

// First import integer labels for each input MNIST digit:

   filefunc::ReadInfile(labels_filename);
   int n_labels = filefunc::text_line.size();
   cout << "n_labels = " << n_labels << endl;

   vector<int> labels;
   for(int i = 0; i < n_labels; i++)
   {
      string curr_label=filefunc::text_line[i];
      labels.push_back(stringfunc::string_to_number(curr_label));
   }

// Open binary data file and export header information to it:

   string output_binary_filename=mnist_subdir+"data.dat";
   ofstream binary_stream;
   filefunc::open_binaryfile(output_binary_filename, binary_stream);

   filefunc::writeobject(binary_stream, n_labels);
   filefunc::writeobject(binary_stream, n_feature_dims);
   filefunc::writeobject(binary_stream, theta);
   filefunc::writeobject(binary_stream, perplexity);
   filefunc::writeobject(binary_stream, n_reduced_dims);

// Next import pixel intensities for each input MNIST digit:

   filefunc::ReadInfile(input_text_filename);
   int n_lines = filefunc::text_line.size();
   for(int i = 0; i < n_lines; i++)
   {
      vector<double> values = stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      int counter = 0;
      for(int py = 0; py < ydim; py++)
      {
         for(int px = 0; px < xdim; px++)
         {
            double curr_intensity = static_cast<double>(values[counter]);
	    filefunc::writeobject(binary_stream, curr_intensity);

// Empirically found on 11/2/15 that MNIST digits coming from TSNE
// text file are transposed:

            curr_texture_rectangle_ptr->set_pixel_intensity_value(
               py, px, 255 * int(curr_intensity));
            counter++;
         } // loop over px index
      } // loop over py index

      string digit_filename = digits_subdir+
         "mnist_"+stringfunc::number_to_string(labels[i])+"_"+
         stringfunc::integer_to_string(i,4)+".jpg";
      curr_texture_rectangle_ptr->write_curr_frame(digit_filename);

//      string banner="Exported MNIST digit file "+digit_filename;
//      outputfunc::write_banner(banner);

   } // loop over index i labeling text line

   filefunc::closefile(output_binary_filename, binary_stream);

   string banner="Exported binary data file to "+output_binary_filename;
   outputfunc::write_banner(banner);

   banner="Exported MNIST digit thumbnails to "+digits_subdir;
   outputfunc::write_banner(banner);
}

