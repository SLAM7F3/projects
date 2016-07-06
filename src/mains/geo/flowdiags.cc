// ==========================================================================
// Program FLOWDIAGS is a specialized utility which we wrote for GEO
// demonstration purposes.  It listens on an ActiveMQ channel for
// "DISPLAY_NEXT_FLOW_DIAGRAM" messages.  When received, FLOWDIAGS
// retrieves a pre-generated GEO diagram and annotates it with
// elapsed times corresponding to different stages of the algorithm
// flow.  The annotated diagrams are subsequently displayed in the
// current screen's upper left corner.
// ==========================================================================
// Last updated on 7/30/13; 7/31/13; 9/11/13; 11/11/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "messenger/Messenger.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;


// --------------------------------------------------------------------------
void display_next_diagram(
   int diagram_ID,vector<twovector>& pxy_start,vector<int>& elapsed_secs)
{
   string projects_rootdir = getenv("PROJECTSROOT");
   string flow_diag_subdir= projects_rootdir+"/src/mains/geo/demo_flow_diagrams/";
   string flow_diag_filename="flow_diag_"+stringfunc::number_to_string(
      diagram_ID)+".png";
   string input_PNG_filename =flow_diag_subdir+flow_diag_filename;
   string output_PNG_filename=input_PNG_filename;

//   cout << "diagram_ID = " << diagram_ID << endl;
//   cout << "elapsed_secs.size() = " << elapsed_secs.size() << endl;

   if (diagram_ID > 0)
   {
      int fontsize=13;
//      int fontsize=15;
      vector<string> text_lines;
      vector<colorfunc::RGB> text_RGB;
      for (unsigned int i=0; i<elapsed_secs.size(); i++)
      {
         string text_line="Elapsed time="+stringfunc::number_to_string(
            elapsed_secs[i])+" secs";
         text_lines.push_back(text_line);
         text_RGB.push_back(colorfunc::RGB(0 , 0.6 , 0) );
      }
   
//      cout << "Text_lines.size() = " << text_lines.size() << endl;

      if (text_lines.size() > 0)
      {
         string png_subdir=filefunc::getdirname(input_PNG_filename);
         string png_basename=filefunc::getbasename(input_PNG_filename);
         output_PNG_filename=png_subdir+"timed_"+png_basename;
      
         pngfunc::add_text_to_PNG_image(
            input_PNG_filename,output_PNG_filename,fontsize,
            text_lines,pxy_start,text_RGB);
      }
//   cout << "output_PNG_filename = " << output_PNG_filename << endl;
   }

   string unix_cmd=
      "display -geometry +0+0 "+output_PNG_filename+" &";
   sysfunc::unix_command(unix_cmd);
}

// --------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   cout.precision(12);

   Clock clock;

   int UTM_zonenumber=19;
   bool daylight_savings_flag=false;
   clock.set_time_based_on_local_computer_clock(
      UTM_zonenumber,daylight_savings_flag);
   double starting_epoch=clock.secs_elapsed_since_reference_date();
   cout << "Starting time = " 
        << clock.YYYY_MM_DD_H_M_S() << endl;

// Set the URI to point to the IPAddress of your broker.  Add any
// optional params to the url to enable things like tightMarshalling
// or tcp logging etc.

   string broker_URL = "tcp://127.0.0.1:61616";
   if ( argc > 1 ) broker_URL = "tcp://" + string(argv[1]);

   string message_queue_channel_name="127.0.0.1";
   
   bool include_sender_and_timestamp_info_flag=false;
   Messenger m(broker_URL,message_queue_channel_name,
               include_sender_and_timestamp_info_flag);
   string banner="Listening on queue channel "+message_queue_channel_name;
//   outputfunc::write_banner(banner);

   int diagram_counter=0;

   vector<twovector> pxy_start;
   pxy_start.push_back(twovector(292,141));
   pxy_start.push_back(twovector(395,225));
   pxy_start.push_back(twovector(489,309));
   pxy_start.push_back(twovector(583,393));
   pxy_start.push_back(twovector(677,477));
   pxy_start.push_back(twovector(771,561));

//   pxy_start.push_back(twovector(306,141));
//   pxy_start.push_back(twovector(400,225));
//   pxy_start.push_back(twovector(494,309));
//   pxy_start.push_back(twovector(588,393));
//   pxy_start.push_back(twovector(682,477));
//   pxy_start.push_back(twovector(776,561));
   vector<int> elapsed_secs;
   
   while (true) 
   {

// Davis says that infinite while loops generally eat up CPU as hard
// as they can!  So he recommends putting in a sleep statement to slow
// this loop way down.  He's right...

      usleep(500);
      int n_mailbox_messages=m.get_n_messages_in_mailbox();
      if (n_mailbox_messages==0) continue;

      if (m.copy_messages_and_purge_mailbox())
      {
	for (unsigned int i=0; i<m.get_n_curr_messages(); i++)
         {
            message* curr_message_ptr=m.get_message_ptr(i);
            if (curr_message_ptr != NULL)
            {
//               cout << "curr_message from messenger = " 
//                    << *curr_message_ptr << endl;

               if (curr_message_ptr->get_text_message()==
                   "DISPLAY_NEXT_FLOW_DIAGRAM")
               {
                  if (diagram_counter > 0)
                  {
                     clock.set_time_based_on_local_computer_clock(
                        UTM_zonenumber,daylight_savings_flag);
                     double curr_epoch=
                        clock.secs_elapsed_since_reference_date();
                     elapsed_secs.push_back(curr_epoch-starting_epoch);
                  }
                  
                  display_next_diagram(
                     diagram_counter,pxy_start,elapsed_secs);

                  diagram_counter++;
               }
               
            } // curr_message_ptr != NULL conditional
         } // loop over index i labeling received messages

      } // m.copy_messages_and_purge_mailbox conditional
      
   } // infinite while loop

}

