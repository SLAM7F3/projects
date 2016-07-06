/******************************************************************************\
* Picking                                                                      *
* Picking in OSG.                                                              *
* Leandro Motta Barros (based on OSG official examples)                        *
\******************************************************************************/


#include <sstream>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgProducer/Viewer>
#include <osgText/Text>


// - operator<< ----------------------------------------------------------------
std::ostream& operator<< (std::ostream& lhs, const osg::Vec3d& rhs)
{
   lhs << '[' << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ']';
   return lhs;
}


// - CreateHUD -----------------------------------------------------------------
osg::ref_ptr<osg::Projection> CreateHUD (osg::ref_ptr<osg::Node> hudGeometry,
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

   geode->setName ("The triangle");

   return geode;
}



// - CreateText ----------------------------------------------------------------
osg::ref_ptr<osg::Group> CreateText()
{
   osg::ref_ptr<osg::Geode> geode1 (new osg::Geode());
   osg::ref_ptr<osgText::Text> text1 (new osgText::Text);
   text1->setText ("Hove the mouse over the things...");
   text1->setFont ("Data/bluehigl.ttf");
   text1->setPosition (osg::Vec3 (10.0f, 600.0f, 0.0f));
   text1->setCharacterSize (50.0);
   geode1->addDrawable (text1.get());
   geode1->setName("Instructions");

   osg::ref_ptr<osg::Geode> geode2 (new osg::Geode());
   osg::ref_ptr<osgText::Text> text2 (new osgText::Text);
   text2->setText ("Nothing is being picked.");
   text2->setFont ("Data/bluehigl.ttf");
   text2->setPosition (osg::Vec3 (100.0f, 400.0f, 0.0f));
   text2->setCharacterSize (25.0);
   geode2->addDrawable (text2.get());
   geode2->setName("Selection indicator");

   osg::ref_ptr<osg::Group> group (new osg::Group());

   group->addChild (geode1.get());
   group->addChild (geode2.get());

   return group;
}



// - PickHandler ---------------------------------------------------------------
class PickHandler: public osgGA::GUIEventHandler
{
   public:
      PickHandler (osgProducer::Viewer* viewer,
                   osg::ref_ptr<osgText::Text> updateText)
         : viewer_(viewer), updateText_(updateText)
      { }

      ~PickHandler() { }

      bool handle (const osgGA::GUIEventAdapter& ea,
                   osgGA::GUIActionAdapter& us);

      virtual void pick (const osgGA::GUIEventAdapter& ea);

      void setLabel(const std::string& name)
      {
         if (updateText_.get() != 0)
            updateText_->setText (name);
      }

   private:

      osgProducer::Viewer* viewer_;
      osg::ref_ptr<osgText::Text> updateText_;
};


bool PickHandler::handle (const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
   // pick every frame
   if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
   {
      pick (ea);
      return true;
   }

   return false;
}


void PickHandler::pick (const osgGA::GUIEventAdapter& ea)
{
    osgUtil::IntersectVisitor::HitList hlist;

    std::string gdlist;

    if (viewer_->computeIntersections (ea.getX(), ea.getY(), hlist))
    {
       for (osgUtil::IntersectVisitor::HitList::iterator p = hlist.begin();
            p != hlist.end();
            ++p)
       {
          std::ostringstream os;
          if (p->_geode.valid() && !p->_geode->getName().empty())
          {
             // the geodes are identified by name.
             os << "Geode name: \"" << p->_geode->getName() << "\"" << std::endl;
          }
          if (p->_drawable.valid())
          {
             os << "Drawable class name: \"" << p->_drawable->className() << "\"" << std::endl;
          }

          os << "        local coords vertex(" << p->getLocalIntersectPoint()
             << ")" << "  normal(" << p->getLocalIntersectNormal() << ")"
             << std::endl;
          os << "        world coords vertex(" << p->getWorldIntersectPoint()
             << ")" << "  normal(" << p->getWorldIntersectNormal() << ")"
             << std::endl;

          osgUtil::Hit::VecIndexList& vil = p->_vecIndexList;
          for (unsigned int i = 0; i < vil.size(); ++i)
          {
             os << "        vertex indices [" << i << "] = " << vil[i] << std::endl;
          }

          gdlist += os.str();
       }
    }
    else
       gdlist = "Nothing is being picked.";

    setLabel(gdlist);
}



// - main ----------------------------------------------------------------------
int main (int argc, char* argv[])
{
   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Create a group and add the geometry and the HUD to it
   osg::ref_ptr<osg::Group> group (new osg::Group());
   group->addChild (CreateGeometry().get());

   osg::ref_ptr<osg::Group> textGroup (CreateText());

   group->addChild (CreateHUD (textGroup.get(), 1440, 900).get());

   // Create and setup the pick handler
   osg::ref_ptr<osgText::Text> text(
      dynamic_cast<osgText::Text*>(
         dynamic_cast<osg::Geode*>(textGroup->getChild(1))->getDrawable(0)));

   osg::ref_ptr<PickHandler> ph (new PickHandler (&viewer, text));

   // When adding the event handler I 'push_back()' it, so that the
   // event handler used by the standard Producer camera control gets
   // higher priority than my humble picker.
   viewer.getEventHandlerList().push_back (ph.get());

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
