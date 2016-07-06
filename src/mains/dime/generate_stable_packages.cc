// =======================================================================
// Program GENERATE_STABLE_PACKAGES generates package files for
// panels corresponding to stabilized and UV-corrected WISP panoramas.

//			  ./generate_stable_packages

// =======================================================================
// Last updated on 3/29/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "track/tracks_group.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string packages_subdir=bundler_IO_subdir+"packages/";
   cout << "packages_subdir = " << packages_subdir << endl;
   string panels_subdir=bundler_IO_subdir+"images/panels/";

   int framenumber_start=100;
   int framenumber_stop=400;
   int d_framenumber=100;
   int nframes=(framenumber_stop-framenumber_start)/d_framenumber+1;

   ofstream outstream;
   for (int framenumber=framenumber_start; 
        framenumber <= framenumber_stop; framenumber += d_framenumber)
   {
      cout << "frame = " << framenumber << endl;
      int n_panels=10;
      double az_0=-78.777777777;	// degs
      for (int p=0; p<n_panels; p++)
      {
         string package_filename=packages_subdir+
            "stable_p"+stringfunc::number_to_string(p)+
            "_uvcorrected_wisp_res0_"+
            stringfunc::integer_to_string(framenumber,5)+".pkg";
         filefunc::openfile(package_filename,outstream);

         string png_filename=panels_subdir+
            "stable_p"+stringfunc::number_to_string(p)+
            "_uvcorrected_wisp_res0_"+
            stringfunc::integer_to_string(framenumber,5)+".png";
         
         double curr_az=az_0-36*p;
         curr_az=basic_math::phase_to_canonical_interval(
            curr_az,-180,180);
         outstream << png_filename << endl;
         outstream << "--Uaxis_focal_length -2.797894125" << endl;
         outstream << "--Vaxis_focal_length -2.797894125" << endl;
         outstream << "--U0 0.9090909091" << endl;
         outstream << "--V0 0.5" << endl;
         outstream << "--relative_az "+stringfunc::number_to_string(
            curr_az) << endl;
         outstream << "--relative_el 0" << endl;
         outstream << "--relative_roll 0" << endl;
         outstream << "--camera_x_posn 339106.0631" << endl;
         outstream << "--camera_y_posn 4690476.511" << endl;
         outstream << "--camera_z_posn 12.73415031" << endl;
         outstream << "--frustum_sidelength 25" << endl;

         filefunc::closefile(package_filename,outstream);



      } // loop over index p labeling panel
   } // loop over index n labeling package ID
   

}
