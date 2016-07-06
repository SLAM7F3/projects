// ==========================================================================
// Signrecogfuncs namespace method definitions
// ==========================================================================
// Last modified on 1/23/14; 3/29/14; 6/7/14; 6/25/14
// ==========================================================================

#include <iostream>
#include <unistd.h>	// needed for usleep call
#include "dlib/svm.h"

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "astro_geo/Clock.h"
#include "image/compositefuncs.h"
#include "video/connected_components.h"
#include "image/extremal_region.h"
#include "image/extremal_regions_group.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "video/RGB_analyzer.h"
#include "classification/signrecogfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace signrecogfunc
{
 
// ==========================================================================
// Image file manipulation methods
// ==========================================================================

// Method generate_timestamped_archive_subdir()

   string generate_timestamped_archive_subdir(string input_images_subdir)
   {
      Clock clock;
      clock.set_time_based_on_local_computer_clock();
      string day_hour_separator_char="_";
      string time_str=clock.YYYY_MM_DD_H_M_S(day_hour_separator_char);
//      cout << "time_str = " << time_str << endl;
      string archived_images_subdir=input_images_subdir+"archived_images/"
         +time_str+"/";
      filefunc::dircreate(archived_images_subdir);
      return archived_images_subdir;
   }

// ---------------------------------------------------------------------     
// Method archive_all_but_latest_image_files() moves all images except
// the latest one to an archive subdirectory which is time-stamped.
// It returns the full path for the next-to-latest image file as an
// output string.

   string archive_all_but_latest_image_files(
      string input_images_subdir,string archived_images_subdir)
   {
//      cout << "inside signrecogfunc::archive_all_but_latest_image_files()"
//           << endl;
      
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         input_images_subdir);

      string latest_filename,next_to_latest_filename;
      bool changed_flag=filefunc::get_latest_files_in_subdir(
         input_images_subdir,latest_filename,next_to_latest_filename);
      if (!changed_flag) return "";

//      cout << "next_to_latest_filename = " << next_to_latest_filename
//           << " latest_filename = " << latest_filename 
//           << endl;

// Move all images EXCEPT latest to archived_images_subdir:

      for (unsigned int i=0; i<image_filenames.size(); i++)
      {
         if (image_filenames[i]==latest_filename) continue;

// Check if current image is in ppm format.  If so, first convert ppm
// to png.  Then delete original ppm in favor of new png file:

         string curr_image_filename=image_filenames[i];
         string curr_suffix=stringfunc::suffix(curr_image_filename);
//         cout << "original curr_image_filename = " << curr_image_filename
//              << " curr_suffix = " << curr_suffix << endl;
         if (curr_suffix=="ppm")
         {
            string curr_prefix=stringfunc::prefix(curr_image_filename);
            string png_filename=curr_prefix+".png";
            string unix_cmd="convert "+curr_image_filename+" "+png_filename;
//            cout << "unix_cmd = " << unix_cmd << endl;
            sysfunc::unix_command(unix_cmd);
            while (!filefunc::fileexist(png_filename))
            {
               cout << "Sleeping while waiting for png file to be generated"
                    << endl;
               usleep(100);
            }
            filefunc::deletefile(curr_image_filename);
            curr_image_filename=png_filename;
         }

         string unix_cmd="mv "+curr_image_filename+" "+archived_images_subdir;
         sysfunc::unix_command(unix_cmd);
      }
      cout << "Moved " << image_filenames.size()-1 << " image files to "+
         archived_images_subdir << endl;

      string basename=filefunc::getbasename(next_to_latest_filename);
      string suffix=stringfunc::suffix(basename);
      if (suffix=="ppm")
      {
         basename=stringfunc::prefix(basename)+".png";
      }
//      cout << "basename = " << basename << endl;
      if (basename.size()==0) return "";

      string new_next_to_latest_filename=archived_images_subdir+basename;
      return new_next_to_latest_filename;
   }
 
// ==========================================================================
// PointGrey camera specific methods
// ==========================================================================

// Method get_Pointgrey_calibration_params() returns focal length and
// curvature parameters based upon checkerboard measurements analyzed
// via Caltech matlab codes.

   void get_PointGrey_calibration_params(
      int PointGrey_camera_ID,
      int& Npu,int& Npv,double& f_pixels,
      double& cu_pixels,double& cv_pixels,double& k2,double& k4)
   {
      if (PointGrey_camera_ID==501207) 		// Mark's camera
      {
         Npu=1280;
         Npv=960;
         f_pixels = 745.448;
//         Aspect_ratio = 1.33333
//         f = -0.776508
//         FOV_u = 81.2951 FOV_v = 65.5556
         cu_pixels=650.69;
         cv_pixels=480.75;
         k2=-0.35513;
         k4=0.12033;
      }
      else if (PointGrey_camera_ID==501208) 	// Pat's camera
      {
         Npu=1280;
         Npv=960;
         f_pixels = 745.448;
//         Aspect_ratio = 1.33333
//         f = -0.776508
//         FOV_u = 81.2951 FOV_v = 65.5556
         cu_pixels=650.69;
         cv_pixels=480.75;
         k2=-0.35513;
         k4=0.12033;
      }
      else if (PointGrey_camera_ID==501890) 	// Bryce's camera
      {
         Npu=1280;
         Npv=960;
         f_pixels = 977.329;
//	Aspect_ratio = 1.33333
//	f_pixels = 977.329 f = -1.01805
//	FOV_u = 66.4373 degs FOV_v = 52.3145 degs
         cu_pixels=686.47;
         cv_pixels=474.79;
         k2=-0.35909;
         k4=0.12782;
      }
   }

// ---------------------------------------------------------------------     
// Method initialize_PointGrey_camera_params()

   void initialize_PointGrey_camera_params(
      int PointGrey_camera_ID,camera* camera_ptr)
   {
//      cout << "inside signrecogfunc::initialize_PointGrey_camera_params()"
//           << endl;
      
      int Npu,Npv;
      double f_pixels=0,cu_pixels,cv_pixels,k2,k4;
      get_PointGrey_calibration_params(
         PointGrey_camera_ID,Npu,Npv,f_pixels,cu_pixels,cv_pixels,k2,k4);
      double f=-double(f_pixels)/Npv;
      
/*
      if (PointGrey_ID=501207)		// "Mark's" camera
      {
         f=-0.776508;
      }
      else if (PointGrey_ID=501208)	// "Pat's" camera
      {
         f=-0.776508;
      }
      else if (PointGrey_ID=501890)	// "Bryce's" camera
      {
         f=-1.01805;
      }
*/
      double aspect_ratio=double(Npu)/double(Npv);
//      cout << "f = " << f << " aspect_ratio = " << aspect_ratio << endl;
      
      double u0=0.5*aspect_ratio;
      double v0=0.5;
      camera_ptr->set_internal_params(f,f,u0,v0);

      double az=0;
      double el=0;
      double roll=0;
      camera_ptr->set_Rcamera(az,el,roll);
      camera_ptr->construct_projection_matrix();
      camera_ptr->print_external_and_internal_params();
   }

// ---------------------------------------------------------------------     
// Method radially_undistort_PointGrey_image()

   string radially_undistort_PointGrey_image(
      int PointGrey_camera_ID,
      string image_filename,texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr)
   {
      int Npu,Npv;
      double f_pixels,cu_pixels,cv_pixels,k2,k4;
      f_pixels=cu_pixels=cv_pixels=k2=k4=0;
      get_PointGrey_calibration_params(
         PointGrey_camera_ID,Npu,Npv,f_pixels,cu_pixels,cv_pixels,k2,k4);

/*
// Mark's PointGrey camera (501207): 

// f= -1.13714 (after white border cropping)
// Aspect ratio = 4/3
// FOV_u = 60.7 degs
// FOV_v = 47.4 degs

      double cropped_scalefactor=732.0/900.0;
//      int Npu=1280*cropped_scalefactor;
//      int Npv=960*cropped_scalefactor;

      int Npu=976;
      int Npv=732;

// Intrinsic camera calibration parameters determined via Caltech
// "checkboard" matlab codes for "Mark's" PointGrey = 501207 (maximal
//  field-of-view):

      double fu_pixels=833.97667*cropped_scalefactor;
      double fv_pixels=830.80215*cropped_scalefactor;
      double aspect_ratio=double(Npu)/double(Npv);

      double cu_pixels=637.814;
      double cv_pixels=463.022;
      double k2=-0.33952;
      double k4=0.10150;
*/

      texture_rectangle_ptr->reset_texture_content(image_filename);
      undistorted_texture_rectangle_ptr->reset_texture_content(image_filename);

      camerafunc::radially_undistort_image(
         Npu,Npv,f_pixels,cu_pixels,cv_pixels,k2,k4,
         texture_rectangle_ptr,undistorted_texture_rectangle_ptr);

      string dirname=filefunc::getdirname(image_filename);
      string basename=filefunc::getbasename(image_filename);
      string undistorted_filename=dirname+"undistorted_"+basename;
      undistorted_texture_rectangle_ptr->write_curr_frame(
         undistorted_filename);
      return undistorted_filename;
   }

// ---------------------------------------------------------------------     
// Method detect_corrupted_PointGrey_image() takes in the filename for
// some PointGrey image which is assumed to have no white border.  It
// searches for a grey-colored vertical stripe located on the RHS of
// the input image.  If such a stripe is found, this boolean method
// returns true.

   bool detect_corrupted_PointGrey_image(
      string image_filename,texture_rectangle* texture_rectangle_ptr)
   {
      texture_rectangle_ptr->reset_texture_content(image_filename);
      int Npu=texture_rectangle_ptr->getWidth();
      int Npv=texture_rectangle_ptr->getHeight();

      int R,G,B;
      int n_grey_pixels=0;
      int n_pixels=0;
      const int Npu_corrupted=783;
      const int grey_threshold=4;
      for (int pv=0; pv<Npv; pv++)
      {
         for (int pu=Npu_corrupted; pu<Npu; pu++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            if (R < 0 || G < 0 || B < 0) continue;
            int delta_RG=fabs(R-G);
            int delta_GB=fabs(G-B);
            int delta_BR=fabs(B-R);
            if (delta_RG < grey_threshold && delta_GB < grey_threshold
            && delta_BR < grey_threshold)
            {
               n_grey_pixels++;
//               cout << "R = " << R << " G = " << G << " B = " << B << endl;
            }
            n_pixels++;
         } // loop over pu index
      } // loop over pv index
      double grey_frac=double(n_grey_pixels)/n_pixels;
//      cout << "image_filename = " << filefunc::getbasename(image_filename)
//           << " grey_frac = " << grey_frac << endl;

      bool corrupted_flag=false;
      double corruption_threshold=0.33;
      if (grey_frac > corruption_threshold)
      {
         corrupted_flag=true;
      }

      return corrupted_flag;
   }

// ---------------------------------------------------------------------     
// Method crop_white_border() removes white border pixels surrounding
// PointGrey images captured within Tennis Bubble on Saturday, Oct 27,
// 2012:

   string crop_white_border(string orig_image_filename)
   {
      int px_min=112;
      int py_min=62;
      int px_max=1088;
      int py_max=794;
      int width=px_max-px_min;
      int height=py_max-py_min;
      string subdir=filefunc::getdirname(orig_image_filename);
      string basename=filefunc::getbasename(orig_image_filename);
      string cropped_image_filename=subdir+"cropped_"+basename;
      imagefunc::crop_image(
         orig_image_filename,cropped_image_filename,
         width,height,px_min,py_min);
      return cropped_image_filename;
   }
   
// ==========================================================================
// TOC12 sign initialization methods
// ==========================================================================

// Initialize TOC12 sign properties:

   vector<SIGN_PROPERTIES> initialize_sign_properties()
   {
      SIGN_PROPERTIES curr_sign_properties;
      vector<SIGN_PROPERTIES> sign_properties;

// Yellow radiation TOC12 ppt sign:

      curr_sign_properties.symbol_name="yellow_radiation";
      curr_sign_properties.sign_hue="yellow";
      curr_sign_properties.bbox_color="blue";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("yellow");
      curr_sign_properties.colors_to_find.push_back("darkyellow");
      curr_sign_properties.colors_to_find.push_back("lightyellow");
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=2.2;
      curr_sign_properties.min_compactness=0.055;
      curr_sign_properties.max_compactness=0.115;
      curr_sign_properties.min_n_holes=3;
      curr_sign_properties.max_n_holes=6;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=6;
      curr_sign_properties.min_n_significant_holes=2;
      curr_sign_properties.max_n_significant_holes=6;
      curr_sign_properties.black_interior_flag=true;
      curr_sign_properties.white_interior_flag=false;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.5;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Orange biohazard sign:

      curr_sign_properties.symbol_name="orange_biohazard";
      curr_sign_properties.sign_hue="orange";
      curr_sign_properties.bbox_color="cyan";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("orange");

// For PointGrey cameras, no longer look for red coloring for biohazard sign

//      curr_sign_properties.colors_to_find.push_back("red");
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=3.9;
      curr_sign_properties.min_compactness=0.045;
      curr_sign_properties.max_compactness=0.19;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=10;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=9;
      curr_sign_properties.black_interior_flag=true;
      curr_sign_properties.white_interior_flag=false;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.5;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Blue-purple radiation sign:

      curr_sign_properties.symbol_name="blue_radiation";
      curr_sign_properties.sign_hue="blue";
      curr_sign_properties.bbox_color="orange";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("blue");
      curr_sign_properties.colors_to_find.push_back("darkblue");
      curr_sign_properties.colors_to_find.push_back("greyblue");
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=2.2;
      curr_sign_properties.min_compactness=0.055;
      curr_sign_properties.max_compactness=0.16;
      curr_sign_properties.min_n_holes=0;	// PointGrey camera
//      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=7;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=6;
      curr_sign_properties.min_n_significant_holes=0;	// PointGrey camera
//      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=5;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=false;
      curr_sign_properties.purple_interior_flag=true;
      curr_sign_properties.min_gradient_mag=0.5;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Blue water TOC12 ppt sign:

      curr_sign_properties.symbol_name="blue_water";
      curr_sign_properties.sign_hue="blue";
      curr_sign_properties.bbox_color="yellow";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("blue");
      curr_sign_properties.colors_to_find.push_back("darkblue");
      curr_sign_properties.colors_to_find.push_back("greyblue");
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=2.2;
      curr_sign_properties.min_compactness=0.055;
      curr_sign_properties.max_compactness=0.16;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=6;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=6;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=4;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.5;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Blue gasoline sign:

      curr_sign_properties.symbol_name="blue_gas";
      curr_sign_properties.sign_hue="blue";	// PointGrey cameras
//      curr_sign_properties.sign_hue="blue2";
      curr_sign_properties.bbox_color="white";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("blue");
      curr_sign_properties.colors_to_find.push_back("darkblue");
      curr_sign_properties.colors_to_find.push_back("greyblue");
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=2.2;
      curr_sign_properties.min_compactness=0.045;
      curr_sign_properties.max_compactness=0.16;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=5;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=3;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.5;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Red stop sign:

      curr_sign_properties.symbol_name="red_stop";
      curr_sign_properties.sign_hue="red";
      curr_sign_properties.bbox_color="green";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("red");

// For PointGrey cameras, no longer consider "lightred" coloring for stop sign

//      curr_sign_properties.colors_to_find.push_back("lightred"); 
      curr_sign_properties.min_aspect_ratio=0.4;
      curr_sign_properties.max_aspect_ratio=2;
      curr_sign_properties.min_compactness=0.045;
      curr_sign_properties.max_compactness=0.23;
      curr_sign_properties.min_n_holes=0;
      curr_sign_properties.max_n_holes=5;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=4;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.3;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Green start sign:

      curr_sign_properties.symbol_name="green_start";
      curr_sign_properties.sign_hue="green";
      curr_sign_properties.bbox_color="purple";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("green");
      curr_sign_properties.colors_to_find.push_back("lightgreen");
      curr_sign_properties.colors_to_find.push_back("darkgreen");
      curr_sign_properties.colors_to_find.push_back("greygreen");
      curr_sign_properties.colors_to_find.push_back("yellow");
      curr_sign_properties.colors_to_find.push_back("lightyellow");
      curr_sign_properties.min_aspect_ratio=0.27;
      curr_sign_properties.max_aspect_ratio=2;
      curr_sign_properties.min_compactness=0.045;
      curr_sign_properties.max_compactness=0.23;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=17;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=14;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.3;
      curr_sign_properties.Ng_threshold=0.5;
      sign_properties.push_back(curr_sign_properties);

// Black & white skull sign:

      curr_sign_properties.symbol_name="bw_skull";
      curr_sign_properties.sign_hue="black";
      curr_sign_properties.bbox_color="brick";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("black");
      curr_sign_properties.colors_to_find.push_back("darkgrey");
      curr_sign_properties.min_aspect_ratio=0.1;
      curr_sign_properties.max_aspect_ratio=4;
      curr_sign_properties.min_compactness=0.02;
      curr_sign_properties.max_compactness=0.56;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=5;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=5;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.75;
      curr_sign_properties.max_bbox_hue_frac=0.33;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

// Black & white eat sign:

      curr_sign_properties.symbol_name="bw_eat";
      curr_sign_properties.sign_hue="black";
      curr_sign_properties.bbox_color="gold";
      curr_sign_properties.colors_to_find.clear();
      curr_sign_properties.colors_to_find.push_back("black");
      curr_sign_properties.colors_to_find.push_back("darkgrey");
      curr_sign_properties.min_aspect_ratio=0.1;
      curr_sign_properties.max_aspect_ratio=4;
      curr_sign_properties.min_compactness=0.02;
      curr_sign_properties.max_compactness=0.56;
      curr_sign_properties.min_n_holes=1;
      curr_sign_properties.max_n_holes=5;
      curr_sign_properties.min_n_crossings=2;
      curr_sign_properties.max_n_crossings=8;
      curr_sign_properties.min_n_significant_holes=1;
      curr_sign_properties.max_n_significant_holes=5;
      curr_sign_properties.black_interior_flag=false;
      curr_sign_properties.white_interior_flag=true;
      curr_sign_properties.purple_interior_flag=false;
      curr_sign_properties.min_gradient_mag=0.75;
      curr_sign_properties.max_bbox_hue_frac=0.33;
      curr_sign_properties.Ng_threshold=0.25;
      sign_properties.push_back(curr_sign_properties);

      return sign_properties;
   }

// ---------------------------------------------------------------------     
// Method import_Ng_probabilistic_classification_functions()

   vector<text_detector*> import_Ng_probabilistic_classification_functions(
      const vector<SIGN_PROPERTIES>& sign_properties,
      vector<Ng_pfunct_type>& Ng_pfuncts)
   {
      Ng_pfunct_type Ng_pfunct;

      string symbols_input_subdir="./images/final_signs/";
//      string symbols_input_subdir="./images/ppt_signs/";
      string learned_funcs_subdir="./learned_functions/";

      vector<text_detector*> text_detector_ptrs;
      for (unsigned int s=0; s<sign_properties.size(); s++)
      {
         string symbol_name=sign_properties[s].symbol_name;
         string learned_Ng_pfunct_filename=learned_funcs_subdir;
         learned_Ng_pfunct_filename += symbol_name+"/";
         learned_Ng_pfunct_filename += "symbols_Ng_pfunct_";
//         learned_Ng_pfunct_filename += "12000_96000";
//         learned_Ng_pfunct_filename += "15000_104000";	// final TOC12 signs
         learned_Ng_pfunct_filename += "15000_84000";	// final TOC12 signs
         learned_Ng_pfunct_filename += ".dat";
         cout << "learned_Ng_pfunct_filename = "
              << learned_Ng_pfunct_filename << endl;
         ifstream fin6(learned_Ng_pfunct_filename.c_str(),ios::binary);

         deserialize(Ng_pfunct, fin6);
         Ng_pfuncts.push_back(Ng_pfunct);

// Import dictionary trained on symbol and non-symbol images:

         string symbol_filename=symbols_input_subdir+symbol_name+".png";
         string synthetic_subdir=symbols_input_subdir+"synthetic_symbols/";
         string synthetic_symbols_subdir=synthetic_subdir+symbol_name+"/";
         string dictionary_subdir=synthetic_symbols_subdir;
         cout << "dictionary_subdir = " << dictionary_subdir << endl;

         bool RGB_pixels_flag=true;
         text_detector* text_detector_ptr=new text_detector(
            dictionary_subdir,RGB_pixels_flag);
         text_detector_ptrs.push_back(text_detector_ptr);
         text_detector_ptr->import_inverse_sqrt_covar_matrix();

      } // loop over index s labeling TOC12 symbols

      return text_detector_ptrs;
   }

// ==========================================================================

// ==========================================================================

   string resize_input_image(string orig_image_filename,int& xdim,int& ydim)
   {
      unsigned int orig_xdim,orig_ydim;
      string image_filename;
      imagefunc::get_image_width_height(
         orig_image_filename,orig_xdim,orig_ydim);
      if (orig_xdim >= 1000 || orig_ydim >= 600)
      {
         string dirname=filefunc::getdirname(orig_image_filename);
         string basename=filefunc::getbasename(orig_image_filename);

         string subdirname=dirname+"downsized/";
         filefunc::dircreate(subdirname);
         image_filename=subdirname+"downsized_"+basename;

         xdim=orig_xdim/2;
         ydim=orig_ydim/2;
         videofunc::resize_image(
            orig_image_filename,orig_xdim,orig_ydim,
            xdim,ydim,image_filename);
      }
      else
      {
         image_filename=orig_image_filename;
         xdim=orig_xdim;
         ydim=orig_ydim;
      }

      cout << "image_filename = " << image_filename << endl;
      return image_filename;
   }

// ---------------------------------------------------------------------     
   void reset_texture_image(
      string image_filename,texture_rectangle* texture_rectangle_ptr)
   {
      if (texture_rectangle_ptr != NULL)
         texture_rectangle_ptr->reset_texture_content(image_filename);
   }
   
// ---------------------------------------------------------------------     
// Method quantize_colors()

   void quantize_colors(
      RGB_analyzer* RGB_analyzer_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* selected_colors_texture_rectangle_ptr,
      texture_rectangle* binary_texture_rectangle_ptr)
   {
      string lookup_map_name=curr_sign_properties.sign_hue;
      RGB_analyzer_ptr->quantize_texture_rectangle_colors(
         quantized_texture_rectangle_ptr,lookup_map_name);

      int n_filter_iters=4;
      for (int filter_iter=0; filter_iter < n_filter_iters; 
           filter_iter++)
      {
         RGB_analyzer_ptr->smooth_quantized_image(
            texture_rectangle_ptr,quantized_texture_rectangle_ptr);
      }
//            string quantized_filename=video_subdir+
//              "quantized_"+stringfunc::number_to_string(frame_number)+".jpg";
//            quantized_texture_rectangle_ptr->write_curr_frame(
//               quantized_filename);

      RGB_analyzer_ptr->isolate_quantized_colors(
         quantized_texture_rectangle_ptr,
         curr_sign_properties.colors_to_find,
         selected_colors_texture_rectangle_ptr,
         binary_texture_rectangle_ptr);
         
//            string selected_colors_filename=video_subdir+
//               "selected_colors_"+stringfunc::number_to_string(frame_number)+
//               ".jpg";
//            selected_colors_texture_rectangle_ptr->write_curr_frame(
//               selected_colors_filename);
   }

// ---------------------------------------------------------------------     
// Method compute_edgemaps()
// Compute edge map.  Require strong edge content within genuine TOC12
// signs:

   twoDarray* xderiv_twoDarray_ptr=NULL;
   twoDarray* yderiv_twoDarray_ptr=NULL;
   twoDarray* gradient_mag_twoDarray_ptr=NULL;

   void compute_edgemaps(
      int curr_sign_ID,int sign_ID_start,
      SIGN_PROPERTIES& curr_sign_properties,
      SIGN_PROPERTIES& prev_sign_properties,
      texture_rectangle* edges_texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,GRADSTEP_MAP& white_gradstep_map)
   {
      bool compute_gradients_flag=false;
      if (curr_sign_ID==sign_ID_start && 
      curr_sign_properties.black_interior_flag)
      {
         edges_texture_rectangle_ptr->
            convert_color_image_to_greyscale(); 
         compute_gradients_flag=true;
      }

// Note added on 9/29/12: For blue-purple radiation sign, should very
// likely convert to greyscale using just red channel and not use
// luminosity!

      if (prev_sign_properties.black_interior_flag &&
      !curr_sign_properties.black_interior_flag)
      {
         edges_texture_rectangle_ptr->convert_color_image_to_luminosity();
         compute_gradients_flag=true;
      }

      if (curr_sign_properties.sign_hue=="black")
      {
         edges_texture_rectangle_ptr->convert_color_image_to_luminosity();
         compute_gradients_flag=true;
      }

//      string greyscale_filename=video_subdir+
//         "greyscale_"+stringfunc::number_to_string(frame_number)+".jpg";
//      edges_texture_rectangle_ptr->write_curr_frame(greyscale_filename);
            
      if (compute_gradients_flag)
      {
         twoDarray* ptwoDarray_ptr=edges_texture_rectangle_ptr->
            get_ptwoDarray_ptr();
         ptwoDarray_ptr->set_deltax(1);
         ptwoDarray_ptr->set_deltay(1);
               
         if (xderiv_twoDarray_ptr==NULL || 
         xderiv_twoDarray_ptr->get_xdim() != ptwoDarray_ptr->get_xdim() ||
         xderiv_twoDarray_ptr->get_ydim() != ptwoDarray_ptr->get_ydim())
         {
            xderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
            yderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
            gradient_mag_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         }

         const double spatial_resolution=0.25;
         imagefunc::compute_x_y_deriv_fields(
            spatial_resolution,ptwoDarray_ptr,
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
         imagefunc::compute_gradient_magnitude_field(
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
            gradient_mag_twoDarray_ptr);
         imagefunc::threshold_intensities_below_cutoff(
            gradient_mag_twoDarray_ptr,
            curr_sign_properties.min_gradient_mag,0);

//            edges_texture_rectangle_ptr->initialize_twoDarray_image(
//               gradient_mag_twoDarray_ptr,3,false);
//            string edges_filename=video_subdir+
//               "edges_"+stringfunc::number_to_string(frame_number)+".jpg";
//            edges_texture_rectangle_ptr->write_curr_frame(edges_filename);
      } // compute_gradients_flag conditional

      double step_distance=-2;
      if (curr_sign_properties.white_interior_flag) step_distance=2;

      black_gradstep_map=
         imagefunc::compute_gradient_steps(
            curr_sign_properties.min_gradient_mag,step_distance,
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
            gradient_mag_twoDarray_ptr);
      white_gradstep_map=
         imagefunc::compute_gradient_steps(
            curr_sign_properties.min_gradient_mag,-step_distance,
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
            gradient_mag_twoDarray_ptr);

//         cout << "black_gradstep_map.size() = " << black_gradstep_map.size()
//              << endl;
//         cout << "white_gradstep_map.size() = " << white_gradstep_map.size()
//              << endl;
   }
   
// ---------------------------------------------------------------------     
// Method find_hot_black_edges()

   void find_hot_black_edges(
      int xdim,vector<pair<int,int> >& black_pixels,
      GRADSTEP_MAP& black_gradstep_map,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr)
   {
      black_pixels.clear();

      for (signrecogfunc::GRADSTEP_MAP::iterator gradstep_iter=
              black_gradstep_map.begin(); 
           gradstep_iter != black_gradstep_map.end(); gradstep_iter++)
      {
         int pixel_ID=gradstep_iter->first;
         int stepped_pixel_ID=gradstep_iter->second;

         unsigned int px,py,qx,qy;
         graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
         graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);

         double stepped_h,stepped_s,stepped_v;
         quantized_texture_rectangle_ptr->get_pixel_hsv_values(
            qx,qy,stepped_h,stepped_s,stepped_v);
            
         if (stepped_v < 0.5)
         {
            black_pixels.push_back(pair<int,int>(px,py));

            if (black_grads_texture_rectangle_ptr != NULL)
            {
               int gradstep_R=255;
               int gradstep_G=0;
               int gradstep_B=255;
               black_grads_texture_rectangle_ptr->
                  set_pixel_RGB_values(px,py,gradstep_R,gradstep_G,gradstep_B);
            }
         } // stepped_v < 0.5 conditional
      } // loop over black_gradstep_map iterator

//               string black_grads_filename=video_subdir+"black_grads_"
//                  +stringfunc::number_to_string(frame_number)+".jpg";
//               cout << "black_grads_filename = " << black_grads_filename 
//                    << endl;
//               black_grads_texture_rectangle_ptr->write_curr_frame(
//                  black_grads_filename);

   }

// ---------------------------------------------------------------------     
// Method find_hot_black_edges() searches for hot edges which have
// "black"-colored pixels on their cool sides in black & white TOC12
// signs.

   void find_hot_black_edges(
      int xdim,int flood_R,int flood_G,int flood_B,
      int black_flood_R,int black_flood_G,int black_flood_B,
      GRADSTEP_MAP& black_gradstep_map,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr)
   {
      unsigned int px,py,qx,qy;
      int stepped_R,stepped_G,stepped_B;
      for (GRADSTEP_MAP::iterator black_iter=black_gradstep_map.begin(); 
           black_iter != black_gradstep_map.end(); black_iter++)
      {
         int pixel_ID=black_iter->first;
         int stepped_pixel_ID=black_iter->second;
         graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
         graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
         black_flooded_texture_rectangle_ptr->get_pixel_RGB_values(
            qx,qy,stepped_R,stepped_G,stepped_B);
         if (stepped_R==black_flood_R && stepped_G==black_flood_G && 
         stepped_B==black_flood_B)
         {
            black_grads_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,flood_R,flood_G,flood_B);
         }
      } // loop over black_gradstep_map iterator
            
//      string black_grads_filename=video_subdir+"black_grads_"
//         +stringfunc::number_to_string(frame_number)+".jpg";
//      black_grads_texture_rectangle_ptr->write_curr_frame(
//         black_grads_filename);
   }

// ---------------------------------------------------------------------     
// Method find_hot_white_edges()

   void find_hot_white_edges(
      int xdim,vector<pair<int,int> >& white_pixels,
      GRADSTEP_MAP& white_gradstep_map,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* white_grads_texture_rectangle_ptr)
   {
      white_pixels.clear();

      for (signrecogfunc::GRADSTEP_MAP::iterator gradstep_iter=
              white_gradstep_map.begin(); 
           gradstep_iter != white_gradstep_map.end(); gradstep_iter++)
      {
         int pixel_ID=gradstep_iter->first;
         int stepped_pixel_ID=gradstep_iter->second;

         unsigned int px,py,qx,qy;
         graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
         graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);

         double stepped_h,stepped_s,stepped_v;
         quantized_texture_rectangle_ptr->get_pixel_hsv_values(
            qx,qy,stepped_h,stepped_s,stepped_v);
            
         if (stepped_v > 0.5 && stepped_s < 0.3)
         {
            white_pixels.push_back(pair<int,int>(px,py));
            if (white_grads_texture_rectangle_ptr != NULL)
            {
               int gradstep_R=255;
               int gradstep_G=0;
               int gradstep_B=255;
               white_grads_texture_rectangle_ptr->
                  set_pixel_RGB_values(
                     px,py,gradstep_R,gradstep_G,gradstep_B);
            }
         } // stepped_v > 0.5 && stepped_s < 0.3 conditional
      } // loop over gradstep iterator

//      string white_grads_filename=video_subdir+"white_grads_"
//         +stringfunc::number_to_string(frame_number)+".jpg";
//      cout << "white_grads_filename = " << white_grads_filename << endl;
//      white_grads_texture_rectangle_ptr->write_curr_frame(
//         white_grads_filename);
      
   }

// ==========================================================================
// Region tests methods
// ==========================================================================

// Method dominant_hue_mismatch()

   bool dominant_hue_mismatch(
      string sign_hue,string lookup_map_name,
      extremal_region* extremal_region_ptr,RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr)
   {
      bool hue_mismatch_flag=false;
      
      if (sign_hue != "black")
      {
         int dominant_hue_index=
            RGB_analyzer_ptr->dominant_extremal_region_hue_content(
               lookup_map_name,extremal_region_ptr,
               quantized_texture_rectangle_ptr);
         string dominant_hue_name=RGB_analyzer_ptr->
            get_hue_given_index(dominant_hue_index);
//                  cout << "Dominant hue = " << dominant_hue_name 
//                       << " sign_hue = " << sign_hue << endl;

// Ignore candidate extremal region if its dominant hue does not agree
// with TOC12 ppt sign:

         if (dominant_hue_name != sign_hue) hue_mismatch_flag=true;
      } // sign_hue != black conditional

      return hue_mismatch_flag;
   }

// ---------------------------------------------------------------------     
// Method small_region_bbox()

   bool small_region_bbox(
      unsigned int left_pu,unsigned int bottom_pv,unsigned int right_pu,
      unsigned int top_pv)
   {
            
//      cout << "right_pu-left_pu = " << right_pu-left_pu 
//           << " bottom_pv-top_pv = " << bottom_pv-top_pv
//           << endl;
      bool small_region_bbox_flag=false;
      if ((right_pu-left_pu <= 10) && fabs(bottom_pv-top_pv) <=10)
      {
         small_region_bbox_flag=true;
      }
      return small_region_bbox_flag;
   }

// ---------------------------------------------------------------------     
// Method large_region_bbox()

   bool large_region_bbox(
      int left_pu,int bottom_pv,int right_pu,int top_pv,int xdim,int ydim)
   {
//      cout << "right_pu-left_pu = " << right_pu-left_pu 
//           << " bottom_pv-top_pv = " << bottom_pv-top_pv
//           << endl;
      bool large_region_bbox_flag=false;
      if ((right_pu-left_pu >= xdim-2) || fabs(bottom_pv-top_pv) >= ydim-2) 
      {
         large_region_bbox_flag=true;
      }
      return large_region_bbox_flag;
   }

// ---------------------------------------------------------------------     
// Method minimal_significant_hole_count() returns the number of
// significant holes within a candidate extremal region based upon
// inverse regions' bounding box coordinates.

   int minimal_significant_hole_count(
      unsigned int left_pu,unsigned int right_pu,
      unsigned int bottom_pv,unsigned int top_pv,
      vector<extremal_region*> inverse_extremal_region_ptrs)
   {
      const int min_inverse_pixel_area=12;
      int n_significant_holes=0;
      for (unsigned int ir=0; ir<inverse_extremal_region_ptrs.size(); ir++)
      {
         extremal_region* inverse_region_ptr=
            inverse_extremal_region_ptrs[ir];

         int inverse_pixel_area=inverse_region_ptr->get_pixel_area();
//               cout << "inverse_pixel_area = " << inverse_pixel_area
//                    << endl;
         if (inverse_pixel_area <= min_inverse_pixel_area) continue;
               
         unsigned int ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv;
         inverse_region_ptr->get_bbox(
            ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv);

//               cout << "ir_left_pu = " << ir_left_pu 
//                    << " left_pu = " << left_pu << endl;
//               cout << "ir_right_pu = " << ir_right_pu 
//                    << " right_pu = " << right_pu << endl;
//               cout << "ir_bottom_pv = " << ir_bottom_pv 
//                    << " bottom_pv = " << bottom_pv << endl;
//               cout << "ir_top_pv = " << ir_top_pv 
//                    << " top_pv = " << top_pv << endl;

         if (ir_left_pu < left_pu) continue;
         if (ir_right_pu > right_pu) continue;
         if (ir_bottom_pv < bottom_pv) continue;
         if (ir_top_pv > top_pv) continue;

         n_significant_holes++;
      } // loop over index ir labeling inverse regions

      cout << "n_significant_holes = " << n_significant_holes
           << endl;
      return n_significant_holes;
   }
   
// ---------------------------------------------------------------------     
// Method minimal_n_hole_pixels()
// Require TOC12 sign bbox to contain some minimal number of hole pixels:

   bool minimal_n_hole_pixels(
      unsigned int left_pu,unsigned int right_pu,
      unsigned int bottom_pv,unsigned int top_pv,
      const vector<pair<int,int> >& black_pixels,
      const vector<pair<int,int> >& white_pixels,
      SIGN_PROPERTIES& curr_sign_properties)
   {
      int n_hole_pixels=0;
      bounding_box curr_bbox(left_pu,right_pu,bottom_pv,top_pv);
      if (curr_sign_properties.black_interior_flag)
      {
//         cout << "black_pixels.size() = " << black_pixels.size() << endl;
         for (unsigned int p=0; p<black_pixels.size(); p++)
         {
            int px=black_pixels[p].first;
            int py=black_pixels[p].second;
            if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
         }
      } 
      else if (curr_sign_properties.white_interior_flag)
      {
         for (unsigned int p=0; p<white_pixels.size(); p++)
         {
            int px=white_pixels[p].first;
            int py=white_pixels[p].second;
            if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
         }
      } // black,white interior flag conditionals
      cout << "n_hole_pixels = " << n_hole_pixels << endl;

      bool minimal_hole_pixels_flag=true;
      if (!curr_sign_properties.purple_interior_flag && 
      n_hole_pixels < 5) minimal_hole_pixels_flag=false;

      return minimal_hole_pixels_flag;
   }
   
// ---------------------------------------------------------------------     
// Method blue_sign_purple_interior

   int n_purple_hole_pixels(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr)
   {
      int n_purple_hole_pixels=0;
      double h,s,v;
      const double min_hue=-100; // blue-purple
      const double max_hue=-20;  // red-purple
      const double min_saturation=0.4;
      const double min_value=0.5;
      for (int py=bottom_pv; py<=top_pv; py++)
      {
         for (int px=left_pu; px<=right_pu; px++)
         {
            texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
            h=basic_math::phase_to_canonical_interval(h,-180,180);
            if (h > min_hue && h < max_hue && s > min_saturation
            && v > min_value) n_purple_hole_pixels++;
         } // loop over px index
      } // loop over py index
      return n_purple_hole_pixels;
   }

// ---------------------------------------------------------------------     
// Method identify_purple_interior_pixels() counts the number of
// purple pixels inside the bounding box specified by input parameters
// left_pu, right_pu, bottom_pv & top_pv.  If the current sign is the
// blue radiation symbol, this boolean method returns false if the
// bounding box contains too few purple pixels.  Otherwise, the method
// returns false if the bounding box contains too many purple pixels.

   bool identify_purple_interior_pixels(
      string image_filename,
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties)
   {
      texture_rectangle_ptr->reset_texture_content(image_filename);

      int n_blue_pixels=0;
      int n_purple_hole_pixels=0;
      double h,s,v;
      const double min_hue=-30; // blue-purple
      const double max_hue=10;  // red-purple
      const double min_saturation=0.5;
      const double min_value=0.35;
      const double min_blue_hue=-140;
      const double max_blue_hue=-45;
      const double max_blue_value=0.7;
      const double min_blue_sat=0.15;

      for (int py=bottom_pv; py<=top_pv; py++)
      {
         for (int px=left_pu; px<=right_pu; px++)
         {
            texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
            h=basic_math::phase_to_canonical_interval(h,-180,180);
//                        cout << "px = " << px << " py = " << py 
//                             << " h = " << h << " s = " << s 
//                             << " v = " << v << endl;
            if (h > min_hue && h < max_hue && s > min_saturation
            && v > min_value) 
            {
               n_purple_hole_pixels++;
               texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,255,0,255);
            }
                        
            if (h > min_blue_hue && h < max_blue_hue &&
            v < max_blue_value && s > min_blue_sat) 
            {
               texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,0,0,255);
               n_blue_pixels++;
            }
         } // loop over px index
      } // loop over py index

//      string purple_filename=video_subdir+
//         "purple_"+stringfunc::integer_to_string(frame_number,5)
//         +".jpg";
//      texture_rectangle_ptr->write_curr_frame(purple_filename);

      double purple_frac=double(n_purple_hole_pixels)/
         ((top_pv-bottom_pv)*(right_pu-left_pu));
//      double blue_frac=double(n_blue_pixels)/
//         ((top_pv-bottom_pv)*(right_pu-left_pu));

//      cout << "n_purple_hole_pixels = " << n_purple_hole_pixels
//           << " n_blue_pixels = " << n_blue_pixels << endl;
//      cout << "purple_frac = " << purple_frac 
//           << " blue_frac = " << blue_frac << endl;

      bool valid_symbol_flag=true;
      if (curr_sign_properties.purple_interior_flag)
      {
         if (purple_frac < 0.08)
         {
//            cout << "Too small interior purple frac" << endl;
            valid_symbol_flag=false;
         }
      }
      else
      {
         if (purple_frac > 0.05)
         {
//            cout << "Too large interior purple frac" << endl;
            valid_symbol_flag=false;
         }
      }
      return valid_symbol_flag;
   }

// ---------------------------------------------------------------------     
// Method classify_dominant_colored_pixels() classifies the color for
// every pixel within the bounding box specified by input parmaeters
// left_pu, right_pu, bottom_pv and top_pv as "dominant" (e.g. orange
// for biohazard sign) or "other".  It subsequently resets every
// "other" colored pixel to the current sign's interior color.

   double classify_dominant_colored_pixels(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* selected_colors_texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties)
   {
      int R,G,B;
      int other_counter=0;
      int all_counter=0;
      for (int pv=bottom_pv; pv<=top_pv; pv++)
      {
         for (int pu=left_pu; pu<=right_pu; pu++)
         {
            selected_colors_texture_rectangle_ptr->
               get_pixel_RGB_values(pu,pv,R,G,B);

            all_counter++;
            if (R==96 && G==0 && B==96) 
            {
               other_counter++;
               if (curr_sign_properties.black_interior_flag)
               {
                  texture_rectangle_ptr->set_pixel_RGB_values(
                     pu,pv,0,0,0);
               }
               else if (curr_sign_properties.white_interior_flag)
               {
                  texture_rectangle_ptr->set_pixel_RGB_values(
                     pu,pv,255,255,255);
               }
               else if (curr_sign_properties.purple_interior_flag)
               {
                  texture_rectangle_ptr->set_pixel_RGB_values(
                     pu,pv,255,0,255);
               }
            }
                     
         } // loop over pu index
      } // loop over pv index

      double other_frac=double(other_counter)/double(all_counter);
//      cout << "other_frac = " << other_frac << endl;
      return other_frac;
   }
   
// ==========================================================================
// Ng classification methods
// ==========================================================================

// Method compute_bbox_chip_dimensions() 

   void compute_bbox_chip_dimensions(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      int& new_width,int& new_height)
   {
      int width=right_pu-left_pu+1;
      int height=fabs(bottom_pv-top_pv)+1;
      double aspect_ratio=double(width)/double(height);
//      cout << "width = " << width << " height = " << height 
//           << " aspect_ratio = " << aspect_ratio << endl;

      if (aspect_ratio > 1)
      {
         new_width=32;
         new_height=new_width/aspect_ratio;
      }
      else
      {
         new_height=32;
         new_width=aspect_ratio*new_height;
      }
//      cout << "new_height = " << new_height
//           << " new_width = " << new_width << endl;
   }

// ---------------------------------------------------------------------     
// Method generate_bbox_chip() copies current bounding box chip into
// *qtwoDarray_ptr.  It subsequently rescales the chip's size so that
// the new version stored in *qnew_twoDarray_ptr has height or width
// precisely equal to 32 pixels in size.

   void generate_bbox_chip(
      string symbol_name,int new_width,int new_height,
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr)
   {
//      cout << "inside signrecogfunc::generate_bbox_chip()" << endl;
      
// Copy TOC12 sign-dependent greyscale values into ptwoDarray_ptr
// member of *texture_rectangle_ptr:

//      texture_rectangle_ptr->refresh_ptwoDarray_ptr();
               
// Conversion from RGB color to grey-scale is TOC12 sign dependent!

      if (symbol_name=="yellow_radiation" || symbol_name=="orange_biohazard")
      {
         texture_rectangle_ptr->convert_color_image_to_greyscale(); 
      }
      else if (symbol_name=="blue_radiation")
      {
         bool generate_greyscale_image_flag=true;
         texture_rectangle_ptr->
            convert_color_image_to_single_color_channel(
               1,generate_greyscale_image_flag);    // red channel
      }
      else
      {
         texture_rectangle_ptr->convert_color_image_to_luminosity();
      }

      twoDarray* qtwoDarray_ptr=texture_rectangle_ptr->
         export_sub_twoDarray(left_pu,right_pu,bottom_pv,top_pv);
      qtwoDarray_ptr->init_coord_system(0,1,0,1);

      twoDarray* qnew_twoDarray_ptr=compositefunc::downsample(
         new_width,new_height,qtwoDarray_ptr);
      delete qtwoDarray_ptr;

/*
// For debugging purposes only, export *qnew_twoDarray_ptr as new JPG
// image chip:

      texture_rectangle* subtexture_rectangle_ptr=new
         texture_rectangle(new_width,new_height,1,3,NULL);
      subtexture_rectangle_ptr->generate_blank_image_file(
         new_width,new_height,"blank.jpg",0.5);
      bool randomize_blue_values_flag=true;
      subtexture_rectangle_ptr->
         convert_single_twoDarray_to_three_channels(
            qnew_twoDarray_ptr,randomize_blue_values_flag);
      string candidate_char_patches_subdir="./candidate_patches/";
      filefunc::dircreate(candidate_char_patches_subdir);
      string patch_filename=candidate_char_patches_subdir+
         "candidate_char_patch_"+stringfunc::integer_to_string(
            candidate_char_counter++,3)+".jpg";
      cout << "patch_filename = " << patch_filename << endl;
      subtexture_rectangle_ptr->write_curr_frame(patch_filename);
      delete subtexture_rectangle_ptr;
*/

// Need to compute K_Ng features for 8x8 patches within
// *qnew_twoDarray_ptr.  Pool features within 3x3 sectors into
// single nineK=9xK_Ng vector.  Then compute probability rescaled chip
// corresponds to text character using learned Ng probability decision
// function...

      *(texture_rectangle_ptr->get_ptwoDarray_ptr()) = *qnew_twoDarray_ptr;
      delete qnew_twoDarray_ptr;
   }
   
// ---------------------------------------------------------------------     
// Method Ng_classification_prob() 

   Ng_sample_type Ng_sample;

   double Ng_classification_prob(
      int curr_sign_ID,text_detector* text_detector_ptr,
      const vector<Ng_pfunct_type>& Ng_pfuncts)
   {
      double Ng_char_prob=0;
      bool flag=text_detector_ptr->average_window_features(0,0);
      if (flag)
      {
         float* window_histogram=text_detector_ptr->
            get_nineK_window_descriptor();
         for (int k=0; k<nineK; k++)
         {
            Ng_sample(k)=window_histogram[k];
//                        cout << "k = " << k << " window_histogram[k] = "
//                             << window_histogram[k] << endl;
         } // loop over index k labeling dictionary descriptors
         Ng_char_prob=Ng_pfuncts[curr_sign_ID](Ng_sample);
         cout << " Ng char probability = " << Ng_char_prob << endl << endl;
//               outputfunc::enter_continue_char();
      }
      else
      {
         cout << "Averaged window features computation failed!"  << endl;
      }
      return Ng_char_prob;
   }

// ==========================================================================
// User interface methods
// ==========================================================================
   
// Method generate_bbox_polygons()

   void generate_bbox_polygons(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      vector<polygon>& bbox_polygons,vector<int>& bbox_color_indices)
   {
      double left_u,top_v,right_u,bottom_v;
      texture_rectangle_ptr->get_uv_coords(left_pu,top_pv,left_u,top_v);
      texture_rectangle_ptr->get_uv_coords(
         right_pu,bottom_pv,right_u,bottom_v);

      vector<threevector> vertices;
      vertices.push_back(threevector(left_u,top_v));
      vertices.push_back(threevector(left_u,bottom_v));
      vertices.push_back(threevector(right_u,bottom_v));
      vertices.push_back(threevector(right_u,top_v));
      polygon bbox(vertices);
      bbox_polygons.push_back(bbox);

      int color_index=colorfunc::get_color_index(
         curr_sign_properties.bbox_color);
      bbox_color_indices.push_back(color_index);
   }

// ---------------------------------------------------------------------     
// Method export_bbox_polygons() draws colored rectangles around
// classified TOC12 signs.  It also annotates these boxes with the
// symbols' labels in their lower left corners.

   void export_bbox_polygons(
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* text_texture_rectangle_ptr,
      vector<polygon>& bbox_polygons,const vector<int>& bbox_color_indices,
      vector<string>& bbox_symbol_names)
   {
//      cout << "inside signrecogfunc::export_bbox_polygons()" << endl;

      unsigned int pu,pv,thickness=1;
      vector<twovector> xy_start;
      vector<colorfunc::Color> text_colors;
      for (unsigned int b=0; b<bbox_polygons.size(); b++)
      {
         videofunc::display_polygon(
            bbox_polygons[b],texture_rectangle_ptr,
            bbox_color_indices[b],thickness);
         text_colors.push_back(colorfunc::white);

         threevector right_top=bbox_polygons[b].get_vertex(1);
         threevector left_bottom=bbox_polygons[b].get_vertex(3);
         double bbox_width=right_top.get(0)-left_bottom.get(0);
         double bbox_height=right_top.get(1)-left_bottom.get(1);

//         cout << "b = " << b
//              << " bbox_polygons[b].get_vertex(1) = "
//              << bbox_polygons[b].get_vertex(1) << endl;
//         cout << "bbox_polygons[b].get_vertex(3) = "
//              << bbox_polygons[b].get_vertex(3) << endl;
//         cout << "bbox_width = " << bbox_width
//              << " bbox_height = " << bbox_height << endl;

         double u_start=left_bottom.get(0);
         u_start += 0.05*bbox_width;
         double v_start=left_bottom.get(1);
         v_start=1-v_start;
         v_start -= 0.05*bbox_height;
         texture_rectangle_ptr->get_pixel_coords(u_start,v_start,pu,pv);
         
         xy_start.push_back(twovector(pu,pv));
      }

      int fontsize=13;
//      int fontsize=15;
      cout << "Before call to annotate_image_with_text" << endl;
      videofunc::annotate_image_with_text(
         texture_rectangle_ptr,text_texture_rectangle_ptr,fontsize,
         bbox_symbol_names,xy_start,text_colors);
   }

// ---------------------------------------------------------------------     
// Method print_processing_time()

   void print_processing_time(int image_counter)
   {
      double total_time=timefunc::elapsed_timeofday_time();
      cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
           << total_time / 60.0 << " minutes" << endl;
      double avg_time_per_image=
         timefunc::elapsed_timeofday_time()/image_counter;
      cout << "***********************************************" << endl;
      cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image 
           << " secs" << " n_images = " << image_counter << endl;
      cout << "***********************************************" << endl;
   }

// ---------------------------------------------------------------------     
// Method compute_connected_components()

   void compute_connected_components(
      string binary_quantized_filename,SIGN_PROPERTIES& curr_sign_properties,
      vector<extremal_region*>& extremal_region_ptrs,
      vector<extremal_region*>& inverse_extremal_region_ptrs)
   {
//      cout << "inside signrecogfunc::compute_connected_components()" << endl;
      
      connected_components* connected_components_ptr=
         new connected_components();
      connected_components* inverse_connected_components_ptr=
         new connected_components();

      int color_channel_ID=-1;
      connected_components_ptr->reset_image(
         binary_quantized_filename,color_channel_ID,0);
      inverse_connected_components_ptr->reset_image(
         binary_quantized_filename,color_channel_ID,0);
      filefunc::deletefile(binary_quantized_filename);

      int index=0;
      int threshold=128;
      int level=threshold;
      bool RLE_flag=true;
      bool invert_binary_values_flag=false;
      bool export_connected_regions_flag=false;
//            bool export_connected_regions_flag=true;
//       int n_components=
         connected_components_ptr->compute_connected_components(
            index,threshold,level,RLE_flag,invert_binary_values_flag,
            export_connected_regions_flag);

//       int n_inverse_components=
         inverse_connected_components_ptr->compute_connected_components(
            index,threshold,level,RLE_flag,!invert_binary_values_flag,
            export_connected_regions_flag);

      extremal_region_ptrs=
         connected_components_ptr->select_extremal_regions(
            level,
            curr_sign_properties.min_aspect_ratio,
            curr_sign_properties.max_aspect_ratio,
            curr_sign_properties.min_compactness,
            curr_sign_properties.max_compactness,
            curr_sign_properties.min_n_holes,
            curr_sign_properties.max_n_holes,
            curr_sign_properties.min_n_crossings,
            curr_sign_properties.max_n_crossings);

      inverse_extremal_region_ptrs=
         inverse_connected_components_ptr->select_extremal_regions(
            level,0.1,10,0.045,10,0,10,0,10);

      delete connected_components_ptr;
      delete inverse_connected_components_ptr;
   }
   
// ==========================================================================
// Black & white sign recognition methods
// ==========================================================================

// Method flood_fill_blackish_pixels() tries to identify blackish
// pixels and then flood-filling their neighbors.  It also converts
// the flood-filled black pixels array into a binary black & white
// image:

   void flood_fill_blackish_pixels(
      int xdim,int ydim,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* binary_texture_rectangle_ptr)
   {
      int black_flood_R=128;
      int black_flood_G=0;
      int black_flood_B=128;
      black_flooded_texture_rectangle_ptr->floodfill_black_pixels(
         black_flood_R,black_flood_G,black_flood_B);
//      string black_flooded_filename=video_subdir+
//         "black_flooded_"+stringfunc::number_to_string(frame_number)+".jpg";
//      black_flooded_texture_rectangle_ptr->write_curr_frame(
//         black_flooded_filename);

// Convert flood-filled black pixels into binary image:

      for (int py=0; py<ydim; py++)
      {
         for (int px=0; px<xdim; px++)
         {
            int R,G,B;
            black_flooded_texture_rectangle_ptr->
               get_pixel_RGB_values(px,py,R,G,B);
            if (R==black_flood_R && G==black_flood_G && B==black_flood_B)
            {
               R=G=B=255;
            }
            else
            {
               R=G=B=0;
            }
            binary_texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         } // loop over px
      } // loop over py
   }

// ---------------------------------------------------------------------     
// Method compute_bright_MSERs

   void compute_bright_MSERs(
      string image_filename,extremal_regions_group& regions_group,
      texture_rectangle* edges_texture_rectangle_ptr)
   {
      regions_group.purge_dark_and_bright_regions();
      regions_group.extract_MSERs(image_filename);
      twoDarray* ptwoDarray_ptr=edges_texture_rectangle_ptr->
         get_ptwoDarray_ptr();
      regions_group.instantiate_twoDarrays(ptwoDarray_ptr);
      regions_group.update_bright_cc_twoDarray();
   }
   
// ---------------------------------------------------------------------     
// Method link_bright_MSERs_and_black_regions()
// Establish links between bright, coalesced MSERs with black borders
// and black flooded regions whose borders overlap bright MSERs:

   void link_bright_MSERs_and_black_regions(
      int xdim,int ydim,int flood_R,int flood_G,int flood_B,
      twoDarray* bright_cc_borders_twoDarray_ptr,
      twoDarray* black_cc_twoDarray_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      extremal_regions_group& regions_group)
   {
      for (int ry=0; ry<ydim; ry++)
      {
         for (int rx=0; rx<xdim; rx++)
         {
            int cc_borders_ID=bright_cc_borders_twoDarray_ptr->get(rx,ry);     
            if (cc_borders_ID <= 0) continue;

            int black_cc_ID=black_cc_twoDarray_ptr->get(rx,ry);
            if (black_cc_ID < 0) continue;

            int gr_R,gr_G,gr_B;
            black_grads_texture_rectangle_ptr->get_pixel_RGB_values(
               rx,ry,gr_R,gr_G,gr_B);
            if (gr_R==flood_R && gr_G==flood_G && gr_B==flood_B)
            {
               regions_group.add_bright_dark_neighbor_pair(
                  cc_borders_ID,black_cc_ID);
            }
         } // loop over rx 
      } // loop over ry

//      cout << "regions_group.get_n_bright_dark_region_neighbors() = "
//           << regions_group.get_n_bright_dark_region_neighbors()
//           << endl;
//      cout << "regions_group.get_n_dark_bright_region_neighbors() = "
//           << regions_group.get_n_dark_bright_region_neighbors()
//           << endl;
//      regions_group.print_bright_dark_neighbor_pairs();
//      outputfunc::enter_continue_char();
   }
   
// ---------------------------------------------------------------------     
// Method form_bright_MSER_bbox_polygons()

   void form_bright_MSER_bbox_polygons(
      int xdim,int ydim,extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* coalesced_bright_region_map_ptr,
      texture_rectangle* texture_rectangle_ptr,
      vector<polygon>& bright_bbox_polygons)
   {
      for (extremal_regions_group::ID_REGION_MAP::iterator iter=
              coalesced_bright_region_map_ptr->begin();
           iter != coalesced_bright_region_map_ptr->end(); iter++)
      {
         extremal_region* bright_extremal_region_ptr=iter->second;
               
         int bright_cc_ID=iter->first;
         bool bright_neighbor_flag=
            regions_group.get_bright_neighbor_pair_flag(bright_cc_ID);

         if (bright_cc_ID >= 1 && bright_neighbor_flag)
         {
            unsigned int left_pu,right_pu,top_pv,bottom_pv;
            bright_extremal_region_ptr->
               get_bbox(left_pu,bottom_pv,right_pu,top_pv);

//                  cout << "bright region ID=" 
//                       << bright_extremal_region_ptr->get_ID()
//                       << " left_pu=" << left_pu << " right_pu=" << right_pu
//                       << " bottom_pv=" << bottom_pv << " top_pv=" << top_pv
//                       << endl;

// Reject bright bbox if it is very small or large:

            if (signrecogfunc::small_region_bbox(
               left_pu,bottom_pv,right_pu,top_pv))
            {
               bright_extremal_region_ptr->set_bbox_polygon_ptr(NULL);
               continue;
            }

            if (signrecogfunc::large_region_bbox(
               left_pu,bottom_pv,right_pu,top_pv,xdim,ydim))
            {
               bright_extremal_region_ptr->set_bbox_polygon_ptr(NULL);
               continue;
            }

            double left_u,right_u,bottom_v,top_v;
            texture_rectangle_ptr->get_uv_coords(
               left_pu,top_pv,left_u,top_v);
            texture_rectangle_ptr->get_uv_coords(
               right_pu,bottom_pv,right_u,bottom_v);
                  
            vector<threevector> vertices;
            vertices.push_back(threevector(left_u,top_v));
            vertices.push_back(threevector(left_u,bottom_v));
            vertices.push_back(threevector(right_u,bottom_v));
            vertices.push_back(threevector(right_u,top_v));
            polygon curr_bright_bbox_polygon(vertices);
            bright_bbox_polygons.push_back(curr_bright_bbox_polygon);
            bright_extremal_region_ptr->set_bbox_polygon_ptr(
               &curr_bright_bbox_polygon);
         }
      } // loop over coalesced bright MSERs
   }
      
// ---------------------------------------------------------------------     
// Method form_flooded_black_region_bbox_polygons()

   void form_flooded_black_region_bbox_polygons(
      int xdim,int ydim,extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr,
      texture_rectangle* texture_rectangle_ptr,
      vector<polygon>& dark_bbox_polygons)
   {
      for (extremal_regions_group::ID_REGION_MAP::iterator iter=
              black_regions_map_ptr->begin();
           iter != black_regions_map_ptr->end(); iter++)
      {
         extremal_region* dark_region_ptr=iter->second;
         int dark_cc_ID=iter->first;
         bool dark_neighbor_flag=
            regions_group.get_dark_neighbor_pair_flag(dark_cc_ID);

         if (dark_cc_ID >= 1 && dark_neighbor_flag)
         {
            unsigned int left_pu,right_pu,top_pv,bottom_pv;
            dark_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);

//            cout << "dark region ID=" 
//                 << dark_region_ptr->get_ID()
//                 << " left_pu=" << left_pu << " right_pu=" << right_pu
//                 << " bottom_pv=" << bottom_pv << " top_pv=" << top_pv
//                 << endl;

// Reject dark bbox if it is very small or large:

            if (signrecogfunc::small_region_bbox(
               left_pu,bottom_pv,right_pu,top_pv))
            {
               dark_region_ptr->set_bbox_polygon_ptr(NULL);
               continue;
            }

            if (signrecogfunc::large_region_bbox(
               left_pu,bottom_pv,right_pu,top_pv,xdim,ydim))
            {
               dark_region_ptr->set_bbox_polygon_ptr(NULL);
               continue;
            }

            double left_u,right_u,bottom_v,top_v;
            texture_rectangle_ptr->get_uv_coords(
               left_pu,top_pv,left_u,top_v);
            texture_rectangle_ptr->get_uv_coords(
               right_pu,bottom_pv,right_u,bottom_v);

            vector<threevector> vertices;
            vertices.push_back(threevector(left_u,top_v));
            vertices.push_back(threevector(left_u,bottom_v));
            vertices.push_back(threevector(right_u,bottom_v));
            vertices.push_back(threevector(right_u,top_v));
            polygon curr_dark_bbox_polygon(vertices);
            dark_bbox_polygons.push_back(curr_dark_bbox_polygon);
            dark_region_ptr->set_bbox_polygon_ptr(&curr_dark_bbox_polygon);
         }
      } // loop over flooded black regions
   }
   
// ---------------------------------------------------------------------     
// Method form_flooded_black_region_bbox_polygons()

   void form_bright_MSER_and_flooded_black_region_bbox_polygons(
      int xdim,int ydim,int curr_sign_ID,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      texture_rectangle* texture_rectangle_ptr,
      vector<text_detector*>& text_detector_ptrs,
      const vector<Ng_pfunct_type>& Ng_pfuncts,
      vector<polygon>& bbox_polygons,vector<int>& bbox_color_indices)
   {
      for (extremal_regions_group::ID_REGION_MAP::iterator iter=
              black_regions_map_ptr->begin();
           iter != black_regions_map_ptr->end(); iter++)
      {
         extremal_region* dark_region_ptr=iter->second;

         if (dark_region_ptr->get_bbox_polygon_ptr()==NULL) continue;

         int dark_cc_ID=iter->first;
         bool dark_neighbor_flag=
            regions_group.get_dark_neighbor_pair_flag(dark_cc_ID);

         if (!(dark_cc_ID >= 1 && dark_neighbor_flag)) continue;

         unsigned int left_pu,right_pu,top_pv,bottom_pv;
         dark_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);

// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

         int new_width,new_height;
         signrecogfunc::compute_bbox_chip_dimensions(
            left_pu,right_pu,bottom_pv,top_pv,new_width,new_height);
         signrecogfunc::generate_bbox_chip(
            curr_sign_properties.symbol_name,new_width,new_height,
            left_pu,right_pu,bottom_pv,top_pv,texture_rectangle_ptr);

         text_detector* text_detector_ptr=
            text_detector_ptrs[curr_sign_ID];

         text_detector_ptr->set_texture_rectangle_ptr(
            texture_rectangle_ptr);
         text_detector_ptr->set_window_width(new_width);
         text_detector_ptr->set_window_height(new_height);

         double Ng_char_prob=signrecogfunc::Ng_classification_prob(
            curr_sign_ID,text_detector_ptr,Ng_pfuncts);
         if (Ng_char_prob < curr_sign_properties.Ng_threshold)
            continue;

         double left_u,right_u,bottom_v,top_v;
         texture_rectangle_ptr->get_uv_coords(
            left_pu,top_pv,left_u,top_v);
         texture_rectangle_ptr->get_uv_coords(
            right_pu,bottom_pv,right_u,bottom_v);

         vector<threevector> vertices;
         vertices.push_back(threevector(left_u,top_v));
         vertices.push_back(threevector(left_u,bottom_v));
         vertices.push_back(threevector(right_u,bottom_v));
         vertices.push_back(threevector(right_u,top_v));
         polygon bbox_polygon(vertices);
         bbox_polygons.push_back(bbox_polygon);

         int color_index=colorfunc::get_color_index(
            curr_sign_properties.bbox_color);
         bbox_color_indices.push_back(color_index);

      } // loop over adjacent bright MSER and flooded black regions

   }

// ---------------------------------------------------------------------     
// Method extract_black_connected_components()

   void extract_black_connected_components(
      string binary_quantized_filename,
      connected_components* connected_components_ptr,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr)
      {
         int color_channel_ID=-1;
         connected_components_ptr->reset_image(
            binary_quantized_filename,color_channel_ID,0);
         filefunc::deletefile(binary_quantized_filename);

         int index=0;
         int threshold=128;
         int level=threshold;
         bool RLE_flag=true;
         bool invert_binary_values_flag=false;
         bool export_connected_regions_flag=false;
//            bool export_connected_regions_flag=true;
         int n_components=
            connected_components_ptr->compute_connected_components(
               index,threshold,level,RLE_flag,invert_binary_values_flag,
               export_connected_regions_flag);
         cout << "n_components = " << n_components << endl;

         connected_components::TREENODES_MAP* treenodes_map_ptr=
            connected_components_ptr->get_treenodes_map_ptr();
         regions_group.destroy_all_regions(black_regions_map_ptr);
         for (connected_components::TREENODES_MAP::iterator treenodes_iter=
                 treenodes_map_ptr->begin(); treenodes_iter != 
                 treenodes_map_ptr->end(); treenodes_iter++)
         {
            connected_components::TREENODE_PTR treenode_ptr=
               treenodes_iter->second;
            extremal_region* extremal_region_ptr=
               treenode_ptr->get_data_ptr();
            (*black_regions_map_ptr)[extremal_region_ptr->get_ID()]=
               extremal_region_ptr;

            unsigned int min_px,min_py,max_px,max_py;
            extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

//               cout << "dark region ID=" << extremal_region_ptr->get_ID()
//                    << " min_px=" << min_px << " max_px=" << max_px
//                    << " min_py=" << min_py << " max_py=" << max_py
//                    << endl;
         }
//            cout << "black_regions_map.size() = "
//                 << black_regions_map_ptr->size() << endl;
      }
   
// ---------------------------------------------------------------------     
// Method generate_linked_bright_MSERs_and_black_regions()

   void generate_linked_bright_MSERs_and_black_regions(
      int xdim,int ydim,int black_flood_R,int black_flood_G,int black_flood_B,
      string image_filename,string binary_quantized_filename,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP*& coalesced_bright_region_map_ptr,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr)
   {
      connected_components* connected_components_ptr=
         new connected_components();

      signrecogfunc::extract_black_connected_components(
         binary_quantized_filename,connected_components_ptr,
         regions_group,black_regions_map_ptr);

// Search for hot edges which have "black"-colored pixels on their
// cool sides:   

      int flood_R=255;
      int flood_G=0;
      int flood_B=255;
      signrecogfunc::find_hot_black_edges(
         xdim,flood_R,flood_G,flood_B,
         black_flood_R,black_flood_G,black_flood_B,
         black_gradstep_map,black_flooded_texture_rectangle_ptr,
         black_grads_texture_rectangle_ptr);
            
// Compute locally bright MSERs:

      signrecogfunc::compute_bright_MSERs(
         image_filename,regions_group,edges_texture_rectangle_ptr);

// Coalesce touching bright MSERs:

      coalesced_bright_region_map_ptr=
         regions_group.coalesce_bright_touching_regions();

/*
// Print bounding boxes around each bright, coalesced MSER region:

      for (extremal_regions_group::ID_REGION_MAP::iterator iter=
              coalesced_bright_region_map_ptr->begin();
           iter != coalesced_bright_region_map_ptr->end();
           iter++)
      {
         extremal_region* extremal_region_ptr=iter->second;
         unsigned int min_px,min_py,max_px,max_py;
         extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

//               cout << "bright region ID=" << extremal_region_ptr->get_ID()
//                    << " min_px=" << min_px << " max_px=" << max_px
//                    << " min_py=" << min_py << " max_py=" << max_py
//                    << endl;
      }
*/

// Identify border pixels around each coalesced bright MSER & store
// their extremal region IDs within *bright_cc_borders_twoDarray_ptr:

//      cout << "Identifying border pixels around coalesced bright MSERs:" 
//           << endl;
      int border_thickness=3;
      regions_group.identify_bright_border_pixels(
         coalesced_bright_region_map_ptr,border_thickness);
      twoDarray* bright_cc_borders_twoDarray_ptr=
         regions_group.get_bright_cc_borders_twoDarray_ptr();

// Establish links between bright, coalesced MSERs with black borders
// and black flooded regions whose borders overlap bright MSERs:

      twoDarray* black_cc_twoDarray_ptr=
         connected_components_ptr->get_cc_twoDarray_ptr();
      signrecogfunc::link_bright_MSERs_and_black_regions(
         xdim,ydim,flood_R,flood_G,flood_B,
         bright_cc_borders_twoDarray_ptr,black_cc_twoDarray_ptr,
         black_grads_texture_rectangle_ptr,regions_group);
      
      delete connected_components_ptr;
   }
   
// ---------------------------------------------------------------------     
// Method merge_bright_MSER_and_flooded_black_region_bboxes()

   void merge_bright_MSER_and_flooded_black_region_bboxes(
      int xdim,int ydim,
      string image_filename,string binary_quantized_filename,
      texture_rectangle* binary_texture_rectangle_ptr,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr)
   {

// Try identifying blackish pixels and then flood-filling their
// neighbors:

      signrecogfunc::flood_fill_blackish_pixels(
         xdim,ydim,black_flooded_texture_rectangle_ptr,
         binary_texture_rectangle_ptr);

      int black_flood_R=128;
      int black_flood_G=0;
      int black_flood_B=128;

      binary_texture_rectangle_ptr->write_curr_frame(
         binary_quantized_filename);

// Extract connected components from flood-filled black blobs image in
// *connected_components_ptr:

      extremal_regions_group::ID_REGION_MAP* coalesced_bright_region_map_ptr=
         NULL;

      signrecogfunc::generate_linked_bright_MSERs_and_black_regions(
         xdim,ydim,black_flood_R,black_flood_G,black_flood_B,
         image_filename,binary_quantized_filename,
         black_flooded_texture_rectangle_ptr,
         black_grads_texture_rectangle_ptr,
         edges_texture_rectangle_ptr,
         black_gradstep_map,regions_group,
         coalesced_bright_region_map_ptr,black_regions_map_ptr);

// Form bounding box polygons around coalesced, bright MSERs which are
// adjacent to flooded black regions:
            
      vector<polygon> bright_bbox_polygons;
      signrecogfunc::form_bright_MSER_bbox_polygons(
         xdim,ydim,regions_group,coalesced_bright_region_map_ptr,
         texture_rectangle_ptr,bright_bbox_polygons);

// Form bounding box polygons around flooded black regions which are
// adjacent to coalesced, bright MSERs:

      vector<polygon> dark_bbox_polygons;
      signrecogfunc::form_flooded_black_region_bbox_polygons(
         xdim,ydim,regions_group,black_regions_map_ptr,
         texture_rectangle_ptr,dark_bbox_polygons);

// Coalesce adjacent black and bright regions' bounding boxes:

      regions_group.merge_adjacent_dark_bright_bboxes(
         black_regions_map_ptr,coalesced_bright_region_map_ptr);

      delete coalesced_bright_region_map_ptr;
   }

   
// ==========================================================================
// High-level colored TOC12 sign recognition methods
// ==========================================================================

   void find_hot_edges(
      int xdim,SIGN_PROPERTIES& curr_sign_properties,
      vector<pair<int,int> >& black_pixels,
      vector<pair<int,int> >& white_pixels,
      GRADSTEP_MAP& black_gradstep_map,GRADSTEP_MAP& white_gradstep_map,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* white_grads_texture_rectangle_ptr)
   {
   
      if (curr_sign_properties.black_interior_flag)
      {

// Search for hot edges which have "black"-colored pixels on their
// cool sides:

         signrecogfunc::find_hot_black_edges(
            xdim,black_pixels,black_gradstep_map,
            quantized_texture_rectangle_ptr,
            black_grads_texture_rectangle_ptr);
      }
      else if (curr_sign_properties.white_interior_flag)
      {

// Search for hot edges which have "white"-colored pixels on their
// warm sides:

         signrecogfunc::find_hot_white_edges(
            xdim,white_pixels,white_gradstep_map,
            quantized_texture_rectangle_ptr,
            white_grads_texture_rectangle_ptr);

      } // black_interior_flag and white_interior_flag conditionals
   }

// ---------------------------------------------------------------------     
// Method form_colored_sign_bbox_polygons()

   void form_colored_sign_bbox_polygons(
      int curr_sign_ID,
      vector<pair<int,int> >& black_pixels,
      vector<pair<int,int> >& white_pixels,
      vector<extremal_region*>& extremal_region_ptrs,
      vector<extremal_region*>& inverse_extremal_region_ptrs,
      SIGN_PROPERTIES& curr_sign_properties,
      RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      vector<text_detector*>& text_detector_ptrs,
      const vector<Ng_pfunct_type>& Ng_pfuncts,
      vector<polygon>& bbox_polygons,vector<int>& bbox_color_indices)
   {
   
// Loop over current sign extremal region candidates starts here:

      string sign_hue=curr_sign_properties.sign_hue;
      for (unsigned int r=0; r<extremal_region_ptrs.size(); r++)
      {
         extremal_region* extremal_region_ptr=extremal_region_ptrs[r];
//               cout << "r = " << r 
//                    << " extremal region = " << *extremal_region_ptr 
//                    << endl;

//               cout << "sign_hue = " << sign_hue << endl;
         if (sign_hue=="blue2") sign_hue="blue";

// Reject candidate extremal region if its dominant hue does not agree
// with TOC12 ppt sign:

         if (signrecogfunc::dominant_hue_mismatch(
            sign_hue,curr_sign_properties.sign_hue,
            extremal_region_ptr,RGB_analyzer_ptr,
            quantized_texture_rectangle_ptr)) continue;

// Reject candidate extremal region if its bbox is very small:
          
         unsigned int left_pu,bottom_pv,right_pu,top_pv;
         extremal_region_ptr->get_bbox(
            left_pu,bottom_pv,right_pu,top_pv);

         if (signrecogfunc::small_region_bbox(
            left_pu,bottom_pv,right_pu,top_pv)) continue;

// Next count number of significant holes within extremal region
// based upon inverse regions' bounding box coordinates:

         int n_significant_holes=
            signrecogfunc::minimal_significant_hole_count(
               left_pu,right_pu,bottom_pv,top_pv,
               inverse_extremal_region_ptrs);

         if (n_significant_holes < 
         curr_sign_properties.min_n_significant_holes ||
         n_significant_holes > 
         curr_sign_properties.max_n_significant_holes) continue;

// Require TOC12 sign bbox to contain some minimal number of hole
// pixels:

         if (!signrecogfunc::minimal_n_hole_pixels(
            left_pu,right_pu,bottom_pv,top_pv,
            black_pixels,white_pixels,curr_sign_properties)) continue;

// Check blue signs for purple interior content:

         if (sign_hue=="blue")
         {
            int n_purple_hole_pixels=
               signrecogfunc::n_purple_hole_pixels(
                  left_pu,right_pu,bottom_pv,top_pv,texture_rectangle_ptr);

            if (curr_sign_properties.purple_interior_flag)
            {
               if (n_purple_hole_pixels < 5) continue;
            }
            else
            {

// Note added on Weds, oct 17 at 1 pm: This next conditional seems
// dangerous !!!

               if (n_purple_hole_pixels > 10) continue;
            }
         } // sign_hue==blue conditional
   
// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

         int new_width,new_height;
         signrecogfunc::compute_bbox_chip_dimensions(
            left_pu,right_pu,bottom_pv,top_pv,new_width,new_height);
         signrecogfunc::generate_bbox_chip(
            curr_sign_properties.symbol_name,new_width,new_height,
            left_pu,right_pu,bottom_pv,top_pv,texture_rectangle_ptr);

// Need to compute K_Ng features for 8x8 patches within
// *qnew_twoDarray_ptr.  Pool features within 3x3 sectors into
// single nineK=9xK_Ng vector.  Then compute probability rescaled chip
// corresponds to text character using learned Ng probability decision
// function...

         text_detector* text_detector_ptr=text_detector_ptrs[curr_sign_ID];
         text_detector_ptr->set_texture_rectangle_ptr(texture_rectangle_ptr);
         text_detector_ptr->set_window_width(new_width);
         text_detector_ptr->set_window_height(new_height);

         double Ng_char_prob=signrecogfunc::Ng_classification_prob(
            curr_sign_ID,text_detector_ptr,Ng_pfuncts);
         if (Ng_char_prob < curr_sign_properties.Ng_threshold)
            continue;

         signrecogfunc::generate_bbox_polygons(
            left_pu,right_pu,bottom_pv,top_pv,
            texture_rectangle_ptr,curr_sign_properties,
            bbox_polygons,bbox_color_indices);

      } // loop over index r labeling selected extremal regions
   }

// ---------------------------------------------------------------------     
// This next version of form_colored_sign_bbox_polygons() is intended
// to be used for PointGrey camera imagery of colored TOC12 signs
// gathered within the Tennis Bubble on HAFB.

   void form_colored_sign_bbox_polygons(
      int curr_sign_ID,string symbol_name,string image_filename,
      vector<extremal_region*>& extremal_region_ptrs,
      vector<extremal_region*>& inverse_extremal_region_ptrs,
      SIGN_PROPERTIES& curr_sign_properties,
      RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* selected_colors_texture_rectangle_ptr,
      vector<text_detector*>& text_detector_ptrs,
      const vector<Ng_pfunct_type>& Ng_pfuncts,
      vector<polygon>& bbox_polygons,vector<int>& bbox_color_indices,
      vector<string>& bbox_symbol_names)
   {

// Loop over current sign extremal region candidates starts here:

      string sign_hue=curr_sign_properties.sign_hue;
      for (unsigned int r=0; r<extremal_region_ptrs.size(); r++)
      {
         extremal_region* extremal_region_ptr=extremal_region_ptrs[r];
//               cout << "r = " << r 
//                    << " extremal region = " << *extremal_region_ptr 
//                    << endl;

//               cout << "sign_hue = " << sign_hue << endl;


// Reject candidate extremal region if its dominant hue does not agree
// with TOC12 ppt sign:

         if (signrecogfunc::dominant_hue_mismatch(
            sign_hue,curr_sign_properties.sign_hue,
            extremal_region_ptr,RGB_analyzer_ptr,
            quantized_texture_rectangle_ptr)) continue;
            
         unsigned int left_pu,bottom_pv,right_pu,top_pv;
         extremal_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);
            
         if (signrecogfunc::small_region_bbox(
            left_pu,bottom_pv,right_pu,top_pv)) continue;

// Next count number of significant holes within extremal region
// based upon inverse regions' bounding box coordinates:

         int n_significant_holes=
            signrecogfunc::minimal_significant_hole_count(
               left_pu,right_pu,bottom_pv,top_pv,
               inverse_extremal_region_ptrs);
//         cout << "n_significant_holes = " << n_significant_holes << endl;

         if (n_significant_holes < 
         curr_sign_properties.min_n_significant_holes || n_significant_holes > 
         curr_sign_properties.max_n_significant_holes) continue;

// Check blue signs for purple interior content.  Ignore blue regions
// corresponding to blue_radiation sign if they do not have enough
// purple content.  Ignore blue regions corresponding to blue water
// and gas signs if they have too much purple content:

         if (sign_hue=="blue")
         {
            if (!signrecogfunc::identify_purple_interior_pixels(
               image_filename,left_pu,right_pu,bottom_pv,top_pv,
               texture_rectangle_ptr,curr_sign_properties)) continue;
         }
               
// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

         int new_width,new_height;
         signrecogfunc::compute_bbox_chip_dimensions(
            left_pu,right_pu,bottom_pv,top_pv,new_width,new_height);

// Classify every pixel's color as "dominant" (e.g. orange for
// biohazard sign) or "other".  Then reset all "other" colored pixels
// to current sign's interior color:

         signrecogfunc::classify_dominant_colored_pixels(
            left_pu,right_pu,bottom_pv,top_pv,
            texture_rectangle_ptr,selected_colors_texture_rectangle_ptr,
            curr_sign_properties);
           
         signrecogfunc::generate_bbox_chip(
            curr_sign_properties.symbol_name,new_width,new_height,
            left_pu,right_pu,bottom_pv,top_pv,texture_rectangle_ptr);

         text_detector* text_detector_ptr=
            text_detector_ptrs[curr_sign_ID];

         text_detector_ptr->set_texture_rectangle_ptr(
            texture_rectangle_ptr);
         text_detector_ptr->set_window_width(new_width);
         text_detector_ptr->set_window_height(new_height);

         double Ng_char_prob=signrecogfunc::Ng_classification_prob(
            curr_sign_ID,text_detector_ptr,Ng_pfuncts);

         if (Ng_char_prob < curr_sign_properties.Ng_threshold) continue;

         signrecogfunc::generate_bbox_polygons(
            left_pu,right_pu,bottom_pv,top_pv,
            texture_rectangle_ptr,curr_sign_properties,
            bbox_polygons,bbox_color_indices);

         bbox_symbol_names.push_back(symbol_name);
      } // loop over index r labeling selected extremal regions

   }

// ==========================================================================
// 3D relative position methods
// ==========================================================================

// Method compute_relative_bbox_position() takes in the separation
// distance between opposite corners of a 2D bounding box measured in
// 3D space.  For TOC12 signs, this diagonal distance = 1.724 meters.
// For the 17"x22" tank sign, the diagonal distance between opposite
// centers of the colored cells = 0.289 meters.

// It computes the 3D rays corresponding to the bounding box's
// opposite corners.  This method computes the range to the TOC12 sign
// which we approximate as equaling the separation distance divided by
// the angle between the two 3D rays.  It sets the TOC12 sign's
// position relative to the camera equal to the range multiplied by
// the average of the two opposite corners' 3D rays.

   threevector compute_relative_bbox_position(
      camera* camera_ptr,double diagonal_corner_separation,
      polygon& bbox_polygon)
   {
//      cout << "inside signrecogfunc::compute_relative_bbox_position()" << endl;
      twovector uv0=bbox_polygon.get_vertex(0);
      twovector uv1=bbox_polygon.get_vertex(1);
      twovector uv2=bbox_polygon.get_vertex(2);
      twovector uv3=bbox_polygon.get_vertex(3);
      threevector rhat_0=camera_ptr->pixel_ray_direction(uv0);
      threevector rhat_1=camera_ptr->pixel_ray_direction(uv1);
      threevector rhat_2=camera_ptr->pixel_ray_direction(uv2);
      threevector rhat_3=camera_ptr->pixel_ray_direction(uv3);
      threevector avg_rhat=0.25*(rhat_0+rhat_1+rhat_2+rhat_3);

      double cos_theta02=rhat_0.dot(rhat_2);
      double cos_theta13=rhat_1.dot(rhat_3);
      double theta02=acos(cos_theta02);
      double theta13=acos(cos_theta13);
      double range02=diagonal_corner_separation/theta02;
      double range13=diagonal_corner_separation/theta13;
      double avg_range=0.5*(range02+range13);
//       cout << " range = " << avg_range << endl;

      threevector rel_bbox_posn=avg_range*avg_rhat;
      return rel_bbox_posn;
   }

   vector<threevector> compute_relative_bbox_positions(
      camera* camera_ptr,double diagonal_corner_separation,
      vector<polygon>& bbox_polygons)
   {
//      cout << "inside signrecogfunc::compute_relative_bbox_positions()" << endl;
      vector<threevector> bbox_posns_rel_to_camera;
      for (unsigned int b=0; b<bbox_polygons.size(); b++)
      {
//         cout << "b = " << b << endl;
//         cout << "bbox_polygon = " << bbox_polygons[b] << endl;

         twovector uv0=bbox_polygons[b].get_vertex(0);
         twovector uv2=bbox_polygons[b].get_vertex(2);
         threevector rhat_0=camera_ptr->pixel_ray_direction(uv0);
         threevector rhat_2=camera_ptr->pixel_ray_direction(uv2);
         threevector avg_rhat=0.5*(rhat_0+rhat_2);

         double cos_theta=rhat_0.dot(rhat_2);
         double theta=acos(cos_theta);
         double range=diagonal_corner_separation/theta;

         threevector rel_bbox_posn=range*avg_rhat;
         bbox_posns_rel_to_camera.push_back(rel_bbox_posn);
//         cout << "theta = " << theta*180/PI 
//              << " range = " << range << endl;
      }
      return bbox_posns_rel_to_camera;
   }

// ==========================================================================
// Tank sign recognition methods
// ==========================================================================

// Method compute_connected_components_for_tank_sign_image()

   void compute_connected_components_for_tank_sign_image(
      int xdim,int ydim,string image_filename,
      extremal_regions_group::ID_REGION_MAP* bright_regions_map_ptr)
   {
//      cout << "inside signrecogfunc::compute_connected_components_for_tank_sign()" << endl;
      
      connected_components* connected_components_ptr=
         new connected_components();
      int color_channel_ID=-1;
      connected_components_ptr->reset_image(image_filename,color_channel_ID,0);

      int index=0;
      int threshold=60;
      int level=threshold;
      bool RLE_flag=true;
      bool invert_binary_values_flag=false;
      bool export_connected_regions_flag=false;
//      bool export_connected_regions_flag=true;
//       int n_components=
         connected_components_ptr->compute_connected_components(
            index,threshold,level,RLE_flag,invert_binary_values_flag,
            export_connected_regions_flag);
//      cout << "n_components = " << n_components << endl;
      twoDarray* cc_twoDarray_ptr=
         connected_components_ptr->get_cc_twoDarray_ptr();
      cc_twoDarray_ptr->clear_values();

//      connected_components::TREE_PTR tree_ptr=connected_components_ptr->
//         get_tree_ptr();
//      cout << "tree.size() = " << tree_ptr->size() << endl;

      connected_components::TREENODES_MAP* treenodes_map_ptr=
         connected_components_ptr->get_treenodes_map_ptr();
      for (connected_components::TREENODES_MAP::iterator treenodes_iter=
              treenodes_map_ptr->begin(); treenodes_iter != 
              treenodes_map_ptr->end(); treenodes_iter++)
      {
         connected_components::TREENODE_PTR treenode_ptr=
            treenodes_iter->second;
         extremal_region* extremal_region_ptr=treenode_ptr->get_data_ptr();

// Ignore any bright region whose pixel size is very large or small:

         int pixel_area=extremal_region_ptr->get_pixel_area();
         double frac_area=double(pixel_area)/double(xdim*ydim);

         const double min_frac_area=50.0/(xdim*ydim);
         const double max_frac_area=0.1;
         if (frac_area < min_frac_area) continue;
         if (frac_area > max_frac_area) continue;

         (*bright_regions_map_ptr)[extremal_region_ptr->get_ID()]=
            extremal_region_ptr;

         unsigned int min_px,min_py,max_px,max_py;
         extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);
         extremal_region_ptr->run_length_decode(cc_twoDarray_ptr);

//         cout << "bright region ID=" << extremal_region_ptr->get_ID()
//              << " min_px=" << min_px << " max_px=" << max_px
//              << " min_py=" << min_py << " max_py=" << max_py
//              << " frac area = " << frac_area
//              << endl;
      }
//      cout << "treenodes_map_ptr->size() = "
//           << treenodes_map_ptr->size() << endl;
//      cout << "bright_regions_map.size() = "
//           << bright_regions_map_ptr->size() << endl;
      
//      string reduced_components_filename=output_subdir+
//         +"reduced_components_"+stringfunc::integer_to_string(frame_number,5)
 //        +".jpg";
//      connected_components_ptr->color_connected_components(
//         reduced_components_filename);

      delete connected_components_ptr;
   }
   
// ---------------------------------------------------------------------     
   void count_colored_pixels(
      int pu,int pv,texture_rectangle* texture_rectangle_ptr,
      int& n_red_pixels,int& n_yellow_pixels,int& n_cyan_pixels,
      int& n_purple_pixels,
      int& red_pu,int& red_pv,int& yellow_pu,int& yellow_pv,
      int& cyan_pu,int& cyan_pv,int& purple_pu,int& purple_pv)
   {
      double h,s,v;
      texture_rectangle_ptr->get_pixel_hsv_values(pu,pv,h,s,v);
      double red_hue=basic_math::phase_to_canonical_interval(
         h,-180,180);

      if (v > 0.45 && s > 0.7 && red_hue > 0 && red_hue < 15)
      {
         red_pu += pu;
         red_pv += pv;
         n_red_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,128,0,0);
      }
      else if (v > 0.35 && s > 0.35 && red_hue > -20 && red_hue < 7)
      {
         purple_pu += pu;
         purple_pv += pv;
         n_purple_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,128,0,128);
      }

/*
      if (v > 0.2 && s > 0.35 && h > 110 && h < 140)    
      {
         green_pu += pu;
         green_pv += pv;
         n_green_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,0,128,0);
      }
      if (v > 0.19 && s > 0.35 && h > 210 && h < 230) 
      {
         blue_pu += pu;
         blue_pv += pv;
         n_blue_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,0,0,128);
      }
*/

      if (v > 0.6 && s > 0.5 && h > 35 && h < 65)	
      {
         yellow_pu += pu;
         yellow_pv += pv;
         n_yellow_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,128,128,0);
      }
      if (v > 0.35 && s > 0.07 && h > 130 && h < 190)   
      {
         cyan_pu += pu;
         cyan_pv += pv;
         n_cyan_pixels++;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,0,128,128);
      }
   }

// ---------------------------------------------------------------------  
// Method draw_colored_cell_COMs()

   void draw_colored_cell_COMs(
      const twovector& red_COM,const twovector& yellow_COM,
      const twovector& cyan_COM,const twovector& purple_COM,
      texture_rectangle* texture_rectangle_ptr,
      vector<polygon>& RYCP_polygons)
   {
//       cout << "Drawing COMs for RYCP regions:" << endl;

      double u,v;
      vector<threevector> red_vertices,green_vertices,blue_vertices;
      vector<threevector> yellow_vertices,cyan_vertices,purple_vertices;
      const int n_sides=4;
      const double d_theta=2*PI/n_sides;
      const double radius=10;	// pixels
      for (int n=0; n<n_sides; n++)
      {
         double theta=n*d_theta;
         twovector r_hat(cos(theta),sin(theta));
         threevector red_vertex(red_COM+radius*r_hat);
         threevector yellow_vertex(yellow_COM+radius*r_hat);
         threevector cyan_vertex(cyan_COM+radius*r_hat);
         threevector purple_vertex(purple_COM+radius*r_hat);
            
         texture_rectangle_ptr->get_uv_coords(
            red_vertex.get(0),red_vertex.get(1),u,v);
         red_vertex.put(0,u);
         red_vertex.put(1,v);
         red_vertices.push_back(red_vertex);

         texture_rectangle_ptr->get_uv_coords(
            yellow_vertex.get(0),yellow_vertex.get(1),u,v);
         yellow_vertex.put(0,u);
         yellow_vertex.put(1,v);
         yellow_vertices.push_back(yellow_vertex);

         texture_rectangle_ptr->get_uv_coords(
            cyan_vertex.get(0),cyan_vertex.get(1),u,v);
         cyan_vertex.put(0,u);
         cyan_vertex.put(1,v);
         cyan_vertices.push_back(cyan_vertex);

         texture_rectangle_ptr->get_uv_coords(
            purple_vertex.get(0),purple_vertex.get(1),u,v);
         purple_vertex.put(0,u);
         purple_vertex.put(1,v);
         purple_vertices.push_back(purple_vertex);
      }

      polygon red_polygon(red_vertices);
      threevector red_polygon_COM=red_polygon.compute_COM();

      polygon yellow_polygon(yellow_vertices);
      threevector yellow_polygon_COM=yellow_polygon.compute_COM();

      polygon cyan_polygon(cyan_vertices);
      threevector cyan_polygon_COM=cyan_polygon.compute_COM();

      polygon purple_polygon(purple_vertices);
      threevector purple_polygon_COM=purple_polygon.compute_COM();

      RYCP_polygons.push_back(red_polygon);
      RYCP_polygons.push_back(yellow_polygon);
      RYCP_polygons.push_back(cyan_polygon);
      RYCP_polygons.push_back(purple_polygon);
   }

// ---------------------------------------------------------------------  
// Method compute_tank_posn_rel_to_camera()

   void compute_tank_posn_rel_to_camera(
      vector<polygon>& RYCP_polygons,camera* camera_ptr,
      threevector& tank_posn_rel_to_camera)
   {
//      cout << "inside signrecogfunc::compute_tank_posn_rel_to_camera()"
//           << endl;
      
      threevector red_polygon_COM=RYCP_polygons[0].compute_COM();
      threevector yellow_polygon_COM=RYCP_polygons[1].compute_COM();
      threevector cyan_polygon_COM=RYCP_polygons[2].compute_COM();
      threevector purple_polygon_COM=RYCP_polygons[3].compute_COM();

      vector<threevector> COM_polygon_vertices;
      COM_polygon_vertices.push_back(red_polygon_COM);
      COM_polygon_vertices.push_back(yellow_polygon_COM);
      COM_polygon_vertices.push_back(purple_polygon_COM);
      COM_polygon_vertices.push_back(cyan_polygon_COM);
      polygon COM_polygon(COM_polygon_vertices);
//      cout << "COM_polygon.compute_COM() = "
//           << COM_polygon.compute_COM() << endl;

      const double tank_sign_diagonal_distance=0.289; // meter
      tank_posn_rel_to_camera=
         signrecogfunc::compute_relative_bbox_position(
            camera_ptr,tank_sign_diagonal_distance,COM_polygon);
//      cout << "tank_posn_rel_to_camera#1 = " << tank_posn_rel_to_camera
//           << endl;
   }

// ---------------------------------------------------------------------  
// Method search_for_colored_checkerboard_in_bright_region()

   bool search_for_colored_checkerboard_in_bright_region(
      int xdim,extremal_region* region_ptr,
      vector<polygon>& RYCP_polygons,camera* camera_ptr,
      texture_rectangle* texture_rectangle_ptr,
      threevector& tank_posn_rel_to_camera)
   {
//      cout << "inside signrecogfunc::search_for_colored_checkerboard_in_bright_region()" << endl;
      
      vector<pair<int,int> > pixel_pairs=region_ptr->run_length_decode(xdim);

      int n_red_pixels=0;
      int n_yellow_pixels=0;
      int n_cyan_pixels=0;
      int n_purple_pixels=0;
      int red_pu,red_pv,green_pu,green_pv,blue_pu,blue_pv;
      int yellow_pu,yellow_pv,cyan_pu,cyan_pv,purple_pu,purple_pv;
      red_pu=red_pv=green_pu=green_pv=blue_pu=blue_pv=0;
      yellow_pu=yellow_pv=cyan_pu=cyan_pv=purple_pu=purple_pv=0;
            
      for (unsigned int j=0; j<pixel_pairs.size(); j++)
      {
         int pu=pixel_pairs[j].first;
         int pv=pixel_pairs[j].second;

         signrecogfunc::count_colored_pixels(
            pu,pv,texture_rectangle_ptr,
            n_red_pixels,n_yellow_pixels,n_cyan_pixels,n_purple_pixels,
            red_pu,red_pv,yellow_pu,yellow_pv,
            cyan_pu,cyan_pv,purple_pu,purple_pv);
      } // loop over index j labeling pixels within extremal region

      int n_YCP_pixels=n_yellow_pixels+n_cyan_pixels+n_purple_pixels;
      int n_RYCP_pixels=n_red_pixels+n_YCP_pixels;

      double R_frac=double(n_red_pixels)/double(n_RYCP_pixels);
      double Y_frac=double(n_yellow_pixels)/double(n_RYCP_pixels);
      double C_frac=double(n_cyan_pixels)/double(n_RYCP_pixels);
      double P_frac=double(n_purple_pixels)/double(n_RYCP_pixels);

      if (n_red_pixels < 10) return false;
      if (n_yellow_pixels < 10) return false;
      if (n_cyan_pixels < 10) return false;
      if (n_purple_pixels < 10) return false;

      const double min_color_frac=0.08;
      if (R_frac < min_color_frac) return false;
      if (Y_frac < min_color_frac) return false;
      if (C_frac < min_color_frac) return false;
      if (P_frac < min_color_frac) return false;

      const double max_color_frac=0.46;
      if (R_frac > max_color_frac) return false;
      if (Y_frac > max_color_frac) return false;
      if (C_frac > max_color_frac) return false;
      if (P_frac > max_color_frac) return false;

//      cout << "n_red_pixels = " << n_red_pixels << endl;
//      cout << "n_yellow_pixels = " << n_yellow_pixels << endl;
//      cout << "n_cyan_pixels = " << n_cyan_pixels << endl;
//      cout << "n_purple_pixels = " << n_purple_pixels << endl << endl;

//      cout << "R_frac = " << R_frac
//           << " Y_frac = " << Y_frac
//           << " C_frac = " << C_frac
//           << " P_frac = " << P_frac
//           << endl;

//         cout << "Exporting RYCP cells" << endl;
//         string RYCP_filename=output_subdir
//            +"RYCP_cells_"+stringfunc::integer_to_string(frame_number,5)
//            +".jpg";
//         texture_rectangle_ptr->write_curr_frame(RYCP_filename);

      double red_COM_u=double(red_pu)/double(n_red_pixels);
      double red_COM_v=double(red_pv)/double(n_red_pixels);
      twovector red_COM(red_COM_u,red_COM_v);
      double yellow_COM_u=double(yellow_pu)/double(n_yellow_pixels);
      double yellow_COM_v=double(yellow_pv)/double(n_yellow_pixels);
      double cyan_COM_u=double(cyan_pu)/double(n_cyan_pixels);
      double cyan_COM_v=double(cyan_pv)/double(n_cyan_pixels);
      double purple_COM_u=double(purple_pu)/double(n_purple_pixels);
      double purple_COM_v=double(purple_pv)/double(n_purple_pixels);
      twovector yellow_COM(yellow_COM_u,yellow_COM_v);
      twovector cyan_COM(cyan_COM_u,cyan_COM_v);
      twovector purple_COM(purple_COM_u,purple_COM_v);

      signrecogfunc::draw_colored_cell_COMs(
         red_COM,yellow_COM,cyan_COM,purple_COM,
         texture_rectangle_ptr,RYCP_polygons);

      signrecogfunc::compute_tank_posn_rel_to_camera(
         RYCP_polygons,camera_ptr,tank_posn_rel_to_camera);
//      cout << "tank_posn_rel_to_camera#2 = " << tank_posn_rel_to_camera
//           << endl;

      return true;
   }
   
} // signrecogfunc namespace
