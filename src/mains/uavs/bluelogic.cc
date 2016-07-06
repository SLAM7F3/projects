// ========================================================================
// Program BLUELOGIC acts as a logic tier within the Bluegrass system.
// Using a Qt-based webserver, it takes in GET requests from
// thick/thin clients and generates XML responses. 

/* 

  bluelogic --WebServer_hostname 127.0.0.1 \
	    --SKSDataServer_host_URL localhost:8080

*/


// ========================================================================
// Last updated on 4/28/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/ArgumentParser>
#include <QtCore/QtCore>

#include "Qt/web/BluegrassServer.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "passes/PassesGroup.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   PassesGroup passes_group(&arguments);

   int pass_ID=passes_group.get_n_passes()-1;
   string WebServer_hostname=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_WebServer_hostname();
   cout << "WebServer_hostname = " << WebServer_hostname << endl;
   string SKSDataServer_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_SKSDataServer_host_URL();
   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;

// Initialize Qt objects:

   QCoreApplication app(argc, argv);

   int WebServer_port=4040;
   BluegrassServer server(WebServer_hostname,WebServer_port);
   server.establish_SKSDataServer_connection(SKSDataServer_URL);
   
   return app.exec();
}

