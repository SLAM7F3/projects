/******************************************************************************\
* GeometryFromCode                                                             *
* Create some very, very simple geometry directly in C++.                      *
* Leandro Motta Barros (based on OSG official examples)                        *
\******************************************************************************/


#include <iostream>
#include <osg/Geometry>
#include <osgProducer/Viewer>


int main (int argc, char* argv[])
{
   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // A geode is a "geometry node". It is-a 'Node' and contains 'Drawable's.
   osg::ref_ptr<osg::Geode> geode (new osg::Geode());

   // 'Geometry' is-a 'Drawable'. It is a collection of vertices, normals,
   // colors, texture coordinates and so on. It is organized in "primitive
   // sets", that allow to say that, e.g., "from vertex to 0 to 8 render as
   // triangles, from 9 to 13 render as points, please". For those
   // OpenGL-inclined, think of 'Geometry' as a wrapper around vertex (and
   // normals, and texcoord) arrays and 'glDrawElements()'
   osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

   // Create and set the vertex array for the geometry object
   osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
   vertices->push_back (osg::Vec3 ( 0.7, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
   vertices->push_back (osg::Vec3 (-0.7, 0.0, 0.0));
   geometry->setVertexArray (vertices.get());

   // The same for colors. But here, also says that every vertex got its own
   // color (hence, BIND_PER_VERTEX).
   osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());
   colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
   colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
   colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
   geometry->setColorArray (colors.get());
   geometry->setColorBinding (osg::Geometry::BIND_PER_VERTEX);

   // For normals, all vertices have the same normal here (BIND_OVERALL).
   osg::ref_ptr<osg::Vec3Array> normals (new osg::Vec3Array());
   normals->push_back (osg::Vec3 (0.0f, -1.0f, 0.0f));
   geometry->setNormalArray (normals.get());
   geometry->setNormalBinding (osg::Geometry::BIND_OVERALL);

   // We have just one primitive set: all vertices, starting from index 0 shall
   // be rendered as triangles.
   geometry->addPrimitiveSet(
      new osg::DrawArrays (osg::PrimitiveSet::TRIANGLES, // how to render?
                           0,                            // index of first vertex
                           vertices->size()));           // how many vertices?
   geode->addDrawable (geometry.get());

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
