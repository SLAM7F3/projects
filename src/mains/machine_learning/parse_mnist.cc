// ==========================================================================
// Program PARSE_MNIST imports binary data files containing 60K MNIST
// digits and their labels.  It reconstructs and exports each digit as
// a JPG thumbnail.  PARSE_MNIST also generates a binary data file
// which can be ingested by the C++ T-SNE binary for plotting,
// clustering and visualization purposes.
// ==========================================================================
// Last updated on 11/3/15; 11/13/15; 11/14/15; 11/15/15
// ==========================================================================

#include <stdint.h>
#include <byteswap.h>
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

using std::ifstream;
using std::ofstream;

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   string mnist_data_subdir = "/data/peter_stuff/imagery/mnist_data/";
   string training_labels_filename = mnist_data_subdir+
      "train-labels-idx1-ubyte";
   string training_images_filename = mnist_data_subdir+
      "train-images-idx3-ubyte";
   string digits_subdir = mnist_data_subdir+"digits/";
   filefunc::dircreate(digits_subdir);
   string training_digits_subdir = digits_subdir+"training/";
   filefunc::dircreate(training_digits_subdir);

// First import labels for MNIST thumbnails:

   ifstream binary_stream;
   filefunc::open_binaryfile(training_labels_filename, binary_stream);

   unsigned char curr_char;
   int magic_number, n_thumbnails;
   filefunc::readobject(binary_stream, magic_number);
   filefunc::readobject(binary_stream, n_thumbnails);

   magic_number = filefunc::swap_int32(magic_number);
   n_thumbnails = filefunc::swap_int32(n_thumbnails);

   cout << "magic_number = " << magic_number << endl;
   cout << "n_thumbnails = " << n_thumbnails << endl;

   vector<int> labels;
   for(int i = 0; i < n_thumbnails; i++)
   {
      filefunc::readobject(binary_stream, curr_char);

      int curr_label = static_cast<int>(curr_char);
      labels.push_back(curr_label);
//      cout << "i = " << i << " label = " << curr_label << endl;
   }
   filefunc::closefile(training_labels_filename, binary_stream);   

// Next import MNIST thumbnails:

   filefunc::open_binaryfile(training_images_filename, binary_stream);

   int xdims, ydims;
   filefunc::readobject(binary_stream, magic_number);
   filefunc::readobject(binary_stream, n_thumbnails);
   filefunc::readobject(binary_stream, ydims);
   filefunc::readobject(binary_stream, xdims);

   n_thumbnails = filefunc::swap_int32(n_thumbnails);
   xdims = filefunc::swap_int32(xdims);
   ydims = filefunc::swap_int32(ydims);

// Open text file to which all MNIST image files will be written:

   string list_filename = training_digits_subdir + "list_tmp.txt";
   ofstream list_stream;
   filefunc::openfile(list_filename, list_stream);

// Open text file for fake edge list:

   string edgelist_filename = training_digits_subdir + "edgelist.dat";
   ofstream edgelist_stream;
   filefunc::openfile(edgelist_filename, edgelist_stream);
   edgelist_stream << "# NodeID  NodeID'  Edge weight" << endl << endl;

// Open text file for class label corresponding to each image:

   string labels_filename = training_digits_subdir + "image_labels.dat";
   ofstream labels_stream;
   filefunc::openfile(labels_filename, labels_stream);
   labels_stream << "# imageID  label" << endl << endl;

// Open binary file to which all MNIST data will be written so that it
// can be processed by T-SNE C++ binary:

   string tsne_binary_filename=mnist_data_subdir+"tsne_data.dat";
   ofstream tsne_binary_stream;
   filefunc::open_binaryfile(tsne_binary_filename, tsne_binary_stream);

   int n_feature_dims = xdims * ydims;
   double perplexity = 30;
   int n_reduced_dims = 2;

// Theta represents a trade-off parameter between speed and accuracy.
// Theta = 0 corresponds to standard, slow t-SNE, while theta = 1
// makes very crude approximations. Appropriate values for theta are
// between 0.1 and 0.7 (default = 0.5).

   double theta = 0.5;

   filefunc::writeobject(tsne_binary_stream, n_thumbnails);
   filefunc::writeobject(tsne_binary_stream, n_feature_dims);
   filefunc::writeobject(tsne_binary_stream, theta);
   filefunc::writeobject(tsne_binary_stream, perplexity);
   filefunc::writeobject(tsne_binary_stream, n_reduced_dims);

// Export thumbnails as JPEG images to training_digits_subdir:

   int n_images = 1;
   int n_channels = 1;
   string blank_filename="blank.jpg";
   double grey_level = 0.5;
   texture_rectangle* curr_texture_rectangle_ptr = 
      new texture_rectangle(xdims, ydims, n_images, n_channels, NULL);
   curr_texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
   curr_texture_rectangle_ptr->generate_blank_image_file(
      xdims, ydims, blank_filename, grey_level);

   for(int i = 0; i < n_thumbnails; i++)
   {
      outputfunc::update_progress_fraction(i, 1000, n_thumbnails);

      for(int py = 0; py < ydims; py++)
      {
         for(int px = 0; px < xdims; px++)
         {
            filefunc::readobject(binary_stream, curr_char);
            curr_texture_rectangle_ptr->set_pixel_intensity_value(
               px, py, int(curr_char));
            double normalized_intensity = static_cast<double>(curr_char) 
               / 255.0;
	    filefunc::writeobject(tsne_binary_stream, normalized_intensity);
         } // loop over px
      } // loop over py

      string digit_filename = training_digits_subdir+
         "mnist_"+stringfunc::number_to_string(labels[i])+"_"+
         stringfunc::integer_to_string(i,5)+".jpg";
      curr_texture_rectangle_ptr->write_curr_frame(digit_filename);

      list_stream << "images/" << filefunc::getbasename(digit_filename) 
                  << endl;
      edgelist_stream << i << "  " << i << "  " << 0 << endl;
      labels_stream << i << "  " << labels[i] << endl;

   } // loop over index i labeling digit thumbnail

   filefunc::closefile(tsne_binary_filename, tsne_binary_stream);
   filefunc::closefile(list_filename, list_stream);
   filefunc::closefile(edgelist_filename, edgelist_stream);
   filefunc::closefile(labels_filename, labels_stream);

   string banner="Exported tsne binary data file to "+tsne_binary_filename;
   outputfunc::write_banner(banner);

   banner="Exported MNIST digit thumbnails to "+training_digits_subdir;
   outputfunc::write_banner(banner);

   banner="Exported MNIST image filenames "+list_filename;
   outputfunc::write_banner(banner);

   banner="Exported fake edgelist to "+edgelist_filename;
   outputfunc::write_banner(banner);

   banner="Exported image labels to "+labels_filename;
   outputfunc::write_banner(banner);
}

