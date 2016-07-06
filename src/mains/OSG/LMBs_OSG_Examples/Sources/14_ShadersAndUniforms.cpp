/******************************************************************************\
* ShadersAndUniforms                                                           *
* This example shows how to use a vertex and a pixel shader simultaneously.    *
* Furthermore, it shows the OSG way to pass "uniforms" to the shaders.         *
* Leandro Motta Barros                                                         *
\******************************************************************************/


#include <osg/Program>
#include <osg/ShapeDrawable>
#include <osg/Timer>
#include <osgProducer/Viewer>


// This simply creates and returns a geode with a drawable sphere attached to
// it. Notice that the sphere is centered at the origin and has radius equal
// to 1. This way, none of the components of a vertex position will have an
// absolute value larger than 1. This is important, because the vertex shader
// will set the vertex color as the absolute value of the vertex position.
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

   // Set scene graph root (the geode, since it is the only 'osg::Node' around)
   viewer.setSceneData (geode.get());

   // Like the previous example, we create an 'osg:Program'. But unlike the
   // previous example we now create two shaders and attach them both to the
   // program. Not a big deal.
   osg::ref_ptr<osg::Program> program (new osg::Program());

   osg::ref_ptr<osg::Shader> vertexShader(
      osg::Shader::readShaderFile (osg::Shader::VERTEX, "Data/uniform.vert"));
   program->addShader (vertexShader.get());

   osg::ref_ptr<osg::Shader> fragmentShader(
      osg::Shader::readShaderFile (osg::Shader::FRAGMENT, "Data/uniform.frag"));
   program->addShader (fragmentShader.get());

   geode->getOrCreateStateSet()->setAttribute (program.get());

   // Here is the really new stuff. Our shaders will now perform some
   // simple animation, but in order to be able to do this, they must
   // know what time is it. This time parameter will be passed from
   // the C++ program to the GLSL program as an uniform named "time".
   // In OSG, uniforms are instances of the class 'osg::Uniform', and
   // they are attached directly to 'StateSet's.  And here we go: we
   // create an 'Uniform' and add it to our scene's 'StateSet'. Notice
   // that our uniform will be of type float, but the 'Uniform'
   // constructor has overloads for other types, like 'int', 'bool'
   // and 'osg::Vec3'.  A final note: There is an interesting feature
   // in OSG I was not aware when creating this example. OSG
   // automatically sets some uniform variables, which can be readily
   // declared and used in shaders. Among them, there is an
   // 'osg_FrameTime' variable, which does exactly the same thing as
   // the 'time' variable we're defining here. Anyway, the example
   // shown here still shows how to pass uniform variables from OSG to
   // GLSL.
   geode->getOrCreateStateSet()->addUniform (new osg::Uniform ("time", 0.0f));

   // Enter rendering loop
   const osg::Timer_t beginOfTime = osg::Timer::instance()->tick();

   viewer.realize();

   while (!viewer.done())
   {
      // Our 'time' uniform variable will hold the number of seconds elapsed
      // since the program started to run. Hence, it must be updated every
      // frame. That's what we do here.
      // Again, notice that the 'osg::Uniform::set()' method has overloads for
      // several different data types, not only the 'float' we are using here.
      const osg::Timer_t now = osg::Timer::instance()->tick();
      const float time = osg::Timer::instance()->delta_s (beginOfTime, now);
      geode->getOrCreateStateSet()->getUniform ("time")->set (time);

      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();
}
