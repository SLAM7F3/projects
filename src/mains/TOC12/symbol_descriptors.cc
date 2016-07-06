// ==========================================================================
// Program SYMBOL_DESCRIPTORS loops over all renormalized greyscale or
// RGB character image files depending upon RGB_pixels_flag.
// It extracts n_patches_per_image 8x8 pixel patches from each image.
// It converts the 8x8 patches into a 64x3 [64] column vector of RGB
// color [greyscale] values. Raw descriptors for all character image
// patches are exported to a single HDF5 binary file.  Descriptor mean
// and covariance are also recursively calculated and written to
// output text files.

//				symbol_descriptors

// ==========================================================================
// Last updated on 9/29/12; 10/5/12; 10/20/12; 5/31/13; 4/6/14
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

   vector<string> symbol_names;
   symbol_names.push_back("yellow_radiation");
   symbol_names.push_back("orange_biohazard");
//   symbol_names.push_back("blue_radiation");
//   symbol_names.push_back("blue_water");
//   symbol_names.push_back("blue_gas");
//   symbol_names.push_back("red_stop");
//   symbol_names.push_back("green_start");
//   symbol_names.push_back("bw_skull");
//   symbol_names.push_back("bw_eat");

   string final_signs_subdir="./images/final_signs/";
//   string ppt_signs_subdir="./images/ppt_signs/";
   string symbols_input_subdir=final_signs_subdir;

   string symbol_name;
   cout << "Enter symbol name:" << endl;
   cout << "  yellow_radiation,orange_biohazard,blue_radiation" << endl;
   cout << "  blue_water,blue_gas,red_stop" << endl;
   cout << "  green_start,bw_skull,bw_eat:" << endl;
   cin >> symbol_name;

   bool RGB_pixels_flag=false;
//   bool RGB_pixels_flag=true;
   cout << "RGB_pixels_flag = " << RGB_pixels_flag << endl;

   int D=64*3;	// RGB color
   if (!RGB_pixels_flag)
   {
      D=64;	// greyscale
   }

   string symbol_filename=symbols_input_subdir+symbol_name+".png";
   string synthetic_subdir=symbols_input_subdir+"synthetic_symbols/";
   string synthetic_symbols_subdir=synthetic_subdir+symbol_name+"/";
   string dictionary_subdir=synthetic_symbols_subdir;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
//   allowed_suffixes.push_back("jpg");
   bool search_all_children_dirs_flag=false;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,synthetic_symbols_subdir,
         search_all_children_dirs_flag);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   bool FLANN_flag=true;
   sift_detector* sift_detector_ptr=new sift_detector(NULL,FLANN_flag);


   const int n_patches_per_image=15;
   
   descriptor* recur_mean_ptr=new descriptor(D);
   recur_mean_ptr->clear_values();

//   flann::Matrix<float>* recur_second_moment_ptr=
//      new flann::Matrix<float>(new float[D*D],D,D);

   genmatrix* recur_second_moment_ptr=new genmatrix(D,D);
   recur_second_moment_ptr->clear_values();

   descriptor curr_X(D);
   vector<descriptor*>* D_ptrs_ptr=new vector<descriptor*>;

   int counter=0;
   int R,G,B;
   int n_images=image_filenames.size();
   for (int i=0; i<n_images; i++)
   {
      texture_rectangle_ptr->import_photo_from_file(image_filenames[i]);
      unsigned int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);
      if (width < 8) continue;

      cout << "i = " << i 
           << " image_filename = " << image_filenames[i]
           << " xdim = " << width << " ydim = " << height
           << endl;

// Conversion from RGB color to grey-scale is TOC12 sign dependent!
// On 10/5/12, we empirically found no difference in the greyscale and
// luminosity versoins of the black-and-white skull and eat signs.

      if (!RGB_pixels_flag)
      {
         if (symbol_name=="yellow_radiation" ||
         symbol_name=="orange_biohazard")
         {
            texture_rectangle_ptr->
               convert_color_image_to_greyscale(); 
         }
         else if (symbol_name=="blue_radiation")
         {
            texture_rectangle_ptr->
               convert_color_image_to_single_color_channel(1,true);  
							// red channel
         }
         else
         {
            texture_rectangle_ptr->
               convert_color_image_to_luminosity();
         }
      } // !RGB_pixels_flag conditional

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

               if (RGB_pixels_flag)
               {
                  curr_X_ptr->put(d,r);
                  curr_X_ptr->put(d+1,g);
                  curr_X_ptr->put(d+2,b);
                  d += 3;
               }
               else
               {
                  curr_X_ptr->put(d,r);
                  d++;
               }
            } // loop over py
         } // loop over px

         mathfunc::recursive_mean(D,counter,curr_X_ptr,recur_mean_ptr);
         mathfunc::recursive_2nd_moment(
            counter,curr_X_ptr,recur_second_moment_ptr);
         counter++;
      } // loop over index p labeling patches within current image
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
   delete recur_second_moment_ptr;

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

