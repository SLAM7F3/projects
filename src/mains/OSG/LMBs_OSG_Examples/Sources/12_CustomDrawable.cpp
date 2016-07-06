/******************************************************************************\
* CustomDrawable                                                               *
* How to implement a custom drawable.                                          *
* Leandro Motta Barros                                                         *
\******************************************************************************/


#include <osg/Drawable>
#include <osg/ShapeDrawable>
#include <osg/Group>
#include <osg/Material>
#include <osgProducer/Viewer>


// Just create a drawable sphere.
osg::ref_ptr<osg::Drawable> CreateSphere (double x, double y, double z,
                                          double radius)
{
   osg::ref_ptr<osg::Sphere> sphere (new osg::Sphere (osg::Vec3(x, y, z), radius));
   osg::ref_ptr<osg::Drawable> sphereShapeDrawable(
      new osg::ShapeDrawable (sphere.get()));
   return sphereShapeDrawable;
}


// Our custom 'Drawable' class. This is a very simple one: it just draws two
// blue triangles.
class CustomDrawable: public osg::Drawable
{
   public:

      // The constructor here does nothing. One thing that may be
      // necessary is disabling display lists. This can be done by
      // calling setSupportsDisplayList (false); Display lists should
      // be disabled for 'Drawable's that can change over time (that
      // is, the vertices drawn change from time to time).
      CustomDrawable()
      {
         // This contructor intentionally left blank. Duh.
      }

      // I can't say much about the methods below, but OSG seems to expect
      // that we implement them.
      CustomDrawable (const CustomDrawable& pg,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
      { }

      virtual osg::Object* cloneType() const { return new CustomDrawable(); }

      virtual osg::Object* clone(const osg::CopyOp& copyop) const
      { return new CustomDrawable (*this, copyop); }

      // Real work is done here. THERE IS A VERY IMPORTANT THING TO
      // NOTE HERE: the 'drawImplementation()' method receives an
      // 'osg::State' as parameter. This can be used to change the
      // OpenGL state, but changing the OpenGL state here is something
      // to be avoided as much as possible.  Do this *only* if it is
      // *absolutely* necessary to make your rendering algorithm
      // work. The "right" (most efficient and flexible) way to change
      // the OpenGL state in OSG is by attaching 'StateSet's to
      // 'Node's and 'Drawable's.  That said, the example below shows
      // how to change the OpenGL state in these rare cases in which
      // it is necessary. But always keep in mind: *Change the OpenGL
      // state only if strictly necessary*.
      virtual void drawImplementation (osg::State& state) const
      {
         // Create a 'StateSet' with a "blue diffuse color" attribute.
         osg::ref_ptr<osg::Material> blue (new osg::Material);
         blue->setDiffuse (osg::Material::FRONT_AND_BACK,
                           osg::Vec4(0.0, 0.0, 1.0, 1.0));

         // Change the OpenGL state so that it uses the blue material created
         // above. From now on, everything will be drawn using this new (blue)
         // OpenGL state.
         state.applyAttribute (blue.get());

         // Draw the geometry, using standard OpenGL calls. Note that
         // the 'State' class has methods like 'setVertexPointer()',
         // that can be used when working with vertex arrays (they may
         // be faster than using the OpenGL calls directly).  Almost
         // always, the implementation of 'drawImplementation()' will
         // contain just this. (Did I mention that is better to keep
         // the OpenGL state untouched here?)
         glBegin(GL_TRIANGLES);
         glNormal3f (0.0, 1.0, 0.0);
         glVertex3f ( 0.0, 0.25,  0.7);
         glVertex3f (-1.0, 0.25, -1.0);
         glVertex3f ( 1.0, 0.25, -1.0);

         glNormal3f (0.0, -1.0, 0.0);
         glVertex3f ( 0.0, -0.25,  0.7);
         glVertex3f (-1.0, -0.25, -1.0);
         glVertex3f ( 1.0, -0.25, -1.0);
         glEnd();

         // Since we changed the OpenGL state when calling
         // 'state.applyAttribute()' above, we restore it here. On the
         // OSG mailing list, Robert Osfield himself said that this
         // shouldn't be necessary, but is not a bad idea either
         // ("Being conservative adding an state.apply() would be no
         // bad thing, but its probably not required").
         state.apply();
      }
};



int main (int argc, char* argv[])
{
   // Create and setup a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Create the scene graph. First, add two red spheres.
   osg::ref_ptr<osg::Geode> sgRoot (new osg::Geode);

   osg::ref_ptr<osg::Material> red (new osg::Material);
   red->setDiffuse (osg::Material::FRONT_AND_BACK,
                    osg::Vec4 (1.0, 0.0, 0.0, 1.0));
   osg::ref_ptr<osg::StateSet> ss (new osg::StateSet);
   ss->setAttribute (red.get());

   sgRoot->setStateSet (ss.get());
   sgRoot->addDrawable (CreateSphere (-1.0, 0.0, 0.0, 0.4).get());
   sgRoot->addDrawable (CreateSphere (1.0, 0.0, 0.0, 0.4).get());

   // Finally, add an instance of our wonderful 'CustomDrawable' to the
   // scene graph.
   osg::ref_ptr<CustomDrawable> drawable (new CustomDrawable);
   sgRoot->addDrawable (drawable.get());

   // Set the data to the viewer
   viewer.setSceneData (sgRoot.get());

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
