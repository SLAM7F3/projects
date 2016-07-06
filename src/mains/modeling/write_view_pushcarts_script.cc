// ==========================================================================
// Program WRITE_VIEW_PUSHCART_SCRIPT is a specialized utility which
// generates an executable script for viewing only Carleton Street
// "pushcart" photos.  
// ==========================================================================
// Last updated on 2/19/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "bundler/bundlerfuncs.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "numrec/nrfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
//   string img_names_filename=bundler_IO_subdir+"img_names.txt";
//   filefunc::ReadInfile(img_names_filename);
   
   string script_filename="run_view_pushcarts";
   ofstream outstream;
   filefunc::openfile(script_filename,outstream);

   outstream << "cd ./" << endl;
   outstream << "../photosynth/viewbundler \\" << endl;
   outstream << "./bundler/Pushcart/thresholded_xyz_points.osga \\" << endl;

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_filename=filefunc::text_line[i];
      int posn=stringfunc::first_substring_location(curr_filename,"D5000_");
      if (posn < 0) continue;
      outstream << "--region_filename ./bundler/Pushcart/packages/";
      outstream << "photo_"+stringfunc::integer_to_string(i,4) 
                << ".pkg \\" << endl;
   }
   outstream << "--image_list_filename ./bundler/Pushcart/image_list.dat \\"
             << endl;
   outstream << "--image_sizes_filename ./bundler/Pushcart/image_sizes.dat \\"
             << endl;
   outstream << "--xyz_pnts_filename ./bundler/Pushcart/thresholded_xyz_points.dat \\" << endl;
   outstream << "--camera_views_filename ./bundler/Pushcart/sorted_camera_views.dat \\" << endl;
   outstream << "--initial_mode Manipulate_Fused_Data_Mode " << endl;
   
   filefunc::closefile(script_filename,outstream);

// Make output script executable:

   string unix_command_str="chmod a+x "+script_filename;
//   cout << "unix_command_str = " << unix_command_str << endl;
   sysfunc::unix_command(unix_command_str);

   string banner="Script file written to output file "+script_filename;
   outputfunc::write_big_banner(banner);
}
