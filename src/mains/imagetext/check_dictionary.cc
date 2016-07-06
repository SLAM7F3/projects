// ==========================================================================
// CHECK_DICTIONARY imports the best trained elements within the
// dictionary exported by program DICTIONARY.  It generates a PNG
// composite that illustrates the Kbest 8x8 dictionary elements.

// 			    ./check_dictionary

// ==========================================================================
// Last updated on 5/26/14; 6/8/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

double **mat_alloc(int n, int m)
{
   int i, j;
   double **ar;

   ar = (double **)malloc(((n + 1) & ~1) * sizeof(double *) + n * m * sizeof(double));
   ar[0] = (double *)(ar + ((n + 1) & ~1));

   for(i = 0; i < n; i++){
      ar[i] = ar[0] + i * m;
      for(j = 0; j < m; j++){
         ar[i][j] = 0.0;
      }
   }
   return(ar);
}

void mat_free(double **a1)
{
   free(a1);
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string dictionary_subdir="./training_data/dictionary/";

// Import dictionary output by program DICTIONARY:

   flann::Matrix<float> dictionary_elements;
   string dictionary_hdf5_filename=dictionary_subdir+"dictionary.hdf5";
   flann::load_from_file(
      dictionary_elements,dictionary_hdf5_filename.c_str(),
      "dictionary_descriptors");

   unsigned int D=dictionary_elements.rows;
   unsigned int K=dictionary_elements.cols;
   genvector curr_Dcol(D);
   cout << "D = " << D << endl;
   cout << "K = " << K << endl;

/*

// On 6/8/14 we experimented with exporting 96K dictionary elements to
// an output C file and importing them again for pwin purposes.  This
// approach is HORRIBLY SLOW and totally impractical.  We will have to
// find some other way to import large dictionaries within pwin!

// Export current dictionary elements to C commands for pwin purposes:

   string dictionary_elements_filename="dictionary_elements.c";   
   ofstream outstream;
   filefunc::openfile(dictionary_elements_filename,outstream);
   int counter=0;
   for (unsigned int k=0; k<K; k++)
   {
      for (unsigned int d=0; d<D; d++)
      {
//         outstream << "D[" << counter++ << "]="
//                   << dictionary_elements[d][k] << "; " << flush;
         outstream << "D[" << d << "][" << k << "] = "
                   << dictionary_elements[d][k] << "; " << flush;
         if (d%4 == 3) outstream << endl;
      }
   }

   filefunc::closefile(dictionary_elements_filename,outstream);
   string banner="Exported dictionary to C format for pwin purposes to "+
      dictionary_elements_filename;
   outputfunc::write_banner(banner);

   exit(-1);
*/

   int n_patches_per_row = 30;
   int total_width=n_patches_per_row * 9;
   int total_height=K/(n_patches_per_row) * 9;
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      total_width,total_height,1,3,NULL);
   string blank_filename="blank.jpg";
   texture_rectangle_ptr->generate_blank_image_file(
      total_width,total_height,blank_filename,0.5);
   texture_rectangle_ptr->import_photo_from_file(blank_filename);

// Export PNG image containing current 64-dimensional dictionary
// elements visualized as 8x8 patches:

   texture_rectangle_ptr->clear_all_RGB_values();
   
   for (unsigned int k=0; k<K; k++)
   {
      for (unsigned int d=0; d<D; d++)
      {
         curr_Dcol.put(d,dictionary_elements[d][k]);
      }

// For visualization purposes only, compute min and max intensity
// values. Then rescale intensities so that extremal values equal +/- 1:

      double min_z=POSITIVEINFINITY;
      double max_z=NEGATIVEINFINITY;
      for (unsigned int d=0; d<D; d++)
      {
         double curr_z=curr_Dcol.get(d);
         min_z=basic_math::min(min_z,curr_z);
         max_z=basic_math::max(max_z,curr_z);
      }
         
      for (unsigned int d=0; d<D; d++)
      {
         double curr_z=curr_Dcol.get(d);
         curr_z=2*(curr_z-min_z)/(max_z-min_z)-1;
         curr_Dcol.put(d,curr_z);
      }

      int row=k/n_patches_per_row;
      int column=k%n_patches_per_row;

      int px_start=column*9;
      int py_start=row*9;
//         cout << "row = " << row << " column = " << column 
//              << " px_start = " << px_start 
//              << " py_start = " << py_start << endl;

      int counter=0;
      for (unsigned int pu=0; pu<8; pu++)
      {
         int px=px_start+pu;
         for (unsigned int pv=0; pv<8; pv++)
         {
            int py=py_start+pv;
            double z=curr_Dcol.get(counter);
            counter++;
            int R=0.5*(z+1)*255;
            texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,R,R);
         } // loop over pv index
      } // loop over pu index

   } // loop over index k labeling dictionary elements

// Export current dictionary to PNG image file:

   string output_filename=dictionary_subdir+"dictionary_elements.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);
   cout << "Exported "+output_filename << endl;
   


   delete texture_rectangle_ptr;
}

   


