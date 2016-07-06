// ========================================================================
// Program WRITE_VIEWBUNDLER_SCRIPT is a specialized utility which
// writes out a script file for running VIEWBUNDLER on dozens to
// thousands of Noah's reconstructed photos.

// write_viewbundler_script
//    --region_filename ./bundler/Noah_MIT2328/packages/bundler_photos.pkg 


// ========================================================================
// Last updated on 5/15/13; 7/3/13; 7/13/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string outlier_indices_filename=bundler_IO_subdir+"outlier.indices";

   filefunc::ReadInfile(image_list_filename);
   int n_photos=int(filefunc::text_line.size());
   cout << "n_photos = " << n_photos << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;
//   string common_planes_filename=passes_group.get_common_planes_filename();
//   cout << "common_planes_filename = " << common_planes_filename << endl;
//   string xyz_points_filename=passes_group.get_xyz_points_filename();
//   cout << "xyz_points_filename = " << xyz_points_filename << endl;
//   string camera_views_filename=passes_group.get_camera_views_filename();
//   cout << "camera_views_filename = " << camera_views_filename << endl;
//   string tdp_filename=bundler_IO_subdir+"thresholded_xyz_points.tdp";
//   cout << "TDP filename = " << tdp_filename << endl;
   string thresholded_xyz_osga_filename=
      bundler_IO_subdir+"thresholded_xyz_points.osga";
   cout << "OSGA filename = " << thresholded_xyz_osga_filename << endl;

   
   string reconstructed_camera_posns_osga_filename=
      bundler_IO_subdir+"reconstructed_camera_posns.osga";

   string packages_subdir=bundler_IO_subdir+"packages/";
//   cout << "packages_subdir = " << packages_subdir << endl;
   filefunc::dircreate(packages_subdir);

   string script_filename="run_viewbundler";
   string video_propagator_script_filename="run_video_propagator";
   cout << "video_propagator_script_filename = "
        << video_propagator_script_filename << endl;
   ofstream outstream,video_propagator_stream;
   filefunc::openfile(script_filename,outstream);
   filefunc::openfile(
      video_propagator_script_filename,video_propagator_stream);

//   string dirname="/home/cho/programs/c++/svn/projects/src/mains/photosynth/";
   string dirname="./";
   outstream << "cd " << dirname << endl;
   outstream << dirname+"viewbundler \\" << endl;
   outstream << thresholded_xyz_osga_filename << " \\" << endl;

   video_propagator_stream << "./video_propagator \\" << endl;
   video_propagator_stream << thresholded_xyz_osga_filename << " \\" << endl;

// On 7/13/13, we empirically found that 3D navigation becomes much
// easier if we include points located at the reconstructed camera
// positions:

   outstream << reconstructed_camera_posns_osga_filename 
             << " \\" << endl;
   video_propagator_stream << reconstructed_camera_posns_osga_filename 
                           << " \\" << endl;

// Choose colormap with lots of dynamic range so that aerial
// reconstructed camera positions don't overwhlem ground reconstructed
// point cloud:

   outstream << "--height_colormap 11 \\" << endl;	 // wrap 4 colormap
   video_propagator_stream << "--height_colormap 11 \\" << endl;	 
		// wrap 4 colormap

   int starting_imagenumber=0;
   cout << endl;
   cout << "Enter starting image number: " << endl << endl;
//   cin >> starting_imagenumber;
//   cout << "starting image number = " << starting_imagenumber << endl;

   int last_imagenumber=n_photos-1;
   cout << "Final image number = " << last_imagenumber << endl;

   int stopping_imagenumber=last_imagenumber;
   cout << "Enter stopping image number: " << endl << endl;
//   cin >> stopping_imagenumber;

   int imagenumber_step=1;
   cout << endl;
   cout << "Enter image number step:" << endl << endl;
//   cin >> imagenumber_step;

// Import any indices for any outlier packages which are to be ignored
// from outlier_indices_filename:

   typedef map<int,bool> OUTLIER_INDICES_MAP;
   OUTLIER_INDICES_MAP outlier_indices_map;
   OUTLIER_INDICES_MAP::iterator iter;

   if (filefunc::fileexist(outlier_indices_filename))
   {
      filefunc::ReadInfile(outlier_indices_filename);
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[0]);
      for (unsigned int i=0; i<column_values.size(); i++)
      {
         outlier_indices_map[int(column_values[i])]=true;
      }
   }
   cout << "outlier_indices_map.size() = " << outlier_indices_map.size()
        << endl;

   int ndigits=4;
   for (int imagenumber=starting_imagenumber; imagenumber <= 
           stopping_imagenumber; imagenumber += imagenumber_step)
   {

// Skip over any image whose index exists within outlier_indices_map:

      iter=outlier_indices_map.find(imagenumber);
      if (iter != outlier_indices_map.end()) continue;

      string photo_package_filename=packages_subdir+"photo_"+
         stringfunc::integer_to_string(imagenumber,ndigits)+".pkg";
      string videoframe_package_filename=packages_subdir+"frame_"+
         stringfunc::integer_to_string(imagenumber,ndigits)+".pkg";

// Explicitly check whether package file actually exists before
// writing it to the output script:

      if (filefunc::fileexist(photo_package_filename))
      {
         outstream << "--region_filename "+photo_package_filename+" \\" 
                   << endl;
         video_propagator_stream 
            << "--region_filename "+photo_package_filename+" \\" << endl;
      }
      else if (filefunc::fileexist(videoframe_package_filename))
      {
         outstream << "--region_filename "+videoframe_package_filename
            +" \\" << endl;
         video_propagator_stream 
            << "--region_filename "+videoframe_package_filename+" \\" << endl;
      }
   }
   outstream << "--image_list_filename "+image_list_filename+" \\" << endl;
   outstream << "--image_sizes_filename "+image_sizes_filename+" \\" << endl;
//   outstream << "--xyz_pnts_filename "+xyz_points_filename+" \\" << endl;
//   outstream << "--camera_views_filename "+camera_views_filename+" \\" 
//             << endl;

//   if (common_planes_filename.size() > 0)
//   {
//      outstream << "--common_planes_filename "+common_planes_filename
//         +" \\" << endl;
//   }
   
   outstream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;

   video_propagator_stream << "--image_list_filename "
      +image_list_filename+" \\" << endl;
   video_propagator_stream << "--image_sizes_filename "
      +image_sizes_filename+" \\" << endl;
   video_propagator_stream << "--ActiveMQ_hostname tcp://127.0.0.1:61616 \\"
                           << endl;
   video_propagator_stream << "--initial_mode Manipulate_Fused_Data_Mode" 
                           << endl;

   filefunc::closefile(script_filename,outstream);
   filefunc::closefile(
      video_propagator_script_filename,video_propagator_stream);

// Make output script executable:

   string unix_command_str="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command_str);
   unix_command_str="chmod a+x "+video_propagator_script_filename;
   sysfunc::unix_command(unix_command_str);

   string mains_geo_subdir="~/programs/c++/svn/projects/src/mains/geo/";
   unix_command_str="mv "+video_propagator_script_filename+" "+
      mains_geo_subdir;
   sysfunc::unix_command(unix_command_str);

   string banner="Viewbundler script written to "+script_filename;
   outputfunc::write_big_banner(banner);
   banner="Video propagator script written to "
      +video_propagator_script_filename;
   outputfunc::write_big_banner(banner);
}
