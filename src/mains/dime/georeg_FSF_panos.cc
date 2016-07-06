// =======================================================================
// Program GEOREG_FSF_PANOS first imports epoch times for a set of
// stabilized FSF WISP panoramas.  It next imports GPS tracks for the
// FSF and Athena ships.  Scene-specific ship tracks are exported to
// output text files in UTM coordinates.  The azimuthal angle of the
// Athena relative to the FSF is calculated for each stabilized
// panorama.

// GEOREG_FSF_PANOS next imports the Athena's UV image-plane track for
// a set of stabilized panoramas previously computed via program
// ATHENA.  It then computes the U coordinate corresponding to due
// east for each panorama, the azimuth corresponding to U=0 and the
// elevation corresponding to V=0.  (U=V=0 corresponds to the lower
// left corner of the WISP panorama.)  GEOREG_FSF_PANOS exports these
// results to output text file Panos_Az_El.txt.

//			   ./georeg_FSF_panos

// ========================================================================
// Last updated on 7/16/13; 7/17/13; 7/21/13; 8/8/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "track/tracks_group.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

//   int UTM_zone=16;	 // Panama City, FL
//   bool northern_hemisphere_flag=true;

   string bundler_subdir="./bundler/DIME/";
   string MayFieldtest_subdir=bundler_subdir+"May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;
   string Miketracks_subdir=FSFdate_subdir+"Mike_tracks/";

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string bundler_IO_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string stable_frames_subdir=bundler_IO_subdir+"stable_frames/";
   string subsampled_stable_frames_subdir=
      stable_frames_subdir+"subsampled/";
   string athena_subdir=subsampled_stable_frames_subdir+"athena/";
   string horizon_subdir=subsampled_stable_frames_subdir+"horizon/";

   string pano_times_filename=stable_frames_subdir+"pano_times.txt";
   string horizons_filename=horizon_subdir+"Horizon_V.dat";

   timefunc::initialize_timeofday_clock();

// Import WISP panorama horizon V values:

   typedef map<int,double> HORIZONS_MAP;
   HORIZONS_MAP horizons_map;
   HORIZONS_MAP::iterator horizons_iter;

// Independent int = pano ID
// Dependent double = V_horizon

   filefunc::ReadInfile(horizons_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      int pano_ID=stringfunc::string_to_number(substrings[0]);
      double V_horizon=stringfunc::string_to_number(substrings[2]);
      horizons_map[pano_ID]=V_horizon;
   }
   cout << "horizons_map.size() = " << horizons_map.size() << endl;

// Import WISP panorama frame times:

   typedef map<int,threevector> PANO_MAP;
   PANO_MAP pano_map;
   PANO_MAP::iterator pano_iter;

// Independent int = pano ID
// Dependent threevector = pano time in secs since 1970-1-1 , 
//    athena azimuth relative to FSF, U_east

   filefunc::ReadInfile(pano_times_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      int pano_ID=stringfunc::string_to_number(substrings[0]);
      double epoch_time=stringfunc::string_to_number(substrings[1]);
      threevector time_az_Ueast(epoch_time,0,0);
      pano_map[pano_ID]=time_az_Ueast;
   }
   cout << "pano_map.size() = " << pano_map.size() << endl;

// Import tracks for FSF and Athena:

   vector<string> ship_track_filename;

   if (scene_ID==25 || scene_ID==27 || scene_ID==29)
   {
      ship_track_filename.push_back(Miketracks_subdir+"FSF_UTM_track.dat");
      ship_track_filename.push_back(Miketracks_subdir+"Athena_UTM_track.dat");
   }
   else
   {
      ship_track_filename.push_back(FSFdate_subdir+"FSF_UTM_track.dat");
      ship_track_filename.push_back(FSFdate_subdir+"Athena_UTM_track.dat");
   }
   

   vector<int> track_labels;
   tracks_group ship_tracks_group;

   for (unsigned int tf=0; tf<ship_track_filename.size(); tf++)
   {
      filefunc::ReadInfile(ship_track_filename[tf]);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         int track_label=column_values[0];
         double elapsed_secs=column_values[1];
         double easting=column_values[2];
         double northing=column_values[3];
         double altitude=column_values[4];
         threevector UTM_posn(easting,northing,altitude);

         track* curr_track_ptr=
            ship_tracks_group.get_track_given_label(track_label);
      
         if (curr_track_ptr==NULL)
         {
            curr_track_ptr=ship_tracks_group.generate_new_track();
            curr_track_ptr->set_label_ID(track_label);
            ship_tracks_group.associate_track_labelID_and_ID(
               curr_track_ptr);
            track_labels.push_back(track_label);
         }

         curr_track_ptr->set_XYZ_coords(elapsed_secs,UTM_posn);
      } // loop over index i labeling lines in SHIP_track_filename
   } // loop over index tf labeling ship track filenames

   int n_tracks=ship_tracks_group.get_n_tracks();
   cout << "n_tracks = " << n_tracks << endl;
   cout << "track_labels.size() = " << track_labels.size() << endl;

   track* FSF_track_ptr=ship_tracks_group.get_track_given_label(
      track_labels[0]);
   vector<threevector> FSF_posns=FSF_track_ptr->get_posns();
   cout << "FSF_posns.size() = " << FSF_posns.size() << endl;

   track* Athena_track_ptr=ship_tracks_group.get_track_given_label(
      track_labels[1]);
   vector<threevector> Athena_posns=Athena_track_ptr->get_posns();
   cout << "Athena_posns.size() = " << Athena_posns.size() << endl;

// Loop over all panorama IDs and recover their times.  Compute FSF
// and Athena positions at the pano times.  Export these reduced ship
// tracks to scene-specific text files.  Also calculate azimuthal
// angle of Athena relative to FSF for each panorama:

   string FSF_scene_track_filename=stable_frames_subdir+
      "FSF_UTM_track.dat";
   string Athena_scene_track_filename=stable_frames_subdir+
      "Athena_UTM_track.dat";
   ofstream FSF_stream,Athena_stream;
   FSF_stream.precision(12);
   Athena_stream.precision(12);
   
   filefunc::openfile(FSF_scene_track_filename,FSF_stream);
   filefunc::openfile(Athena_scene_track_filename,Athena_stream);
   FSF_stream << "# Track_ID  Secs since 1970-1-1  Easting  Northing  Altitude"
              << endl << endl;
   Athena_stream << "# Track_ID  Secs since 1970-1-1  Easting  Northing  Altitude"
                 << endl << endl;
   
   for (pano_iter=pano_map.begin(); 
        pano_iter != pano_map.end(); pano_iter++)
   {
//      int pano_ID=pano_iter->first;
      double pano_time=pano_iter->second.get(0);

      cout.precision(12);
//      cout << "pano_ID = " << pano_ID
//           << " pano_time = " << pano_time << endl;

      threevector FSF_posn,Athena_posn;
      if (!FSF_track_ptr->get_interpolated_posn(pano_time,FSF_posn))
      {
         cout << "No FSF track point found at this pano time" << endl;
         outputfunc::enter_continue_char();
         continue;
      }
      
      if (!Athena_track_ptr->get_interpolated_posn(pano_time,Athena_posn)) 
      {
         cout << "No Athena track point found at this pano time" << endl;
         outputfunc::enter_continue_char();
         continue;
      }

      threevector rhat=(Athena_posn-FSF_posn).unitvector();
      double Athena_az=atan2(rhat.get(1),rhat.get(0))*180/PI;
      pano_iter->second.put(1,Athena_az);

      FSF_stream << FSF_track_ptr->get_ID() << "  "
                 << pano_time << "  "
                 << FSF_posn.get(0) << "  "
                 << FSF_posn.get(1) << "  "
                 << FSF_posn.get(2) << endl;
      Athena_stream << Athena_track_ptr->get_ID() << "  "
                 << pano_time << "  "
                 << Athena_posn.get(0) << "  "
                 << Athena_posn.get(1) << "  "
                 << Athena_posn.get(2) << endl;
   } // loop over pano_iter

   filefunc::closefile(FSF_scene_track_filename,FSF_stream);
   filefunc::closefile(Athena_scene_track_filename,Athena_stream);
   
   string banner="Exported FSF track for current scene to "
      +FSF_scene_track_filename;
   outputfunc::write_big_banner(banner);
   banner="Exported Athena track for current scene to "
      +Athena_scene_track_filename;
   outputfunc::write_big_banner(banner);

// Import Athena UV track.  After associating azimuthal angle with
// the Athena's image-plane U coordinate, compute U for due east which
// corresponds to azimuth = 0.  Export Ueast as function of panorama
// ID to output text file:

   string athena_UVtrack_filename=athena_subdir+"athena_UVtrack.dat";

   double Umax=40000.0/2200.0;

   string output_filename=stable_frames_subdir+"Panos_Az_El.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Pano_ID  	Pano_Filename   		      U_east   Az_{U=0}  El_{V=0}" << endl << endl;

   bool athena_file_read_flag=filefunc::ReadInfile(athena_UVtrack_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string pano_filename=substrings[0];
      vector<string> subsubstrings=
         stringfunc::decompose_string_into_substrings(
            pano_filename,"_.");
      int n_subsubstrings=subsubstrings.size();
      int pano_ID=stringfunc::string_to_number(
         subsubstrings[n_subsubstrings-2]);

      pano_iter=pano_map.find(pano_ID);
      if (pano_iter==pano_map.end()) continue;

      double Athena_az=pano_iter->second.get(1);
      double Uathena=stringfunc::string_to_number(substrings[1]);
      double Uathena_frac=Uathena/Umax;

      double East_az=0;
      double delta_az=East_az-Athena_az;
      double delta_az_frac=delta_az/360.0;
      double Ueast_frac=Uathena_frac-delta_az_frac;
      double Ueast=Ueast_frac*Umax;

      double U0_az=Athena_az+(0-Uathena)/(Ueast-Uathena)*(East_az-Athena_az);

      horizons_iter=horizons_map.find(pano_ID);
      if (horizons_iter==horizons_map.end()) continue;

      double V_horizon=horizons_iter->second;
      double V0_el=-V_horizon*30;  // Recall V1_el-V0_el = 30 degs

      cout << "pano_ID = " << pano_ID 
           << " Athena_az = " << Athena_az 
           << " Uathena = " << Uathena 
           << " Uathena_frac = " << Uathena_frac
//           << " delta_az = " << delta_az 
           << " delta_az_frac = " << delta_az_frac
           << " Ueast_frac = " << Ueast_frac
           << " Ueast = " << Ueast 
           << endl;

      outstream << pano_ID << "  " 
                << pano_filename << "  "
                << Ueast << "  " 
                << U0_az << "  "
                << V0_el << endl;
      
   } // loop over index i labeling lines in athena_UVtrack_filename
   filefunc::closefile(output_filename,outstream);

   banner="Exported Ueast, Az_{U=0} and El_{V=0} as functions of panorama ID to "
      +output_filename;
   outputfunc::write_big_banner(banner);
 
   banner="Finished running program GEOREG_FSF_PANOS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();
}

