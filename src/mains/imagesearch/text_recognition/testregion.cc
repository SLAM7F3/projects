// ==========================================================================
// TESTREGION

// 				testregion


// ==========================================================================
// Last updated on 7/1/12
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

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   int width=5;
   int height=5;
//   int width=31;
//   int height=31;
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);

   string blank_filename="blank.jpg";
   texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,0.0);

   texture_rectangle_ptr->set_pixel_RGB_values(0,0,252,252,252);
   texture_rectangle_ptr->set_pixel_RGB_values(1,0,250,250,250);
//   texture_rectangle_ptr->set_pixel_RGB_values(2,0,253,253,253);
//   texture_rectangle_ptr->set_pixel_RGB_values(1,1,255,255,255);
//   texture_rectangle_ptr->set_pixel_RGB_values(1,0,251,251,251);

//   texture_rectangle_ptr->set_pixel_RGB_values(1,1,249,249,249);



//   texture_rectangle_ptr->set_pixel_RGB_values(0,1,255,255,255);
//   texture_rectangle_ptr->set_pixel_RGB_values(1,0,253,0,0);

   string output_filename="sim_image.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);

   delete texture_rectangle_ptr;

}

   
