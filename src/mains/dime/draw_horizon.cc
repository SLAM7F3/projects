// =======================================================================
// Program DRAW_HORIZON

// =======================================================================
// Last updated on 3/22/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "video/camerafuncs.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "math/mathfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(0);
   string input_image_filename=photo_ptr->get_filename();
   cout << "input_image_filename = " << input_image_filename << endl;
   string basename=filefunc::getbasename(input_image_filename);
   basename=stringfunc::prefix(basename);
   cout << "basename = " << basename << endl;
   string separator_chars="_";
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      basename,separator_chars);
   string framenumber_str=substrings[2];
   int frame_number=stringfunc::string_to_number(framenumber_str);
   cout << "Frame number = " << frame_number << endl;

   texture_rectangle* input_texture_rectangle_ptr=
      new texture_rectangle(input_image_filename,NULL);
   int width=input_texture_rectangle_ptr->getWidth();
   int height=input_texture_rectangle_ptr->getHeight();
   cout << "width = " << width << " height = " << height << endl;

   string output_image_filename="wisp_horizon_"+framenumber_str+".jpg";
//   string unix_cmd="cp "+input_image_filename+" "+output_image_filename;
//   sysfunc::unix_command(unix_cmd);
   texture_rectangle* output_texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);
   string blank_filename="blank.jpg";
   output_texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,0.5);

// wisp_res0_00000.jpg

   double A_start=-13.4095;
   double phi_0_start=138.269 * PI/180;
   double py_avg_start=1117.31;

// wisp_res0_00420.jpg

   double A_stop=343.331;
   double phi_0_stop=156.858 * PI/180;
   double py_avg_stop=1114.04;

// Intermediate wisp pano JPG:

   double f=double(frame_number-0)/double(420-0);

   double A=(1-f)*(-13.4095)+f*343.331;
   double phi_0=((1-f)*(138.269)+f*156.858)*PI/180;
   double py_avg=(1-f)*(1117.31)+f*1114.04;

   int R=255;
   int G=0;
   int B=0;
   for (int pu=0; pu<width; pu++)
   {
      if (pu%1000==0) cout << pu << " " << flush;
      for (int pv=0; pv<height; pv++)
      {
         input_texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,R,G,B);
         output_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      }
      
      int pv=basic_math::round(py_avg+A*sin(2*PI/width*pu+phi_0));

// FAKE FAKE:  Fri Mar 22, 2013 at 4:57 pm

//      output_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,255,0,0);

   } // loop over pu index
   cout << endl;
   
   delete input_texture_rectangle_ptr;
   
   output_texture_rectangle_ptr->write_curr_frame(output_image_filename);
   delete output_texture_rectangle_ptr;

   string banner="Exported "+output_image_filename;
   outputfunc::write_big_banner(banner);

   outputfunc::print_elapsed_time();
}
