// =======================================================================
// Program ATHENA attempts to semi-automatically track the ATHENA ship
// seen in WISP-360 imagery from the FSF.  It queries the user to
// enter an initial set of pixel coordinates (px,py) for the ATHENA
// manually determined via GIMPing the first subsampled stabilized
// panorama.  

// Working with subsampled stabilized WISP panoramas, ATHENA then
// searches for hot pixels in the vicinity of the initial UV
// coordinates.  It applies alpha-beta filtering to the
// new U coordinate and alpha filtering to the new V coordinate.
// ATHENA superposes a red dot in an output image indicating its best
// estimate for the second ship's image plane location.  It
// generates an AVI movie of the annotated subsampled stabilized
// panoramas.  ATHENA also exports a text file containing the UV
// coordinates as functions of input panorama filename.

//				./athena

// ========================================================================
// Last updated on 7/12/13; 7/15/13; 7/16/13; 8/8/13; 8/12/13; 4/6/14
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "image/imagefuncs.h"
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
   string scene_ID_str="Scene"+stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string subsampled_stable_frames_subdir=
      stable_frames_subdir+"subsampled/";

   timefunc::initialize_timeofday_clock();

   double init_px_athena,init_py_athena;
   cout << "Enter px pixel location for Athena " << endl;
   cout << " in subsampled_stable_uvcorrected_wisp_res0_00001.jpg" << endl;
   cin >> init_px_athena;

   cout << "Enter py pixel location for Athena " << endl;
   cout << " in subsampled_stable_uvcorrected_wisp_res0_00001.jpg" << endl;
   cin >> init_py_athena;

   if (scene_ID==12)
   {
      init_px_athena=1523;
      init_py_athena=110;
   }
   else if (scene_ID==18)
   {
      init_px_athena=1777;	// Frame 150
      init_py_athena=114;	// Frame 150
   }
   else if (scene_ID==19)
   {
      init_px_athena=1001;
      init_py_athena=113;
   }
   else if (scene_ID==25)
   {
      init_px_athena=1164;
      init_py_athena=112;
   }
   else if (scene_ID==27)
   {
      init_px_athena=1314;
      init_py_athena=113;
   }
   else if (scene_ID==29)
   {
      init_px_athena=1364;
      init_py_athena=113;
   }

//    int subsampled_pano_width=3960;
   int subsampled_pano_height=218;
   double Uathena=init_px_athena/subsampled_pano_height;
   double Vathena=1-init_py_athena/subsampled_pano_height;
 
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      subsampled_stable_frames_subdir);

   string athena_subdir=subsampled_stable_frames_subdir+"athena/";
   filefunc::dircreate(athena_subdir);

   string athena_track_filename=athena_subdir+"athena_UVtrack.dat";
   ofstream athena_stream;
   athena_stream.precision(10);
   filefunc::openfile(athena_track_filename,athena_stream);
   athena_stream << "# Pano_frame   			                    Uathena_full Vathena pu_athena pv_athena"
                 << endl << endl;

   int pu_athena=NEGATIVEINFINITY;
   int prev_pu_athena=NEGATIVEINFINITY;
   double vu_athena=NEGATIVEINFINITY;
   double prev_vu_athena=NEGATIVEINFINITY;

   int pv_athena=NEGATIVEINFINITY;
   int prev_pv_athena=NEGATIVEINFINITY;

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

      if (pu_athena < 0 && pv_athena < 0)
      {
         unsigned int qu,qv;
         texture_rectangle_ptr->get_pixel_coords(Uathena,Vathena,qu,qv);
         prev_pu_athena=pu_athena=qu;
         prev_pv_athena=pv_athena=qv;
      }
//      cout << "Starting pu_athena = " << pu_athena
//           << " pv_athena = " << pv_athena << endl;
      
      int pu_lo=pu_athena-20;
      int pu_hi=pu_athena+15;
      pu_lo=basic_math::max(0,pu_lo);
      pu_hi=basic_math::min(width-1,pu_hi);
      
      int pv_lo=pv_athena-3;
      int pv_hi=pv_athena+6;
      pv_lo=basic_math::max(0,pv_lo);
      pv_hi=basic_math::min(height-1,pv_hi);

//      cout << "pu_lo = " << pu_lo << " pu_hi = " << pu_hi << endl;
//      cout << "pv_lo = " << pv_lo << " pv_hi = " << pv_hi << endl;

      int R,G,B;
      vector<double> v;
      for (int pu=pu_lo; pu<pu_hi; pu++)
      {
         for (int pv=pv_lo; pv<pv_hi; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
//            cout << "pu = " << pu << " pv = " << pv
//                 << " R = " << R << " G = " << G << " B = " << B << endl;
            v.push_back(R);
         } // loop over pv
      } // loop over pu
      
      int n_output_bins=255;
      prob_distribution prob(v,n_output_bins,0);

      double threshold_prob=0.85;
//      double threshold_prob=0.90;
//      double threshold_prob=0.99;
//      prob.writeprobdists(false);

//      double v_90=prob.find_x_corresponding_to_pcum(0.90);
//      double v_95=prob.find_x_corresponding_to_pcum(0.95);
//      double v_99=prob.find_x_corresponding_to_pcum(0.99);
//      cout << "v_90 = " << v_90 << endl;
//      cout << "v_95 = " << v_95 << endl;
//      cout << "v_99 = " << v_99 << endl;
//      outputfunc::enter_continue_char();

      texture_rectangle* RGB_texture_rectangle_ptr=
         texture_rectangle_ptr->generate_RGB_from_grey_texture_rectangle();

      bool athena_not_detected_flag=false;
      int denom;
      const int min_denom=12;
      double pu_COM,pv_COM,v_threshold;
      do
      {
         denom=0;
         pu_COM=pv_COM=0;
         v_threshold=prob.find_x_corresponding_to_pcum(threshold_prob);
         for (int pu=pu_lo; pu<pu_hi; pu++)
         {
            for (int pv=pv_lo; pv<pv_hi; pv++)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
               if (R > v_threshold)
               {
//                  R=0;
//                  G=0;
//                  B=255;
//                  RGB_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

                  pu_COM += pu;
                  pv_COM += pv;
                  denom++;
               }
            } // loop over pv
         } // loop over pu

//         cout << "denom = " << denom 
//              << " threshold_prob = " << threshold_prob 
//              << " v_threhsold = " << v_threshold << endl;
         threshold_prob -= 0.02;

         if (threshold_prob < 0.5)
         {
            cout << "Error! threshold_prob = " << threshold_prob << endl;
            athena_not_detected_flag=true;
            break;
         }
      }
      while (denom < min_denom );

      if (athena_not_detected_flag)
      {
         pu_athena=prev_pu_athena;
         vu_athena=prev_vu_athena;
         prev_vu_athena=NEGATIVEINFINITY;
         pv_athena=prev_pv_athena;
      }
      else
      {
         pu_COM /= denom;
         pv_COM /= denom;

         double alpha_u=0.75;
         double alpha_v=0.25;

         double beta_u=sqr(alpha_u)/(2-alpha_u);
         double dt=1;

         if (i==i_start)
         {
            pu_athena=filterfunc::alpha_filter(pu_COM,prev_pu_athena,alpha_u);
            prev_pu_athena=pu_athena;
         }
         else if (i > i_start)
         {
            prev_vu_athena=(pu_athena-prev_pu_athena)/dt;

            double athena_pu=pu_athena;
            filterfunc::alphabeta_filter(
               pu_COM,prev_pu_athena,prev_vu_athena,
               athena_pu,vu_athena,alpha_u,beta_u,dt);

            pu_athena=basic_math::round(athena_pu);
            prev_pu_athena=pu_athena;
            prev_vu_athena=vu_athena;
         }
      
         pv_athena=filterfunc::alpha_filter(pv_COM,prev_pv_athena,alpha_v);
         prev_pv_athena=pv_athena;
      } // athena_not_detected_flag conditional
      
//      cout << "Stopping pu_athena = " << pu_athena
//           << " pv_athena = " << pv_athena << endl;

      colorfunc::Color circle_color=colorfunc::red;
      int radius=2;
      RGB_texture_rectangle_ptr->fill_circle(
         pu_athena,pv_athena,radius,circle_color);

// Convert pu_athena and pv_athena into Uathena and Vathena with
// respect to FULL WISP panorama which has Umax=40000/2200:
      
      Uathena=double(pu_athena)/height;
      Vathena=1-double(pv_athena)/height;

      athena_stream << basename << "  " 
                    << Uathena << "  " << Vathena << "  "
                    << pu_athena << "  " << pv_athena << endl;

      if (pu_athena < 0 || pv_athena < 0) 
         outputfunc::enter_continue_char();

      string athena_filename=athena_subdir+
         "athena_"+stringfunc::integer_to_string(i,4)+".jpg";

      RGB_texture_rectangle_ptr->write_curr_frame(athena_filename);
      string banner="Exported "+filefunc::getbasename(athena_filename);
      outputfunc::write_banner(banner);

      delete texture_rectangle_ptr;
      delete RGB_texture_rectangle_ptr;

//      outputfunc::enter_continue_char();
   } // loop over index i labeling input panorama
   cout << endl;

   filefunc::closefile(athena_track_filename,athena_stream);

   string unix_cmd="cd "+athena_subdir+";";
   string AVI_filename="athena_UVtrack_15fps_"+scene_ID_str+".avi";
   unix_cmd += "mkmpeg4 -v -f 15 -b 24000000 -o "+AVI_filename
      +" *.jpg";
   cout << "unix_cmd = " << unix_cmd << endl << endl;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv "+AVI_filename+" "+athena_subdir;
   cout << "unix_cmd = " << unix_cmd << endl << endl;
   sysfunc::unix_command(unix_cmd);
   AVI_filename=athena_subdir+AVI_filename;
   string banner="Exported Athena AVI to "+AVI_filename;
   outputfunc::write_big_banner(banner);

   banner="Exported Athena UV track to "+athena_track_filename;
   outputfunc::write_big_banner(banner);

   banner="Finished running program ATHENA";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

