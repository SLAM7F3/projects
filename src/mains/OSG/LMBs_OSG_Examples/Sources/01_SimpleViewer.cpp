/******************************************************************************\
* SimpleViewer                                                                 *
* An OSG-based 3D viewer. Almost as simple as possible.                        *
* Leandro Motta Barros (based on Tutorials from TerseSolutions)                *
\******************************************************************************/


#include <iostream>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>


int main (int argc, char* argv[])
{
   // Check command-line parameters
   if (argc != 2)
   {
      std::cerr << "Usage: " << argv[0] << " <model file>\n";
      exit (1);
   }

   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Load the model
   osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);

   if (!loadedModel)
   {
      std::cerr << "Problem opening '" << argv[1] << "'\n";
      exit (1);
   }
   viewer.setSceneData (loadedModel.get());

   // Optimize the scene graph (a simple as possible viewer doesn't really
   // need this.
   osgUtil::Optimizer optOSGFile;
   optOSGFile.optimize (loadedModel.get());

   // Enter rendering loop
   viewer.realize();

   while (!viewer.done())
   {
      // wait for all cull and draw threads to complete.
      viewer.sync();

      // update the scene by traversing it with the the update visitor which will
      // call all node update callbacks and animations.
      viewer.update();

      // fire off the cull and draw traversals of the scene.
      viewer.frame();
   }

   // Wait for all cull and draw threads to complete before exit.
   viewer.sync();
}
