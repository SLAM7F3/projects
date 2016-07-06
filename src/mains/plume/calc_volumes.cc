// ==========================================================================
// Program CALC_VOLUMES automatically searches for TDP files generated
// by program DBVOLUME corresponding to reconstructed 3D plume volumes
// at different time slices. For each time slice, a
// VolumetricCoincidenceProcessor is filled with the point clouds
// contents.  The number of filled voxels multiplied by voxel volume
// yields the total volume for the 3D plume at the time slice.

// CALC_VOLUMES exports plume volume vs time slice number to
// volumes_vs_timeslices.txt and plume centroid rho & Z vs time.   It
// also generates JPEG plot of plume volume, centroid rho and centroid
// Z vs time.  These output files are exported to the same
// subdirectory of bundler_IO_subdir/plume_results/ as where program
// DBVOLUME wrote the final OpenSceneGraph OSG files for each time
// slice.

//				calc_volumes

// ==========================================================================
// Last updated on 1/28/13; 1/30/13; 2/4/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "passes/PassesGroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Constants:

//   double voxel_binsize=0.05; // meter  Expt 2B
//   double voxel_binsize=0.1;	 // meter  Expt 5C
   double voxel_binsize=0.175;	 // meter  Nov 2012 Expt 2H
   double voxel_volume=voxel_binsize*voxel_binsize*voxel_binsize;

   cout << "voxel_binsize = " << voxel_binsize << endl;
   outputfunc::enter_continue_char();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int mission_ID=22;
   cout << "Enter mission ID:" << endl;
   cin >> mission_ID;

   int fieldtest_ID=plumedatabasefunc::retrieve_fieldtest_ID_given_mission_ID(
      postgis_db_ptr,mission_ID);
//   cout << "fieldtest_ID = " << fieldtest_ID << endl;

   string start_timestamp;
   plumedatabasefunc::retrieve_fieldtest_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,start_timestamp);
   cout << "start_timestamp = " << start_timestamp << endl;
   Clock clock;
   cout.precision(13);

   bool UTC_flag=true;
   double epoch=
      clock.timestamp_string_to_elapsed_secs(start_timestamp,UTC_flag);
   int year=clock.get_year();
   string month_name=clock.get_month_name();

   int day_number;
   string experiment_label;
   plumedatabasefunc::retrieve_mission_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,mission_ID,
      day_number,experiment_label);

   string results_subdir=bundler_IO_subdir+"plume_results/";
   results_subdir += stringfunc::number_to_string(day_number)+
      experiment_label+"/";
//   cout << "results_subdir = " << results_subdir << endl;

   string TDP_subdir=results_subdir+"TDP/";
//   cout << "TDP_subdir = " << TDP_subdir << endl;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("tdp");
   vector<string> tdp_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,TDP_subdir);

   vector<int> time_slice_numbers;
   for (int t=0; t<tdp_filenames.size(); t++)
   {
      cout << "t = " << t << " tdp_filename = " << tdp_filenames[t]
           << endl;
      string basename=filefunc::getbasename(tdp_filenames[t]);
      string prefix=stringfunc::prefix(basename);
//      cout << "prefix = " << prefix << endl;
      string substring=prefix.substr(11,3);
//      cout << "substring = " << substring << endl;
      time_slice_numbers.push_back(stringfunc::string_to_number(substring));
//      cout << "time_slice_number = " << time_slice_numbers.back() << endl;
   }

   const double dt_per_slice=1.0/30.0;	// Nov 2012 Expt 2H
   int first_time_slice=time_slice_numbers.front();
   vector<double> time_since_ignition;
   for (int i=0; i<time_slice_numbers.size(); i++)
   {
      double t_since_ignition=(time_slice_numbers[i]-first_time_slice)*
         dt_per_slice;
      time_since_ignition.push_back(t_since_ignition);
   }

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* P_ptr=new vector<double>;

   string plume_volume_filename=results_subdir+"volumes_vs_timeslice.txt";
   ofstream outstream;
   filefunc::openfile(plume_volume_filename,outstream);
   outstream << "# Timeslice  Reconstructed plume volume (m**3)" << endl;
   outstream << endl;

   vector<double> framenumbers,plume_volumes,X_COM,Y_COM,Z_COM,rho_COM;
   vector<double> Z_max;
   for (int ts=0; ts<time_slice_numbers.size(); ts++)
   {
      int slice_number=time_slice_numbers[ts];
      string tdp_filename=TDP_subdir+"plume_hull_"+
         stringfunc::integer_to_string(slice_number,3)+".tdp";

// Read in point cloud:

      X_ptr->clear();
      Y_ptr->clear();
      Z_ptr->clear();
      P_ptr->clear();
      tdpfunc::read_XYZP_points_from_tdpfile(
         tdp_filename,*X_ptr,*Y_ptr,*Z_ptr,*P_ptr);

      int n_points=Z_ptr->size();
      double volume=n_points*voxel_volume;
//      cout << "slice_number = " << slice_number 
//           << " VCP volume = " << volume << endl;

      outstream << slice_number << "  " << volume << endl;

      framenumbers.push_back(slice_number);
      plume_volumes.push_back(volume);

      X_COM.push_back(mathfunc::mean(*X_ptr));
      Y_COM.push_back(mathfunc::mean(*Y_ptr));
      Z_COM.push_back(mathfunc::mean(*Z_ptr));
      Z_max.push_back(mathfunc::maximal_value(*Z_ptr));
      double curr_rho_COM=sqrt(sqr(X_COM.back())+sqr(Y_COM.back()));
      rho_COM.push_back(curr_rho_COM);
   } // loop over slice_number index

   filefunc::closefile(plume_volume_filename,outstream);

// Export volume vs time to metafile plot:

   metafile M;
   string meta_filename="volume_vs_time";
   string title="Reconstructed plume volume vs time";
   string x_label="Time since ignition (secs)";
   string y_label="Visible plume volume (cubic meters)";
//   double x_min=time_slice_numbers.front();
//   double x_max=time_slice_numbers.back();
   double x_min=0;
   double x_max=time_since_ignition.back();
   double y_min=0;
   double y_max=1.1*mathfunc::maximal_value(plume_volumes);
   M.set_parameters(
      meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
   M.openmetafile();
   M.write_header();
//   M.write_curve(framenumbers,plume_volumes);
   M.write_curve(time_since_ignition,plume_volumes);
   M.closemetafile();
   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv volume_vs_time.jpg "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv volume_vs_time.meta "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="rm volume_vs_time.ps";
   sysfunc::unix_command(unix_cmd);

   string banner="Wrote "+plume_volume_filename;
   outputfunc::write_big_banner(banner);

   banner="Wrote "+results_subdir+"volume_vs_time.jpg";
   outputfunc::write_big_banner(banner);

// Export Z_COM vs time to metafile plot:

   metafile M2;
   meta_filename="ZCOM_vs_time";
   title="Reconstructed plume centroid height vs time";
   x_label="Time since ignition (secs)";
   y_label="Plume centroid's height above ground (meters)";
   x_min=0;
   x_max=time_since_ignition.back();
   y_min=0;
   y_max=1.1*mathfunc::maximal_value(Z_COM);
   M2.set_parameters(
      meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
   M2.openmetafile();
   M2.write_header();
   M2.write_curve(time_since_ignition,Z_COM);
   M2.closemetafile();
   unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv ZCOM_vs_time.jpg "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv ZCOM_vs_time.meta "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="rm ZCOM_vs_time.ps";
   sysfunc::unix_command(unix_cmd);

   banner="Wrote "+results_subdir+"ZCOM_vs_time.jpg";
   outputfunc::write_big_banner(banner);

// Export rho_COM vs time to metafile plot:

   metafile M3;
   meta_filename="rhoCOM_vs_time";
   title="Reconstructed plume radial distance from origin vs time";
   x_label="Time since ignition (secs)";
   y_label="Plume centroid's radial distance from origin (meters)";
   x_min=0;
   x_max=time_since_ignition.back();
   y_min=0;
   y_max=1.1*mathfunc::maximal_value(rho_COM);
   M3.set_parameters(
      meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
   M3.openmetafile();
   M3.write_header();
   M3.write_curve(time_since_ignition,rho_COM);
   M3.closemetafile();
   unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv rhoCOM_vs_time.jpg "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv rhoCOM_vs_time.meta "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="rm rhoCOM_vs_time.ps";
   sysfunc::unix_command(unix_cmd);

   banner="Wrote "+results_subdir+"rhoCOM_vs_time.jpg";
   outputfunc::write_big_banner(banner);



// Export Z_max vs time to metafile plot:

   metafile M4;
   meta_filename="Zmax_vs_time";
   title="Reconstructed plume centroid height vs time";
   x_label="Time since ignition (secs)";
   y_label="Visible plume centroid height (meters)";
   x_min=0;
   x_max=time_since_ignition.back();
   y_min=0;
   y_max=1.1*mathfunc::maximal_value(Z_max);
   M4.set_parameters(
      meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
   M4.openmetafile();
   M4.write_header();
   M4.write_curve(time_since_ignition,Z_max);
   M4.closemetafile();
   unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv Zmax_vs_time.jpg "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv Zmax_vs_time.meta "+results_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="rm Zmax_vs_time.ps";
   sysfunc::unix_command(unix_cmd);

   banner="Wrote "+results_subdir+"Zmax_vs_time.jpg";
   outputfunc::write_big_banner(banner);


}
