// =======================================================================
// Program SEPARATED_PACKAGES imports an entire set of video frames
// for some flight.  It then queries the user to enter the maximum
// permissible angular separation between subsampled frames which will
// be bundle adjusted.  On 5/7/13, we empirically found that setting
// this angular threhsold to 5 degs yielded quite good reconstruction
// results for GEO pass 20120105_1402!  SEPARATED_PACKAGES exports
// executable scripts for programs RESTRICTED_ASIFT, TRIANGULATE,
// GPSFIT & VIDEO_PROPAGATOR.  It also outputs the image filenames
// corresponding to those packages to bundler_IO_subdir/list_tmp.txt.
// =======================================================================
// Last updated on 8/26/13; 8/28/13; 9/11/13; 9/13/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

#include "numrec/nrfuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   string thresholded_pts_filename=bundler_IO_subdir+
      "thresholded_xyz_points.osga";
   string densecloud_filename=bundler_IO_subdir+
      "dense_cloud.osga";
   string reconstructed_camera_posns_filename=bundler_IO_subdir+
      "reconstructed_camera_posns.osga";
   string GPS_camera_posns_filename=bundler_IO_subdir+
      "GPS_camera_path.osga";
   string packages_subdir=bundler_IO_subdir+"packages/";
   string hardware_packages_subdir=packages_subdir+"hardware_in/";

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
   }

   double theta_threshold=5*PI/180;
   cout << "Enter maximum angle (degs) difference between separated hardware package files:" << endl;
//   cin >> theta_threshold;
   cout << "Max angle differences between separated hardware package files = "
        << theta_threshold*180/PI << endl;
//   theta_threshold *= PI/180;

   string restricted_asift_scriptfilename=
      bundler_IO_subdir+"run_restricted_asift";
   string triangulate_scriptfilename=bundler_IO_subdir+"run_triangulate";
   string gpsfit_scriptfilename=bundler_IO_subdir+"run_gpsfit";
   string videoprop_scriptfilename=bundler_IO_subdir+"RUN_video_propagator";
   string orthorectify_scriptfilename=bundler_IO_subdir+"RUN_orthorectify";

   string list_filename=bundler_IO_subdir+"list_tmp.txt";
   ofstream liststream,restricted_asift_stream,triangulate_stream;
   ofstream gpsfit_stream,videoprop_stream,orthorectify_stream;
   filefunc::openfile(restricted_asift_scriptfilename,restricted_asift_stream);
   filefunc::openfile(triangulate_scriptfilename,triangulate_stream);
   filefunc::openfile(gpsfit_scriptfilename,gpsfit_stream);
   filefunc::openfile(videoprop_scriptfilename,videoprop_stream);
   filefunc::openfile(orthorectify_scriptfilename,orthorectify_stream);
   filefunc::openfile(list_filename,liststream);

   restricted_asift_stream << "./restricted_asift \\" << endl;
   triangulate_stream << "./triangulate \\" << endl;
   gpsfit_stream << "./gpsfit \\" << endl;
   videoprop_stream << "./video_propagator \\" << endl;
//   videoprop_stream << thresholded_pts_filename << " \\" << endl;
   videoprop_stream << densecloud_filename << " \\" << endl;
   videoprop_stream << GPS_camera_posns_filename << " \\" << endl;
   videoprop_stream << "--height_colormap 11 \\" << endl;
   
   orthorectify_stream << "./orthorectify \\" << endl;

// FAKE FAKE:  Sun Jul 28, 2013 at 7:04 am
// Intentionally randomize i_start to test robustness of GEO pipeline

//   int i_start=0;
//   nrfunc::init_time_based_seed();
   int i_start=20*nrfunc::ran1();
//   cout << "i_start = " << i_start << endl;
//   cout << "Enter index for starting image:" << endl;
//   cin >> i_start;

   int i_stop=n_images-1;
   int i=i_start;
   
// Explicitly check that ith and jth images actually exist:

   int package_counter=0;
   while (i <= i_stop)
   {
//      cout << "i = " << i << endl;
      photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_i_filename=photo_i_ptr->get_filename();
      if (!filefunc::fileexist(image_i_filename))
      {
         cout << "ERROR! i = " << i << " image = " << image_i_filename
              << " doesn't exist" << endl;
         outputfunc::enter_continue_char();
         continue;
      }
      
      camera* camera_i_ptr=camera_ptrs[i];
      threevector point_i_hat=-camera_i_ptr->get_What();

      int j_start=i+1;
      int j_stop=n_images;
      for (int j=j_start; j<j_stop; j++)
      {
         photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(j);
         string image_j_filename=photo_j_ptr->get_filename();
//         cout << "image j = " << image_j_filename << endl;

         if (!filefunc::fileexist(image_j_filename))
         {
            cout << "ERROR! j = " << j << " image = " << image_j_filename
                 << " doesn't exist" << endl;
            outputfunc::enter_continue_char();
            continue;
         }
         
         camera* camera_j_ptr=camera_ptrs[j];
         threevector point_j_hat=-camera_j_ptr->get_What();

         double dotproduct=point_i_hat.dot(point_j_hat);
         double curr_theta=acos(dotproduct);

         if (curr_theta > theta_threshold) 
         {
            cout << "Raw angle between cameras i = " << i << " and j = " << j 
                 << " equals " << curr_theta*180/PI << endl;
            cout << endl;

            string imagenumber_str=stringfunc::integer_to_string(i,4);
            string filename=hardware_packages_subdir+"photo_"
               +imagenumber_str+".pkg";
            string command="--region_filename "+filename+" \\";
            restricted_asift_stream << command << endl;
            triangulate_stream << command << endl;

            string cmd2="--region_filename "+packages_subdir
               +"photo_"+stringfunc::integer_to_string(package_counter,4)
               +".pkg \\";
            videoprop_stream << cmd2 << endl;
            orthorectify_stream << cmd2 << endl;

            string curr_image_filename="images/"+filefunc::getbasename(
               photo_i_ptr->get_filename());
            liststream << curr_image_filename << endl;

            i=j-1;
            package_counter++;
            break;
         }
      } // loop over index j labeling neighboring input images
      i++;
   } // loop over index i labeling input images

   restricted_asift_stream << "--image_list_filename "
                           << image_list_filename << endl;
   triangulate_stream << "--image_list_filename "
                      << image_list_filename << endl;
   gpsfit_stream << "--region_filename " << packages_subdir
                 << "peter_inputs.pkg" << endl;
   videoprop_stream << "--image_list_filename "
                    << image_list_filename << " \\" << endl;
   videoprop_stream << "--image_sizes_filename "
                    << image_sizes_filename << " \\" << endl;
   videoprop_stream << "--ActiveMQ_hostname tcp://127.0.0.1:61616 \\"
                    << endl;
   videoprop_stream << "--initial_mode Manipulate_Fused_Data_Mode"
                    << endl;
   orthorectify_stream << "--image_list_filename "
                       << image_list_filename << endl;
   
   filefunc::closefile(list_filename,liststream);
   filefunc::closefile(
      restricted_asift_scriptfilename,restricted_asift_stream);
   filefunc::closefile(
      triangulate_scriptfilename,triangulate_stream);
   filefunc::closefile(gpsfit_scriptfilename,gpsfit_stream);
   filefunc::closefile(videoprop_scriptfilename,videoprop_stream);
   filefunc::closefile(orthorectify_scriptfilename,orthorectify_stream);

   filefunc::make_executable(restricted_asift_scriptfilename);
   filefunc::make_executable(triangulate_scriptfilename);
   filefunc::make_executable(gpsfit_scriptfilename);
   filefunc::make_executable(videoprop_scriptfilename);
   filefunc::make_executable(orthorectify_scriptfilename);
   
   string banner="Exported "+stringfunc::number_to_string(package_counter)
      +" separated package images to "+list_filename;
   outputfunc::write_big_banner(banner);
   banner="Exported executable script "+restricted_asift_scriptfilename;
   outputfunc::write_big_banner(banner);
   banner="Exported executable script "+triangulate_scriptfilename;
   outputfunc::write_big_banner(banner);
   banner="Exported executable script "+gpsfit_scriptfilename;
   outputfunc::write_big_banner(banner);
   banner="Exported executable script "+videoprop_scriptfilename;
   outputfunc::write_big_banner(banner);
   banner="Exported executable script "+orthorectify_scriptfilename;
   outputfunc::write_big_banner(banner);

   string puma_mains_subdir="~/programs/c++/svn/projects/src/mains/puma/";
   string unix_cmd="cp "+restricted_asift_scriptfilename+" "+puma_mains_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="cp "+triangulate_scriptfilename+" "+puma_mains_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="cp "+gpsfit_scriptfilename+" "+puma_mains_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="cp "+videoprop_scriptfilename+" "+puma_mains_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="cp "+orthorectify_scriptfilename+" "+puma_mains_subdir;
   sysfunc::unix_command(unix_cmd);

   cout << "At end of separated_packages.cc" << endl;
   outputfunc::print_elapsed_time();
}
