// ==========================================================================
// Program WARP_PMVS computes a relatively fine transformation needed
// to map 3D feature tiepoints manually extracted from ladar and PMVS
// point clouds in order to map the latter onto the former.  After
// reading in corresponding sets of 3D features from text files,
// WARP_PMVS computes a translation, rotation and scaling.  It then
// applies this combined transformation to both the PMVS points output
// by mains/aerosynth/PLY2TDP.  It also transforms the reconstructed
// cameras' world positions and pointing directions.  WARP_PMVS
// outputs a (hopefully improved) PMVS point cloud in ladar_pmvs.tdp,
// and it overwrites reconstructed camera package files with new ones
// which should (hopefully) align with ladar truth data.

// ./warp_pmvs --region_filename ./bundler/lighthawk/packages/peter_inputs.pkg

// ==========================================================================
// Last updated on 1/27/11; 1/28/11; 2/28/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(10);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string bundle_filename=passes_group.get_bundle_filename();
   cout << "bundle_filename = " << bundle_filename << endl;

   double fitted_world_to_bundler_distance_ratio=
      passes_group.get_fitted_world_to_bundler_distance_ratio();
   cout << "world_to_bundler_ratio = " 
        << fitted_world_to_bundler_distance_ratio << endl;
   threevector fitted_bundler_trans=passes_group.get_bundler_translation();
   cout << "fitted_bundler_trans = " << fitted_bundler_trans << endl;
   double global_az=passes_group.get_global_az();
   double global_el=passes_group.get_global_el();
   double global_roll=passes_group.get_global_roll();
   cout << "global_az = " << global_az*180/PI << endl;
   cout << "global_el = " << global_el*180/PI << endl;
   cout << "global_roll = " << global_roll*180/PI << endl;
   threevector bundler_rotation_origin=
      passes_group.get_bundler_rotation_origin();
   cout << "bundler_rotation_origin = " << bundler_rotation_origin << endl;

// Read in 3D tiepoints manually identified in ladar and PMVS point clouds:

   string ladar_features_filename="features_3D_ladar.txt";
   string pmvs_features_filename="features_3D_pmvs.txt";

   vector<threevector> ladar_features,pmvs_features;
   
   filefunc::ReadInfile(pmvs_features_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector curr_pmvs_feature(
         column_values[3],column_values[4],column_values[5]);
      pmvs_features.push_back(curr_pmvs_feature);
   }
   
   filefunc::ReadInfile(ladar_features_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector curr_ladar_feature(
         column_values[3],column_values[4],column_values[5]);
      ladar_features.push_back(curr_ladar_feature);
   }
   
// Compute COM for ladar and PMVS point cloud features:

   threevector ladar_COM,pmvs_COM;
   for (unsigned int i=0; i<ladar_features.size(); i++)
   {
      ladar_COM += ladar_features[i];
      pmvs_COM += pmvs_features[i];
   }
   
   ladar_COM /= ladar_features.size();
   pmvs_COM /= pmvs_features.size();
   
   cout << "ladar_COM = " << ladar_COM << endl;
   cout << "pmvs_COM = " << pmvs_COM << endl;
   threevector trans=ladar_COM-pmvs_COM;
   
   cout << "trans = " << trans << endl;

// Translate PMVS and ladar features s.t. their COMs align with
// (0,0,0):

   for (unsigned int i=0; i<ladar_features.size(); i++)
   {
      pmvs_features[i] -= pmvs_COM;
      ladar_features[i] -= ladar_COM;
   }

   vector<double> scale_factors;
   vector<threevector> rhat_ladar,rhat_pmvs;
   for (unsigned int i=0; i<ladar_features.size(); i++)
   {
      double curr_scalefactor=ladar_features[i].magnitude()/
         pmvs_features[i].magnitude();
      scale_factors.push_back(curr_scalefactor);

      rhat_ladar.push_back(ladar_features[i].unitvector());
      rhat_pmvs.push_back(pmvs_features[i].unitvector());
   }
   
   double mu_scale=mathfunc::mean(scale_factors);
   double sigma_scale=mathfunc::std_dev(scale_factors);
   cout << "Scale from pmvs to ladar = " << mu_scale 
        << " +/- " << sigma_scale << endl << endl;

// Compute rotation which maps PMVS feature vectors onto ladar feature
// vectors:

   rotation R;
   R.rotation_between_ray_bundles(rhat_ladar,rhat_pmvs);

   double az,el,roll;
   R.az_el_roll_from_rotation(az,el,roll);

   cout << "Angles to rotate PMVS to ladar coordinates:" << endl << endl;
   cout << "d_az = " << az*180/PI << ";" << endl;
   cout << "d_el = " << el*180/PI << ";" << endl;
   cout << "d_roll = " << roll*180/PI << ";" << endl << endl;

// Read in PMVS TDP file and store its XYZ points within STL vectors:

   string tdp_filename=bundler_IO_subdir+"pmvs_options.txt.10.tdp";
   
   vector<int> Red,G,B;
   vector<double> X,Y,Z;
   tdpfunc::read_XYZRGB_points_from_tdpfile(tdp_filename,X,Y,Z,Red,G,B);

// Apply transformation to XYZ points read in from TDP file:

   for (unsigned int i=0; i<X.size(); i++)
   {
      threevector curr_XYZ(X[i],Y[i],Z[i]);
      curr_XYZ -= pmvs_COM;
      curr_XYZ *= mu_scale;
      curr_XYZ = R*curr_XYZ;
      curr_XYZ += ladar_COM;
      X[i]=curr_XYZ.get(0);
      Y[i]=curr_XYZ.get(1);
      Z[i]=curr_XYZ.get(2);
   } // loop over index i labeling XYZ points

// Write out new TDP file containing XYZ points from PMVS which better
// align with ladar data:

   string UTMzone="";
   tdp_filename=bundler_IO_subdir+"pmvs_ladar.tdp";
   tdpfunc::write_relative_xyzrgba_data(
      UTMzone,tdp_filename,X,Y,Z,Red,G,B);

// ------------------------------------------------------------------------
// Instantiate reconstructed photos:

   cout << "Instantiating photogroup:" << endl;
   photogroup* photogroup_ptr=new photogroup();
   photogroup_ptr->set_UTM_zonenumber(14);	// Lubbock, TX
//   photogroup_ptr->set_UTM_zonenumber(19);	// Boston/Lowell
   photogroup_ptr->set_northern_hemisphere_flag(true);

   int n_photos_to_reconstruct=-1;
   photogroup_ptr->reconstruct_bundler_cameras(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      bundle_filename,n_photos_to_reconstruct);

   string packages_subdir=bundler_IO_subdir+"packages/";
   cout << "packages_subdir = " << packages_subdir << endl;
   filefunc::dircreate(packages_subdir);

// ==========================================================================
// Loop over n photos starts here.  First convert reconstructed
// cameras from Noah's bundler coordinate system into georegistered
// coordinates.  Then instantiate and write out package file for nth
// photo:

   vector<threevector> reconstructed_camera_posn;

   string camera_posns_filename=
      bundler_IO_subdir+"reconstructed_camera_posns.dat";
   ofstream outstream;
   filefunc::openfile(camera_posns_filename,outstream);

   int n_start=0;
   for (unsigned int n=n_start; n<photogroup_ptr->get_n_photos(); n++)
   {
//      cout << n << "  " << flush;
      if (n%100==0) cout << n/100 << "  " << flush;

      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

// Sailplane reconstruction (Dec 2010) method for converting bundler
// to world coordinates:

      camera_ptr->convert_bundler_to_world_coords(
         bundler_rotation_origin,
         global_az,global_el,global_roll,
         fitted_world_to_bundler_distance_ratio,
         fitted_bundler_trans);

//      rotation* R_camera_ptr=camera_ptr->get_Rcamera_ptr();
//      cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//      cout << "Uhat = " << Uhat << " Vhat = " << Vhat << endl;

// Apply translation+scaling+rotation transformation to reconstructed
// camera world positions as well as their Uhat and Vhat direction
// vectors:

      threevector camera_posn(camera_ptr->get_world_posn());
      threevector Uhat(camera_ptr->get_Uhat());
      threevector Vhat(camera_ptr->get_Vhat());
      threevector camera_U=camera_posn+Uhat;
      threevector camera_V=camera_posn+Vhat;

      camera_posn -= pmvs_COM;
      camera_posn *= mu_scale;
      camera_posn = R*camera_posn;
      camera_posn += ladar_COM;
      camera_ptr->set_world_posn(camera_posn);

      camera_U -= pmvs_COM;
      camera_U *= mu_scale;
      camera_U = R*camera_U;
      camera_U += ladar_COM;

      camera_V -= pmvs_COM;
      camera_V *= mu_scale;
      camera_V = R*camera_V;
      camera_V += ladar_COM;

      threevector dU=camera_U-camera_posn;
      threevector dV=camera_V-camera_posn;
//      cout << "dU.dV = " << dU.dot(dV) << endl;

      camera_ptr->set_Rcamera(dU.unitvector(),dV.unitvector());
//      cout << "Modified *Rcamera_ptr = " << *R_camera_ptr << endl;

      camera_ptr->set_geolocation(
         photogroup_ptr->get_northern_hemisphere_flag(),
         photogroup_ptr->get_UTM_zonenumber(),
         camera_posn.get(0),camera_posn.get(1),camera_posn.get(2));

      const double TINY=0.001;
      if (camera_posn.magnitude() > TINY)
      {
         reconstructed_camera_posn.push_back(camera_posn);
      }

//      double az,el,roll;
//      camera_ptr->get_az_el_roll_from_Rcamera(az,el,roll);
//      cout << "az = " << az*180/PI << endl;
//      cout << "el = " << el*180/PI << endl;
//      cout << "roll = " << roll*180/PI << endl;

      int ndigits=4;
      string package_filename=packages_subdir+"photo_"+
         stringfunc::integer_to_string(n,ndigits)+".pkg";

      double frustum_sidelength=100;	// meters
      double downrange_distance=-1;	// meters

// Write out package file for current camera:

      camera_ptr->write_camera_package_file(
         package_filename,photograph_ptr->get_ID(),
         photograph_ptr->get_filename(),
         frustum_sidelength,downrange_distance);

      outstream.precision(12);
      outstream << photograph_ptr->get_ID() << "  "
                << filefunc::getbasename(photograph_ptr->get_filename())
                << "  "
                << camera_posn.get(0) << "  "
                << camera_posn.get(1) << "  "
                << camera_posn.get(2) << endl;

   } // loop over index n labeling cameras
   cout << endl;
   
   filefunc::closefile(camera_posns_filename,outstream);

// ---------------------------------------------------------------------
// Create TDP file with reconstructed camera track generated after
// fitting bundler's original camera track to GPS data:

   tdp_filename=bundler_IO_subdir+"fitted_bundler_track.tdp";
   tdpfunc::write_relative_xyz_data(tdp_filename,reconstructed_camera_posn);
}
