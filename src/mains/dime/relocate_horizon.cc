// =======================================================================
// Program RELOCATE_HORIZON imports a set of subsampled, stabilized
// WISP panoramas for some user-specified May 2013 scene.  For each
// input panorama, it computes horizontal line integrals near the
// vertical middle.  RELOCATE_HORIZON then computes the first
// derivative of the horizontal line integrals as a function of
// vertical pixel py.  The py value for which the first derivative is
// maximal is taken to be the vertical coordinate for the horizontal
// horizon.  

// RELOCATE_HORIZON exports text file Horizon_V.dat which reports
// V_horizon as a function of panorama ID and filename.  It also
// generates AVI movie Horizon_15fps_SceneNN.avi that illustrates
// extracted horizons as red lines within subsampled WISP panos.

//				./relocate_horizon

// ========================================================================
// Last updated on 7/16/13; 8/8/13; 9/3/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "image/imagefuncs.h"
#include "plot/metafile.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013)";
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str="Scene"+stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string subsampled_stable_frames_subdir=
      stable_frames_subdir+"subsampled/";

   timefunc::initialize_timeofday_clock();

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      subsampled_stable_frames_subdir);

   string horizon_subdir=subsampled_stable_frames_subdir+"horizon/";
   filefunc::dircreate(horizon_subdir);

   string horizon_track_filename=horizon_subdir+"Horizon_V.dat";
   ofstream horizon_stream;
   horizon_stream.precision(10);
   filefunc::openfile(horizon_track_filename,horizon_stream);
   horizon_stream << "# PanoID           Pano Filename                      V_horizon  pv_horizon" << endl << endl;

   int px_FSF_lo=2000;
   int px_FSF_hi=2500;
   int py_horizon_lo=95;
   int py_horizon_hi=125;

   int nsize=5;
   int deriv_order=1;
   double std_dev=2;
   double delta_x=1;
   vector<double> deriv_filter=filterfunc::gaussian_filter(
      nsize,deriv_order,std_dev,delta_x);


   int i_start=0;
   if (scene_ID==18) i_start=150;
   int i_stop=image_filenames.size()-1;
   for (int i=i_start; i<=i_stop; i++)
   {
      outputfunc::print_elapsed_time();

      cout << i << " " << image_filenames[i] << endl;
      string basename=filefunc::getbasename(image_filenames[i]);

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filenames[i],NULL);
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      int R,G,B;
      vector<double> py_values,line_integral_values,
         deriv_line_integral_values;
      for (int py=py_horizon_lo; py<=py_horizon_hi; py++)
      {
         double line_integral=0;
         for (int px=0; px<px_FSF_lo; px++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            line_integral += R;
         }
         for (int px=px_FSF_hi; px<width; px++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            line_integral += R;
         }
         
//         cout << "py = " << py << " line_integral = " << line_integral
//              << endl;
         py_values.push_back(py);
         line_integral_values.push_back(line_integral);

      } // loop over py 

// Differentiate horizontal line integrals wrt to py:

      bool wrap_around_input_values=false;
      filterfunc::brute_force_filter(
         line_integral_values,deriv_filter,deriv_line_integral_values,
         wrap_around_input_values);

// Zero out starting and stopping values within deriv_line_integral_values:
      
      for (int q=0; q<10; q++)
      {
         deriv_line_integral_values[q]=0;
         deriv_line_integral_values[deriv_line_integral_values.size()-1-q]=0;
      }

      int py_horizon=-1;
      int max_deriv_line_integral=-1;
      for (unsigned int q=0; q<deriv_line_integral_values.size(); q++)
      {
         if (deriv_line_integral_values[q] > max_deriv_line_integral)
         {
            max_deriv_line_integral=deriv_line_integral_values[q];
            py_horizon=py_values[q];
         }
      }
      cout << "py_horizon = " << py_horizon << endl;
      double v_horizon=1-double(py_horizon)/double(height);
      horizon_stream << i << "   " 
                     << basename << "   "
                     << stringfunc::number_to_string(v_horizon,6) << "   " 
                     << py_horizon << endl;

// Draw horizontal line within subsampled pano indicating extracted
// horizon's vertical location:

      texture_rectangle* RGB_texture_rectangle_ptr=
         texture_rectangle_ptr->generate_RGB_from_grey_texture_rectangle();

      for (int pu=0; pu<width; pu++)
      {
         R=255;
         G=0;
         B=0;
         RGB_texture_rectangle_ptr->set_pixel_RGB_values(pu,py_horizon,R,G,B);
      }
      string horizon_filename=horizon_subdir+
         "horizon_"+stringfunc::integer_to_string(i,4)+".jpg";

      RGB_texture_rectangle_ptr->write_curr_frame(horizon_filename);
      string banner="Exported "+filefunc::getbasename(horizon_filename);
      outputfunc::write_banner(banner);

      delete texture_rectangle_ptr;
      delete RGB_texture_rectangle_ptr;

/*
      string metafile_filename="horizon";
//      string title="Horizon line integrals";
      string title="Horizon line integral derivatives";
      string x_label="py";
//      string y_label="Line integral";
      string y_label="Line integral derivative";
      double x_min=95;
      double x_max=125;
      double y_min=mathfunc::minimal_value(deriv_line_integral_values);
      double y_max=mathfunc::maximal_value(deriv_line_integral_values);
//      double y_min=mathfunc::minimal_value(line_integral_values);
//      double y_max=mathfunc::maximal_value(line_integral_values);

      metafile* metafile_ptr=new metafile();
      metafile_ptr->set_parameters(
         metafile_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      metafile_ptr->openmetafile();
      metafile_ptr->write_header();
      metafile_ptr->write_curve(
//         py_values,line_integral_values,colorfunc::blue);
         py_values,deriv_line_integral_values,colorfunc::blue);
      metafile_ptr->closemetafile();
      delete metafile_ptr;
*/

   } // loop over index i labeling input panorama
   cout << endl;

   filefunc::closefile(horizon_track_filename,horizon_stream);
   string banner="Exported Horizon V values to "+horizon_track_filename;
   outputfunc::write_big_banner(banner);

   string unix_cmd="cd "+horizon_subdir+";";
   string AVI_filename="Horizon_15fps_"+scene_ID_str+".avi";
   unix_cmd += "mkmpeg4 -v -f 15 -b 24000000 -o "+AVI_filename+" *.jpg";
   cout << "unix_cmd = " << unix_cmd << endl << endl;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv "+AVI_filename+" "+horizon_subdir;
   cout << "unix_cmd = " << unix_cmd << endl << endl;
   sysfunc::unix_command(unix_cmd);
   AVI_filename=horizon_subdir+AVI_filename;
   banner="Exported Horizon AVI to "+AVI_filename;
   outputfunc::write_big_banner(banner);

   banner="Finished running program HORIZON";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

