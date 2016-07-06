// ========================================================================
// Program PACKAGEWRITER is a specialized utility which writes out a
// script file for running BUNDLECITIES on dozens to hundreds of Noah's
// reconstructed photos.
// ========================================================================
// Last updated on 4/12/09; 4/16/09; 4/18/09; 6/24/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
   string script_filename="run_1kdemo";
//   string script_filename="run_baddemo";
//   string script_filename="run_bigdemo";
   ofstream outstream;
   filefunc::openfile(script_filename,outstream);

   outstream << "cd /home/cho/programs/c++/svn/projects/src/mains/newyork"
             << endl;
   outstream 
      << "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundlecities \\" 
      << endl;

   outstream << "--region_filename ./packages/bundle_nyc.pkg \\" << endl;

   int starting_imagenumber=0;
//   int starting_imagenumber=155;
//   int stopping_imagenumber=300;
//   int stopping_imagenumber=155;
//   int stopping_imagenumber=317;
   int stopping_imagenumber=1012;
   vector<int> pathological_imagenumbers;
//   pathological_imagenumbers.push_back(90);
//   pathological_imagenumbers.push_back(132);
//   pathological_imagenumbers.push_back(155);
//   int ndigits=3;
   int ndigits=4;
   for (int imagenumber=starting_imagenumber; imagenumber <= 
           stopping_imagenumber; imagenumber++)
   {
      bool good_image_flag=true;
      for (int p=0; p<pathological_imagenumbers.size(); p++)
      {
         if (imagenumber==pathological_imagenumbers[p]) good_image_flag=false;
      }
      if (!good_image_flag) continue;

      outstream << "--region_filename ./packages/bundler/photo_"+
         stringfunc::integer_to_string(imagenumber,ndigits)+".pkg \\"
                << endl;
   }
   outstream << "--GIS_layer ./packages/nyc_landmarks.pkg \\" << endl;
   outstream << "--ActiveMQ_hostname tcp://127.0.0.1:61616 \\" << endl;
   outstream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;

   filefunc::closefile(script_filename,outstream);

// Make output script executable:

   string unix_command_str="chmod a+x "+script_filename;
//   cout << "unix_command_str = " << unix_command_str << endl;
   sysfunc::unix_command(unix_command_str);

}
