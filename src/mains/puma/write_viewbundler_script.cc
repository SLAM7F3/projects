// ========================================================================
// mains/puma/WRITE_VIEWBUNDLER_SCRIPT is a variant of
// mains/photosynth/WRITE_VIEWBUNDLER_SCRIPT.  It exports script files
// for running progrmas VIEWBUNDLER, VIDEO_PROPAGATOR, EARTHBUNDLER
// and ORTHORECTIFY on dozens to hundreds of small UAV video frames
// which are temporally ordered.
// ========================================================================
// Last updated on 7/13/13; 12/4/13; 12/30/13
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

   bool VSFM_flag=true;
//   bool VSFM_flag=false;
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string outlier_indices_filename=bundler_IO_subdir+"outlier.indices";

// Recall renamed VSFM filenames are generally NOT temporally ordered.
// So we instantiate and fill an STL map which relates frame IDs to
// package IDs:

   typedef map<int,int> FRAME_VS_PACKAGE_IDS_MAP;
   FRAME_VS_PACKAGE_IDS_MAP frame_vs_package_ids_map;
   FRAME_VS_PACKAGE_IDS_MAP::iterator frame_vs_package_ids_iter;

   if (VSFM_flag)
   {
      string orig_vs_vsfm_filename=bundler_IO_subdir+"orig_vs_VSFM_images.dat";

// Import relationships between original and VSFM image IDs
// calculated via program VSFM_VS_ORIG_IMAGES and store within STL
// map:

      vector<vector<string> > substrings=filefunc::ReadInSubstrings(
         orig_vs_vsfm_filename);
      cout << "substrings.size() = " << substrings.size() << endl;
      for (int s=0; s<substrings.size(); s++)
      {
         string orig_filename=substrings[s].at(0);

// Recall GoPro video frames do NOT have canonical filenames like
// frame-05377.jpg.  Instead, they are named like G0037536.JPG:

         int orig_ID=-1;
         vector<string> subsubstrings;
         string separator_chars="-.";
         if (orig_filename.substr(0,1)=="G")
         {
            orig_ID=stringfunc::string_to_number(
               orig_filename.substr(1,7));
         }
         else
         {
            subsubstrings=stringfunc::decompose_string_into_substrings(
               orig_filename,separator_chars);
            orig_ID=stringfunc::string_to_number(subsubstrings[1]);
         }
         
         string vsfm_filename=substrings[s].at(1);
         subsubstrings.clear();
         subsubstrings=stringfunc::decompose_string_into_substrings(
            vsfm_filename,separator_chars);
         int vsfm_ID=stringfunc::string_to_number(subsubstrings[0]);
//         cout << "orig_ID = " << orig_ID << " vsfm_ID = " << vsfm_ID << endl;
         frame_vs_package_ids_map[orig_ID]=vsfm_ID;
      } // loop over index s labeling lines in orig_vs_vsfm_filename
   }
   else
   {
      filefunc::ReadInfile(image_list_filename);
      int n_photos=int(filefunc::text_line.size());
      cout << "n_photos = " << n_photos << endl;

      int starting_imagenumber=0;
      int last_imagenumber=n_photos-1;
      int stopping_imagenumber=last_imagenumber;
      int imagenumber_step=1;

      for (int imagenumber=starting_imagenumber; imagenumber <= 
              stopping_imagenumber; imagenumber += imagenumber_step)
      {
         frame_vs_package_ids_map[imagenumber]=imagenumber;
      }
   }
   cout << "frame_vs_package_ids_map.size() = "
        << frame_vs_package_ids_map.size() << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   string thresholded_xyz_osga_filename=
      bundler_IO_subdir+"thresholded_xyz_points.osga";
   string densecloud_osga_filename=
      bundler_IO_subdir+"dense_cloud.osga";
   
   string reconstructed_camera_posns_osga_filename=
      bundler_IO_subdir+"reconstructed_camera_posns.osga";

   string packages_subdir=bundler_IO_subdir+"packages/";
//   cout << "packages_subdir = " << packages_subdir << endl;
   filefunc::dircreate(packages_subdir);

   string viewbundler_script_filename="run_viewbundler";
   string video_propagator_script_filename="run_video_propagator";
   string earthbundler_script_filename="run_earthbundler";
   string orthorectify_script_filename="run_orthorectify";

   ofstream viewbundler_stream,video_propagator_stream,earthbundler_stream,
      orthorectify_stream;
   filefunc::openfile(
      viewbundler_script_filename,viewbundler_stream);
   filefunc::openfile(
      video_propagator_script_filename,video_propagator_stream);
   filefunc::openfile(
      earthbundler_script_filename,earthbundler_stream);
   filefunc::openfile(
      orthorectify_script_filename,orthorectify_stream);

   string dirname="./";
   viewbundler_stream << "cd " << dirname << endl;
   viewbundler_stream << dirname+"viewbundler \\" << endl;
   viewbundler_stream << thresholded_xyz_osga_filename << " \\" << endl;

   video_propagator_stream << "./video_propagator \\" << endl;
   video_propagator_stream << densecloud_osga_filename << " \\" << endl;

   earthbundler_stream << "./earthbundler \\" << endl;
   orthorectify_stream << "./orthorectify \\" << endl;

// On 7/13/13, we empirically found that 3D navigation becomes much
// easier if we include points located at the reconstructed camera
// positions:

   viewbundler_stream << reconstructed_camera_posns_osga_filename 
                      << " \\" << endl;
   video_propagator_stream << reconstructed_camera_posns_osga_filename 
                           << " \\" << endl;
   earthbundler_stream << reconstructed_camera_posns_osga_filename 
                       << " \\" << endl;
   earthbundler_stream << "--surface_texture ./bundler/Puma/GE/YPG.pkg \\" 
                       << endl;

// Choose colormap with lots of dynamic range so that aerial
// reconstructed camera positions don't overwhlem ground reconstructed
// point cloud:

   viewbundler_stream << "--height_colormap 11 \\" << endl;	 // wrap 4 colormap
   video_propagator_stream << "--height_colormap 11 \\" << endl;	 
		// wrap 4 colormap

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
      for (int i=0; i<column_values.size(); i++)
      {
         outlier_indices_map[int(column_values[i])]=true;
      }
   }
   cout << "outlier_indices_map.size() = " << outlier_indices_map.size()
        << endl;

   int ndigits=4;
   for (frame_vs_package_ids_iter=frame_vs_package_ids_map.begin(); 
        frame_vs_package_ids_iter != frame_vs_package_ids_map.end(); 
        frame_vs_package_ids_iter++)
   {
      int orig_ID=frame_vs_package_ids_iter->first;
      int vsfm_ID=frame_vs_package_ids_iter->second;
      
// Skip over any image whose index exists within outlier_indices_map:

      iter=outlier_indices_map.find(vsfm_ID);
      if (iter != outlier_indices_map.end()) continue;

      string photo_package_filename=packages_subdir+"photo_"+
         stringfunc::integer_to_string(vsfm_ID,ndigits)+".pkg";
      string videoframe_package_filename=packages_subdir+"frame_"+
         stringfunc::integer_to_string(vsfm_ID,ndigits)+".pkg";

// Explicitly check whether package file actually exists before
// writing it to the output script:

      if (filefunc::fileexist(photo_package_filename))
      {
         viewbundler_stream << "--region_filename "+
            photo_package_filename+" \\" << endl;
         video_propagator_stream 
            << "--region_filename "+photo_package_filename+" \\" << endl;
         earthbundler_stream 
            << "--region_filename "+photo_package_filename+" \\" << endl;
         orthorectify_stream 
            << "--region_filename "+photo_package_filename+" \\" << endl;
      }
      else if (filefunc::fileexist(videoframe_package_filename))
      {
         viewbundler_stream << "--region_filename "+videoframe_package_filename
            +" \\" << endl;
         video_propagator_stream 
            << "--region_filename "+videoframe_package_filename+" \\" << endl;
         earthbundler_stream 
            << "--region_filename "+videoframe_package_filename+" \\" << endl;
         orthorectify_stream 
            << "--region_filename "+videoframe_package_filename+" \\" << endl;

      }
   } // iterator over frame_vs_package_ids_map
   viewbundler_stream << "--image_list_filename "+image_list_filename+" \\"
                      << endl;
   viewbundler_stream << "--image_sizes_filename "+image_sizes_filename+" \\" 
                      << endl;
   viewbundler_stream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;

   video_propagator_stream << "--image_list_filename "
      +image_list_filename+" \\" << endl;
   video_propagator_stream << "--image_sizes_filename "
      +image_sizes_filename+" \\" << endl;
   video_propagator_stream << "--ActiveMQ_hostname tcp://127.0.0.1:61616 \\"
                           << endl;
   video_propagator_stream << "--initial_mode Manipulate_Fused_Data_Mode" 
                           << endl;

   earthbundler_stream << "--world_start_UTC 2013,5,30,12,35,17 \\" << endl;
   earthbundler_stream << "--world_stop_UTC 2013,5,30,12,44,03 \\" << endl;
   earthbundler_stream << "--world_time_step 1.0 \\" << endl;
   earthbundler_stream << "--image_list_filename "
      +image_list_filename+" \\" << endl;
   earthbundler_stream << "--initial_mode Manipulate_Fused_Data_Mode" 
                       << endl;

   orthorectify_stream << "--image_list_filename "
      +image_list_filename+" \\" << endl;
   orthorectify_stream << "--initial_mode Manipulate_Fused_Data_Mode" 
                       << endl;

   filefunc::closefile(
      viewbundler_script_filename,viewbundler_stream);
   filefunc::closefile(
      video_propagator_script_filename,video_propagator_stream);
   filefunc::closefile(
      earthbundler_script_filename,earthbundler_stream);
   filefunc::closefile(
      orthorectify_script_filename,orthorectify_stream);

// Make output script executable:

   string unix_command_str="chmod a+x "+viewbundler_script_filename;
   sysfunc::unix_command(unix_command_str);
   unix_command_str="chmod a+x "+video_propagator_script_filename;
   sysfunc::unix_command(unix_command_str);
   unix_command_str="chmod a+x "+earthbundler_script_filename;
   sysfunc::unix_command(unix_command_str);
   unix_command_str="chmod a+x "+orthorectify_script_filename;
   sysfunc::unix_command(unix_command_str);

   string banner="Viewbundler script written to "+viewbundler_script_filename;
   outputfunc::write_big_banner(banner);
   banner="Video propagator script written to "
      +video_propagator_script_filename;
   outputfunc::write_big_banner(banner);
   banner="Earthbundler script written to "+earthbundler_script_filename;
   outputfunc::write_big_banner(banner);
   banner="Orthorectify script written to "+orthorectify_script_filename;
   outputfunc::write_big_banner(banner);
}
