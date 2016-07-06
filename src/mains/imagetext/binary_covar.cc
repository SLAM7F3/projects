// ==========================================================================
// BINARY_COVAR exports Dtrans_inverse_sqrt_covar matrix which has K rows
// and D columns to a flat binary file which can be imported via Pwin C code.

// 			    ./binary_covar

// ==========================================================================
// Last updated on 6/23/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"

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

   string testing_images_subdir="./testing_data/";
   string dictionary_subdir="./training_data/dictionary/";
   bool RGB_pixels_flag=false;
   text_detector* text_detector_ptr=new text_detector(
      dictionary_subdir,RGB_pixels_flag);

   text_detector_ptr->import_inverse_sqrt_covar_matrix();
   text_detector_ptr->compute_Dtrans_inverse_sqrt_covar_matrix();

   unsigned int D=text_detector_ptr->get_D();
   unsigned int K=text_detector_ptr->get_K();
   cout << "D = " << D << " K = " << K << endl;

   for (unsigned int k = 0; k < 1; k++)
   {
     for (unsigned int d = 0; d < D; d++)
     {
        cout << "k = " << k << " d = " << d 
             << " dict = " << text_detector_ptr->get_Dictionary_value(d,k)
             << endl;
     }
   }


   exit(-1);
      
   const int MAX_PATH=256;
   char Dtrans_filename[MAX_PATH];
   sprintf(Dtrans_filename,"./training_data/dictionary/Dtrans_inv_root_covar.bin");
   cout << "Dtrans_filename = " << Dtrans_filename << endl;

/*
// Export Dtrans_inverse_sqrt_covar (K x D matrix) to flat binary file output:

   FILE *fp = fopen(Dtrans_filename, "wb");
   fwrite(&K, sizeof(unsigned int), 1, fp);
   fwrite(&D, sizeof(unsigned int), 1, fp);
   for (unsigned int k = 0; k < K; k++)
   {
     for (unsigned int d = 0; d < D; d++)
     {
       float curr_val = text_detector_ptr->get_Dtrans_inverse_sqrt_covar(k,d);
        fwrite(&curr_val, sizeof(float), 1, fp);
     }
   }
   fclose(fp);

   string banner="Exported "+string(Dtrans_filename);
   outputfunc::write_banner(banner);
*/

// Import Dtrans_inverse_sqrt_covar matrix from binary file:

   FILE *fp2 = fopen(Dtrans_filename, "rb");
   fread(&K, sizeof(unsigned int), 1, fp2);
   cout << "K = " << K << endl;
   fread(&D, sizeof(unsigned int), 1, fp2);
   cout << "D = " << D << endl;
   float **Dtrans_matrix = mat_alloc(K,D);
   for (unsigned int k = 0; k < K; k++)
   {
     for (unsigned int d = 0; d < D; d++)
     {
        float curr_val;
        fread(&curr_val, sizeof(float), 1, fp2);
        Dtrans_matrix[k][d] = curr_val;
	float truth_val = text_detector_ptr->get_Dtrans_inverse_sqrt_covar(k,d);
	cout << "k = " << k << " d = " << d 
	     << " truth-curr = " << truth_val - curr_val << endl;
     }
   }
   fclose(fp2);

   delete text_detector_ptr;
   mat_free(Dtrans_matrix);
}

   


