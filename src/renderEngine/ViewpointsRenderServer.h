// ========================================================================
// Header file for Brad Boven's ViewpointsRenderServer class
// ========================================================================
// Last updated on 9/6/05
// ========================================================================

#ifndef VIEWPOINTS_RENDER_SERVER_H
#define VIEWPOINTS_RENDER_SERVER_H

#include <iostream>
#include <string>

#include <osgProducer/Viewer>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <osg/ArgumentParser>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/Optimizer>

#include "osg/AbstractOSGCallback.h"
#include "osg/AnimationPathCreator.h"
#include "osg/osg3D/CloudKeyHandler.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/communicator.h"
#include "osg/osg3D/Custom3DManipulator.h"
#include "osg/osgFeatures/FeaturesController.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturesKeyHandler.h"
#include "osg/osgFeatures/featurePickHandler.h"
#include "osg/osg3D/FlyOverController.h"
#include "osg/osg3D/FlyOverHUD.h"
#include "osg/ModeController.h"
#include "osg/ModeHUD.h"
#include "osg/ModeKeyHandler.h"
#include "osg/MyApplicationUsage.h"
#include "osg/MyViewerEventHandler.h"
#include "osg/osgfuncs.h"
#include "fusion/FilesData.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osg3D/AxisSpinner.h"
#include "osg/osgAnnotators/SignPostsController.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgAnnotators/SignPostsKeyHandler.h"
#include "osg/osgAnnotators/SignPostPickHandler.h"
#include "general/stringfuncs.h"
#include "fusion/TextDialogBox.h"
#include "osg/osg3D/threeDdatafuncs.h"
#include "osg/osg3D/Transformer.h"

class ViewpointsRenderServer : public OpenThreads::Thread
{
  public:

   ViewpointsRenderServer(int argc, char** argv, communicator* com_ptr);
   virtual ~ViewpointsRenderServer();

// Member function run does all the work for the program, called by
// the thread when thread->start is called in main program

   virtual void run();

// Tell the server data is now being sent, only do this once, because
// it resets the camera to a position where you can see your data

   void set_data_ready();

   //send an OSG group and specify a name, this name is used to later
   //delete or update this object (thus do not ever send two with same
   //name)
   void send_osg_group(osg::ref_ptr<osg::Group> data, std::string name);

   //delete previously send osg::Group* with name "name"
   void delete_osg_group(std::string name);

   //called by send_osg_group if the group being sent already exists in
   //the scenegraph
   void replace_osg_group(osg::ref_ptr<osg::Group> data, std::string name);

   //add a callback to the root node (workaround for problems with our
   //HUD's)
   void add_osg_callback(osg::Projection* projection_ptr);
   void add_osg_callback(osg::Geode* geode_ptr);

   //tell the server you are done sending over callbacks to be added to
   //the root node
   void set_callbacks_done();

   //just checks whether or not the thread has been killed
   bool if_killed();

   //kill the thread
   void kill();

  private:
        
   int ndims;
   communicator* com_ptr;
   osg::ArgumentParser* arguments;
   Transformer* transform;
   ColorMap* colormap;
   ModeController* ModeController_ptr;
   FlyOverController* FlyOverController_ptr;
   //osg::Group* signposts_OSGgroup_ptr;
   //SignPostsGroup* signposts_group;
   //SignPostsController* SignPostsController_ptr;
   //SignPostPickHandler* SignPostPickHandler_ptr;
   AxisSpinner* axis_spinner;
   osgProducer::MyViewerEventHandler* myVEH_ptr;
   osg::MyApplicationUsage* usage;
   osg::ref_ptr<osgProducer::Viewer> viewer;
   osg::ref_ptr<osg::Group> root;
   osg::ref_ptr<osgGA::Custom3DManipulator> c3Dm_ptr;
   bool killed;
   bool data_ready;
   bool callbacks_done;
};

#endif

