/******************************************************************************\
* Multitexturing                                                               *
* Using multitexturing in geometry created by C++ code.                        *
* Leandro Motta Barros (somewhat based on Terse Solutions tutorials)           *
\******************************************************************************/


#include <iostream>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgProducer/Viewer>
#include <osgDB/ReadFile>



// - CreateGeometry ------------------------------------------------------------
osg::ref_ptr<osg::Geode> CreateGeometry ()
{
   osg::ref_ptr<osg::Geode> geode (new osg::Geode());
   osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

   osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());

   vertices->push_back (osg::Vec3 ( 0.7, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
   vertices->push_back (osg::Vec3 (-0.7, 0.0, 0.0));

   geometry->setVertexArray (vertices.get());

   // All vertices are white this time (it's hard to see that we have two
   // textures with all the colors...)
   osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());
   colors->push_back (osg::Vec4 (1.0f, 1.0f, 1.0f, 1.0f));
   geometry->setColorArray (colors.get());
   geometry->setColorBinding (osg::Geometry::BIND_OVERALL);

   osg::ref_ptr<osg::Vec3Array> normals (new osg::Vec3Array());
   normals->push_back (osg::Vec3 (0.0f, -1.0f, 0.0f));
   geometry->setNormalArray (normals.get());
   geometry->setNormalBinding (osg::Geometry::BIND_OVERALL);

   // Load texture from file and map it to the triangle
   osg::ref_ptr<osg::Image> image1 (osgDB::readImageFile("Data/foo.png"));
   if (image1.get() == 0)
   {
      std::cerr << "Error loading 'Data/foo.png'.\n";
      exit (EXIT_FAILURE);
   }

   osg::ref_ptr<osg::Texture2D> texture1 (new osg::Texture2D);
   texture1->setImage (image1.get());

   osg::ref_ptr<osg::StateSet> stateSet (geode->getOrCreateStateSet());
   stateSet->setTextureAttributeAndModes (0,   // unit
                                          texture1.get(),
                                          osg::StateAttribute::ON);

   // And do the same for the second "texture unit".
   osg::ref_ptr<osg::Image> image2 (osgDB::readImageFile("Data/bar.png"));
   if (image2.get() == 0)
   {
      std::cerr << "Error loading 'Data/bar.png'.\n";
      exit (EXIT_FAILURE);
   }

   osg::ref_ptr<osg::Texture2D> texture2 (new osg::Texture2D);
   texture2->setImage (image2.get());

   stateSet->setTextureAttributeAndModes (1,   // unit
                                          texture2.get(),
                                          osg::StateAttribute::ON);

   osg::ref_ptr<osg::Vec2Array> texCoords (new osg::Vec2Array());

   texCoords->push_back (osg::Vec2 (1.0, 0.0));
   texCoords->push_back (osg::Vec2 (0.5, 1.0));
   texCoords->push_back (osg::Vec2 (0.0, 0.0));

   // Here, the two texture units (0 and 1) share the same texture coordinates.
   geometry->setTexCoordArray (0, texCoords.get());
   geometry->setTexCoordArray (1, texCoords.get());

   // Back to the usual: setup a primitive set and add the geometry to the geode.
   geometry->addPrimitiveSet(
      new osg::DrawArrays (osg::PrimitiveSet::TRIANGLES, // how to render?
                           0,                            // index of first vertex
                           vertices->size()));           // how many vertices?
   geode->addDrawable (geometry.get());

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
