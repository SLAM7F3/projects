//  Flight 1: April 24, 2013

// Circle around huts starts around frame 1345 ("frame-01715.png") and
// goes until at least frame 1545 ("frame-01915.png").

// .....................................................................
// Day 2 flight 3: April 25, 2013:

// Circle around cars at widest FOV begins at frame-03292.jpg and ends
// at frame-03420.jpg.

// Circle around cars at 2nd most narrow FOV begins at frame-03423 and
// ends at frame-03541.jpg

// Circle around cars at 2nd most narrow FOV begins at frame-03542 and
// ends at frame-03660.jpg

// .....................................................................
// May 30, 2013 at Camp Edwards (Day 1) flight 2

// "Scripted" cars and people moving around huts begins around frame 2500
// and proceeds through frame 2741 at wide FOV.

// More "scripted" cars and people moving around huts from frames 2742
// - 2907 at medium FOV 

// More "scripted" cars and people moving around huts from frames 2908
// - 3069 at narrow FOV


// ========================================================================
// Program VISUALIZE_PUMA_METADATA reads in the metadata ascii file generated
// by program PARSE_PUMA_METADATA.  It extracts aircraft GPS
// video camera pointing information from this metadata
// file. This program generates a TDP/OSGA file containing the
// aircraft's GPS track and line-of-sight rays from the aircraft
// towards the ground. VISUALIZE_PUMA_METADATA also outputs package
// files containing video frame filenames and their hardware derived
// camera parameters to bundler_IO_subdir/packages/hardware_in.

//			    visualize_puma_metadata

// ========================================================================
// Last updated on 5/14/13; 5/20/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "passes/PassesGroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << endl;
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string metadata_filename=bundler_IO_subdir+"puma.metadata";
   cout << endl;
//   cout << "Enter name of Puma metadata file:" << endl;
//   cin >> metadata_filename;
//   metadata_filename=bundler_IO_subdir+metadata_filename;
//   cout << "metadata_filename = " << metadata_filename << endl;

   string frames_subdir=bundler_IO_subdir+"images/";
   vector<string> image_filenames=
      filefunc::image_files_in_subdir(frames_subdir);
   int n_images=image_filenames.size();

   string packages_subdir=bundler_IO_subdir+"packages/";
   string hardware_packages_subdir=packages_subdir+"hardware_in/";
   filefunc::dircreate(packages_subdir);
   filefunc::dircreate(hardware_packages_subdir);
   
   cout << "metadata_filename = " << metadata_filename << endl;
   filefunc::ReadInfile(metadata_filename);

// Puma image size in pixels:

   const int n_horiz_pixels=640;
   const int n_vert_pixels=480;
   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);

   int package_ID=0;
//   cout << "Enter starting package ID:" << endl;
//   cin >> package_ID;

   int n_waypoints=filefunc::text_line.size();
   cout << "n_waypoints = " << n_waypoints << endl;
 
   int start_frame_ID=0;
   int stop_frame_ID=n_waypoints-1;
   cout << "Enter starting frame number:" << endl;
   cin >> start_frame_ID;
   cout << "Enter stopping frame number:" << endl;
   cin >> stop_frame_ID;

// Extract GPS flight path:

   vector<threevector> GPS_position;
   ofstream GPSstream,FOVstream,orientstream,packagestream,liststream;
   string waypoints_filename=bundler_IO_subdir+"GPS_waypoints.txt";
   string orientation_filename=bundler_IO_subdir+"orientation.txt";
   string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
   string FOV_filename=bundler_IO_subdir+"FOV.txt";
   filefunc::openfile(waypoints_filename,GPSstream);
   filefunc::openfile(FOV_filename,FOVstream);
   filefunc::openfile(orientation_filename,orientstream);
   filefunc::openfile(list_tmp_filename,liststream);

   GPSstream.precision(12);
   FOVstream.precision(12);
   orientstream.precision(10);

   GPSstream << "# i  HHMMSS     Epoch secs      Easting      Northing      Altitude (m)" << endl;
   GPSstream << endl;

   FOVstream << "# i  HHMMSS     Epoch secs      FOV_U (degs)" << endl;
   FOVstream << endl;

   orientstream << "# i  Frame      Az               El      phi_h      Pitch  	   Roll" << endl;
   orientstream << endl;

   double z_ground=51;	// meters (reasonable for Cape Cod)
//   cout << "Enter z_ground estimate in meters above sea-level:" << endl;
//   cout << " (z_ground is used for visualizing lines-of-sight only)" << endl;
//   cin >> z_ground;

   int UTM_zone=19;	// MA
   bool northern_hemisphere_flag=true;

   Clock clock;
   geopoint curr_waypoint,prev_waypoint;
   vector<threevector> candidate_tgt_posns;
   for (int i=0; i<n_waypoints; i++)
   {
      if (i%1000==0) cout << i << " " << flush;

      vector<string> column_values=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i]);
      string curr_frame_filename=column_values[0];
      
      string separator_chars="-.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         curr_frame_filename,separator_chars);
      int frame_ID=stringfunc::string_to_number(substrings[1]);
      if (frame_ID < start_frame_ID || frame_ID > stop_frame_ID)
         continue;

      double epoch=stringfunc::string_to_number(column_values[1]);
      clock.convert_elapsed_secs_to_date(epoch);
      string HHMMSS=clock.YYYY_MM_DD_H_M_S("","",true,0,false);

      double UAV_easting=stringfunc::string_to_number(column_values[2]);
      double UAV_northing=stringfunc::string_to_number(column_values[3]);
      double UAV_alt=stringfunc::string_to_number(column_values[4]);

      prev_waypoint=curr_waypoint;
      curr_waypoint=geopoint(
         northern_hemisphere_flag,UTM_zone,UAV_easting,UAV_northing,UAV_alt);
      
      double prev_easting=prev_waypoint.get_UTM_easting();
      double prev_northing=prev_waypoint.get_UTM_northing();
      double prev_altitude=prev_waypoint.get_altitude();

      GPSstream << i << "  "
                << HHMMSS << "  "
                << clock.secs_elapsed_since_reference_date() << "  "
                << UAV_easting << "  "
                << UAV_northing << "  "
                << UAV_alt << "  "
                << endl;

      threevector curr_GPS_posn(UAV_easting,UAV_northing,UAV_alt);
      GPS_position.push_back(curr_GPS_posn);

      double tgt_easting=stringfunc::string_to_number(column_values[8]);
      double tgt_northing=stringfunc::string_to_number(column_values[9]);
      double tgt_alt=stringfunc::string_to_number(column_values[10]);
      threevector curr_target_posn(tgt_easting,tgt_northing,tgt_alt);

// f_hat = Puma camera pointing vector:

      threevector f_hat=(curr_target_posn-curr_GPS_posn).unitvector();

      double curr_az,curr_el,curr_roll=0;
      mathfunc::decompose_direction_vector(f_hat,curr_az,curr_el);
      rotation R;
      R=R.rotation_from_az_el_roll(curr_az,curr_el,curr_roll);
//      cout << "f_hat = " << f_hat << endl;
//      cout << "az = " << curr_az*180/PI
//           << " el = " << curr_el*180/PI
//           << " roll = " << curr_roll*180/PI << endl;

      threevector U_hat=-R*y_hat;
      threevector V_hat=R*z_hat;
      threevector W_hat=-f_hat;

      double curr_FOV_U=31.5;	// wide
//      double curr_FOV_U=17.9;	// medium
//      double curr_FOV_U=10.8;	// narrow
//      double curr_FOV_U=6.7;	// ultra
      
      FOVstream << i << "  "
                << HHMMSS << "  "
                << clock.secs_elapsed_since_reference_date() << "  "
                << curr_FOV_U << endl;

//      cout << "U_hat = " << U_hat << endl;
//      cout << "V_hat = " << V_hat << endl;
//      cout << "w_hat = " << W_hat << endl;
//       outputfunc::enter_continue_char();

//      cout << "FLIR dir: " 
//           << f_hat.get(0) << ", "
//           << f_hat.get(1) << ", "
//           << f_hat.get(2) << endl << endl;
//      cout << "(U_hat x V_hat) . f_hat = " << (U_hat.cross(V_hat)).dot(f_hat)
//           << endl;

      const int frame_skip=1;
//      const int frame_skip=2;
//      const int frame_skip=5;
      if (i%frame_skip > 0) continue;

// Add 3D points illustrating line-of-sight along pointing direction
// f_hat from aircraft's position to ground Z-plane.  Ignore any
// upward pointing rays:

//      bool draw_LOS_rays_flag=true;
      bool draw_LOS_rays_flag=false;
      
      if (draw_LOS_rays_flag)
      {
         double d_range=1;	// meters
         if (f_hat.get(2) < 0)
         {
            double range=1000;
            int n_range_points=range/d_range+1;

            for (int r=0; r<n_range_points; r++)
            {
               threevector LOS_posn=curr_GPS_posn+r*d_range*f_hat;
               double curr_z=LOS_posn.get(2);
               if (curr_z < z_ground) continue;

               GPS_position.push_back(LOS_posn);
            } // loop over index r labeling range points
            candidate_tgt_posns.push_back(GPS_position.back());
         }
      } // draw_LOS_rays_flag conditional

/*
// Also add 3D points illustrating instantaneous U_hat and
// V_hat direction vectors:

      int n_UV_points=20;
      for (int p=0; p<n_UV_points; p++)
      {
         threevector curr_U_posn=curr_GPS_posn+p*d_range*U_hat;
         threevector curr_V_posn=curr_GPS_posn+p*d_range*V_hat;
         GPS_position.push_back(curr_U_posn);
         GPS_position.push_back(curr_V_posn);
      } // loop over index p labeling UV points
*/

      double f,curr_FOV_V;
      curr_FOV_U *= PI/180;
      camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
         curr_FOV_U,aspect_ratio,f,curr_FOV_V);
//      cout << "f = " << f
//           << " FOV_V = " << curr_FOV_V*180/PI << endl;

//      cout << "U_hat = " << U_hat << endl;
//      cout << "V_hat = " << V_hat << endl;
//      outputfunc::enter_continue_char();

// Write out package files containing 3D FLIR camera parameters derived
// from Ross' metadata file:

      string package_filename=hardware_packages_subdir+"photo_"+
         stringfunc::integer_to_string(package_ID,4)+".pkg";

      filefunc::openfile(package_filename,packagestream);
      curr_frame_filename=frames_subdir+curr_frame_filename;

      liststream << "images/" << filefunc::getbasename(curr_frame_filename)
                 << endl;

      packagestream.precision(12);
      packagestream << curr_frame_filename << endl;
      packagestream << "--photo_ID " << package_ID << endl;
      packagestream << "--Uaxis_focal_length " << f << endl;
      packagestream << "--Vaxis_focal_length " << f << endl;
      packagestream << "--U0 " << 0.5*aspect_ratio << endl;
      packagestream << "--V0 0.5" << endl;
      packagestream << "--relative_az " << curr_az*180/PI << endl;
      packagestream << "--relative_el " << curr_el*180/PI << endl;
      packagestream << "--relative_roll " << curr_roll*180/PI << endl;
      packagestream << "--camera_x_posn " << curr_GPS_posn.get(0) << endl;
      packagestream << "--camera_y_posn " << curr_GPS_posn.get(1) << endl;
      packagestream << "--camera_z_posn " << curr_GPS_posn.get(2) << endl;
      packagestream << "--frustum_sidelength 15 " << endl;
      filefunc::closefile(package_filename,packagestream);

      package_ID++;      

   } // loop over index i labeling frame number
   cout << endl;

   filefunc::closefile(waypoints_filename,GPSstream);
   filefunc::closefile(FOV_filename,FOVstream);
   filefunc::closefile(orientation_filename,orientstream);
   filefunc::closefile(list_tmp_filename,liststream);

   cout << "n_waypoints = " << n_waypoints << endl;

/*
// Calculate mean and standard deviation for candidate ground target
// positions:

   vector<double> tgt_X,tgt_Y,tgt_Z;
   for (int t=0; t<candidate_tgt_posns.size(); t++)
   {
      tgt_X.push_back(candidate_tgt_posns[t].get(0));
      tgt_Y.push_back(candidate_tgt_posns[t].get(1));
      tgt_Z.push_back(candidate_tgt_posns[t].get(2));
   }
   double mu_X=mathfunc::mean(tgt_X);
   double mu_Y=mathfunc::mean(tgt_Y);
   double mu_Z=mathfunc::mean(tgt_Z);
   double sigma_X=mathfunc::std_dev(tgt_X);
   double sigma_Y=mathfunc::std_dev(tgt_Y);
   double sigma_Z=mathfunc::std_dev(tgt_Z);
   
   threevector mean_tgt_posn(mu_X,mu_Y,mu_Z);
   threevector sigma_tgt_posn(sigma_X,sigma_Y,sigma_Z);
   cout << "Averaged tgt posn = " << mean_tgt_posn << endl;
   cout << "sigma tgt posn = " << sigma_tgt_posn << endl;
   cout << "sigma magnitude = " << sigma_tgt_posn.magnitude() << endl;
*/

// Generate TDP file containing flight path, Puma lines-of-sight, Uhat
// and Vhat direction vectors extracted from Puma metadata file:

   string tdp_filename=bundler_IO_subdir+"hardware_GPS_flightpath.tdp";
   tdpfunc::write_relative_xyz_data(tdp_filename,GPS_position);

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv hardware_GPS_flightpath.osga "+bundler_IO_subdir;
   sysfunc::unix_command(unix_cmd);   

   string banner="Flight path and lines-of-sight exported to "+
      bundler_IO_subdir+"hardware_GPS_flightpath.osga";
   outputfunc::write_big_banner(banner);
}


