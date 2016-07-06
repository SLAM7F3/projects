// =======================================================================
// Program ERSA_TRACKS imports ERSA track information extracted from
// MATLAB files via Allyn Dullighan's CSV parser matlab code.  It
// instantiates and fills track objects with aircraft timing and
// UTM position information.  ERSA_TRACKS exports the aerial tracks to
// an output text file.

//				./ersa_tracks

// =======================================================================
// Last updated on 4/1/13; 4/2/13; 4/3/13; 4/26/13
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

   const double min_range=0*1000;
//   const double max_range=30*1000;
   
//   const double min_range=15*1000;	// meters
//   const double max_range=25*1000;	// meters
//   const double max_range=45*1000;	// meters
//   const double max_range=60*1000;	// meters
//   const double max_range=75*1000;	// meters
//   const double max_range=100*1000;	// meters
   const double max_range=200*1000;	// meters
   const double min_Z=10;		// meters

   double dt=2;		// secs
//   const double dt=10;	// secs
//   double dt=20;	// secs
//   const double dt=30;	// secs
//   double dt=60;
   
   
   bool northern_hemisphere_flag=true;
   int specified_UTM_zonenumber=19;
   Clock clock;
   tracks_group ersa_tracks_group;

// Calibrated geolocation for Deer Island Wisp sensor:

   double wisp_x=339106.2355;
   double wisp_y=4690476.765;
   double wisp_z=9.120232601;
//   double wisp_x=339106.0631;
//   double wisp_y=4690476.511;
//   double wisp_z=12.73415031;
   threevector wisp_XYZ(wisp_x,wisp_y,wisp_z);
   geopoint wisp_tangent_point(
      northern_hemisphere_flag,specified_UTM_zonenumber,wisp_x,wisp_y,0);

   int pano_width=40000;
   int pano_height=2200;
//   double Umin=0;
   double Umax=double(pano_width)/double(pano_height);
//   double Vmin=0;
   double Vmax=1;

// In April 2013, Gary Long discovered that WISP pixels are not square!

   double IFOV_u=360*PI/180 / Umax;
   double IFOV_v=30*PI/180 / Vmax;
//   cout << "IFOV_u = " << IFOV_u << endl;
//   cout << "IFOV_v = " << IFOV_v << endl;
//   cout << "IFOV_v/IFOV_u = " << IFOV_v/IFOV_u << endl;

// Import ERSA tracks:

   string ERSA_subdir="./ERSA/";
   string ERSA_tracks_filename=
//      ERSA_subdir+"wisp8-spin-.5hz-ocean_ERSA_tracks.csv";
      ERSA_subdir+"Tracks_20130214.csv";
   filefunc::ReadInfile(ERSA_tracks_filename);

   string separator_chars=",";
   double start_time=1.0E25;
   double stop_time=0;
   cout << "filefunc::text_line.size() = " << filefunc::text_line.size()
        << endl;
   
   vector<int> track_labels;

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i],separator_chars);
      int track_label=stringfunc::string_to_number(substrings[0]);
      double elapsed_secs=stringfunc::string_to_number(substrings[1]);
      start_time=basic_math::min(start_time,elapsed_secs);
      stop_time=basic_math::max(stop_time,elapsed_secs);

      if (i%100000==0)
      {
         cout << "i = " << i 
              << " track_label = " << track_label
              << endl;
      }

      double latitude=stringfunc::string_to_number(substrings[2]);
      double longitude=stringfunc::string_to_number(substrings[3]);
      double altitude=stringfunc::string_to_number(substrings[4]);
      
      geopoint curr_geopoint(
         longitude,latitude,altitude,specified_UTM_zonenumber);

      track* curr_track_ptr=
         ersa_tracks_group.get_track_given_label(track_label);
      
      if (curr_track_ptr==NULL)
      {
         curr_track_ptr=ersa_tracks_group.generate_new_track();
         curr_track_ptr->set_label_ID(track_label);
         ersa_tracks_group.associate_track_labelID_and_ID(
            curr_track_ptr);
         track_labels.push_back(track_label);
      }

      curr_track_ptr->set_XYZ_coords(
         elapsed_secs,curr_geopoint.get_UTM_posn());

//      cout << "track_label = " << track_label
//           << " track_ID = " << curr_track_ptr->get_ID()
//           << " curr_track_ptr = " << curr_track_ptr << endl;
//      outputfunc::enter_continue_char();


   } // loop over index i labeling lines in ERSA_tracks_filename

   int n_tracks=ersa_tracks_group.get_n_tracks();
   cout << "n_tracks = " << n_tracks << endl;
   cout << "track_labels.size() = " << track_labels.size() << endl;

//  2013-02-14 00:00:00.00 UTC corresponds to elapsed_secs= 1360800000 
//  2013-02-14 00:00:00.00 EST corresponds to elapsed_secs =1360818000

//   start_time= 1360867771;	// 1:49:31 pm EST 
//   start_time -= 125;	

   start_time=1360867646;	// 2013-02-14 18:47:26.00 UTC = 1:47:26 EST
   stop_time= start_time+2*429;	// 2013-02-14 19:01:43.100 UTC = 2:01:43 EST

   clock.convert_elapsed_secs_to_date(start_time);
   string start_stamp=clock.YYYY_MM_DD_H_M_S();
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      start_stamp);
   string start_HHMMSS=substrings[1];
   cout << "Starting track time = " << start_stamp << endl;
   cout << "Start HHMMSS = " << start_HHMMSS << endl;

   clock.convert_elapsed_secs_to_date(stop_time);
   string stop_stamp=clock.YYYY_MM_DD_H_M_S();
   substrings=stringfunc::decompose_string_into_substrings(stop_stamp);
   string stop_HHMMSS=substrings[1];
   cout << "Stopping track time = " << stop_stamp << endl;
   cout << "Stop HHMMSS = " << stop_HHMMSS << endl;

   threevector interp_posn;

// First export ERSA tracks as function of track ID:

   string tracks_filename=ERSA_subdir+"tracks_"+start_HHMMSS+"-"+
      stop_HHMMSS+".dat";
   ofstream tracks_stream;
   tracks_stream.precision(12);
   filefunc::openfile(tracks_filename,tracks_stream);

   tracks_stream << 
      "# Track ID   Time   Easting  	Northing  Altitude  U   V"
                 << endl << endl;

   double min_track_U=100;
   double max_track_U=-100;

//   int track_ID_start=0;
//   int track_ID_stop=n_tracks-1;
//   track_ID_start=2694;	// bright aircraft #1
//   track_ID_stop=2694;

   for (int track_counter=0; track_counter<n_tracks; track_counter++)
   {
      int track_label=track_labels[track_counter];
      
//      cout << track_label << " " << flush;

      bool track_points_written_flag=false;
      track* curr_track_ptr=ersa_tracks_group.get_track_ptr(track_counter);
      
      double az,el,prev_az,prev_el,azdot,eldot;
      az=el=azdot=eldot=NEGATIVEINFINITY;
      
      double curr_t=start_time;
      while (curr_t < stop_time)
      {
         if (curr_track_ptr->get_interpolated_posn(curr_t,interp_posn))
         {

// Modify track_point's altitude to compensate for earth's curvature:

            geopoint track_point(
               northern_hemisphere_flag,specified_UTM_zonenumber,
               interp_posn.get(0),interp_posn.get(1),interp_posn.get(2));
            double eff_track_Z=geofunc::altitude_relative_to_tangent_plane(
               wisp_tangent_point,track_point);
            interp_posn.put(2,eff_track_Z);

            threevector rel_XYZ=(interp_posn-wisp_XYZ);
            double range=rel_XYZ.magnitude();
            threevector r_hat=rel_XYZ.unitvector();
//            cout << "range = " << range*0.001 << endl;

            prev_az=az;
            prev_el=el;
            az=atan2(r_hat.get(1),r_hat.get(0));
            el=PI/2-acos(r_hat.get(2));
            const double delta_az=-60.868*PI/180;
            const double delta_el=0;

            if (prev_az > 0.5*NEGATIVEINFINITY)
            {
               prev_az=basic_math::phase_to_canonical_interval(
                  prev_az,az-PI,az+PI);
               azdot=(az-prev_az)/dt;
               eldot=(el-prev_el)/dt;
               cout.precision(12);
//               cout << "curr_t = " << curr_t 
//                    << " az = " << az*180/PI
//                    << " prev_az = " << prev_az*180/PI
//                    << " azdot = " << azdot*180/PI << endl;
            }
            
            double wisp_az=az+delta_az;
            wisp_az=basic_math::phase_to_canonical_interval(wisp_az,0,2*PI);
            double wisp_el=el+delta_el;
            double U=(2*PI-wisp_az)/IFOV_u;
            double V=0.5+wisp_el/IFOV_v;

//            if (interp_posn.get(2) > min_Z)
            if (range < max_range && range > min_range && 
            interp_posn.get(2) > min_Z)
//            azdot > 0.5*NEGATIVEINFINITY && fabs(azdot*180/PI) > 0.05 &&
//            azdot < 0)
            {
//               clock.convert_elapsed_secs_to_date(curr_t);

               tracks_stream << track_label << "  "
                             << curr_t << "   "
//                             << clock.YYYY_MM_DD_H_M_S() << "  "
                             << interp_posn.get(0) << "  "
                             << interp_posn.get(1) << "  "
                             << interp_posn.get(2) << "  "
                             << U << "  " << V 
                             << endl;
               track_points_written_flag=true;

               min_track_U=basic_math::min(min_track_U,U);
               max_track_U=basic_math::max(max_track_U,U);
            }
         }
         curr_t += dt;
      } // curr_t < stop_time while loop

      if (track_points_written_flag)
      {
         track_points_written_flag=false;
         tracks_stream << endl;
      }

   } // loop over track_counter
   cout << endl;

   filefunc::closefile(tracks_filename,tracks_stream);
   string banner="Exported "+tracks_filename;
   outputfunc::write_big_banner(banner);

   cout << "min_track_U = " << min_track_U
        << " max_track_U = " << max_track_U << endl;

// Next export tracks as functions of time rather than track number:

// Must reset dt=2 so that 1 ERSA frame = 1 video frame:

   dt=2;	// secs

//   double panel0_center_az=-78.77777778;
//   double panel0_start_az=-78.77777778+18;
//   double panel0_stop_az=-78.77777778-18;
//   double panel1_start_az=panel0_start_az-36;
//   double panel1_stop_az=panel0_stop_az-36;
   
   vector<double> panel_start_az;
   panel_start_az.push_back(-78.77777778-18+360);
   for (int p=1; p<=9; p++)
   {
      double curr_panel_az=panel_start_az[p-1]-36;
      curr_panel_az=basic_math::phase_to_canonical_interval(
         curr_panel_az,0,360);
      panel_start_az.push_back(curr_panel_az);
      cout << "p = " << p 
           << " start panel az = " << panel_start_az.back()
           << endl;
   }

   typedef std::map<int,std::vector<twovector> > AZEL_MAP;
   AZEL_MAP* azel_map_ptr=new AZEL_MAP;
   AZEL_MAP::iterator iter;

// independent integer = track ID
// Dependent STL vec of twovectors = (az,el) as functions of time

//   double min_discrepancy_sum=POSITIVEINFINITY;
   vector<twovector> tracks_UV;

   tracks_filename=ERSA_subdir+"timestamps_"+start_HHMMSS+"-"+
      stop_HHMMSS+".dat";

   tracks_stream.precision(12);
   filefunc::openfile(tracks_filename,tracks_stream);

   tracks_stream << 
"# Time 	Track ID 	Easting 	Northing 	Altitude 	Az 	El	Azdot    Eldot    n_panel		 panel_U	   V"
                 << endl << endl;

   double curr_t=start_time;
   while (curr_t < stop_time)
   {
//      cout << curr_t-start_time << " " << flush;

//      tracks_UV.clear();
      double azdot=NEGATIVEINFINITY;
      double eldot=NEGATIVEINFINITY;
      for (int track_counter=0; track_counter<n_tracks; track_counter++)
      {
         int track_label=track_labels[track_counter];
         track* curr_track_ptr=ersa_tracks_group.get_track_ptr(track_counter);
      
         if (curr_track_ptr->get_interpolated_posn(curr_t,interp_posn))
         {

// Modify track_point's altitude to compensate for earth's curvature:

            geopoint track_point(
               northern_hemisphere_flag,specified_UTM_zonenumber,
               interp_posn.get(0),interp_posn.get(1),interp_posn.get(2));
            double eff_track_Z=geofunc::altitude_relative_to_tangent_plane(
               wisp_tangent_point,track_point);
            interp_posn.put(2,eff_track_Z);

            threevector rel_XYZ=(interp_posn-wisp_XYZ);
            double range=rel_XYZ.magnitude();
            threevector r_hat=rel_XYZ.unitvector();
//            cout << "range = " << range*0.001 << endl;

            double az=atan2(r_hat.get(1),r_hat.get(0));
            double el=PI/2-acos(r_hat.get(2));
//            cout << "az = " << az*180/PI << " el = " << el*180/PI << endl;

            iter=azel_map_ptr->find(track_label);
            if (iter==azel_map_ptr->end())
            {
               vector<twovector> V;
               V.push_back(twovector(az,el));
               (*azel_map_ptr)[track_label]=V;
            }
            else
            {
               vector<twovector> V=iter->second;
               twovector prev_azel=V.back();
               double prev_az=prev_azel.get(0);
               prev_az=basic_math::phase_to_canonical_interval(
                  prev_az,az-PI,az+PI);
               double d_az=az-prev_az;
               double d_el=el-prev_azel.get(1);
               azdot=d_az/dt;
               eldot=d_el/dt;

//               cout << "d_az = " << d_az*180/PI
//                    << " dt = " << dt << " azdot = " << azdot*180/PI
//                    << endl;
               iter->second.push_back(twovector(az,el));
            }

// Azimuthal alignment between (faulty) WISP az and true az:

            const double delta_az=-60.868*PI/180;
            const double delta_el=0;
            
            double wisp_az=az+delta_az;
            wisp_az=basic_math::phase_to_canonical_interval(
               wisp_az,0,2*PI);
            double wisp_el=el+delta_el;

// Compute panel in which current (az,el) lies as well as its (U,V)
// for that panel:

            const double panel0_max_az=263.222;
            const double panel0_min_az=panel0_max_az-36;

            double delta_panel_az=az*180/PI-panel0_min_az;
            delta_panel_az=basic_math::phase_to_canonical_interval(
               delta_panel_az,0,360);
            int n_panel=basic_math::mytruncate(delta_panel_az/36);
            delta_panel_az -= n_panel*36;

            n_panel=10-n_panel;
            n_panel=n_panel+1;
            n_panel=modulo(n_panel,10);
//            if (n_panel==10) n_panel=0;
            delta_panel_az=36-delta_panel_az;

            double panel_Umax=Umax/10;
            double panel_U=panel_Umax*delta_panel_az/36.0;
            
//            double U=(2*PI-az)/IFOV_u;
            double V=0.5+wisp_el/IFOV_v;

//            if (interp_posn.get(2) > min_Z)
            if (range < max_range && range > min_range &&
            interp_posn.get(2) > min_Z)
            {
               if (azdot < NEGATIVEINFINITY/2) azdot=0;
               if (eldot < NEGATIVEINFINITY/2) eldot=0;

// FAKE FAKE:  Mon Apr 1, 2013 at 7:20 pm

//               if (azdot >=0) continue;
//               if (fabs(azdot*180/PI) < 0.05) continue;

               tracks_stream << curr_t << "   "
                             << track_label << "   "
                             << interp_posn.get(0) << "   "
                             << interp_posn.get(1) << "   "
                             << interp_posn.get(2) << "   "
                             << az*180/PI << "  "
                             << el*180/PI << "  "
                             << azdot*180/PI << "  "
                             << eldot*180/PI << "  "
                             << n_panel << "  "
                             << panel_U << "  "
                             << V << endl;
//               tracks_UV.push_back(twovector(U,V));
            }
         } // interpolated_posn conditional
      } // loop over track_counter
      tracks_stream << endl;

/*

// Import manually selected candidate aircraft pixel positions for
// frame 0:

   string aircraft_filename=ERSA_subdir+"wisp_aircraft.dat";
   filefunc::ReadInfile(aircraft_filename);

   vector<twovector> aircraft_UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int aircraft_px=column_values[1];
      int aircraft_py=column_values[2];
      double aircraft_U=double(aircraft_px)/double(pano_height);
      double aircraft_V=1-double(aircraft_py)/double(pano_height);
      aircraft_UV.push_back(twovector(aircraft_U,aircraft_V));
   }
   cout << "aircraft_UV.size() = " << aircraft_UV.size() << endl;

// Perform brute force comparison between aircraft_UV with all entries
// in tracks_UV

      double discrepancy_sum=0;
      for (int a=0; a<aircraft_UV.size(); a++)
      {
         twovector curr_aircraft_UV=aircraft_UV[a];
         double min_discrepancy=POSITIVEINFINITY;
         for (int j=0; j<tracks_UV.size(); j++)
         {
            twovector curr_track_UV=tracks_UV[j];
            double discrepancy=(curr_aircraft_UV-curr_track_UV).magnitude();
            if (discrepancy < min_discrepancy)
            {
               min_discrepancy=discrepancy;
            }
         }
         discrepancy_sum += min_discrepancy;
      } // loop over index a labeling aircraft seen in WISP pano
      
      if (discrepancy_sum < min_discrepancy_sum)
      {
         min_discrepancy_sum=discrepancy_sum;
         
         clock.convert_elapsed_secs_to_date(curr_t);

         cout << "curr_t = " << clock.YYYY_MM_DD_H_M_S()
              << " min_discrepancy_sum = " << min_discrepancy_sum
              << endl;
      }

*/

      curr_t += dt;
   } // curr_t < stop_time while loop
   cout << endl;

   filefunc::closefile(tracks_filename,tracks_stream);
   banner="Exported "+tracks_filename;
   outputfunc::write_big_banner(banner);

}
