/******************************************************************************\
* RenderToTexture                                                              *
* Using an 'osg::CameraNode' to render a subgraph to a texture                 *
* Leandro Motta Barros (somewhat inspired by the standard 'osgprerender'       *
* example)                                                                     *
\******************************************************************************/

// There are quite a lot of options that allow us to fine tune the way
// in which render to texture is done in OSG. The 'osgprerender'
// example, that can be found in the OSG source distribution shows
// many of these options.  My render to texture example is simpler,
// and so hopefully easier to understand.

#include <osg/CameraNode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>


// Just creates a 'Drawable' sphere.
osg::ref_ptr<osg::Drawable> CreateSphere (double x, double y, double z,
                                          double radius)
{
   osg::ref_ptr<osg::Sphere> sphere (new osg::Sphere (osg::Vec3(x, y, z), radius));
   osg::ref_ptr<osg::Drawable> sphereShapeDrawable(
      new osg::ShapeDrawable (sphere.get()));
   return sphereShapeDrawable;
}


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
   viewer.setUpViewer (osgProducer::Viewer::STANDARD_SETTINGS);

   // Create the node to be used as the root of our scene graph. In a few
   // moments, we'll add two subgraphs to it.
   osg::ref_ptr<osg::Group> root (new osg::Group());

   // This is a render to texture example. We will render the object passed as
   // command-line parameter to a texture and map this texture to a another
   // object (a sphere). So, we will need a texture. Here it is.
   const int TEX_WIDTH = 512;
   const int TEX_HEIGHT = 256;
   osg::ref_ptr<osg::Texture2D> texture (new osg::Texture2D());
   texture->setTextureSize (TEX_WIDTH, TEX_HEIGHT);

   // Here is the first subgraph. It contains only a sphere with the texture
   // mapped to it.
   osg::ref_ptr<osg::Geode> sphereGeode (new osg::Geode());
   sphereGeode->addDrawable (CreateSphere (0.0, 0.0, 0.0, 1.0).get());
   osg::ref_ptr<osg::StateSet> ss (new osg::StateSet());
   ss->setTextureAttributeAndModes (0, texture.get(), osg::StateAttribute::ON);
   sphereGeode->setStateSet (ss.get());

   root->addChild (sphereGeode.get());

   // And here is the second subgraph. This one will do the actual render to
   // texture job. First, we load the model that will be rendered to the
   // texture.
   osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);

   if (!loadedModel)
   {
      std::cerr << "Problem opening '" << argv[1] << "'\n";
      exit (1);
   }

   // Now, we create an 'osg::CameraNode', which will do all the hard
   // work. The 'CameraNode' will render its subgraph to a texture. In
   // this example, we want to render the loaded model to a texture,
   // so we add it as a 'CameraNode''s child.
   osg::ref_ptr<osg::CameraNode> camera (new osg::CameraNode);
   camera->addChild (loadedModel.get());

   // The camera node must be positioned in order to make it point to
   // the things we want rendered to a texture. This is no different
   // that positioning a "normal" camera for on-screen rendering. For
   // simplicity, in this example, I use some hardcoded values to
   // position the 'CameraNode' (and therefore the results may be not
   // very nice depending on the size and position of the object
   // passed as command-line parameter). The 'osgprerender' example
   // has some sample code on a smarter way to position the
   // 'CameraNode' based on the object's bounding volume.
   camera->setProjectionMatrixAsFrustum (-.05, .05, -.025, .025, 0.1, 1.0);
   camera->setReferenceFrame (osg::Transform::ABSOLUTE_RF);
   camera->setViewMatrixAsLookAt (osg::Vec3 (0.0, 3.0, 0.0),
                                  osg::Vec3 (0.0, 0.0, 0.0),
                                  osg::Vec3 (0.0, 0.0, 1.0));
   camera->setViewport (0, 0, TEX_WIDTH, TEX_HEIGHT);

   // By default, black is used as the 'CameraNode''s clear color. It
   // think this one in nicer.
   camera->setClearColor (osg::Vec4 (0.8f, 0.8f, 0.8f, 1.0f));

   // We want to render to texture before the other subgraph is
   // rendered (otherwise we could not properly use this texture in
   // the other subgraph).  So, we set the render order flag to
   // 'PRE_RENDER'.
   camera->setRenderOrder (osg::CameraNode::PRE_RENDER);

   // OpenGL offers many ways to render to texture, and
   // 'osg::CameraNode' allows us to select which one we want to
   // use. In this case, I'm telling OpenGL to use an FBO (frame
   // buffer object). Other possibilities are described in the
   // 'osg::CameraNode::RenderTargetImplementation' enumeration.
   camera->setRenderTargetImplementation (osg::CameraNode::FRAME_BUFFER_OBJECT);

   // At this point we have a 'Texture2D' and a 'CameraNode', but they
   // are ignorant about each other's existence. The 'CameraNode'
   // doesn't know to which texture it shall render, and also doesn't
   // know what shall be rendered. Here is the point at which we say
   // what (the color buffer) and to where (to the texture 'texture')
   // the rendering should be done.  Notice that our render target
   // ('texture') is the same texture mapped to our sphere. Hence, the
   // geometry rendered to the texture will be automatically mapped to
   // the sphere.
   camera->attach (osg::CameraNode::COLOR_BUFFER, texture.get());

   // Finally, we also add the 'CameraNode' to the scene graph.
   root->addChild (camera.get());

   // Set scene graph root
   viewer.setSceneData (root.get());

   // Enter rendering loop
   viewer.realize();

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();
}
