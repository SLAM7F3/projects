// ==========================================================================
// Program DT is a testing playpen for our wrappers of Felzenszwalb's
// distance transform codes.
// ==========================================================================
// Last updated on 8/9/12; 8/10/12
// ==========================================================================

#include <iostream>
#include <string>
#include "distance_transform/imconv.h"
#include "distance_transform/pnmfile.h"
#include "distance_transform/dt.h"

#include "image/binaryimagefuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

#include "distance_transform/dtfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string image_filename="input_binary.png";
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->import_photo_from_file(image_filename);

   int n_channels=texture_rectangle_ptr->getNchannels();
   cout << "n_channels = " << n_channels << endl;

   texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
   texture_rectangle_ptr->fill_ptwoDarray_from_single_channel_byte_data();
   twoDarray* ptwoDarray_ptr=
      texture_rectangle_ptr->get_ptwoDarray_ptr();
   int mdim=ptwoDarray_ptr->get_mdim();
   int ndim=ptwoDarray_ptr->get_ndim();



   twoDarray* pbinary_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
   double threshold=128;
   double znull=0;
   double zfill=1;
//   double zfill=255;
   binaryimagefunc::binary_threshold(
      threshold,ptwoDarray_ptr,pbinary_twoDarray_ptr,znull,zfill);
   
/*
   texture_rectangle_ptr->convert_single_twoDarray_to_three_channels(
      pbinary_twoDarray_ptr,true);
   string binary_filename="binary.png";
   texture_rectangle_ptr->write_curr_frame(binary_filename);
*/


   twoDarray* qbinary_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);
   qbinary_twoDarray_ptr->clear_values();

   int x_offset,y_offset;
   cout << "Enter x_offset:" << endl;
   cin >> x_offset;
   cout << "Enter y_offset:" << endl;
   cin >> y_offset;
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         double curr_p=pbinary_twoDarray_ptr->get(px,py);
         if (px < mdim-x_offset && py < ndim-y_offset)
         {
            qbinary_twoDarray_ptr->put(px+x_offset,py+y_offset,curr_p);
         }
      }
   }

/*
   texture_rectangle_ptr->convert_single_twoDarray_to_three_channels(
      qbinary_twoDarray_ptr,true);
   string binary_filename="qbinary.png";
   texture_rectangle_ptr->write_curr_frame(binary_filename);
*/

   threshold=0.5;
   double max_pixel_distance;
   twoDarray* p_dt_twoDarray_ptr=dtfunc::compute_distance_transform(
      threshold,pbinary_twoDarray_ptr,max_pixel_distance);
   texture_rectangle_ptr->reset_ptwoDarray_ptr(p_dt_twoDarray_ptr);   

/*
// For display purposes only, rescale maximum distance so that it
// corresponds to 255:

   double max_distance=NEGATIVEINFINITY;
   for (int py=0; py<ndim; py++)
   {
      for (int px=0; px<mdim; px++)
      {
         max_distance=basic_math::max(max_distance,p_dt_twoDarray_ptr->get(px,py));
      }
   }
   cout << "max_distance = " << max_distance << endl;

   for (int py=0; py<ndim; py++)
   {
      for (int px=0; px<mdim; px++)
      {
         double curr_dist=p_dt_twoDarray_ptr->get(px,py);
         curr_dist *= 255/max_distance;
         p_dt_twoDarray_ptr->put(px,py,curr_dist);
      }
   }
*/

   
   texture_rectangle_ptr->convert_single_twoDarray_to_three_channels(
      p_dt_twoDarray_ptr,true);
   string distances_filename="distances.png";
   texture_rectangle_ptr->write_curr_frame(distances_filename);


   cout << "Calculating chamfer score:" << endl;
   double score=dtfunc::chamfer_matching_score(
      threshold,pbinary_twoDarray_ptr,qbinary_twoDarray_ptr);
   cout << "chamfer score = " << score << endl;

   delete texture_rectangle_ptr;

}
