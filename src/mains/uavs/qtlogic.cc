// ========================================================================
// Program QTLOGIC acts as a logic tier within the SAM system.  Using
// a Qt-based webserver, it takes in GET requests from thick/thin
// clients and generates XML responses.  It also dynamically generates
// KML files which can be networked linked to automatically display
// within Google Earth.

/*

		qtlogic --LogicServer_URL 155.34.162.125:4040 \
			--SKSDataServer_URL 155.34.125.216:8080 \
			--HTMLServer_URL 155.34.162.125 

		qtlogic --LogicServer_URL 127.0.0.1:4040 \
			--SKSDataServer_URL 127.0.0.1:8080 \
			--HTMLServer_URL 127.0.0.1


*/

// ========================================================================
// Last updated on 3/25/08; 4/28/08; 4/29/08; 5/1/08; 5/9/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/ArgumentParser>
#include <QtCore/QtCore>

#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "gearth/kml_parser.h"
#include "passes/PassesGroup.h"
#include "robots/SAMs_group.h"
#include "Qt/web/SAMServer.h"
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
   cout << "HTMLServer_URL = " << HTMLServer_URL << endl;

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
   SAMs.generate_empty_KML_files();

// Initialize Qt objects:

   QCoreApplication app(argc, argv);

   string LogicServer_hostname=stringfunc::get_hostname_from_URL(
      LogicServer_URL);
   int LogicServer_portnumber=stringfunc::get_portnumber_from_URL(
      LogicServer_URL);
   SAMServer server(&SAMs,LogicServer_hostname,LogicServer_portnumber);
   server.establish_SKSDataServer_connection(SKSDataServer_URL);
   server.set_HTMLServer_URL(HTMLServer_URL);
   
   return app.exec();
}

