// ==========================================================================
// Program EXTREMAL_REGIONS

//				./extremal_regions

// ==========================================================================
// Last updated on 9/27/13; 10/1/13; 10/2/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "image/binaryimagefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::ofstream;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   string image_filename="kermit000.jpg";
//   string image_filename="mini_binary.jpg";
//   string image_filename="wiki_binary.jpg";

/*
   connected_components* connected_components_ptr=
      new connected_components();
   int color_channel_ID=-2;	// luminosity
   connected_components_ptr->reset_image(image_filename,color_channel_ID);

   int threshold=128;
   cout << "Enter threshold:" << endl;
   cin >> threshold;
   
   bool invert_binary_values_flag=false;
   bool export_connected_regions_flag=true;
   int n_components=connected_components_ptr->compute_connected_components(
      threshold,invert_binary_values_flag,export_connected_regions_flag);
   cout << "n_connected_components = " << n_components << endl;
   exit(-1);
*/

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->import_photo_from_file(image_filename);
   texture_rectangle_ptr->convert_color_image_to_greyscale();

   twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();
   twoDarray* pbinary_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
   twoDarray* cc_labels_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);

   double p_threshold=128;
   cout << "Enter p_threshold:" << endl;
   cin >> p_threshold;

   binaryimagefunc::binary_threshold(
      p_threshold,ptwoDarray_ptr,pbinary_twoDarray_ptr);

   string output_filename="greyscale.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);

   int n_channels=3;
   texture_rectangle_ptr->initialize_twoDarray_image(
      pbinary_twoDarray_ptr,n_channels);
   output_filename="binary.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);

//   int n_neighbors=4;
   int n_neighbors=8;
//   cout << "Enter n_neighbors:" << endl;
//   cin >> n_neighbors;
   
   int label_offset=0;
   double z_null=0;
   int n_components=graphicsfunc::Label_Connected_Components(
      n_neighbors,label_offset,z_null,
      pbinary_twoDarray_ptr,cc_labels_twoDarray_ptr);
   cout << "From Label_Connected_Components, n_components = " 
        << n_components << endl;

   n_components=graphicsfunc::twopass_cc_labeling(
      n_neighbors,label_offset,z_null,
      pbinary_twoDarray_ptr,cc_labels_twoDarray_ptr);
   
   cout << "From twopass_cc_labeling, n_components = " 
        << n_components << endl;


   delete cc_labels_twoDarray_ptr;
   delete pbinary_twoDarray_ptr;
   delete texture_rectangle_ptr;
//   delete connected_components_ptr;
}

