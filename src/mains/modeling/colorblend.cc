// ==========================================================================
// Program COLORBLEND queries the user to enter building, polyhedron
// and rectangular side-face IDs.  It then searches the
// rectified_views subdirectory for any rectified "decal" images which
// project onto the specified rectangle.  COLORBLEND also extracts
// normal weight information encoded into each decals filename.  It
// forms a composite mosaic from all rectified decals and exports it
// to a single output image.

//	 			colorblend

// ==========================================================================
// Last updated on 4/23/12; 4/24/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/twovector.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   int building_ID,polyhedron_ID,rectangle_ID;
   cout << "Enter building ID:" << endl;
   cin >> building_ID;
   cout << "Enter polyhedron ID:" << endl;
   cin >> polyhedron_ID;
   cout << "Enter rectangle ID:" << endl;
   cin >> rectangle_ID;
   string bldg_polyhedron_rectangle_IDs=
      stringfunc::number_to_string(building_ID)+"_"+
      stringfunc::number_to_string(polyhedron_ID)+"_"+
      stringfunc::number_to_string(rectangle_ID)+"_";

   string rectified_subdir="./rectified_views/";
   vector<string> image_filenames=filefunc::files_in_subdir_matching_substring(
      rectified_subdir,bldg_polyhedron_rectangle_IDs);

// Import all rectified decals matching specified building rectangle
// side face:

   vector<double> weights;
   vector<texture_rectangle*> texture_rectangle_ptrs;
   for (int i=0; i<image_filenames.size(); i++)
   {
      cout << "i = " << i
           << " image filename = " << image_filenames[i] << endl;

      string separator_chars="_";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         image_filenames[i],separator_chars);
      double weight=0.01*stringfunc::string_to_number(substrings[4]);
      weight = sqr(weight);
      cout << "weight = " << weight << endl;
      weights.push_back(weight);

      texture_rectangle* texture_rectangle_ptr=new
         texture_rectangle(image_filenames[i],NULL);
      texture_rectangle_ptrs.push_back(texture_rectangle_ptr);
   }

   int width,height;
   imagefunc::get_image_width_height(
      image_filenames.back(),width,height);

   string colorblend_filename="colorblend.jpg";
   texture_rectangle* avg_texture_rectangle_ptr=new
      texture_rectangle(colorblend_filename,NULL);
   avg_texture_rectangle_ptr->generate_blank_image_file(
      width,height,colorblend_filename,0);

// Set pixel colors within composite to weighted average of input
// decal RGB values:

   int R,G,B;
   for (int px=0; px<width; px++)
   {
      for (int py=0; py<height; py++)
      {
         double Rnumer=0;
         double Gnumer=0;
         double Bnumer=0;
         double denom=0;
         for (int i=0; i<texture_rectangle_ptrs.size(); i++)
         {
            texture_rectangle* texture_rectangle_ptr=texture_rectangle_ptrs[i];
            texture_rectangle_ptr->get_pixel_RGB_values(
               px,py,R,G,B);

            double weight=weights[i];

// Ignore any "black" pixels which we assume indicate missing data:

//            const int min_value=5;
            const int min_value=7;
            if (R < min_value && G < min_value && B < min_value) weight=0;


            Rnumer += weight*R;
            Gnumer += weight*G;
            Bnumer += weight*B;
            denom += weight;
         } // loop over index i labeling texture rectangles

         if (!nearly_equal(denom,0))
         {
            int Ravg=Rnumer/denom;
            int Gavg=Gnumer/denom;
            int Bavg=Bnumer/denom;
            avg_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,Ravg,Gavg,Bavg);
         }
         
      } // loop over py index
   } // loop over px index
   
   avg_texture_rectangle_ptr->write_curr_frame(colorblend_filename);

   string colorblends_subdir=rectified_subdir+"colorblend/";
   filefunc::dircreate(colorblends_subdir);
   string final_colorblend_filename=colorblends_subdir+
      bldg_polyhedron_rectangle_IDs+"blend.jpg";
   string unix_cmd="mv "+colorblend_filename+" "+final_colorblend_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported blended file "+final_colorblend_filename;
   outputfunc::write_big_banner(banner);
}
