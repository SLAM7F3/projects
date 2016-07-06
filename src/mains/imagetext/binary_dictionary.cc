// ==========================================================================
// BINARY_DICTIONARY imports the best trained elements within the
// dictionary exported by program DICTIONARY from an HDF5 file.  It
// calls fwrite to export the dictionary matrix D rows and K columns
// to a flat binary file which could be imported via Pwin C code.


// 			    ./binary_dictionary

// ==========================================================================
// Last updated on 6/13/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "general/outputfuncs.h"
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


float **mat_alloc(int n, int m)
{
   int i, j;
   float **ar;

   ar = (float **)malloc(((n + 1) & ~1) * sizeof(float *) + n * m * sizeof(float));
   ar[0] = (float *)(ar + ((n + 1) & ~1));

   for(i = 0; i < n; i++){
      ar[i] = ar[0] + i * m;
      for(j = 0; j < m; j++){
         ar[i][j] = 0.0;
      }
   }
   return(ar);
}

void mat_free(float **a1)
{
   free(a1);
}


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string dictionary_subdir="./training_data/dictionary/";

   const int MAX_PATH=256;
   char dictionary_filename[MAX_PATH];
//   sprintf(dictionary_filename,"./dictionary.bin");
   sprintf(dictionary_filename,"./training_data/dictionary/dictionary.bin");
   cout << "dictionary_filename = " << dictionary_filename << endl;
   unsigned int D, K;

/*
// Import dictionary output by program DICTIONARY:

   flann::Matrix<float> dictionary_elements;
   string dictionary_hdf5_filename=dictionary_subdir+"dictionary.hdf5";
   flann::load_from_file(
      dictionary_elements,dictionary_hdf5_filename.c_str(),
      "dictionary_descriptors");

   D=dictionary_elements.rows;
   K=dictionary_elements.cols;
   cout << "D = " << D << endl;
   cout << "K = " << K << endl;

// Export dictionary to flat binary file output:

   FILE *fp = fopen(dictionary_filename, "wb");
   fwrite(&D, sizeof(unsigned int), 1, fp);
   fwrite(&K, sizeof(unsigned int), 1, fp);
   for (unsigned int d = 0; d < D; d++){
     for (unsigned int k = 0; k < K; k++){
        float curr_val = dictionary_elements[d][k];
        fwrite(&curr_val, sizeof(float), 1, fp);
     }
   }
   fclose(fp);
*/

// Import dictionary from binary file:

   FILE *fp2 = fopen(dictionary_filename, "rb");
   fread(&D, sizeof(unsigned int), 1, fp2);
   cout << "D = " << D << endl;
   fread(&K, sizeof(unsigned int), 1, fp2);
   cout << "K = " << K << endl;
   float **dictionary_matrix = mat_alloc(D,K);
   for (unsigned int d = 0; d < D; d++){
     for (unsigned int k = 0; k < K; k++){
        float curr_val;
        fread(&curr_val, sizeof(float), 1, fp2);
        dictionary_matrix[d][k] = curr_val;
     }
   }
   fclose(fp2);

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
   
   genvector curr_Dcol(D);
   for (unsigned int k=0; k<K; k++)
   {
//      cout << "k = " << k << endl;
      for (unsigned int d=0; d<D; d++)
      {
//	curr_Dcol.put(d,dictionary_elements[d][k]);
        curr_Dcol.put(d,dictionary_matrix[d][k]);
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
//      cout << "row = " << row << " column = " << column 
//           << " px_start = " << px_start 
//           << " py_start = " << py_start << endl;

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
   mat_free(dictionary_matrix);
}

   


