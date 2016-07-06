// ==========================================================================
// Program WISP_CARDINAL_DIRS annotates a de-warped & georegistered
// Deer Island pano with east, north, west and south directions.

//			      ./wisp_cardinal_dirs

// ==========================================================================
// Last updated on 3/28/13; 4/25/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "image/imagefuncs.h"
#include "image/pngfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(16);

// Import UV corrected frame 0 from Deer Island data collection:

   string bundler_IO_subdir="./bundler/DIME/Feb2013_DeerIsland/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string images_subdir=bundler_IO_subdir+"images/";
   string wisp_frame_filename=images_subdir+
      "stable_subsampled_00000.jpg";
//      "stable_uvcorrected_wisp_res0_00000.jpg";
   
   texture_rectangle* grey_texture_rectangle_ptr=
      new texture_rectangle(wisp_frame_filename,NULL);
   texture_rectangle* texture_rectangle_ptr=grey_texture_rectangle_ptr->
      generate_RGB_from_grey_texture_rectangle();
   delete grey_texture_rectangle_ptr;

// Load 4 cardinal directions into STL vector az:

   vector<double> az;
   az.push_back(0);
   az.push_back(PI/2);
   az.push_back(PI);
   az.push_back(3*PI/2);

   vector<string> cardinal_dir_label;
   cardinal_dir_label.push_back("East");
   cardinal_dir_label.push_back("North");
   cardinal_dir_label.push_back("West");
   cardinal_dir_label.push_back("South");

   unsigned int pano_width,pano_height;
   imagefunc::get_image_width_height(
      wisp_frame_filename,pano_width,pano_height);
   cout << "pano_width = " << pano_width 
        << " pano_height = " << pano_height << endl;

   double Umax=double(pano_width)/double(pano_height); // = 18.18182
   double IFOV=2*PI/Umax;	// = 0.34557
   double delta_az=-60.868*PI/180; // -60.8 =  -78.77 + 36/2

   vector<int> cardinal_px;
   for (unsigned int a=0; a<az.size(); a++)
   {
      double wisp_az=az[a]-delta_az;

      double U=(2*PI-wisp_az)/IFOV;
      if (U > Umax) U -= Umax;
      if (U < 0) U += Umax;
      int px=basic_math::round(U*pano_height);
      cout << "U = " << U << " px = " << px << endl;
      cardinal_px.push_back(px);

// EAST:  az = 0 degs; 		U = 15.10767; 	px = 33237
// NORTH: az = 90 degs;		U = 10.56222;	px = 23237
// WEST:  az = 180 degs;	U = 6.016767;	px = 13237
// SOUTH: az = 270 degs;	U = 1.471313;	px = 03237

      int R=255;
      int G=0;
      int B=0;
      for (unsigned int py=0; py<pano_height; py++)
      {
         texture_rectangle_ptr->set_pixel_RGB_values(px-2,py,R,G,B);
         texture_rectangle_ptr->set_pixel_RGB_values(px-1,py,R,G,B);
         texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         texture_rectangle_ptr->set_pixel_RGB_values(px+1,py,R,G,B);
         texture_rectangle_ptr->set_pixel_RGB_values(px+2,py,R,G,B);
      }
   } // loop over index a labeling cardinal directions

   string cardinal_dirs_filename="WISP_cardinal_dirs.png";
   texture_rectangle_ptr->write_curr_frame(cardinal_dirs_filename);
   delete texture_rectangle_ptr;

   for (unsigned int a=0; a<cardinal_dir_label.size(); a++)
   {
      int px_start=cardinal_px[a];
      int py_start=pano_height/2;
      
      double angle=PI/2;
      colorfunc::RGB text_rgb(1,0,0);
      string font_path=
         "/home/cho/programs/c++/svn/projects/data/OpenSceneGraph-Data/fonts/arial.ttf";
      int fontsize=50;

      string next_cardinal_dirs_filename="./dummy.png";
      pngfunc::add_text_to_PNG_image(
         cardinal_dirs_filename,next_cardinal_dirs_filename,
         px_start,py_start,angle,
         cardinal_dir_label[a],text_rgb,font_path,fontsize);

      string unix_cmd="mv "+next_cardinal_dirs_filename+" "+
         cardinal_dirs_filename;
      sysfunc::unix_command(unix_cmd);

   } // loop over index a labeling cardinal directions

   string banner="Exported WISP frame annotated with cardinal directions to "+
      cardinal_dirs_filename;
   outputfunc::write_big_banner(banner);

} 

