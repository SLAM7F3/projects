// ========================================================================
// Program FLIRCLIETTEST
// ========================================================================
// Last updated on 10/20/11; 10/24/11; 10/27/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "Qt/web/FLIRClient.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"


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

/*
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

   string FLIRServer_URL=passes_group.get_LogicServer_URL();
*/

//   string FLIRServer_hostname="127.0.0.1";		
//   int FLIRServer_portnumber=4678;

   string FLIRServer_hostname="155.34.162.142";		// True xserve
   cout << "Enter FLIR server's IP address: (e.g. 155.34.162.142)"
        << endl;
   cin >> FLIRServer_hostname;

   int FLIRServer_portnumber=4567;

// Instantiate FLIRClient:

   FLIRClient* FLIRClient_ptr=new FLIRClient(
      FLIRServer_hostname,FLIRServer_portnumber);

   while (true)
   {
      app.processEvents();
   }
}



