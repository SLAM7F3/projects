// ==========================================================================
// Program QTPYXIS_CLIENT is a skeletal example of the PYXIS client
// which G104 will need to implement in order to call LOST as a "black
// box" server.  It utilizes the post method within Qt's QHttp class
// in order to send potentially large amounts of flight path and other
// metadata to the LOST server.
// ==========================================================================
// Last updated on 1/25/12; 1/30/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

#include "Qt/web/LOSTClient.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// On 1/25/12, Dave Ceddia recommended unsetting the HTTP_PROXY and
// http_proxy environment variables for the current application job.
// Otherwise, Qt could get confused with trying to send client
// commands to LL's proxy rather than to the LOST Server's URL
// explicitly defined below:

   vector<string> proxy_env_vars;
   proxy_env_vars.push_back("http_proxy");
   proxy_env_vars.push_back("HTTP_PROXY");
   proxy_env_vars.push_back("https_proxy");
   proxy_env_vars.push_back("HTTPS_PROXY");
   for (int p=0; p<proxy_env_vars.size(); p++)
   {
      unsetenv(proxy_env_vars[p].c_str());
   }

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Instantiate LOSServer which receives get calls from web page
// buttons that can be mapped onto ModeController state changes:

   string LOST_Server_hostname="127.0.0.1";
   int LOST_Server_portnumber=4051;
   cout << "LOST_Server_hostname = " << LOST_Server_hostname
        << " LOST_Server_portnumber = " << LOST_Server_portnumber
        << endl;
   string LOST_Server_URL=
      LOST_Server_hostname+":"+stringfunc::number_to_string(
         LOST_Server_portnumber);

// Instantiate LOSTClient which uses Qt http functionality:

   LOSTClient* LOST_Client_ptr=new LOSTClient(LOST_Server_URL);

   string URL_subdir="/Start_New_Analysis/";
   string Full_LOST_Server_URL = "http://"+LOST_Server_URL+URL_subdir;
   cout << "Full LOST_Server_URL = " << Full_LOST_Server_URL << endl;

   vector<string> key,value;

// Region of Interest:

   key.push_back("ROI_LowerLeftLongitude");
   key.push_back("ROI_LowerLeftLatitude");
   key.push_back("ROI_UpperRightLongitude");
   key.push_back("ROI_UpperRightLatitude");

   value.push_back("69.73046875");	// degs
   value.push_back("34.7904296875");	// degs
   value.push_back("70.73046875");	// degs
   value.push_back("35.7904296875");	// degs

// Sensor parameters:

   key.push_back("Sensor_MinRange");
   key.push_back("Sensor_MaxRange");
   key.push_back("Sensor_HorizontalFOV");
   key.push_back("Sensor_LobePattern");

   value.push_back("31");	// kms
   value.push_back("200");	// kms
   value.push_back("120");	// degs
   value.push_back("Right");	// Right, Left, Both
   
// Temporal sampling:

   key.push_back("ClockIncrement");
   value.push_back("10");	// minutes

// Flight path:

   key.push_back("N_waypoints");
   value.push_back("5");

   key.push_back("Lon:0000");
   key.push_back("Lat:0000");
   key.push_back("Alt:0000");
   key.push_back("Time:0000");

   key.push_back("Lon:0001");
   key.push_back("Lat:0001");
   key.push_back("Alt:0001");
   key.push_back("Time:0001");

   key.push_back("Lon:0002");
   key.push_back("Lat:0002");
   key.push_back("Alt:0002");
   key.push_back("Time:0002");

   key.push_back("Lon:0003");
   key.push_back("Lat:0003");
   key.push_back("Alt:0003");
   key.push_back("Time:0003");

   key.push_back("Lon:0004");
   key.push_back("Lat:0004");
   key.push_back("Alt:0004");
   key.push_back("Time:0004");

   value.push_back("70.914368925781");
   value.push_back("35.166833808594");
   value.push_back("9154");		// meters
   value.push_back("1327907584.5");	// secs since midnight Jan 1, 1980

   value.push_back("70.126099882812");
   value.push_back("34.672449042969");
   value.push_back("9144");		// meters
   value.push_back("1327908489.69");

   value.push_back("69.381776152344");
   value.push_back("35.0707034375");
   value.push_back("9164");		// meters
   value.push_back("1327909300.8");

   value.push_back("69.788270292969");
   value.push_back("35.864465644531");
   value.push_back("9145");		// meters
   value.push_back("1327910255.28");

   value.push_back("70.741334257812");
   value.push_back("35.850732734375");
   value.push_back("9174");		// meters   
   value.push_back("1327911116.07");

   string query_params;
   for (int i=0; i<key.size(); i++)
   {
      query_params += key[i]+"="+value[i];
      if (i < key.size()-1)
      {
         query_params += "&";
      }
   }
   string query = query_params;

   cout << "Client query = " << endl;
   cout << query << endl;

   LOST_Client_ptr->issue_post_request(Full_LOST_Server_URL,query_params);

// Dave Ceddia taught us that we must execute the following Qt call in
// order for signals and slots to work!

   return app.exec();
//   cout << "At end of main()" << endl;
}

