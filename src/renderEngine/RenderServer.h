// ==========================================================================
// Header file for Brad Boven's RenderServer class
// ==========================================================================
// Last modified on 9/13/05
// ==========================================================================

#ifndef RENDER_SERVER_H
#define RENDER_SERVER_H

#include <string>
#include <osgProducer/Viewer>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <osg/ArgumentParser>

#include "osg/examplevideos/ExampleCustomManipulator.h"
#include "osg/MyApplicationUsage.h"

class RenderServer : public OpenThreads::Thread
{
  public:

   RenderServer(int argc, char** argv);
   virtual ~RenderServer();

   //run method does all the work for the program, called by the thread
   //when thread->start is called in main program
   virtual void run ();

   //tell the server data is now being sent, only do this once, because
   //it resets the camera to a position where you can see your data
   void set_data_ready();

   //send an OSG group and specify a name, this name is used to later
   //delete or update this object (thus do not ever send two with same
   //name)
   void send_osg_group(osg::ref_ptr<osg::Group> data, std::string name);

   //delete previously sent osg::Group* with name "name"
   void delete_osg_group(std::string name);

   //called by send_osg_group if the group being sent already exists in
   //the scenegraph
   void replace_osg_group(osg::ref_ptr<osg::Group> data, std::string name);

   //just checks whether or not the thread has been killed
   bool if_killed();

   //kill the thread
   void kill();

  private:
        
   osg::MyApplicationUsage* usage;
   osg::ref_ptr<osgProducer::Viewer> viewer;
   osg::ref_ptr<osg::Group> root;
   osg::ref_ptr<osgGA::ExampleCustomManipulator> c3Dm_ptr;
   bool killed;
   bool data_ready;
};

#endif

