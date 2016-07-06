// ==========================================================================
// Program DESCRIPTORS loops over all renormalized character
// image files within ./training_data/renorm_chars.  It extracts
// n_patches_per_image 8x8 pixel patches from each image.  It
// converts the 8x8 patches into a 64x1 column vector of grey-scale
// intensities.  Raw descriptors for all character image patches are
// exported to a single HDF5 binary file.  Descriptor mean and
// covariance are also recursively calculated and written to output text
// files.

//				descriptors

// ==========================================================================
// Last updated on 11/24/12; 5/31/13; 4/6/14; 6/17/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "cluster/akm.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "video/sift_detector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

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
//   allowed_suffixes.push_back("jpg");
//   string renorm_subdir="./training_data/char_intensity/extremal_regions/all_symbols_extremal_regions/";
//   string renorm_subdir="./training_data/synthetic_Ng_chars_July/";
   string renorm_subdir="./training_data/synthetic_chars/";
//   string renorm_subdir="./training_data/renorm_chars/";
   
   string dictionary_subdir="./training_data/dictionary/";
   filefunc::dircreate(dictionary_subdir);
   bool search_all_children_dirs_flag=false;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,renorm_subdir,search_all_children_dirs_flag);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);

   const int n_patches_per_image=5;
//   const int n_patches_per_image=15;
   const int D=64;

   descriptor* recur_mean_ptr=new descriptor(D);
   recur_mean_ptr->clear_values();
   genmatrix* recur_second_moment_ptr=new genmatrix(D,D);
   recur_second_moment_ptr->clear_values();
   genmatrix* outerproduct_ptr=new genmatrix(D,D);
   descriptor curr_X(D);
   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;

   int descriptor_counter=0;
   int max_n_descriptors = 100;
   int R,G,B;
   double h,s,v;
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      texture_rectangle_ptr->import_photo_from_file(image_filenames[i]);
      unsigned int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);
      if (width < 8) continue;

      cout << "i = " << i 
           << " image_filename = " << image_filenames[i]
           << " xdim = " << width << " ydim = " << height
           << endl;

      for (int p=0; p<n_patches_per_image; p++)
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
               colorfunc::RGB_to_hsv(r,g,b,h,s,v);
               curr_X_ptr->put(d,v);
               d++;
            } // loop over py
         } // loop over px

         mathfunc::recursive_mean(
            D,descriptor_counter,curr_X_ptr,recur_mean_ptr);
         mathfunc::recursive_2nd_moment(
            D,descriptor_counter,curr_X_ptr,outerproduct_ptr,
            recur_second_moment_ptr);

         descriptor_counter++;

      } // loop over index p labeling patches within current image

      cout << "descriptor_counter = " << descriptor_counter << endl;
      if (descriptor_counter > max_n_descriptors) break;
      
   } // loop over index i labeling all char image files
  
   delete texture_rectangle_ptr;

// Export patch descriptors for entire imagery database to single
// binary file:

   string output_filename=dictionary_subdir+"raw_patch_features.hdf5";
   sift_detector_ptr->get_akm_ptr()->export_all_SIFT_features(
      output_filename,"patch_descriptors",D_ptrs_ptr);
   delete sift_detector_ptr;

// Compute covariance matrix from recursively calculated first and
// second moments for all descriptors:

   genmatrix recur_mean_outerprod(D,D);
   recur_mean_ptr->outerproduct(*recur_mean_ptr,recur_mean_outerprod);
//   cout << "recur_mean_outerprod = " << recur_mean_outerprod << endl;

   genmatrix* recur_covar_ptr=new genmatrix(D,D);
   *recur_covar_ptr = *recur_second_moment_ptr - recur_mean_outerprod;
//   cout << "*recur_covar_ptr = " << *recur_covar_ptr << endl;

   genmatrix* covar_sqrt_ptr=new genmatrix(D,D);
   recur_covar_ptr->square_root(*covar_sqrt_ptr);
   delete recur_covar_ptr;

   genmatrix* inverse_covar_sqrt_ptr=new genmatrix(D,D);
   covar_sqrt_ptr->inverse(*inverse_covar_sqrt_ptr);

// Export inverse square root of covariance matrix to a text file so
// that it does not need to be computed more than once:

   string mean_filename=dictionary_subdir+"mean_vector.dat";
   string covar_sqrt_filename=
      dictionary_subdir+"sqrt_covar_matrix.dat";
   string inverse_covar_sqrt_filename=
      dictionary_subdir+"inverse_sqrt_covar_matrix.dat";
   ofstream meanstream,outstream,inverse_outstream;
   filefunc::openfile(mean_filename,meanstream);
   filefunc::openfile(covar_sqrt_filename,outstream);
   filefunc::openfile(inverse_covar_sqrt_filename,inverse_outstream);

   double avg_recur_mean=0;
   for (int i=0; i<D; i++)
   {
      meanstream << recur_mean_ptr->get(i) << endl;
      avg_recur_mean += recur_mean_ptr->get(i);
      
      for (int j=0; j<D; j++)
      {
         outstream << covar_sqrt_ptr->get(i,j) << endl;
         inverse_outstream << inverse_covar_sqrt_ptr->get(i,j) << endl;
      }
   }
   filefunc::closefile(mean_filename,outstream);   
   filefunc::closefile(covar_sqrt_filename,outstream);   
   filefunc::closefile(inverse_covar_sqrt_filename,inverse_outstream);   

   avg_recur_mean /= D;
   cout << "<recur_mean> = " << avg_recur_mean << endl;

   string banner="Exported mean vector, sqrt and inverse sqrt covar matrices ";
   banner += "to "+dictionary_subdir;
   outputfunc::write_big_banner(banner);
}

