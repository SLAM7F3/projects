/******************************************************************************\
* FragmentShader                                                               *
* Using a GLSL fragment shader. (But notice that using vertex shaders in OSG   *
* requires essentially the same steps, so this example may help in this case,  *
* too).                                                                        *
* Leandro Motta Barros                                                         *
\******************************************************************************/


#include <osg/Program>
#include <osg/ShapeDrawable>
#include <osgProducer/Viewer>


// This simply creates and returns a geode with a drawable sphere attached to
// it. Soon, we'll apply our fragment shader to it.
osg::ref_ptr<osg::Geode> CreateGeometry()
{
   osg::ref_ptr<osg::Sphere> sphere(
      new osg::Sphere (osg::Vec3(0.0, 0.0, 0.0), 1.0));

   osg::ref_ptr<osg::Drawable> sphereShapeDrawable(
      new osg::ShapeDrawable (sphere.get()));

   osg::ref_ptr<osg::Geode> geode (new osg::Geode());

   geode->addDrawable (sphereShapeDrawable.get());

   return geode;
}



int main (int argc, char* argv[])
{
   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Here's our geometry
   osg::ref_ptr<osg::Geode> geode (CreateGeometry().get());

   // Set scene graph root (the geode, since it is the only
   // 'osg::Node' around)
   viewer.setSceneData (geode.get());

   // In the GLSL, shaders are not used directly. One or more shaders
   // (possibly a combination of vertex and pixel shaders) must be
   // compiled one by one and linked together to originate a
   // "program". OSG automates most of this work, but the concept of
   // "shaders" and "programs" still exists.  A "program" is just
   // another state attribute, just like material or line width. So,
   // in OSG, programs are represented as a class derived from
   // 'StateAttribute'. More precisely, GLSL programs are represented
   // by objects of the class 'osg::Program'.
   osg::ref_ptr<osg::Program> program (new osg::Program());

   // Now we have a program, but without shaders attached to it, the program
   // will not do anything. GLSL shaders in OSG are instances of the class
   // 'osg::Shader'. For convenience, 'osg::Shader' has a static method named
   // 'readShaderFile()' that constructs a new shader object and loads its
   // source code from a file. Notice that we have to tell OSG what kind of
   // shader we are loading (in this case, a fragment shader).
   osg::ref_ptr<osg::Shader> fragmentShader(
      osg::Shader::readShaderFile (osg::Shader::FRAGMENT, "Data/simple.frag"));

   // So far, we created a program and a shader, but they are not "linked"
   // together. We must add the shader to the program.
   program->addShader (fragmentShader.get());

   // And, since the program is a state attribute, we simply have to
   // "attach" it to the 'StateSet' of our geode. Once this is done,
   // whenever this subgraph (in this case the subgraph is a single
   // node) is rendered, the fragment shader will be used for
   // rendering it.
   geode->getOrCreateStateSet()->setAttribute (program.get());

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
