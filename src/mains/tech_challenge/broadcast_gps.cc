// ========================================================================
// Program BROADCAST_GPS retrieves GPS track metadata stored within the
// track_points table of the TOC database for the specified
// mission and sensor.  It broadcasts the filtered positions so
// that they can be displayed as a moving dot within the Blue Force
// Tracker thin client.

/*

	    broadcast_gps --GIS_layer ./packages/gps_tracks.pkg

*/

// ========================================================================
// Last updated on 8/14/10; 8/16/10; 9/12/10; 9/21/10
// ========================================================================

#include <iostream>
#include <map>
#include <set>
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "postgres/gis_databases_group.h"
#include "astro_geo/GPS_datastream.h"
#include "math/mathfuncs.h"
#include "messenger/Messenger.h"
#include "track/mover_funcs.h"
#include "passes/PassesGroup.h"
#include "math/statevector.h"
#include "general/sysfuncs.h"
#include "general/stringfuncs.h"
#include "track/tracks_group.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::pair;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   gis_databases_group* gis_databases_group_ptr=new gis_databases_group;
   gis_database* gis_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

// Messenger to communicate between this program and BLUETRACKER thin
// client:

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( broker_URL, message_queue_channel_name );

// Messenger to listen for game clock update information:

   message_queue_channel_name="AnimationController";
   Messenger AnimationController_messenger(
      broker_URL,message_queue_channel_name);

   vector<int> fieldtest_IDs,mission_IDs,platform_IDs,sensor_IDs;
   vector<string> fieldtest_labels,mission_labels,platform_labels,
      sensor_labels;

   bool just_mission_ID_flag=false;
   mover_func::retrieve_mission_metadata_from_database(
      gis_database_ptr,mission_labels,mission_IDs,
      fieldtest_labels,fieldtest_IDs,platform_labels,platform_IDs,
      just_mission_ID_flag);

// Store platform ID and label as a function of mission ID within 
// STL map missionid_platform_map:

//   cout << "mission_IDs.size() = " << mission_IDs.size() << endl;
//   cout << "platform_labels.size() = " << platform_labels.size() << endl;

   typedef map<int,pair<int,string> > MISSIONID_PLATFORM_MAP; 

// First int = mission ID
// First int of second pair = platform ID
// Second string of second pair = platform label

   MISSIONID_PLATFORM_MAP missionid_platform_map;

   for (int i=0; i<mission_IDs.size(); i++)
   {
//      cout << "mission ID = " << mission_IDs[i]
//           << " mission label = " << mission_labels[i]
//           << " platform ID = " << platform_IDs[i] 
//           << " platform label = " << platform_labels[i] 
//           << endl;
      pair<int,string> p(platform_IDs[i],platform_labels[i]);
      missionid_platform_map[mission_IDs[i]]=p;
   }

   fieldtest_IDs.clear();
   fieldtest_labels.clear();
   mission_IDs.clear();
   mission_labels.clear();
   platform_IDs.clear();
   platform_labels.clear();

// Retrieve ALL mission and sensor IDs from TOC database:

   mover_func::retrieve_mission_metadata_from_database(
      gis_database_ptr,mission_IDs);
   mover_func::retrieve_sensor_metadata_from_database(
      gis_database_ptr,sensor_labels,sensor_IDs);

// Instantiate group for ALL raw GPS tracks.  Fill their contents with
// data retrieved from track_points table of TOC database:

   tracks_group raw_GPS_tracks_group;
   for (int m=0; m<mission_IDs.size(); m++)
   {
//      cout << "m = " << m << endl;
      int curr_mission_ID=mission_IDs[m];

      for (int s=0; s<sensor_IDs.size(); s++)
      {
         int curr_sensor_ID=sensor_IDs[s];
      
         bool daylight_savings_flag=true;
         vector<int> trackpoint_ID,fix_quality,n_satellites;
         vector<double> GPS_elapsed_secs,horiz_dilution;
         vector<double> longitude,latitude,altitude,roll,pitch,yaw;

         mover_func::retrieve_track_points_metadata_from_database(
            daylight_savings_flag,gis_database_ptr,
            curr_mission_ID,curr_sensor_ID,
            trackpoint_ID,GPS_elapsed_secs,
            fix_quality,n_satellites,horiz_dilution,
            longitude,latitude,altitude,roll,pitch,yaw);

         if (longitude.size()==0) continue;

// Store curr_mission_ID as raw track's label ID.  Store
// modified GPStrack_ID within raw track's plain ID:

         int GPStrack_ID=1000*curr_mission_ID+curr_sensor_ID;
         track* raw_GPS_track_ptr=raw_GPS_tracks_group.generate_new_track(
            GPStrack_ID);
         raw_GPS_track_ptr->set_label_ID(curr_mission_ID);
         
         MISSIONID_PLATFORM_MAP::iterator iter=missionid_platform_map.
            find(curr_mission_ID);
         string curr_platform_label;
         if (iter != missionid_platform_map.end())
         {
            curr_platform_label=iter->second.second;
         }
         raw_GPS_track_ptr->set_label(curr_platform_label);

         cout << "Generating new track with total ID = " << GPStrack_ID
              << ", missionID = " << curr_mission_ID
              << " & sensorID = " << curr_sensor_ID << endl;

         for (int i=0; i<trackpoint_ID.size(); i++)
         {
            threevector lla_posn(longitude[i],latitude[i],altitude[i]);
            threevector velocity(0,0,0);
            threevector quality(fix_quality[i],n_satellites[i],
               horiz_dilution[i]);
            raw_GPS_track_ptr->set_posn_velocity_GPSquality(
               GPS_elapsed_secs[i],lla_posn,velocity,quality);
         } // loop over index i labeling waypoints within current track

      } // loop over index s labeling sensor IDs 
   } // loop over index m labeling Mission IDs & raw GPS tracks

   cout << "raw_GPS_tracks_group.get_n_tracks() = "
        << raw_GPS_tracks_group.get_n_tracks() << endl;

   vector<track*> raw_GPS_track_ptrs=raw_GPS_tracks_group.get_all_track_ptrs();
   for (int t=0; t<raw_GPS_track_ptrs.size(); t++)
   {
      track* track_ptr=raw_GPS_track_ptrs[t];
//      cout << "t = " << t
//           << " track label = " << track_ptr->get_label() << endl;
   }

//   cout << "Before infinite while loop in BROADCAST_GPS" << endl;

// Instantiate STL map to hold track previous visibility information.
// Assume all tracks are NOT initially visible:

   typedef map<int,bool > MISSIONID_PREV_VISIBILITY_MAP; 

// First int = mission ID
// Second bool = track previously visible

   MISSIONID_PREV_VISIBILITY_MAP missionid_prev_visibility_map;

   for (int i=0; i<mission_IDs.size(); i++)
   {
      missionid_prev_visibility_map[mission_IDs[i]]=false;
   }


   Clock clock;
   while(true)
   {
      int n_mailbox_messages=AnimationController_messenger.
         get_n_messages_in_mailbox();
      if (n_mailbox_messages==0) continue;

      if (AnimationController_messenger.copy_messages_and_purge_mailbox())
      {
         for (int i=0; i<AnimationController_messenger.get_n_curr_messages(); 
              i++)
         {
            const message* curr_message_ptr=
               AnimationController_messenger.get_message_ptr(i);
            if (curr_message_ptr== NULL) continue;

//            cout << "text_message = " 
//                 << curr_message_ptr->get_text_message() << endl;
            int n_properties=curr_message_ptr->get_n_properties();
//               cout << "n_properties = " << n_properties << endl;
            for (int p=0; p<n_properties; p++)
            {
               message::Property curr_property=curr_message_ptr->
                  get_property(p);
               string key=curr_property.first;
               if (key != "Time") continue;

               string value=curr_property.second;               
               double secs_elapsed=stringfunc::string_to_number(value);

//               cout << "p = " << p
//                    << " key = " << key
//                    << " value = " << value << endl;

               clock.convert_elapsed_secs_to_date(secs_elapsed);
//               cout << "secs_elapsed = " << secs_elapsed << endl;
//               cout << "UTC time = "
//                    << clock.YYYY_MM_DD_H_M_S(" ",":",true) << endl;
//               cout << "Local time = "
//                    << clock.YYYY_MM_DD_H_M_S(" ",":",false) << endl;
//               cout << "UTM_zone time offset = " 
//                    << clock.get_UTM_zone_time_offset() << endl;
               
// Loop over each raw GPS track:	

               for (int r=0; r<raw_GPS_track_ptrs.size(); r++)
               {
                  track* raw_GPS_track_ptr=raw_GPS_track_ptrs[r];
//                  cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//                  cout << "r = " << r 
//                       << " raw_GPS_track_ptr->get_label_ID() = missionID = "
//                       << raw_GPS_track_ptr->get_label_ID() << endl;
//                  cout << "raw_GPS_track_ptr->get_ID() = sensorID = "
//                       << raw_GPS_track_ptr->get_ID() << endl;

                  int curr_mission_ID=raw_GPS_track_ptr->get_label_ID();

                  MISSIONID_PLATFORM_MAP::iterator iter=missionid_platform_map.
                     find(curr_mission_ID);
                  int curr_platform_ID=-1;
                  if (iter != missionid_platform_map.end())
                  {
                     curr_platform_ID=iter->second.first;
                  }
                  colorfunc::Color dot_color=
                     colorfunc::get_platform_color(curr_platform_ID);

                  bool curr_visible_flag=
                     raw_GPS_track_ptr->broadcast_statevector(
                        secs_elapsed,&viewer_messenger,dot_color);

                  bool prev_visible_flag=false;
                  MISSIONID_PREV_VISIBILITY_MAP::iterator vis_iter=
                     missionid_prev_visibility_map.find(curr_mission_ID);
                  if (vis_iter != missionid_prev_visibility_map.end())
                  {
                     prev_visible_flag=vis_iter->second;
                     missionid_prev_visibility_map[curr_mission_ID]=
                        curr_visible_flag;
                  }

                  if (curr_visible_flag && prev_visible_flag)
                  {
                     // curr track posn already broadcast
                  }
                  else if (curr_visible_flag && !prev_visible_flag)
                  {
                     // curr track posn already broadcast
                  }
                  else if (!curr_visible_flag && prev_visible_flag)
                  {
                     threevector grid_origin=Zero_vector;
                     statevector dummy_statevector(
                        secs_elapsed,Zero_vector,Zero_vector);
                     raw_GPS_track_ptr->broadcast_statevector(
                        dummy_statevector,&grid_origin,&viewer_messenger,
                        dot_color);
                  }
                  else if (!curr_visible_flag && !prev_visible_flag)
                  {
                  }
                  
               } // loop over index r labeling raw GPS tracks

            } // loop over index p labeling properties
         } // loop over index i labeling received messages
      } // m.copy_messages_and_purge_mailbox conditional


   } // infinite while loop

}



