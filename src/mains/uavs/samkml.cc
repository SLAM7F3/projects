// ========================================================================
// Program SAMKML is a test playground for generating KML files to
// show SAM sites within GoogleEarth.  It simply fills in templates
// generated from within GoogleEarth itself with new long,lat,alt
// vertices.
// ========================================================================
// Last updated on 3/2/08; 3/4/08; 3/5/08; 3/6/08; 3/7/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "gearth/kml_parser.h"
#include "messenger/Messenger.h"
#include "robots/SAMs_group.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize SAM sites from input text file information:

   SAMs_group SAMs;

   string SAM_sites_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/sam_sites/";
//   string SAM_list_filename=SAM_sites_subdir+"single.sams";
//   string SAM_list_filename=SAM_sites_subdir+"triple.sams";
//   string SAM_list_filename=SAM_sites_subdir+"north_korea.sams";
//   string SAM_list_filename=SAM_sites_subdir+"iran.sams";
//   string SAM_list_filename=SAM_sites_subdir+"china.sams";
   string SAM_list_filename=SAM_sites_subdir+"total.sams";
   SAMs.generate_all_SAMs(SAM_list_filename);

// Instantiate messengers:

   string broker_URL="tcp://155.34.125.216:61616";	// family day
//   string broker_URL="tcp://155.34.135.239:61616";	// G104 conf room
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string UAV_message_queue_channel_name="SAM";
   Messenger UAV_messenger( 
      broker_URL, UAV_message_queue_channel_name );

// Infinite event loop:

   while (true)
   {

// Query user to enter particular SAM type:
      
      string query_SAM_name="";

//         string query_SAM_name="SA-10";
//      cout << "Enter SAM name:" << endl;
//      cin >> query_SAM_name;


      int n_messages=UAV_messenger.get_n_messages_in_mailbox();
      if ( n_messages <= 0) continue;
      
      cout << "n_messages = " << n_messages << endl;
      UAV_messenger.copy_messages_and_purge_mailbox();

      vector<message> messages;
      for (int i=0; i<n_messages; i++)
      {
         const message* curr_message_ptr=UAV_messenger.get_message_ptr(i);
         if (curr_message_ptr != NULL)
         {
//               cout << "Message " << i << " : "
//                    << curr_message_ptr->get_text_message() << endl;
            messages.push_back(*curr_message_ptr);
         }
      } // loop over index i labeling received messages
      message last_message=messages.back();
      cout << "Received message from queue: " << last_message << endl;
      messages.clear();

// Parse string properties for last message and extract queried SAM
// type:

      if (last_message.get_text_message()=="QUERY" &&
          last_message.get_n_properties() > 1)
      {
   
         for (int p=0; p<last_message.get_n_properties(); p++)
         {
            string key=last_message.get_property(p).first;
            string value=last_message.get_property(p).second;
//            cout << "p = " << p
//                 << " key = " << key
//                 << " value = " << value << endl;

            if (key=="sam_type")
            {
               query_SAM_name=value;
            }
         }
//         cout << "query_SAM_name = " << query_SAM_name << endl;

      } // last_message is QUERY and has more than 1 property conditional

// Determine countries owning queried SAM type:

      vector<string> country_owner_names=
         SAMs.countries_owning_particular_SAMs(
            query_SAM_name,&UAV_messenger);

// Broadcast locations of sites corresponding to queries SAM type:

      SAMs.queried_SAM_sites(query_SAM_name,&UAV_messenger);

// Generate KML file illustrating owner countries for queried SAM
// type:

      SAMs.generate_owner_countries_KML_file(country_owner_names);

// Next generate separate KML file containing actual sites
// corresponding to queried SAM type:

      SAMs.generate_SAM_sites_KML_file(query_SAM_name);

   } // infinite while event loop
   
}

