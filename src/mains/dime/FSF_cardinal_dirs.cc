// ==========================================================================
// Program FSF_CARDINAL_DIRS annotates stabilized & georegistered
// FSF panos with east, north, west and south directions.

//			      ./FSF_cardinal_dirs

// ==========================================================================
// Last updated on 6/26/13; 7/16/13; 8/8/13; 8/12/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "image/pngfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(16);

   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string cardinal_dirs_subdir=stable_frames_subdir+"cardinal_dirs/";
   filefunc::dircreate(cardinal_dirs_subdir);

   timefunc::initialize_timeofday_clock();

// Import Ueast vs pano_ID from text file exported via program
// GEOREG_FSF_PANOS into an STL map:

   string Ueast_filename=stable_frames_subdir+"Ueast.dat";
   filefunc::ReadInfile(Ueast_filename);
   
   typedef map<int,double> UEAST_MAP;
   UEAST_MAP ueast_map;
   UEAST_MAP::iterator iter;

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int pano_ID=column_values[0];
      double U_east=column_values[1];
      ueast_map[pano_ID]=U_east;
   }

// Load 4 cardinal direction labels into STL vector:

   vector<string> cardinal_dir_label;
   cardinal_dir_label.push_back("East");
   cardinal_dir_label.push_back("North");
   cardinal_dir_label.push_back("West");
   cardinal_dir_label.push_back("South");

   int pano_width=40000;
   int pano_height=2200;
   double Umax=double(pano_width)/double(pano_height); // = 18.18182

   int counter=0;
   for (iter=ueast_map.begin(); iter != ueast_map.end(); iter++)
   {
      double progress_frac=double(counter++)/double(ueast_map.size()-1);
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      int pano_ID=iter->first;

      double U_east=iter->second;
      string stable_pano_filename=stable_frames_subdir+
         "stable_uvcorrected_wisp_res0_"+
         stringfunc::integer_to_string(pano_ID,5)+".jpg";

      if (!filefunc::fileexist(stable_pano_filename)) continue;
   
      string downsized_pano_filename="downsized.png";
      int max_xdim=10000;
      int max_ydim=550;
      videofunc::downsize_image(
         stable_pano_filename,max_xdim,max_ydim,downsized_pano_filename);
      pano_width=max_xdim;
      pano_height=max_ydim;

      texture_rectangle* grey_texture_rectangle_ptr=
         new texture_rectangle(downsized_pano_filename,NULL);
      texture_rectangle* texture_rectangle_ptr=grey_texture_rectangle_ptr->
         generate_RGB_from_grey_texture_rectangle();
      delete grey_texture_rectangle_ptr;

      vector<int> cardinal_px;
      for (int a=0; a<4; a++)
      {

// Recall U_north lies to left of U_east in WISP imagery!

         double U=U_east-0.25*a*Umax;

         if (U > Umax) U -= Umax;
         if (U < 0) U += Umax;
         int px=basic_math::round(U*pano_height);
         cout << "U = " << U << " px = " << px << endl;
         cardinal_px.push_back(px);

         double hue=0.25*a*360;
         double value=1;
         double sat=1;

         double r,g,b;
         colorfunc::hsv_to_RGB(hue,sat,value,r,g,b);
         
         int R=255*r;
         int G=255*g;
         int B=255*b;
         for (int py=0; py<pano_height; py++)
         {
//            cout << "px = " << px << " py = " << py << endl;
            texture_rectangle_ptr->set_pixel_RGB_values(px-2,py,R,G,B);
            texture_rectangle_ptr->set_pixel_RGB_values(px-1,py,R,G,B);
            texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
            texture_rectangle_ptr->set_pixel_RGB_values(px+1,py,R,G,B);
            texture_rectangle_ptr->set_pixel_RGB_values(px+2,py,R,G,B);
         }
      } // loop over index a labeling cardinal directions

      string cardinal_dirs_filename=cardinal_dirs_subdir+
         "cardinal_dirs_"+stringfunc::integer_to_string(pano_ID,5)+".png";
      texture_rectangle_ptr->write_curr_frame(cardinal_dirs_filename);
      delete texture_rectangle_ptr;

      for (unsigned int a=0; a<cardinal_dir_label.size(); a++)
      {
         int px_start=cardinal_px[a];
         int py_start=pano_height/2;
      
         double hue=0.25*a*360;
         double value=1;
         double sat=1;

         double r,g,b;
         colorfunc::hsv_to_RGB(hue,sat,value,r,g,b);
         colorfunc::RGB text_rgb(r,g,b);

         string font_path=
            "/home/cho/programs/c++/svn/projects/data/OpenSceneGraph-Data/fonts/arial.ttf";
         int fontsize=50;

         double angle=PI/2;
         string next_cardinal_dirs_filename="./dummy.png";

         pngfunc::add_text_to_PNG_image(
            cardinal_dirs_filename,next_cardinal_dirs_filename,
            px_start,py_start,angle,
            cardinal_dir_label[a],text_rgb,font_path,fontsize);

         string unix_cmd="mv "+next_cardinal_dirs_filename+" "+
            cardinal_dirs_filename;
         sysfunc::unix_command(unix_cmd);

      } // loop over index a labeling cardinal directions

      string banner=
         "Exported WISP pano annotated with cardinal directions to "+
         cardinal_dirs_filename;
      outputfunc::write_big_banner(banner);
      
   } // loop over iter labeling entries in ueast_map

   string banner="Finished running program FSF_CARDINAL_DIRS";
   outputfunc::write_banner(banner);
   outputfunc::print_elapsed_time();
} 

