/******************************************************************************\
* HUD                                                                          *
* Head-up Display in OSG.                                                      *
* Leandro Motta Barros (based on OSG examples)                                 *
\******************************************************************************/


#include <osg/Geometry>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgProducer/Viewer>



// - CreateGeometry ------------------------------------------------------------
osg::ref_ptr<osg::Geode> CreateGeometry (bool isHUD)
{
   // This is little more than the core of the previous example encapsulated
   // in a function.

   osg::ref_ptr<osg::Geode> geode (new osg::Geode());
   osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

   osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());


   if (isHUD)
   {
      // The geometry for the HUD uses the 'x' and 'y' axes.
      vertices->push_back (osg::Vec3 (300.0, 100.0, 0.0));
      vertices->push_back (osg::Vec3 (100.0, 230.0, 0.0));
      vertices->push_back (osg::Vec3 (100.0, 100.0, 0.0));
   }
   else
   {
      vertices->push_back (osg::Vec3 ( 0.7, 0.0, 0.0));
      vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
      vertices->push_back (osg::Vec3 (-0.7, 0.0, 0.0));
   }

   geometry->setVertexArray (vertices.get());

   osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array());
   colors->push_back (osg::Vec4 (1.0f, 0.0f, 0.0f, 1.0f));
   colors->push_back (osg::Vec4 (0.0f, 1.0f, 0.0f, 1.0f));
   colors->push_back (osg::Vec4 (0.0f, 0.0f, 1.0f, 1.0f));
   geometry->setColorArray (colors.get());
   geometry->setColorBinding (osg::Geometry::BIND_PER_VERTEX);

   osg::ref_ptr<osg::Vec3Array> normals (new osg::Vec3Array());
   normals->push_back (osg::Vec3 (0.0f, -1.0f, 0.0f));
   geometry->setNormalArray (normals.get());
   geometry->setNormalBinding (osg::Geometry::BIND_OVERALL);

   geometry->addPrimitiveSet(
      new osg::DrawArrays (osg::PrimitiveSet::TRIANGLES, // how to render?
                           0,                            // index of first vertex
                           vertices->size()));           // how many vertices?
   geode->addDrawable (geometry.get());

   return geode;
}



// - main ----------------------------------------------------------------------
int main (int argc, char* argv[])
{
   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // We'll have two copies of the same geometry; one is in the HUD, the other
   // is free to move around. So, we create a 'Group', which is a 'Node' that
   // can have children.
   osg::ref_ptr<osg::Group> group (new osg::Group());

   // Here are the two copies of the same geometry.
   osg::ref_ptr<osg::Geode> geometryMoveable = CreateGeometry (false);
   osg::ref_ptr<osg::Geode> geometryHUD = CreateGeometry (true);

   // Now, let's setup 'geometryHUD' as real HUD.
   osg::ref_ptr<osg::StateSet> stateSet = geometryHUD->getOrCreateStateSet();

   // First, disable lightning and depth test
   stateSet->setMode (GL_LIGHTING, osg::StateAttribute::OFF);
   stateSet->setMode (GL_DEPTH_TEST, osg::StateAttribute::OFF);

   // Then assign the HUD to a "high" render bin (in this case,
   // 11). Render bins are rendered in order, so this one will be
   // rendered after everything else (hence, the HUD gets rendered
   // over everything).
   stateSet->setRenderBinDetails (11, "RenderBin");

   // The HUD must be in an absolute reference system. So, the HUD will be
   // attached to this transform
   osg::ref_ptr<osg::MatrixTransform> modelviewAbs (new osg::MatrixTransform);
   modelviewAbs->setReferenceFrame (osg::Transform::ABSOLUTE_RF);
   modelviewAbs->setMatrix (osg::Matrix::identity());

   // Attach the geometry to the transform we just created
   modelviewAbs->addChild (geometryHUD.get());

   // We'll also want the HUD to use an orthogonal projection. Notice that
   // I'm using some hardcoded constants here, instead of getting the actual
   // window dimensions. (Curiously, I don't use any monitor configured to work
   // at 1280x1024. I have no idea why I used those numbers when creating this
   // example...)
   osg::ref_ptr<osg::Projection> projection (new osg::Projection());
   projection->setMatrix (osg::Matrix::ortho2D (0, 1280, 0, 1024));
   projection->addChild (modelviewAbs.get());

   // Add the moveable geometry and the HUD as children of the group
   group->addChild (geometryMoveable.get());
   group->addChild (projection.get());

   // The viewer views both geometries, that is, the group.
   viewer.setSceneData (group.get());

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
