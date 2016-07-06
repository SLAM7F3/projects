/******************************************************************************\
* TextAndHUD                                                                   *
* Using text in OSG (and, again, a HUD).                                       *
* Leandro Motta Barros (based on OSG examples)                                 *
\******************************************************************************/


#include <osg/Geometry>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgProducer/Viewer>
#include <osgText/Text>


// - CreateHUD -----------------------------------------------------------------
osg::ref_ptr<osg::Projection> CreateHUD (osg::ref_ptr<osg::Geode> hudGeometry,
                                         int width, int height,
                                         int binNumber = 11,
                                         const char* binName = "RenderBin")
{
   osg::ref_ptr<osg::StateSet> stateSet = hudGeometry->getOrCreateStateSet();
   stateSet->setMode (GL_LIGHTING, osg::StateAttribute::OFF);
   stateSet->setMode (GL_DEPTH_TEST, osg::StateAttribute::OFF);
   stateSet->setRenderBinDetails (binNumber, binName);

   osg::ref_ptr<osg::MatrixTransform> modelviewAbs (new osg::MatrixTransform);
   modelviewAbs->setReferenceFrame (osg::Transform::ABSOLUTE_RF);
   modelviewAbs->setMatrix (osg::Matrix::identity());

   modelviewAbs->addChild (hudGeometry.get());

   osg::ref_ptr<osg::Projection> projection (new osg::Projection());
   projection->setMatrix (osg::Matrix::ortho2D (0, width, 0, height));
   projection->addChild (modelviewAbs.get());

   return projection;
}



// - CreateGeometry ------------------------------------------------------------
osg::ref_ptr<osg::Geode> CreateGeometry()
{
   osg::ref_ptr<osg::Geode> geode (new osg::Geode());
   osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());

   osg::ref_ptr<osg::Vec3Array> vertices (new osg::Vec3Array());
   vertices->push_back (osg::Vec3 ( 0.7, 0.0, 0.0));
   vertices->push_back (osg::Vec3 ( 0.0, 0.0, 1.0));
   vertices->push_back (osg::Vec3 (-0.7, 0.0, 0.0));

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

   // Create a group and add the geometry to it
   osg::ref_ptr<osg::Group> group (new osg::Group());
   group->addChild (CreateGeometry().get());

   // Create text to be used as HUD. 'Text's are 'Drawable's, added to a 'Geode'.
   osg::ref_ptr<osg::Geode> geode(new osg::Geode());

   osg::ref_ptr<osgText::Text> text (new osgText::Text);
   text->setText ("Hooray! Does this thing\naccept àçênts, töó?");
   text->setFont ("Data/bluehigl.ttf");
   text->setPosition (osg::Vec3 (10.0f, 600.0f, 0.0f));
   text->setCharacterSize (50.0);

   geode->addDrawable (text.get());

   group->addChild (CreateHUD (geode, 1440, 900).get());

   // The group is the data to be viewed
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
