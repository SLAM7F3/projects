// ========================================================================
// Program CARS broadcasts time-dependent Bluegrass truth tracks
// translated to the Milwaukee region.
// ========================================================================
// Last updated on 4/30/09; 5/1/09; 6/27/09; 8/26/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "Qt/web/BluegrassClient.h"
#include "astro_geo/Clock.h"
#include "messenger/Messenger.h"
#include "passes/PassesGroup.h"
#include "math/statevector.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "track/tracks_group.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   vector<int> dataserver_IDs=passes_group.get_dataserver_IDs();
   int dataserverpass_ID=-1;

   string BluegrassServer_URL=passes_group.get_LogicServer_URL();
   cout << "BluegrassServer_URL = " << BluegrassServer_URL << endl;
   string SKSDataServer_URL=passes_group.get_SKSDataServer_URL();
   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;

// Instantiate BluegrassClient which uses Qt http functionality:

   BluegrassClient* BluegrassClient_ptr=new BluegrassClient(
      BluegrassServer_URL);

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate cars messenger for communication with 3D geospatial
// viewer:

   string broker_URL = "tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   
   string cars_message_queue_channel_name="cars";
   string message_sender_ID="CARS";
   bool include_sender_and_timestamp_info_flag=false;
   Messenger cars_messenger( 
      broker_URL,cars_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

// Retrieve vehicle ground truth tracks from track server:

   int d=0;
   PassInfo* passinfo_ptr=
      passes_group.get_pass_ptr(dataserver_IDs[d])->get_PassInfo_ptr();
   string SKS_query=BluegrassClient_ptr->
      form_mover_tracks_query(passinfo_ptr);
//   cout << "SKS_query = " << SKS_query << endl;
   BluegrassClient_ptr->query_BluegrassServer(SKS_query);
   string SKS_response=BluegrassClient_ptr->get_BluegrassServer_response();
//      cout << "SKS_response = " << SKS_response << endl;
//   cout << "SKS_response.size() = " << SKS_response.size() << endl;

   tracks_group* groundtruth_tracks_group_ptr=new tracks_group();

// In order to use Bluegrass truth tracks for Milwaukee dynamic ground
// mover simulations, use following call to
// BluegrassClient::retrieve_mover_tracks():

   double secs_offset=0;
   const int Lubbock_UTM_zonenumber=14;
   threevector old_origin_offset(227715,3711685); // AR1 center
   double track_rescaling_factor=1.0;
//   double track_rescaling_factor=4.0;
//   threevector new_origin_offset( 415285.996354 , 4765781.51378 );	
						      // Milwaukee center
   threevector new_origin_offset( 308976.151461 , 4723470.48264 );
						      // Lowell center

   double tracks_altitude=0;
   int n_tracks=BluegrassClient_ptr->retrieve_mover_tracks(
      SKS_response,Lubbock_UTM_zonenumber,
      old_origin_offset,track_rescaling_factor,
      new_origin_offset,secs_offset,tracks_altitude,
      groundtruth_tracks_group_ptr);

// In order to maintain ground vehicle speeds, we need to rescale
// track time values by the same value as track ground positions:

//         cout << "Curr track rescaling factor = "
//              << track_rescaling_factor << endl;
//         cout << "Enter track rescaling factor:" << endl;
//         cin >> track_rescaling_factor;

   groundtruth_tracks_group_ptr->rescale_time_values_for_all_tracks(
      track_rescaling_factor);

   vector<track*> groundtruth_track_ptrs=
      groundtruth_tracks_group_ptr->get_all_track_ptrs();

// Reset track times so that they're referenced in secs relative to
// current UTC:

   cout.precision(13);
//   cout << "clock.secs_elapsed_since_reference_date() = "
//        << clock.secs_elapsed_since_reference_date() << endl;
//   outputfunc::enter_continue_char();

   for (int t=0; t< int(groundtruth_track_ptrs.size()); t++)
   {
      track* curr_track_ptr=groundtruth_track_ptrs[t];
      double delta_t=
         clock.secs_elapsed_since_reference_date()
         -curr_track_ptr->get_earliest_time();
      curr_track_ptr->offset_time_values(delta_t);
//      cout << "t = " << t << endl;
//      cout << "*curr_track_ptr = " << *curr_track_ptr << endl;
   } // loop over index t labeling ground truth tracks

// ========================================================================

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="UPDATE_TRACK_POSN";	

   int prev_sec=-1;
   while( true )
   {
      clock.current_local_time_and_UTC();
      int curr_sec=clock.get_seconds();
      if (curr_sec==prev_sec) continue;

      prev_sec=curr_sec;
      double secs_elapsed_since_epoch=
         clock.secs_elapsed_since_reference_date();
//      cout << "elapsed secs = " << secs_elapsed_since_epoch << endl;

      int n_tracks=5;
//      int n_tracks=groundtruth_track_ptrs.size();
      for (int t=0; t< n_tracks; t++)
      {
         track* curr_track_ptr=groundtruth_track_ptrs[t];

         statevector curr_statevector;
         if (curr_track_ptr->get_interpolated_statevector(
            secs_elapsed_since_epoch,curr_statevector))
         {
            threevector curr_posn(curr_statevector.get_position());
//            cout << "t=" << t 
//                 << " secs=" << secs_elapsed_since_epoch
//                 << " X=" << curr_posn.get(0)
//                 << " Y=" << curr_posn.get(1) << endl;

            properties.clear();
            
            key="ID";
            int ID=curr_track_ptr->get_ID();
            value=stringfunc::number_to_string(ID);
            properties.push_back(Messenger::Property(key,value));

            key="Time";
            value=stringfunc::number_to_string(secs_elapsed_since_epoch);
            properties.push_back(Messenger::Property(key,value));

            key="X Y";
            value=stringfunc::number_to_string(curr_posn.get(0))+" "+
               stringfunc::number_to_string(curr_posn.get(1));
            properties.push_back(Messenger::Property(key,value));

            cars_messenger.sendTextMessage(command,properties);
         }
      } // loop over index t labeling ground truth tracks

   } // infinite while loop

}



