/******************************************************************************\
* ParticleSystem                                                               *
* A simple particle system example.                                            *
* Leandro Motta Barros (but this is based on the particle system examples from *
* the standard set of examples in OSG.                                         *
\******************************************************************************/

// Well, the "standard" particle examples that are distributed with OSG are so
// well documented that this example is not really useful. I created just to get
// a first contact with particle systems in OSG, so I added it to my examples,
// too. But, please, check the standard examples for a good (and reliable!)
// description of the features used here (and features not used here).

#include <osgProducer/Viewer>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>


int main (int argc, char* argv[])
{
   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Create the node to be used as the root of our scene graph
   osg::ref_ptr<osg::Group> root (new osg::Group());

   // Create the particle system and set its default attributes. The parameters
   // for 'osgParticle::ParticleSystem::setDefaultAttributes()' are (1) the
   // texture to be mapped to the particles, (2) a flag indicating whether the
   // particles "emit light", and (3) a flag indicating whether the particles
   // shall be shaded or not.
   osg::ref_ptr<osgParticle::ParticleSystem> ps(
      new osgParticle::ParticleSystem());
   ps->setDefaultAttributes ("Data/foo.png", false, true);

   // Particles are emitted by emitters. The 'ModularEmitter' used here is
   // composed of three objects: (1) a particle placer, that defines the
   // starting position of a newly created particle, (2) a particle counter,
   // that decides how many particles create at a given time, and (3) a particle
   // shooter, that sets the staring velocity of new particles.
   // Just after creating the emitter, we "connect" the particle system to it by
   // calling 'osgParticle::Emitter::setParticleSystem()'.
   osg::ref_ptr<osgParticle::ModularEmitter> emitter(
      new osgParticle::ModularEmitter());
   emitter->setParticleSystem (ps.get());

   // By default, the 'ModularEmitter' uses a 'RandomRateCounter' as counter.
   // This counter creates particles at a random rate (random, but limited to a
   // given interval). Here, we just take a pointer to the counter and set its
   // rate to something between 3 and 5 particles per second.
   osg::ref_ptr<osgParticle::RandomRateCounter> rrc(
      static_cast<osgParticle::RandomRateCounter*>(emitter->getCounter()));
   rrc->setRateRange (3, 5);

   // We add the particle emitter to the scene graph. This way, the particles
   // will be automatically created during the cull traversal.
   root->addChild (emitter.get());

   // The particle system is an 'osg::Drawable'. So, we attach to the scene
   // graph (through a 'Geode', as usual).
   osg::ref_ptr<osg::Geode> geode (new osg::Geode());
   geode->addDrawable (ps.get());
   root->addChild (geode.get());

   // Finally, we want that our particles are automatically updated (their
   // positions and lifetime, for example). That's the purpose of a
   // 'ParticleSystemUpdater'. So, we create of those, "attach" the particle
   // system to it and add the updater to the scene graph. Things will be
   // automatically updated during the culling traversals.
   osg::ref_ptr<osgParticle::ParticleSystemUpdater> psu(
      new osgParticle::ParticleSystemUpdater());
   psu->addParticleSystem (ps.get());
   root->addChild (psu.get());

   // Set scene graph root
   viewer.setSceneData (root.get());

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
