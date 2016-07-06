// ==========================================================================
// Program INITIAL_DESCRIPTORS loops over all input image files within
// raw_char_subdir.  It extracts n_patches_per_image 8x8 pixel patches
// from each image.  It converts the 8x8 patches into a 192x1 [64x1] column
// vector of RGB values [grey-scale intensities].  Raw descriptors for all
// character image patches are "contrast normalized" and then exported
// to a single HDF5 binary file.  

//			   initial_descriptors

// ==========================================================================
// Last updated on 5/24/14; 6/7/14; 6/21/14; 6/22/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "video/sift_detector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("jpg");
   string trees_subdir="./trees/";
   string chips_subdir=trees_subdir+"all_32x32_tree_chips/";
   
   string dictionary_subdir=trees_subdir+"dictionary/";
   filefunc::dircreate(dictionary_subdir);
   bool search_all_children_dirs_flag=false;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,chips_subdir,search_all_children_dirs_flag);

   const unsigned int D=64*3;	// 8 x 8 RGB patches
//   const unsigned int D=64;	// 8 x 8 greyscale patches
   const unsigned int n_patches_per_image=10;

   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   unsigned int max_n_descriptors = 1000000;		
   int R,G,B;

   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      outputfunc::update_progress_fraction(i, 1000, image_filenames.size());
      texture_rectangle_ptr->import_photo_from_file(image_filenames[i]);
      unsigned int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);
      if (width < 8) continue;

//      cout << "i = " << i 
//           << " image_filename = " << image_filenames[i]
//           << " xdim = " << width << " ydim = " << height
//           << endl;

      for (unsigned int p=0; p<n_patches_per_image; p++)
      {
         int px_start=nrfunc::ran1()*(width-8);
         int py_start=nrfunc::ran1()*(height-8);

         descriptor* curr_X_ptr=new descriptor(D);
         D_ptrs_ptr->push_back(curr_X_ptr);

         int d=0;         
         for (int px=px_start; px<px_start+8; px++)
         {
            for (int py=py_start; py<py_start+8; py++)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
               double r=R/255.0;
               double g=G/255.0;
               double b=B/255.0;
               curr_X_ptr->put(d,r);
               curr_X_ptr->put(d+1,g);
               curr_X_ptr->put(d+2,b);
               d += 3;

// 	         double h,s,v;
//               colorfunc::RGB_to_hsv(r,g,b,h,s,v);
//               curr_X_ptr->put(d,v);
//               d++;

            } // loop over py
         } // loop over px

         const float eps_norm = 10.0/sqr(255.0);
         curr_X_ptr->contrast_normalize(eps_norm);

// Check descriptor's mean and std_dev after contrast normalization:

//         float mu, sigma;
//         curr_X_ptr->mean_and_std_dev(mu, sigma);
//         cout << "mu = " << mu << " sigma = " << sigma << endl;

      } // loop over index p labeling patches within current image

      if (D_ptrs_ptr->size() > max_n_descriptors) break;
      
   } // loop over index i labeling all char image files
  
   delete texture_rectangle_ptr;

   unsigned int N=D_ptrs_ptr->size();
   cout << "N = " << N << endl;
   cout << "D = " << D << endl;
   flann::Matrix<float> *descriptors_matrix_ptr
      =new flann::Matrix<float>(new float[N*D],N,D);

   for (unsigned int f=0; f<N; f++)
   {
      descriptor* curr_D_ptr=D_ptrs_ptr->at(f);
      for (unsigned int d=0; d<D; d++)
      {
         (*descriptors_matrix_ptr)[f][d]=curr_D_ptr->get(d);
      } // loop over index d
   } // loop over index f labeling descriptors

// Export contrast normalized patch descriptors for entire imagery
// database to single binary file:

   string output_filename=dictionary_subdir+
      "contrast_normalized_patch_descriptors.hdf5";
   filefunc::deletefile(output_filename);
   string HDF5_label="contrast_normalized_patch_descriptors";
   flann::save_to_file(*descriptors_matrix_ptr,output_filename.c_str(),
                       HDF5_label.c_str());
   string banner="Exported "+stringfunc::number_to_string(N)
      +" contrast normalized patch descriptors to "+output_filename;
   outputfunc::write_big_banner(banner);

   delete [] descriptors_matrix_ptr->ptr();
   delete descriptors_matrix_ptr;
}

