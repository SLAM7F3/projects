// ========================================================================
// Program FLIRSERVERTEST 
// ========================================================================
// Last updated on 10/24/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "Qt/web/FLIRServer.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"
#include "track/tracks_group.h"
#include "osg/osgWindow/ViewerManager.h"

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
*/


   string FLIRServer_hostname="";		
   int FLIRServer_portnumber=4678;

   FLIRServer FLIR_server(FLIRServer_hostname,FLIRServer_portnumber);

   while( true )
   {
      app.processEvents();
   }
}

