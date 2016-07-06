// ========================================================================
// Program GENERATE_WAGONWHEELS takes in the directories for the
// initial, raw D7 jpeg images as well as for the text file containing
// D7 position and orientation metadata.  It generates output
// "package" files containing pinhole camera parameters (postion,
// orientation, field-of-view, frustum size) which are needed to view
// each individual D7 panel as a view frustum in 3D.
// GENERATE_WAGONWHEELS writes out the package files to a new
// subdirectory.  It also creates an executable script which can be
// used to easily view all of the 3D frusta via program VIEWPANOS.

// To run this program, chant  

//      ~/programs/c++/svn/projects/src/mains/hmi/generate_wagonwheels

// ========================================================================
// Last updated on 7/5/11; 7/13/11; 9/2/11
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
#include "general/sysfuncs.h"
#include "math/threevector.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   sysfunc::clearscreen();   

   string images_subdir="./D7/TEST_072011";
//   string images_subdir="/media/66368D22368CF3F9/hmi/killian/TEST_073111/";
//   string images_subdir="./killian/TEST_073111/";
//   cout << "Enter full path for directory containing initial raw D7 jpeg images:" 
//        << endl;
//   cin >> images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);

   string cropped_images_subdir=images_subdir+"cropped/";
   string panels_subdir=cropped_images_subdir+"panels/";

   cout << "panels_subdir = " << panels_subdir << endl;

   string panels_package_subdir=panels_subdir+"packages/";
   filefunc::dircreate(panels_package_subdir);

//   string camera_posns_filename=cropped_images_subdir+"pano_centers.dat";
   string camera_posns_filename=cropped_images_subdir+"wagonwheel_UTM_posn_pointing.dat";

   cout << endl;
//   cout << "Enter full pathname for input text file containing D7 camera positions and azimuthal orientations:" << endl;
//   cin >> camera_posns_filename;

// For each D7 wagonwheel, first read in 10 panel JPG files generated
// by program GENERATE_D7_PANELS:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> image_panel_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,panels_subdir);

   photogroup* photogroup_ptr=new photogroup;
   for (unsigned int i=0; i<image_panel_filenames.size(); i++)
   {
      photograph* photo_ptr=photogroup_ptr->generate_single_photograph(
         image_panel_filenames[i]);
//      cout << "i = " << i 
//           << " image_panel_filename = " << image_panel_filenames[i]
//           << " xdim = " << photo_ptr->get_xdim()
//           << " ydim = " << photo_ptr->get_ydim() 
//           << endl;
   }

   const int n_panels_per_pano=10;
   int n_panoramas(photogroup_ptr->get_n_photos()/n_panels_per_pano);
   cout << "n_panoramas = " << n_panoramas << endl;

   double global_az=0;	// degs
   double FOVu=360.0/n_panels_per_pano;
//   cout << "FOVu = " << FOVu << endl;
   FOVu *= PI/180;
//   double fudge_factor=1.16;	// n_panels==5
   double fudge_factor=1.04;	// n_panels==10
//   cout << "Enter fudge factor:" << endl;
//   cin >> fudge_factor;

// Parse text file containing relative camera positions & azimuthal
// orientations:

//   cout << "camera_posns_filename = " << camera_posns_filename << endl;
   filefunc::ReadInfile(camera_posns_filename);
//   cout << "text_line.size() = " << filefunc::text_line.size() << endl;

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
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      if (filefunc::text_line[i].size() < 10) continue;
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i],",");
      int curr_pano_ID=column_values[0];
      threevector curr_pano_posn(
         column_values[1],column_values[2],camera_height);
//      double curr_pano_yaw=-column_values[6]*180/PI;
      double curr_pano_yaw=-column_values[6];
      double curr_pano_az=180-curr_pano_yaw;

      pano_ID.push_back(curr_pano_ID);
      pano_center_posn.push_back(curr_pano_posn);
      pano_az.push_back(curr_pano_az);

      min_camera_X=basic_math::min(min_camera_X,curr_pano_posn.get(0));
      min_camera_Y=basic_math::min(min_camera_Y,curr_pano_posn.get(1));
      max_camera_X=basic_math::max(max_camera_X,curr_pano_posn.get(0));
      max_camera_Y=basic_math::max(max_camera_Y,curr_pano_posn.get(1));
      cout << "pano_ID = " << pano_ID.back() 
           << " pano_center_posn = " << pano_center_posn.back() << endl;
   }
   double delta_camera_X=max_camera_X-min_camera_X;
   double delta_camera_Y=max_camera_Y-min_camera_Y;

   cout << "min_camera_X = " << min_camera_X
        << " min_camera_Y = " << min_camera_Y << endl;
   cout << "max_camera_X = " << max_camera_X
        << " max_camera_Y = " << max_camera_Y << endl;
   cout << "delta_camera_X = " << delta_camera_X
        << " delta_camera_Y = " << delta_camera_Y << endl;

   for (unsigned int i=0; i<pano_ID.size(); i++)
   {
      int curr_pano_ID=pano_ID[i];
      (*pano_posn_az_map_ptr)[curr_pano_ID]=
         fourvector(pano_center_posn[i],pano_az[i]);
//      cout << curr_pano_ID << "  " << pano_center_posn[i] 
//           << "  " << pano_az[i] << endl;
   }

   ofstream outstream,viewscriptstream;
   string viewscript_filename="run_viewpanos";
   filefunc::openfile(viewscript_filename,viewscriptstream);
   viewscriptstream << "~/programs/c++/svn/projects/src/mains/hmi/viewpanos \\" << endl;

   double frustum_sidelength=1.0;	// Outdoor D7 panos
   vector<threevector> pano_center_posns;

   int photo_ID=0;
   for (int pano_number=0; pano_number < n_panoramas; pano_number++)
   {
      int pano_ID=pano_number;
      string pano_label=stringfunc::integer_to_string(pano_number,3);
      cout << "pano_label = " << pano_label << endl;

      PANO_POSN_AZ_MAP::iterator iter=pano_posn_az_map_ptr->find(pano_ID);
      if (iter == pano_posn_az_map_ptr->end()) continue;

      fourvector pano_posn_az=iter->second;
      threevector pano_posn(pano_posn_az);
      pano_center_posns.push_back(pano_posn);

      for (int panel=0; panel<n_panels_per_pano; panel++)
      {
//         cout << "panel = " << panel << endl;
         string prefix="pano_"+pano_label+
            "_p"+stringfunc::number_to_string(panel);
         string pkg_filename=panels_package_subdir+prefix+".pkg";
//         cout << "pkg_filename = " << pkg_filename << endl;
         filefunc::openfile(pkg_filename,outstream);

         double curr_panel_az=global_az-panel*FOVu*180/PI+pano_posn_az.get(3);
//         cout << "panel*FOVu = " << panel*FOVu << endl;
//         cout << "pano_az[pano_number] = " << pano_az[pano_number] << endl;

         photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(photo_ID++);
         string image_filename=photo_ptr->get_filename();
         double Nu=photo_ptr->get_xdim();
         double Nv=photo_ptr->get_ydim();
         double U0=0.5*Nu/Nv;
//         double V0=0.5;
         double f=1/(2*Nv) / (tan(fudge_factor*FOVu/(2*Nu)) );

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
         filefunc::closefile(pkg_filename,outstream);

         viewscriptstream << "--region_filename "+pkg_filename+" \\" << endl;

      } // loop over panel index

   } // loop over pano_number index

   viewscriptstream << "--initial_mode Manipulate_Fused_Data_Mode" << endl;
   filefunc::closefile(viewscript_filename,viewscriptstream);
   string unix_cmd="chmod a+x "+viewscript_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Executable script for viewing D7 images as 3D 'wagonwheels' written to "+viewscript_filename;
   outputfunc::write_big_banner(banner);
}
