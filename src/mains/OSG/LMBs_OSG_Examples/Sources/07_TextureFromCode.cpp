/******************************************************************************\
* TextureFromCode                                                              *
* Texture a polygon directly in C++.                                           *
* Leandro Motta Barros (based on Terse Solutions tutorials)                    *
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


   vertices->push_back (osg::Vec3 ( 0, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 1, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 1, 1.0, 0.0));
   vertices->push_back (osg::Vec3 ( 0, 1.0, 0.0));
/*
   vertices->push_back (osg::Vec3 ( 0.7, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
   vertices->push_back (osg::Vec3 (-0.7, 0.0, 0.0));
*/

   geometry->setVertexArray (vertices.get());

   osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());

   for (int i=0; i<vertices->size(); i++)
   {
      colors->push_back (osg::Vec4 (1.0f, 1.0f, 1.0f, 1.0f));
   }
   

//   colors->push_back (osg::Vec4 (1.0f, 1.0f, 1.0f, 1.0f));
//   colors->push_back (osg::Vec4 (1.0f, 1.0f, 1.0f, 1.0f));
   geometry->setColorArray (colors.get());
   geometry->setColorBinding (osg::Geometry::BIND_PER_VERTEX);

   osg::ref_ptr<osg::Vec3Array> normals (new osg::Vec3Array());
   normals->push_back (osg::Vec3 (0.0f, -1.0f, 0.0f));
   geometry->setNormalArray (normals.get());
   geometry->setNormalBinding (osg::Geometry::BIND_OVERALL);

   // This is new: load texture from file and map it to the triangle
   osg::ref_ptr<osg::Image> image (osgDB::readImageFile("Data/foo.png"));
   if (image.get() == 0)
   {
      std::cerr << "Error loading 'Data/foo.png'.\n";
      exit (EXIT_FAILURE);
   }

   osg::ref_ptr<osg::Texture2D> texture (new osg::Texture2D);
   texture->setImage (image.get());

   osg::ref_ptr<osg::StateSet> stateSet (geode->getOrCreateStateSet());
   stateSet->setTextureAttributeAndModes (0, texture.get(),
                                          osg::StateAttribute::ON);

   osg::ref_ptr<osg::Vec2Array> texCoords (new osg::Vec2Array());

   texCoords->push_back (osg::Vec2 (0,0));
   texCoords->push_back (osg::Vec2 (1,0));
   texCoords->push_back (osg::Vec2 (1,1));
   texCoords->push_back (osg::Vec2 (0,1));

//   texCoords->push_back (osg::Vec2 (1.0, 0.2));
//   texCoords->push_back (osg::Vec2 (0.7, 0.7));
//   texCoords->push_back (osg::Vec2 (0.3, 0.3));
//   texCoords->push_back (osg::Vec2 (0.0, 0.1));

   geometry->setTexCoordArray (0,                // Texture unit (for multi-texturing?)
                               texCoords.get()); // The coordinates vector
   // Binding is always per vertex, I guess. Anything different than this does
   // not make sense for me.


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
