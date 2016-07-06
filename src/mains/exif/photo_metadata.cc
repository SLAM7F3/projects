// ========================================================================
// Program PHOTOS_METADATA parses exif metadata for a set of input
// photos residing in a specified imagery subdirectory.  It removes the 
// mean easting, northing and altitude from the input GPS coordinates.
// 3D residuals relative to the GPS COM are exported to an OSGA file.
// ========================================================================
// Last updated on 3/8/14
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/outputfuncs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   const double f_iphone = -1.22;

   string bundler_IO_subdir = "./bundler/pear_tree/";
   string images_subdir = bundler_IO_subdir+"images/";
   string raw_packages_subdir = bundler_IO_subdir+"raw_packages/";
   filefunc::dircreate(raw_packages_subdir);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(images_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   vector<double> easting, northing, altitude;
   vector<double> az, el, roll;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();

      photo_ptr->parse_Exif_metadata();
      cout << "==================================================" << endl;
      cout << "photo filename = " << photo_ptr->get_filename()
           << endl;
      cout << "photo.date = " 
           << photo_ptr->get_clock().YYYY_MM_DD_H_M_S() << endl;

      rpy curr_rpy = photo_ptr->get_pointing();
      double curr_az = basic_math::phase_to_canonical_interval(
         90 - curr_rpy.get_yaw(), 0, 360);
      az.push_back(curr_az*PI/180);
      el.push_back(0);
      roll.push_back(0);

//      cout << "photo.rpy = " << curr_rpy  << endl;
      cout << "  curr_az = " << curr_az << endl;
      cout << endl;

      easting.push_back(photo_ptr->get_geolocation().get_UTM_easting());
      northing.push_back(photo_ptr->get_geolocation().get_UTM_northing());
      altitude.push_back(photo_ptr->get_geolocation().get_altitude());

      camera_ptr->set_f(f_iphone);
//      camera_ptr->construct_internal_parameter_K_matrix();
      camera_ptr->set_Rcamera(curr_az * PI/180, 0, 0);
   }

   outputfunc::enter_continue_char();

   double mu_easting = mathfunc::mean(easting);
   double mu_northing = mathfunc::mean(northing);
   double mu_altitude = mathfunc::mean(altitude);

   int ndigits=4;
   vector<threevector> XYZ;
   cout.precision(5);
   for (int n=0; n<n_photos; n++)
   {
      double delta_easting = easting[n]-mu_easting;
      double delta_northing = northing[n]-mu_northing;
      double delta_altitude = altitude[n]-mu_altitude;
      cout << "n = " << n 
           << " d_east = " << delta_easting
           << " d_north = " << delta_northing 
           << " d_alt = " << delta_altitude << endl;
      XYZ.push_back(threevector(delta_easting,delta_northing,delta_altitude));

      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptr->set_world_posn(XYZ.back());
      camera_ptr->construct_seven_param_projection_matrix();
      cout << "camera = " << *camera_ptr << endl;

      string photo_filename=photo_ptr->get_filename();
      cout << "photo_filename = " << photo_filename << endl;
      string prefix=filefunc::getprefix(photo_filename);
      string separator_chars="_.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         prefix,separator_chars);
      int photo_ID=stringfunc::string_to_number(substrings[1]);
      string package_filename=raw_packages_subdir+"photo_"+
         stringfunc::integer_to_string(photo_ID,ndigits)+".pkg";
      cout << "package_filename = " << package_filename << endl;
      double frustum_sidelength=1.0;	// meters
      camera_ptr->write_camera_package_file(
         package_filename,n,photo_filename,frustum_sidelength);
   }
   
   string prefix=bundler_IO_subdir+"reconstructed_camera_posns";
   string tdp_filename=prefix+".tdp";
   threevector origin(0,0,0);
   tdpfunc::write_relative_xyz_data(tdp_filename,origin,XYZ);

   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);
   string osga_filename=prefix+".osga";
   unix_cmd="mv reconstructed_camera_posns.osga "+bundler_IO_subdir;
   sysfunc::unix_command(unix_cmd);

   string banner="Relative camera GPS positions exported to "+
      osga_filename;
   outputfunc::write_big_banner(banner);

}
