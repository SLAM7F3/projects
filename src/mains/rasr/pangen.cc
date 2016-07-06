// ========================================================================
// Program PANGEN generates package files for panorama panels.  The
// panoramas can subsequently be viewed via VIEWPANOS.

//				pangen

// ========================================================================
// Last updated on 2/9/11; 2/22/11; 2/23/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
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
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/rasr/";
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
   int pano_start=0;
   int pano_stop=162;
//   int pano_stop=30;	

   double global_az=0;	// degs

   int n_panels=10;	
   int horiz_size=10*432;		// Auditorium panoramas (Feb 9)
//   int horiz_size=5120;		// Auditorium panoramas
//   int horiz_size=3796;		// G47 area panoramas

   double Nu=horiz_size/n_panels;
   double Nv=768;	// Auditorium panoramas
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

// Parse text file containing relative camera positions:

//   string camera_posns_filename=bundler_IO_subdir+
//      "rotated_pano_centers.dat";
   string camera_posns_filename=bundler_IO_subdir+"pano_centers.dat";
//   string camera_posns_filename=bundler_IO_subdir+"g47_pano_centers.dat";
   cout << "camera_posns_filename = " << camera_posns_filename << endl;
   filefunc::ReadInfile(camera_posns_filename);
   cout << "text_line.size() = " << filefunc::text_line.size() << endl;

   typedef std::map<int,fourvector> PANO_POSN_AZ_MAP;
   PANO_POSN_AZ_MAP* pano_posn_az_map_ptr=new PANO_POSN_AZ_MAP;

   double min_camera_X=POSITIVEINFINITY;
   double min_camera_Y=POSITIVEINFINITY;
   double max_camera_X=NEGATIVEINFINITY;
   double max_camera_Y=NEGATIVEINFINITY;
   const double camera_height=0;	// meters
   vector<int> pano_ID;
   vector<double> pano_az;
   vector<threevector> pano_center_posn;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      if (filefunc::text_line[i].size() < 10) continue;
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i],",");
      int curr_pano_ID=column_values[0];
      threevector curr_pano_posn(
         column_values[1],column_values[2],camera_height);
      double curr_pano_yaw=-column_values[6]*180/PI;
//      double curr_pano_az=-90-curr_pano_yaw;	// WAY WRONG
//      double curr_pano_az=0-curr_pano_yaw;	// WRONG
//      double curr_pano_az=45-curr_pano_yaw;	//REASONABLE
//      double curr_pano_az=90-curr_pano_yaw;
//      double curr_pano_az=135-curr_pano_yaw;	//REASONABLE
      double curr_pano_az=180-curr_pano_yaw;

      pano_ID.push_back(curr_pano_ID);
      pano_center_posn.push_back(curr_pano_posn);
      pano_az.push_back(curr_pano_az);

      min_camera_X=basic_math::min(min_camera_X,curr_pano_posn.get(0));
      min_camera_Y=basic_math::min(min_camera_Y,curr_pano_posn.get(1));
      max_camera_X=basic_math::max(max_camera_X,curr_pano_posn.get(0));
      max_camera_Y=basic_math::max(max_camera_Y,curr_pano_posn.get(1));
//      cout << "pano_ID = " << pano_ID.back() 
//           << " pano_center_posn = " << pano_center_posn.back() << endl;
   }
   double delta_camera_X=max_camera_X-min_camera_X;
   double delta_camera_Y=max_camera_Y-min_camera_Y;

   cout << "min_camera_X = " << min_camera_X
        << " min_camera_Y = " << min_camera_Y << endl;
   cout << "max_camera_X = " << max_camera_X
        << " max_camera_Y = " << max_camera_Y << endl;
   cout << "delta_camera_X = " << delta_camera_X
        << " delta_camera_Y = " << delta_camera_Y << endl;

   for (int i=0; i<pano_ID.size(); i++)
   {
      int curr_pano_ID=pano_ID[i];
      (*pano_posn_az_map_ptr)[curr_pano_ID]=
         fourvector(pano_center_posn[i],pano_az[i]);
//      cout << curr_pano_ID << "  " << pano_center_posn[i] 
//           << "  " << pano_az[i] << endl;
   }

   string panels_subdir=bundler_IO_subdir+"images/panels/";
   string panels_package_subdir="./packages/aud_dset2/panels/";
//   string panels_package_subdir="./packages/auditorium/panels/";
   ofstream outstream,viewscriptstream;
   
   string viewscript_filename="run_viewpanos";
   filefunc::openfile(viewscript_filename,viewscriptstream);
   viewscriptstream << "/home/cho/programs/c++/svn/projects/src/mains/rasr/viewpanos \\" << endl;

   double frustum_sidelength=0.5;	// Auditorium panos
   vector<threevector> pano_center_posns;
   for (int pano_number=0; pano_number < n_panoramas; pano_number++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(pano_number);
      string pano_filename=photo_ptr->get_filename();
      string pano_basename=stringfunc::prefix(filefunc::getbasename(
         pano_filename));

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         pano_basename,"_.");
      int pano_ID=stringfunc::string_to_number(substrings[0]);	// auditorium
//      int pano_ID=stringfunc::string_to_number(substrings[2]);  // G47 area
//      cout << pano_basename << " pano ID = " << pano_ID << endl;

      PANO_POSN_AZ_MAP::iterator iter=pano_posn_az_map_ptr->find(pano_ID);
      if (iter == pano_posn_az_map_ptr->end()) continue;

      fourvector pano_posn_az=iter->second;
      threevector pano_posn(pano_posn_az);

// Check if candidate new panorama lies too close to already existing
// panoramas.  If so, do NOT write its package files out to disk:

      bool instantiate_panorama_flag=true;
      for (int k=0; k<pano_center_posns.size(); k++)
      {
         if ( (pano_posn-pano_center_posns[k]).magnitude() < 
              2*frustum_sidelength)
         {
            instantiate_panorama_flag=false;
            break;
         }
      } // loop over index k labeling previously already existing panos
      
      if (!instantiate_panorama_flag) continue;
      pano_center_posns.push_back(pano_posn);

      for (int panel=0; panel<n_panels; panel++)
      {
//         cout << "panel = " << panel << endl;
         string prefix=substrings[0]+"_p"+stringfunc::number_to_string(panel);
         string pkg_filename=panels_package_subdir+prefix+".pkg";
//         cout << "pkg_filename = " << pkg_filename << endl;
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

}
