// ========================================================================
// Program PANELPKGS generates package files for panorama panels.  The
// panoramas can subsequently be viewed via VIEWPANOS.

//				PANELPKGS

// ========================================================================
// Last updated on 2/22/11; 2/23/11; 5/29/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

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

   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/wisp/";
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_panoramas(photogroup_ptr->get_n_photos());
   cout << "n_panoramas = " << n_panoramas << endl;

//   int pano_number;
//   cout << "Enter panorama number:" << endl;
//   cin >> pano_number;
//   int pano_start=0;
//   int pano_stop=23;
//   int pano_stop=30;	

   double global_az=0;	// degs

   int n_panels=10;	
   int horiz_size=10*4000;		// WISP panoramas 
//   int horiz_size=10*432;		// Auditorium panoramas (Feb 9)
//   int horiz_size=5120;		// Auditorium panoramas
//   int horiz_size=3796;		// G47 area panoramas

   double Nu=horiz_size/n_panels;
   double Nv=2200;	// WISP panoramas
//   double Nv=768;	// Auditorium panoramas
//   double Nv=1113;	// G47 area panoramas
   double U0=0.5*Nu/Nv;

   double FOVu=360.0/n_panels;
   cout << "FOVu = " << FOVu << endl;
   FOVu *= PI/180;
//   double fudge_factor=1.16;	// n_panels==5
   double fudge_factor=1.04;	// n_panels==10
//   cout << "Enter fudge factor:" << endl;
//   cin >> fudge_factor;
   double f=1/(2*Nv) / (tan(fudge_factor*FOVu/(2*Nu)) );
   FOVu *= 180/PI;
   cout << "f = " << f << endl;

   typedef std::map<int,fourvector> PANO_POSN_AZ_MAP;
   PANO_POSN_AZ_MAP* pano_posn_az_map_ptr=new PANO_POSN_AZ_MAP;

   for (int i=0; i<n_panoramas; i++)
   {
      int curr_pano_ID=i;
      (*pano_posn_az_map_ptr)[curr_pano_ID]=
         fourvector(0,0,0,0);
//      cout << curr_pano_ID << "  " << pano_center_posn[i] 
//           << "  " << pano_az[i] << endl;
   }

   string panels_subdir=bundler_IO_subdir+"images/panels/";
   string panels_package_subdir="./packages/panels/";
   ofstream outstream,viewscriptstream;
   
   string viewscript_filename="run_newfov";
   filefunc::openfile(viewscript_filename,viewscriptstream);
   viewscriptstream << "/home/cho/programs/c++/svn/projects/src/mains/china/new_fov \\" << endl;

   double frustum_sidelength=10;	// Wisp panos
//   double frustum_sidelength=0.5;	// Auditorium panos
   vector<threevector> pano_center_posns;
   for (int pano_number=0; pano_number < n_panoramas; pano_number++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(pano_number);
      string pano_filename=photo_ptr->get_filename();
      string pano_basename=stringfunc::prefix(filefunc::getbasename(
         pano_filename));

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         pano_basename,"_.");
      int pano_ID=stringfunc::string_to_number(substrings[1]);	// WISP
//      int pano_ID=stringfunc::string_to_number(substrings[0]);	// auditorium
//      int pano_ID=stringfunc::string_to_number(substrings[2]);  // G47 area
//      cout << pano_basename << " pano ID = " << pano_ID << endl;

      PANO_POSN_AZ_MAP::iterator iter=pano_posn_az_map_ptr->find(pano_ID);
      if (iter == pano_posn_az_map_ptr->end()) continue;

      fourvector pano_posn_az=iter->second;
      threevector pano_posn(pano_posn_az);

      pano_center_posns.push_back(pano_posn);

      for (int panel=0; panel<n_panels; panel++)
      {
//         cout << "panel = " << panel << endl;
         string prefix=substrings[0]+"_"+substrings[1]+"_p"
            +stringfunc::number_to_string(panel);
         string pkg_filename=panels_package_subdir+prefix+".pkg";
         cout << "pkg_filename = " << pkg_filename << endl;
         filefunc::openfile(pkg_filename,outstream);

//         double curr_panel_az=global_az-panel*FOVu+pano_az[pano_number];
         double curr_panel_az=global_az-panel*FOVu+pano_posn_az.get(3);

//         cout << "panel*FOVu = " << panel*FOVu << endl;
//         cout << "pano_az[pano_number] = " << pano_az[pano_number] << endl;

//         string image_suffix="jpg";
         string image_suffix="png";
         string image_filename=panels_subdir+prefix+"."+image_suffix;
         outstream << image_filename << endl;
         outstream << "--Uaxis_focal_length "+stringfunc::number_to_string(-f)
                   << endl;
         outstream << "--Vaxis_focal_length "+stringfunc::number_to_string(-f)
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
//         outstream << "--frustum_sidelength 1" << endl;	
//         outstream << "--frustum_sidelength 1.5" << endl;
//         outstream << "--frustum_sidelength 50" << endl;
         filefunc::closefile(pkg_filename,outstream);

         viewscriptstream << "--region_filename "+pkg_filename+" \\" << endl;

      } // loop over panel index

   } // loop over pano_number index
   
   viewscriptstream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;
   filefunc::closefile(viewscript_filename,viewscriptstream);

   string unix_cmd="chmod a+x "+viewscript_filename;
   sysfunc::unix_command(unix_cmd);

}
