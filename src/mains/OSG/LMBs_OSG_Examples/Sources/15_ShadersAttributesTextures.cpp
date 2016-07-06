/******************************************************************************\
* ShadersAttributesTextures                                                    *
* This example shows how to use "custom" vertex attributes in a GLSL shader    *
* with OSG. It also shows the OSG way to access from a GLSL shader, texture    *
* data attached to geometry in OSG.                                            *
* Leandro Motta Barros                                                         *
\******************************************************************************/


#include <iostream>
#include <osg/Program>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>


// This example uses two texture units. These constants are used to identify
// each of them.
const int FIRST_TEXUNIT = 0;
const int SECOND_TEXUNIT = 1;


// This function is similar to one that appeared in previous
// examples. The biggest difference is that, for each vertex, we also
// set a "custom" attribute that will be used by the shaders.
osg::ref_ptr<osg::Geode> CreateGeometry()
{
   // Create geometry and attach two textures (with coords) to it, as
   // done in the multi-texturing example. This time we don't add
   // normals or colors, because they'll not be used in the simple
   // shaders used in this example.
   osg::ref_ptr<osg::Geode> geode (new osg::Geode());
   osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

   osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());

   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 1.0, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 1.0, 0.0, 1.0));
   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));

   geometry->setVertexArray (vertices.get());

   osg::ref_ptr<osg::Image> image1 (osgDB::readImageFile("Data/foo.png"));
   if (image1.get() == 0)
   {
      std::cerr << "Error loading 'Data/foo.png'.\n";
      exit (EXIT_FAILURE);
   }

   osg::ref_ptr<osg::Texture2D> texture1 (new osg::Texture2D);
   texture1->setImage (image1.get());

   osg::ref_ptr<osg::StateSet> stateSet (geode->getOrCreateStateSet());
   stateSet->setTextureAttributeAndModes (FIRST_TEXUNIT,
                                          texture1.get(),
                                          osg::StateAttribute::ON);

   osg::ref_ptr<osg::Image> image2 (osgDB::readImageFile("Data/bar.png"));
   if (image2.get() == 0)
   {
      std::cerr << "Error loading 'Data/bar.png'.\n";
      exit (EXIT_FAILURE);
   }

   osg::ref_ptr<osg::Texture2D> texture2 (new osg::Texture2D);
   texture2->setImage (image2.get());

   stateSet->setTextureAttributeAndModes (SECOND_TEXUNIT,
                                          texture2.get(),
                                          osg::StateAttribute::ON);

   osg::ref_ptr<osg::Vec2Array> texCoords (new osg::Vec2Array());

   texCoords->push_back (osg::Vec2 (0.0, 0.0));
   texCoords->push_back (osg::Vec2 (1.0, 0.0));
   texCoords->push_back (osg::Vec2 (1.0, 1.0));
   texCoords->push_back (osg::Vec2 (0.0, 1.0));

   geometry->setTexCoordArray (0, texCoords.get());
   geometry->setTexCoordArray (1, texCoords.get());

   // Here is the news: every vertex will have a 'float' value
   // associated to it.  So, we create a 'FloatArray' with the proper
   // values for each vertex (we are not limited to use 'float'
   // attributes; other types, like 'osg::Vec3's would be OK,
   // too. It's just a matter of creating the proper array type -- for
   // example, an 'osg::Vec3Array' instead of the 'osg::FloatArray').
   // Here is a good point to explain how these values will be used in
   // the shaders. Recall that two textures are attached to the
   // geometry. The shaders will blend these two textures, but the
   // "weight" of each one will not be constant throughout the
   // geometry. The "custom" vertex attribute will be used to set the
   // weights instead. The attribute is a number between 0 and 1,
   // let's call it 'x'. When blending the textures, the weight of the
   // first texture will be 'x', and the weight of the second one will
   // be '1-x'.
   osg::ref_ptr<osg::FloatArray> attributes (new osg::FloatArray());
   attributes->push_back (0.0f);
   attributes->push_back (0.5f);
   attributes->push_back (1.0f);
   attributes->push_back (0.5f);

   // Here we attach the custom attribute to the geometry. The
   // function 'setVertexAttribArray()' is used to accomplish this
   // task. Notice that we have to pass a parameter indicating the
   // "index" to which the attributes will be attached. If I
   // understand correctly, GLSL/OpenGL specifications allow any
   // integer. Unfortunately, some implementations (NVidia seems to be
   // an example) "reserve" some indices for the "standard" attributes
   // like normals and texture coordinates. Also, it seems that
   // indices 6 and 7 are safe to use in every implementation around,
   // so we use 6 here.
   geometry->setVertexAttribArray (osg::Drawable::ATTRIBUTE_6, attributes.get());
   geometry->setVertexAttribBinding (osg::Drawable::ATTRIBUTE_6,
                                     osg::Geometry::BIND_PER_VERTEX);

   // Back to the usual: setup a primitive set and add the geometry to the geode.
   geometry->addPrimitiveSet(
      new osg::DrawArrays (osg::PrimitiveSet::QUADS, // primitive type
                           0,                        // index of first vertex
                           vertices->size()));       // how many vertices?
   geode->addDrawable (geometry.get());

   return geode;
}



int main (int argc, char* argv[])
{
   // Create a Producer-based viewer, some geometry, and link them together
   osgProducer::Viewer viewer;

   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   osg::ref_ptr<osg::Geode> geode (CreateGeometry());

   viewer.setSceneData (geode.get());

   // Here we link the textures and the shaders. The textures will be
   // available to the shaders as a 'uniform sampler2D' variable. So,
   // the "link" is done just like in the previous example: we pass
   // the desired variable name and an integer representing the
   // texture unit with the texture that will be accessed through the
   // sampler.
   geode->getOrCreateStateSet()->addUniform(
      new osg::Uniform("firstTex", FIRST_TEXUNIT));
   geode->getOrCreateStateSet()->addUniform(
      new osg::Uniform("secondTex", SECOND_TEXUNIT));

   // Create the GLSL program instance, as we did in previous examples.
   osg::ref_ptr<osg::Program> program (new osg::Program());

   // This is new, too: we tell the vertex program that the vertex info
   // available at index 6 will be accessible to the vertex program as an
   // attribute variable named 'vertexTexRatio'.
   program->addBindAttribLocation ("vertexTexRatio", osg::Drawable::ATTRIBUTE_6);

   // As in the previous example, load both the vertex and fragment shaders.
   osg::ref_ptr<osg::Shader> vertexShader(
      osg::Shader::readShaderFile (osg::Shader::VERTEX,
                                   "Data/attributes_textures.vert"));
   program->addShader (vertexShader.get());

   osg::ref_ptr<osg::Shader> fragmentShader(
      osg::Shader::readShaderFile (osg::Shader::FRAGMENT,
                                   "Data/attributes_textures.frag"));
   program->addShader (fragmentShader.get());

   geode->getOrCreateStateSet()->setAttribute (program.get());

   // Enter rendering loop
   viewer.realize();

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   // Wait for all cull and draw threads to complete before exit.
   viewer.sync();
}
