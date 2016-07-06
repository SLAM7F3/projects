// ========================================================================
// Program VISUALIZE_FLIR_METADATA is a variant of
// PARSE_FLIR_METADATA.  It reads in the metadata ascii file generated
// by Ross Anderson's program which ran on the Mac-Mini along with the
// FLIR for Twin Otter flights.  It extracts aircraft GPS, aircraft
// orientation and FLIR pointing information from this metadata
// file. This program generates a TDP/OSGA file containing the
// aircraft's GPS track, line-of-sight rays from the aircraft towards
// the ground, and Uhat-Vhat direction vectors based upon FLIR
// metadata input.  VISUALIZE_FLIR_METADATA also outputs package files
// containing low-defn frame filenames and their hardware derived
// camera parameters to bundler_IO_subdir/packages/hardware_in.
// Finally, it exports an executable script for program
// SEPARATED_PACKAGES.

//			    visualize_FLIR_metadata

// ========================================================================
// Last updated on 8/28/13; 9/11/13; 9/15/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "messenger/Messenger.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"

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

   bool demo_flag=true;
   if (demo_flag)
   {
      string broker_URL = "tcp://127.0.0.1:61616";
      string message_queue_channel_name="127.0.0.1";
      string message_sender_ID="MESSAGE_SENDER";
      bool include_sender_and_timestamp_info_flag=false;
      Messenger m(broker_URL,message_queue_channel_name,message_sender_ID,
                  include_sender_and_timestamp_info_flag);

      string command="DISPLAY_NEXT_FLOW_DIAGRAM";
      m.sendTextMessage(command);
   }
   timefunc::initialize_timeofday_clock();      
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   int pass_ID=passes_group.get_n_passes()-1;
//   cout << "pass_ID = " << pass_ID << endl;
   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
//   cout << "image_list_filename = " << image_list_filename << endl;

   string metadata_filename="camera_metadata.txt";
   cout << endl;
//   cout << "Enter name of Ross Anderson's FLIR metadata file:" << endl;
//   cin >> metadata_filename;
   metadata_filename=bundler_IO_subdir+metadata_filename;
//   cout << "metadata_filename = " << metadata_filename << endl;
   filefunc::ReadInfile(metadata_filename);

   string frames_subdir=bundler_IO_subdir+"images/";
   string packages_subdir=bundler_IO_subdir+"packages/";
   string hardware_packages_subdir=packages_subdir+"hardware_in/";
   filefunc::dircreate(packages_subdir);
   filefunc::dircreate(hardware_packages_subdir);
   
/*
// Hi-defn FLIR image size in pixels:
      
   const int n_horiz_pixels=1280;
   const int n_vert_pixels=720;
   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);
//	aspect_ratio=1.77777 for hi-defn FLIR images
*/

// Cropped low-defn FLIR image size in pixels:

   const int n_horiz_pixels=704;
//   const int n_vert_pixels=344; // = 416 - 72
   const int n_vert_pixels=359; // GEO Pass 4
   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);
//	aspect_ratio=1.9610 for lo-defn FLIR images

   int package_ID=0;
//   cout << "Enter starting package ID:" << endl;
//   cin >> package_ID;

   int n_waypoints=filefunc::text_line.size();
//   cout << "n_waypoints = " << n_waypoints << endl;

// If sentinel -100 values have been passed as input values for
// start_frame_ID and stop_frame_ID, reset start_frame_ID to 0 and
// stop_frame_ID to n_waypoints-1:

   int start_frame_ID=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_start_frame_ID();
   int stop_frame_ID=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_stop_frame_ID();
   if (start_frame_ID != -100)
   {
      cout << "Enter starting frame numer (default=0) :" << endl;
      cin >> start_frame_ID;
   }
   else
   {
      start_frame_ID=0;
   }
   
   if (stop_frame_ID != -100)
   {
      cout << "Enter stopping frame numer (default=" << n_waypoints-1 
           << ") :" << endl;
      cin >> stop_frame_ID;
   }
   else
   {
      stop_frame_ID=n_waypoints-1;
   }
//   cout << "start_frame_ID = " << start_frame_ID
//        << " stop_frame_ID = " << stop_frame_ID << endl;
   
// Extract GPS flight path:

   vector<threevector> GPS_position;
   ofstream GPSstream,FOVstream,orientstream,packagestream,liststream;
   ofstream separated_packages_stream;
   string waypoints_filename=bundler_IO_subdir+"GPS_waypoints.txt";
   string orientation_filename=bundler_IO_subdir+"orientation.txt";
   string list_tmp_filename=bundler_IO_subdir+"list_tmp.txt";
   string FOV_filename=bundler_IO_subdir+"FOV.txt";
   string separated_packages_scriptname=
      bundler_IO_subdir+"run_separated_packages";
   filefunc::openfile(waypoints_filename,GPSstream);
   filefunc::openfile(FOV_filename,FOVstream);
   filefunc::openfile(orientation_filename,orientstream);
   filefunc::openfile(list_tmp_filename,liststream);
   filefunc::openfile(separated_packages_scriptname,separated_packages_stream);

   GPSstream.precision(12);
   FOVstream.precision(12);
   orientstream.precision(10);

   GPSstream << "# i  HHMMSS     Epoch secs      Easting      Northing      Altitude (m)" << endl;
   GPSstream << endl;

   FOVstream << "# i  HHMMSS     Epoch secs      FOV_U (degs)" << endl;
   FOVstream << endl;

   orientstream << "# i  Frame      Az               El      phi_h      Pitch  	   Roll" << endl;
//    orientstream << "# HHMMSS     Az               El      heading      Pitch  	   Roll" << endl;
//   orientstream << "# HHMMSS     Az               El      heading      phi_h   phi" << endl;
   orientstream << endl;

   separated_packages_stream << "./separated_packages \\" << endl;

   double z_ground=0;
//   cout << "Enter z_ground estimate in meters above sea-level:" << endl;
//   cout << " (z_ground is used for visualizing lines-of-sight only)" << endl;
//   cin >> z_ground;

   Clock clock;
   geopoint curr_waypoint,prev_waypoint;
   vector<double> LOS_ranges,f_angles;
   vector<threevector> candidate_tgt_posns;
   for (int i=start_frame_ID; i<=stop_frame_ID; i++)
   {
      if (i%1000==0) cout << i << " " << flush;

      vector<string> column_values=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i],",");
//      cout << "i = " << i 
//           << " column_values.size() = " << column_values.size() << endl;

      if (column_values.size() != 21) continue;
      if (!stringfunc::is_number(column_values[5])) continue;

      vector<string> date_time_substrings=
         stringfunc::decompose_string_into_substrings(
         column_values[0],"_");

      string YYYYMMDD=date_time_substrings[0];
      string HHMMSS=date_time_substrings[1];
//      cout << "HHMMSS = " << HHMMSS << endl;
//      outputfunc::enter_continue_char();

      int year,month,day,hour,minute;
      double secs;
      clock.parse_YYYYMMDD_string(YYYYMMDD,year,month,day);
      clock.parse_HHMMSS_string(HHMMSS,hour,minute,secs);
      const int hour_offset_to_UTC=4;	// Boston
      int UTC_hour=hour+hour_offset_to_UTC;
      clock.set_UTC_time(year,month,day,UTC_hour,minute,secs);
//      double elapsed_secs=clock.secs_elapsed_since_reference_date();

      string curr_frame_filename=column_values[0];
      string image_filename=frames_subdir+curr_frame_filename+".jpg";

// Make sure image_filename actually exists!  If not, skip to next
// input image:

      if (!filefunc::fileexist(image_filename))
      {
         cout << "JPEG image file " << image_filename 
              << " is missing!" << endl;
//         outputfunc::enter_continue_char();
         continue;
      }

      liststream << "images/" << filefunc::getbasename(image_filename)
                 << endl;

      double curr_az=stringfunc::string_to_number(column_values[1]);

// FAKE FAKE:  Thurs Mar 7, 2013 at 9:45 am
// Experiment with adding 90 degrees to curr_az for MarApr2012 GEO flights:

//      curr_az += 90;
      
      double curr_el=stringfunc::string_to_number(column_values[2]);
      double curr_heading=stringfunc::string_to_number(column_values[8]);
      double curr_pitch=stringfunc::string_to_number(column_values[9]);
      double curr_roll=stringfunc::string_to_number(column_values[10]);
      double curr_FOV_U=stringfunc::string_to_number(column_values[3]);
      double curr_latitude=stringfunc::string_to_number(column_values[5]);
      double curr_longitude=stringfunc::string_to_number(column_values[6]);
      double curr_altitude=stringfunc::string_to_number(column_values[7]);

// FAKE FAKE:  Fri May 20, 2013 at 4:12 pm
// Experiment with ignoring any hardware metadata which does NOT
// correspond to a 5 deg FOV:

      if (!nearly_equal(curr_FOV_U,4.5,0.5)) continue;

      prev_waypoint=curr_waypoint;
      curr_waypoint=geopoint(curr_longitude,curr_latitude,curr_altitude);
      
      double curr_easting=curr_waypoint.get_UTM_easting();
      double curr_northing=curr_waypoint.get_UTM_northing();
//      double prev_easting=prev_waypoint.get_UTM_easting();
//      double prev_northing=prev_waypoint.get_UTM_northing();
//      double prev_altitude=prev_waypoint.get_altitude();

//      cout.precision(12);
//      cout << "curr_easting = " << curr_easting 
//           << " prev_easting = " << prev_easting
//           << " curr_northing = " << curr_northing
//           << " prev_northing = " << prev_northing << endl;

      GPSstream << i << "  "
                << HHMMSS << "  "
                << clock.secs_elapsed_since_reference_date() << "  "
                << curr_easting << "  "
                << curr_northing << "  "
                << curr_altitude << "  "
                << endl;

      threevector curr_GPS_posn(curr_easting,curr_northing,curr_altitude);
      GPS_position.push_back(curr_GPS_posn);

      FOVstream << i << "  "
                << HHMMSS << "  "
                << clock.secs_elapsed_since_reference_date() << "  "
                << curr_FOV_U << endl;

// On 29 Sep 2011, we empirically determined the following procedure
// for converting "az", "el", "heading", "pitch", and "roll" from Ross'
// metadata file into an accurate 3D FLIR pointing vector f_hat:

      double phi_h=-90-curr_heading;
      phi_h=basic_math::phase_to_canonical_interval(phi_h,-180,180);
      phi_h *= PI/180;
      
      orientstream << curr_frame_filename << "  "
//                   << clock.secs_elapsed_since_reference_date() << "  "
//                   << curr_easting << "  "
//                   << curr_northing << "  "
//                   << curr_altitude << "  "
                   << curr_az << "  "
                   << curr_el << "  "
//                   << curr_heading << "  "
                   << phi_h*180/PI << "  "
//                   << phi*180/PI << "  "
                   << curr_pitch << "  "
                   << curr_roll << "  "
                   << endl;

// pitch = 0:  |sigma| = 1539.63
// +pitch & roll then pitch:  |sigma|=2303
// -pitch & roll then pitch:  |sigma|=167  
// - pitch & pitch then roll:  |sigma|=112 
// +pitch & pitch then roll: |sigma|=2312 

// Work with basis set l_hat (aircraft left), b_hat (aircraft behind)
// and t_hat (aircraft top).  Note l_hat x b_hat = t_hat . 

// Firstly, right-hand rotate aircraft coordinate system about z_hat
// by angle phi_h:

      threevector h_hat0(cos(phi_h),sin(phi_h),0); // heading dir vector

      threevector b_hat0=-h_hat0;	// backwards = -heading dir vector
      threevector t_hat0=z_hat;			// "top" pointing direction
      threevector l_hat0=b_hat0.cross(t_hat0);  // LHS pointing dir

// Secondly, pitch aircraft coordinate system about r_hat=-l_hat:

      double cos_pitch=cos(-curr_pitch*PI/180);
      double sin_pitch=sin(-curr_pitch*PI/180);

      threevector l_hat=l_hat0;
      threevector t_hat=cos_pitch*t_hat0+sin_pitch*b_hat0;
      threevector b_hat=-sin_pitch*t_hat0+cos_pitch*b_hat0;

// Finally, roll aircraft coordinate system about h_hat=-b_hat:

      double cos_roll=cos(-curr_roll*PI/180);
      double sin_roll=sin(-curr_roll*PI/180);

      threevector b_hat_prime=b_hat;
      threevector l_hat_prime=cos_roll*l_hat+sin_roll*t_hat;
      threevector t_hat_prime=-sin_roll*l_hat+cos_roll*t_hat;

      l_hat=l_hat_prime;
      b_hat=b_hat_prime;
      t_hat=t_hat_prime;

      double cos_az=cos(curr_az*PI/180);
      double sin_az=sin(curr_az*PI/180);
      double cos_el=cos(curr_el*PI/180);
      double sin_el=sin(curr_el*PI/180);

      threevector U_hat=-sin_az*b_hat+cos_az*l_hat;
      threevector V_hat=-sin_el*cos_az*b_hat-sin_el*sin_az*l_hat+cos_el*t_hat;
      threevector f_hat=cos_el*cos_az*b_hat+cos_el*sin_az*l_hat+sin_el*t_hat;
      threevector W_hat=-f_hat;

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

      double d_range=10;	// meters
      if (f_hat.get(2) < 0)
      {
//         const double z_ground=10;// HAFB altitude near flight facility (ALIRT)
//      const double z_ground=38; // HAFB altitude above sea level (GE)
//      const double z_ground=1300; // Rough estimate for Tucson border 
				    //	elevation above sea level in meters
         double range=10000;
         int n_range_points=range/d_range+1;

         double curr_LOS_range=-1;
         for (int r=0; r<n_range_points; r++)
         {
            threevector LOS_posn=curr_GPS_posn+r*d_range*f_hat;
            double curr_z=LOS_posn.get(2);
            if (curr_z < z_ground) continue;

            curr_LOS_range=r*d_range;
            GPS_position.push_back(LOS_posn);
         } // loop over index r labeling range points
         candidate_tgt_posns.push_back(GPS_position.back());
//         cout << "i = " << i << " curr_LOS_range = "
//              << 0.001*curr_LOS_range << " kms " << endl;
         LOS_ranges.push_back(curr_LOS_range);

         double curr_f_angle=atan2(f_hat.get(1),f_hat.get(0));
         curr_f_angle *= 180/PI;
         curr_f_angle=basic_math::phase_to_canonical_interval(
            curr_f_angle,0,360);
         cout << "curr_f_angle = " << curr_f_angle << endl;
         f_angles.push_back(curr_f_angle);
      }

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

      double f,curr_FOV_V;
      curr_FOV_U *= PI/180;
      camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
         curr_FOV_U,aspect_ratio,f,curr_FOV_V);
//      cout << "f = " << f
//           << " FOV_V = " << curr_FOV_V*180/PI << endl;

      rotation R,R1;
      R=R.rotation_taking_pqr_to_uvw(-y_hat,z_hat,-x_hat,U_hat,V_hat,W_hat);

      double frustum_az,frustum_el,frustum_roll;
      R.az_el_roll_from_rotation(frustum_az,frustum_el,frustum_roll);
//      cout << "frustum_az = " << frustum_az*180/PI
//           << " frustum_el = " << frustum_el*180/PI
//           << " frustum_roll = " << frustum_roll*180/PI << endl;

      R1=R.rotation_from_az_el_roll(frustum_az,frustum_el,frustum_roll);

//      cout << "U_hat = " << U_hat << endl;
//      cout << "V_hat = " << V_hat << endl;

// Write out package files containing 3D FLIR camera parameters derived
// from Ross' metadata file:

      string package_filename=hardware_packages_subdir+"photo_"+
         stringfunc::integer_to_string(package_ID,4)+".pkg";

      filefunc::openfile(package_filename,packagestream);
      packagestream.precision(12);
      packagestream << image_filename << endl;
      packagestream << "--photo_ID " << package_ID << endl;
      packagestream << "--Uaxis_focal_length " << f << endl;
      packagestream << "--Vaxis_focal_length " << f << endl;
      packagestream << "--U0 " << 0.5*aspect_ratio << endl;
      packagestream << "--V0 0.5" << endl;
      packagestream << "--relative_az " << frustum_az*180/PI << endl;
      packagestream << "--relative_el " << frustum_el*180/PI << endl;
      packagestream << "--relative_roll " << frustum_roll*180/PI << endl;
      packagestream << "--camera_x_posn " << curr_GPS_posn.get(0) << endl;
      packagestream << "--camera_y_posn " << curr_GPS_posn.get(1) << endl;
      packagestream << "--camera_z_posn " << curr_GPS_posn.get(2) << endl;
      packagestream << "--frustum_sidelength 250 " << endl;
      filefunc::closefile(package_filename,packagestream);

      separated_packages_stream 
         << "--region_filename " << package_filename 
         << " \\" << endl;

      package_ID++;      

   } // loop over index i labeling frame number
   cout << endl;

// Construct histogram for all "f-angles" = azimuths of f_hat pointing
// vectors:

   double fangle_start=0;
   double fangle_stop=360;
   double dfangle=5;
   int n_fangle_bins=(fangle_stop-fangle_start)/dfangle+1;
   vector<int> fangle_histogram;
   for (int n=0; n<n_fangle_bins; n++)
   {
      fangle_histogram.push_back(0);
   }
   for (unsigned int f=0; f<f_angles.size(); f++)
   {
      double curr_f_angle=f_angles[f];
      int n=basic_math::round((curr_f_angle-fangle_start)/dfangle);
      fangle_histogram[n]=fangle_histogram[n]+1;
   }

// Compute total azimuthal fractional coverage of all pointing vectors:

   double fangle_coverage_frac=0;
   for (int n=0; n<n_fangle_bins; n++)
   {
      if (fangle_histogram[n] > 0)
      {
         fangle_coverage_frac += 1.0/n_fangle_bins;
      }
   }

   separated_packages_stream << "--image_list_filename "
                             << image_list_filename << endl;

   filefunc::closefile(waypoints_filename,GPSstream);
   filefunc::closefile(waypoints_filename,FOVstream);
   filefunc::closefile(orientation_filename,orientstream);
   filefunc::closefile(list_tmp_filename,liststream);
   filefunc::closefile(
      separated_packages_scriptname,separated_packages_stream);

   cout << "n_waypoints = " << n_waypoints << endl;

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();

   bool northern_hemisphere_flag=true;
   int UTM_zone=19;	// Boston
   TilesGroup_ptr->set_specified_UTM_zonenumber(UTM_zone);	
   TilesGroup_ptr->set_northern_hemisphere_flag(northern_hemisphere_flag);   
   geopoint lower_left_corner(
      northern_hemisphere_flag,UTM_zone, 325361.000, 4689703.000 , 0);
   geopoint upper_right_corner(
      northern_hemisphere_flag,UTM_zone, 331302.000, 4692873.000 , 0);

/*
   double delta_x=1;	// meter
   double delta_y=1;	// meter
   twoDarray* DTED_ztwoDarray_ptr=
      TilesGroup_ptr->generate_subtile_twoDarray(
         delta_x,delta_y,lower_left_corner,upper_right_corner);

   string geotif_subdir=
      "/data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/tif_files/";
   string geotif_filename=geotif_subdir+"boston_heights_TEC04.tif";
   TilesGroup_ptr->read_geotif_subtile_height_data(
      geotif_filename,DTED_ztwoDarray_ptr);
   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;
*/

// Calculate mean and standard deviation for candidate ground target
// positions:

   vector<double> tgt_X,tgt_Y,tgt_Z;
   for (unsigned int t=0; t<candidate_tgt_posns.size(); t++)
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

// Calculate mean and standard deviatoin for line-of-sight ranges:

   double mu_LOS_range=mathfunc::mean(LOS_ranges);
   double sigma_LOS_range=mathfunc::std_dev(LOS_ranges);
   cout << "LOS range = " << 0.001*mu_LOS_range << " +/- " 
        << 0.001*sigma_LOS_range << " kms" << endl;
   cout << endl;

// Generate TDP file containing flight path, FLIR lines-of-sight, Uhat
// and Vhat direction vectors extracted from Ross' metadata file:

   string tdp_filename=bundler_IO_subdir+"hardware_GPS_flightpath.tdp";
   tdpfunc::write_relative_xyz_data(tdp_filename,GPS_position);

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv hardware_GPS_flightpath.osga "+bundler_IO_subdir;
   sysfunc::unix_command(unix_cmd);   
   string hardware_GPS_flightpath_osga_filename=
      bundler_IO_subdir+"hardware_GPS_flightpath.osga";

   string banner="Flight path and lines-of-sight exported to "+
      bundler_IO_subdir+"hardware_GPS_flightpath.osga";
   outputfunc::write_big_banner(banner);

// Make separated_packages script executable:

   filefunc::make_executable(separated_packages_scriptname);
   banner="Exported "+separated_packages_scriptname;
   outputfunc::write_big_banner(banner);

   string geo_mains_subdir="~/programs/c++/svn/projects/src/mains/geo/";
   unix_cmd="cp "+separated_packages_scriptname+" "+
      geo_mains_subdir;
   sysfunc::unix_command(unix_cmd);

// Call program VIEWPOINTS in order to view 3D image of hardware-based
// flightpath and camera point directions:

//   unix_cmd="viewpoints "+hardware_GPS_flightpath_osga_filename;
//   sysfunc::unix_command(unix_cmd);

   cout << "Pointing vectors' azimuthal coverage fraction = " 
        << fangle_coverage_frac << endl;

   cout << "At end of visualize_FLIR_metadata.cc" << endl;
   outputfunc::print_elapsed_time();
}


