// ========================================================================
// Program BLUELOGIC acts as a logic tier within the Bluegrass system.
// Using a Qt-based webserver, it takes in GET requests from
// thick/thin clients and generates XML responses. 

/* 

  bluelogic --LogicServer_URL 127.0.0.1:4040 \
	    --SKSDataServer_URL localhost:8080 \
	    --HTMLServer_URL 127.0.0.1:8080
*/


// ========================================================================
// Last updated on 4/28/08; 4/30/08; 5/2/08; 5/9/08
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
   string LogicServer_URL=passes_group.get_LogicServer_URL();
   cout << "LogicServer_URL = " << LogicServer_URL << endl;
   string SKSDataServer_URL=passes_group.get_SKSDataServer_URL();
   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
   string HTMLServer_URL=passes_group.get_HTMLServer_URL();
   cout << "HTML_Server_URL = " << HTMLServer_URL << endl;
   string Dynamic_WikiPage_URL=passes_group.get_Dynamic_WikiPage_URL();
   cout << "Dynamic_WikiPage_URL = " << Dynamic_WikiPage_URL << endl;

// Initialize Qt objects:

   QCoreApplication app(argc, argv);

   string LogicServer_hostname=stringfunc::get_hostname_from_URL(
      LogicServer_URL);
   int LogicServer_portnumber=stringfunc::get_portnumber_from_URL(
      LogicServer_URL);
   BluegrassServer server(LogicServer_hostname,LogicServer_portnumber);
   server.set_HTMLServer_URL(HTMLServer_URL);
   server.set_Dynamic_WikiPage_URL(Dynamic_WikiPage_URL);
   server.establish_SKSDataServer_connection(SKSDataServer_URL);
   
   return app.exec();
}

