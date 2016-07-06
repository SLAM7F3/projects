// ========================================================================
// Program PANELPKGS generates package files for WISP panorama panels.
// The panoramas can subsequently be viewed via VIEW_WISP.

//				./panelpkgs

// ========================================================================
// Last updated on 3/13/13; 4/24/13; 6/21/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_panoramas(photogroup_ptr->get_n_photos());
   cout << "n_panoramas = " << n_panoramas << endl;
   n_panoramas=1;

//   int pano_number;
//   cout << "Enter panorama number:" << endl;
//   cin >> pano_number;

//   double global_az=0;	// degs
   double global_az=-78.7777;	// degs	(Feb 2013 Deer Island video)
   cout << "Enter global azimuth (in degs) by which entire WISP panorama should be rotated:" << endl;
   cin >> global_az;
   
   int n_panels=10;	
   int horiz_size=n_panels*4000;		// WISP panoramas 

   double Nu=horiz_size/n_panels;
   double Nv=2200;	// WISP panoramas
   double Umax=double(Nu)/double(Nv);
   double U0=0.5*Nu/Nv;

   double FOVu=360.0/n_panels;	
   cout << "FOVu = " << FOVu << endl;
   FOVu *= PI/180;		// Horizontal FOV for each panel in rads
   double FOVv=30*PI/180;	// Vertical FOV for each WISP panel in rads

// Important note:  On 4/8/13, Gary Long discovered that raw WISP
// panorama pixels are NOT square!

   double fu=camerafunc::fu_from_horiz_FOV_and_Umax(FOVu,Umax);
   double fv=camerafunc::fv_from_vert_FOV(FOVv);

   FOVu *= 180/PI;
   cout << "fu = " << fu << endl;
   cout << "fv = " << fv << endl;

   typedef std::map<int,fourvector> PANO_POSN_AZ_MAP;
   PANO_POSN_AZ_MAP* pano_posn_az_map_ptr=new PANO_POSN_AZ_MAP;

//   double wisp_easting=100;
//   double wisp_northing=100;
   double wisp_easting=339105.7;
   double wisp_northing=4690478.5;
   double wisp_altitude=10;
   threevector camera_world_posn(wisp_easting,wisp_northing,wisp_altitude);
   for (int i=0; i<n_panoramas; i++)
   {
      int curr_pano_ID=i;
      (*pano_posn_az_map_ptr)[curr_pano_ID]=fourvector(camera_world_posn,0);
   }

   string panels_subdir=bundler_IO_subdir+"images/panels/";
   string panels_package_subdir=bundler_IO_subdir+"packages/panels/";
   filefunc::dircreate(panels_package_subdir);
   ofstream outstream,viewscriptstream;
   
   string viewscript_filename="run_view_wisp";
   filefunc::openfile(viewscript_filename,viewscriptstream);
//   viewscriptstream << "/home/cho/programs/c++/svn/projects/src/mains/china/new_fov \\" << endl;
   viewscriptstream << "/home/cho/programs/c++/svn/projects/src/mains/dime/view_wisp \\" << endl;

//   double frustum_sidelength=10;	// Wisp panos
   double frustum_sidelength=30;	// Deer Island Wisp panos
   vector<threevector> pano_center_posns;
   for (int pano_number=0; pano_number < n_panoramas; pano_number++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(pano_number);
      string pano_filename=photo_ptr->get_filename();
      string pano_basename=stringfunc::prefix(filefunc::getbasename(
         pano_filename));
//      cout << "pano_basename = " << pano_basename << endl;

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         pano_basename,"_.");
      int pano_ID=stringfunc::string_to_number(substrings[2]);	// WISP
//       cout << pano_basename << " pano ID = " << pano_ID << endl;

      PANO_POSN_AZ_MAP::iterator iter=pano_posn_az_map_ptr->find(pano_ID);
      if (iter == pano_posn_az_map_ptr->end()) continue;

      fourvector pano_posn_az=iter->second;
      threevector pano_posn(pano_posn_az);

      pano_center_posns.push_back(pano_posn);

      for (int panel=0; panel<n_panels; panel++)
      {
//         cout << "panel = " << panel << endl;
         string prefix=substrings[0]+"_p"+stringfunc::number_to_string(panel);
         for (unsigned int s=1; s<substrings.size(); s++)
         {
            prefix += +"_"+substrings[s];
         }
//         cout << " prefix = " << prefix << endl;
         string pkg_filename=panels_package_subdir+prefix+".pkg";
//         cout << "pkg_filename = " << pkg_filename << endl;
         filefunc::openfile(pkg_filename,outstream);
//         double curr_panel_az=global_az-panel*FOVu+pano_az[pano_number];
         double curr_panel_az=global_az-panel*FOVu+pano_posn_az.get(3);
         //cout << "curr_panel_az = " << curr_panel_az << " degs" << endl;

//         cout << "panel*FOVu = " << panel*FOVu << endl;
//         cout << "pano_az[pano_number] = " << pano_az[pano_number] << endl;

//         string image_suffix="jpg";
         string image_suffix="png";
         string image_filename=panels_subdir+prefix+"."+image_suffix;
//         cout << "image_filename = " << image_filename << endl;

         outstream << image_filename << endl;
         outstream << "--Uaxis_focal_length "+stringfunc::number_to_string(fu)
                   << endl;
         outstream << "--Vaxis_focal_length "+stringfunc::number_to_string(fv)
                   << endl;
         outstream << "--U0 "+stringfunc::number_to_string(U0)
            +" --V0 0.5" << endl;
         outstream << "--relative_az "+stringfunc::number_to_string(
            curr_panel_az) << endl;
         outstream << "--relative_el 0" << endl;
         outstream << "--relative_roll 0" << endl;
         outstream << "--camera_x_posn "+stringfunc::number_to_string(
            pano_posn_az.get(0)) << endl;
         outstream << "--camera_y_posn "+stringfunc::number_to_string(
            pano_posn_az.get(1)) << endl;
         outstream << "--camera_z_posn "+stringfunc::number_to_string(
            pano_posn_az.get(2)) << endl;
         outstream << "--frustum_sidelength "+stringfunc::number_to_string(
            frustum_sidelength) << endl;	
         filefunc::closefile(pkg_filename,outstream);

         viewscriptstream << "--region_filename "+pkg_filename+" \\" << endl;

      } // loop over panel index

   } // loop over pano_number index
   
//   viewscriptstream << "--image_list_filename ./bundler/DIME/Feb2013_DeerIsland/image_list.dat \\" << endl;
   viewscriptstream << "--image_list_filename " 
                    << image_list_filename << " \\" << endl;
   viewscriptstream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;
   filefunc::closefile(viewscript_filename,viewscriptstream);

   string unix_cmd="chmod a+x "+viewscript_filename;
   sysfunc::unix_command(unix_cmd);

}
