// ========================================================================
// Program MYCLIENT

// 				myclient

// ========================================================================
// Last updated on 3/10/08; 3/12/08; 3/14/08; 3/16/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/ArgumentParser>
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "gearth/kml_parser.h"
#include "passes/PassesGroup.h"
#include "robots/SAMs_group.h"
#include "SKSDataServerInterfacer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

#include <QtCore/QtCore>
#include "SAMServer.h"
#include "WebClient.h"
#include "WebServer.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   PassesGroup passes_group(&arguments);

   int pass_ID=passes_group.get_n_passes()-1;

// Initialize Qt objects:

   QCoreApplication app(argc, argv);

//   string DataServer_host_IP="155.34.162.78"; // fusion1
//   int port=4040;

   string SKSDataServer_IP="127.0.0.1";	// localhost dataserver
//   string SKSDataServer_IP="155.34.125.216";	// touchy dataserver
//   string SKSDataServer_IP="155.34.135.168";	// dsherrill dataserver
   int port=8080;

   SKSDataServerInterfacer sks_interface(SKSDataServer_IP,port);

   double range=100;	// kms
   int IOC=1975;
   
   string query1=sks_interface.form_all_SAM_attributes_query();
   string query2=sks_interface.form_particular_SAM_attributes_query("SA-4");
   string query3=sks_interface.form_filtered_SAM_range_query(range);
   string query4=sks_interface.form_filtered_SAM_IOC_query(IOC);
   string query5=sks_interface.form_filtered_SAM_range_and_IOC_query(
      range,IOC);
   string query6=sks_interface.form_countries_owning_SAM_query("SA-10");
   string query7=sks_interface.form_SAM_sites_query("SA-4");


   string query=query6;

   string GET_command="GET "+query+" HTTP/1.0\r\n\r\n";

/*
   string GET_command=
      "GET http://155.34.162.78:4040/global/?sam_type=Unknown&country=South+Korea&ioc=1950&range=5-10&command=getSamInfo HTTP/1.0\r\n\r\n";
*/

/*
   string GET_command=
     "GET http://dsherrill:8080/SKSDataServer/query?type=sam&attrib=sam/system,sam/ioc.year,sam/site HTTP/1.0\r\n\r\n";
*/

//   GET_command=
//     "GET http://localhost:8080/SKSDataServer/query?type=sam&attrib=sam/system,sam/ioc.year,sam/site HTTP/1.0\r\n\r\n";

   cout << GET_command << endl;

   WebClient* WebClient_ptr = new WebClient(
      SKSDataServer_IP,port,GET_command);
 

   cout << "inside MAIN before call to get_returned_output()" << endl;
   string xml_output=WebClient_ptr->get_returned_output();

   cout << "At end of main, xml_output = " << xml_output << endl;

   return app.exec();
}

