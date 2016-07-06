/******************************************************************************\
* StateSets                                                                    *
* Understanding how 'StateSet's work in a scene graph.                         *
* Leandro Motta Barros                                                         *
\******************************************************************************/


// Warning: This example is quite chaotic. It should be cleaned-up and
//          better commented. Good luck.


#include <osg/Group>
#include <osg/Material>
#include <osg/PolygonMode>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgProducer/Viewer>
#include <osgDB/ReadFile>


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
   // Create and setup a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Create the scene graph

   // root (node A). Just a group.
   osg::ref_ptr<osg::Group> nodeA (new osg::Group());

   // node B, a Geode with a Drawable. No 'StateSet's.
   osg::ref_ptr<osg::Geode> nodeB (new osg::Geode());
   osg::ref_ptr<osg::Drawable> d1 (CreateSphere (0.0, 0.0, 0.0, 0.4));
   nodeB->addDrawable (d1.get());
   nodeA->addChild (nodeB.get());

   // node C. a Geode with a "use-blue-color" 'StateSet'
   osg::ref_ptr<osg::Geode> nodeC (new osg::Geode());
   osg::ref_ptr<osg::StateSet> ssC (new osg::StateSet());
   osg::ref_ptr<osg::Material> blue (new osg::Material);
   blue->setDiffuse (osg::Material::FRONT_AND_BACK,
                     osg::Vec4(0.0, 0.0, 1.0, 1.0));
   ssC->setAttribute (blue.get());
   nodeC->setStateSet (ssC.get());
   nodeA->addChild (nodeC.get());

   // now, add 'Drawable's to node C. The first does not have a 'StateSet'; the
   // others have, using green color with different 'StateAttribute's.
   osg::ref_ptr<osg::Drawable> d2 (CreateSphere (0.0, 0.0, -1.0, 0.4));
   nodeC->addDrawable (d2.get());

   osg::ref_ptr<osg::Material> green (new osg::Material);
   green->setDiffuse (osg::Material::FRONT_AND_BACK,
                     osg::Vec4(0.0, 1.0, 0.0, 1.0));

   osg::ref_ptr<osg::Drawable> d3 (CreateSphere (1.0, 0.0, -1.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss3 (new osg::StateSet());
   ss3->setAttribute (green.get(), osg::StateAttribute::OFF);
   d3->setStateSet (ss3.get());
   nodeC->addDrawable (d3.get());

   osg::ref_ptr<osg::Drawable> d4 (CreateSphere (2.0, 0.0, -1.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss4 (new osg::StateSet());
   ss4->setAttribute (green.get(), osg::StateAttribute::ON);
   d4->setStateSet (ss4.get());
   nodeC->addDrawable (d4.get());

   osg::ref_ptr<osg::Drawable> d5 (CreateSphere (3.0, 0.0, -1.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss5 (new osg::StateSet());
   ss5->setAttribute (green.get(), osg::StateAttribute::OVERRIDE);
   d5->setStateSet (ss5.get());
   nodeC->addDrawable (d5.get());

   osg::ref_ptr<osg::Drawable> d6 (CreateSphere (4.0, 0.0, -1.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss6 (new osg::StateSet());
   ss6->setAttribute (green.get(), osg::StateAttribute::PROTECTED);
   d6->setStateSet (ss6.get());
   nodeC->addDrawable (d6.get());

   osg::ref_ptr<osg::Drawable> d7 (CreateSphere (5.0, 0.0, -1.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss7 (new osg::StateSet());
   ss7->setAttribute (green.get(), osg::StateAttribute::INHERIT);
   d7->setStateSet (ss7.get());
   nodeC->addDrawable (d7.get());

   // node D, a Geode with a "use-red-color" 'StateSet'
   osg::ref_ptr<osg::Geode> nodeD (new osg::Geode());
   osg::ref_ptr<osg::StateSet> ssD (new osg::StateSet());
   osg::ref_ptr<osg::Material> red (new osg::Material);
   red->setDiffuse (osg::Material::FRONT_AND_BACK,
                    osg::Vec4(1.0, 0.0, 0.0, 1.0));
   ssD->setAttribute (red.get());
   nodeD->setStateSet (ssD.get());
   nodeA->addChild (nodeD.get());

   // now, add 'Drawable's to node D. The first does not have a 'StateSet'; the
   // second changes the polygon mode to LINE.
   osg::ref_ptr<osg::Drawable> d8 (CreateSphere (0.0, 0.0, -2.0, 0.4));
   nodeD->addDrawable (d8.get());

   osg::ref_ptr<osg::PolygonMode> polygonLine (new osg::PolygonMode);
   polygonLine->setMode (osg::PolygonMode::FRONT_AND_BACK,
                         osg::PolygonMode::LINE);
   osg::ref_ptr<osg::Drawable> d9 (CreateSphere (1.0, 0.0, -2.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss9 (new osg::StateSet());
   ss9->setAttribute (polygonLine.get());
   d9->setStateSet (ss9.get());
   nodeD->addDrawable (d9.get());

   // node E, a Group with a "2D Texture" 'StateSet'
   osg::ref_ptr<osg::Group> nodeE (new osg::Group());
   osg::ref_ptr<osg::StateSet> ssE (new osg::StateSet());
   osg::ref_ptr<osg::Texture2D> texFoo (new osg::Texture2D());
   texFoo->setImage (osgDB::readImageFile("Data/foo.png"));
   ssE->setTextureAttributeAndModes (0, texFoo.get(), osg::StateAttribute::ON);
   nodeE->setStateSet (ssE.get());
   nodeA->addChild (nodeE.get());

   // node F, a Geode, child of E, with a 'StateSet'less 'Drawable'
   osg::ref_ptr<osg::Geode> nodeF (new osg::Geode());
   osg::ref_ptr<osg::Drawable> d10 (CreateSphere (0.0, 0.0, -3.0, 0.4));
   nodeF->addDrawable (d10.get());
   nodeE->addChild (nodeF.get());

   // Node G, a 'StateSet'less Geode, child of E
   osg::ref_ptr<osg::Geode> nodeG (new osg::Geode());
   nodeE->addChild (nodeG.get());

   // Now, add 'Drawable's to G. Try assorted tricks.

   osg::ref_ptr<osg::Drawable> d11 (CreateSphere (1.0, 0.0, -3.0, 0.4));
   d11->setStateSet (ss9.get()); // shared 'StateSet' for "Polygon Mode LINES"
   nodeG->addDrawable (d11.get());

   osg::ref_ptr<osg::Drawable> d12 (CreateSphere (2.0, 0.0, -3.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss12 (new osg::StateSet());
   osg::ref_ptr<osg::Texture2D> texNone (new osg::Texture2D());
   ss12->setTextureAttributeAndModes (0, texNone.get(),
                                      osg::StateAttribute::OFF);
   d12->setStateSet (ss12.get());
   nodeG->addDrawable (d12.get());

   osg::ref_ptr<osg::Drawable> d13 (CreateSphere (3.0, 0.0, -3.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss13 (new osg::StateSet());
   osg::ref_ptr<osg::Texture2D> texBar(
      new osg::Texture2D(osgDB::readImageFile("Data/bar.png")));
   ss13->setTextureAttributeAndModes (0, texBar.get(),
                                      osg::StateAttribute::ON);
   d13->setStateSet (ss13.get());
   nodeG->addDrawable (d13.get());

   osg::ref_ptr<osg::Drawable> d14 (CreateSphere (4.0, 0.0, -3.0, 0.4));
   osg::ref_ptr<osg::StateSet> ss14 (new osg::StateSet());
//    osg::ref_ptr<osg::Texture2D> texBar(
//       new osg::Texture2D(osgDB::readImageFile("Data/bar.png")));
   ss14->setTextureAttributeAndModes (1, texBar.get(),
                                      osg::StateAttribute::ON);
//    ss14->setTextureAttributeAndModes (0, texFoo.get(),
//                                       osg::StateAttribute::ON);
   d14->setStateSet (ss14.get());
   nodeG->addDrawable (d14.get());


   // Set the data to the viewer
   viewer.setSceneData (nodeA.get());

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
