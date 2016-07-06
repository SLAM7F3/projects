/******************************************************************************\
* FindingTriangles                                                             *
* Using a 'TriangleFunctor' to find the triangles of a geometry.               *
* Leandro Motta Barros (Based on sample code sent to the OSG mailing list by   *
* Christian Pick on March 21st 02005)                                          *
\******************************************************************************/

#include <iostream>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/TriangleFunctor>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>
#include <osg/io_utils>

// This example shows how to use an 'osg::TriangleVisitor' to find out
// which triangles compose the geometry of a scene. More precisely,
// we'll be drawing a line representing every triangle normal found in
// the scene.

// In some sense, this is the most important class of our example. It collects
// the triangle vertices and creates the lines representing the normals.

class NormalLinesCreationFunctor
{
   public:
      // This will store all triangle vertices.
      std::vector<osg::Vec3> vertices;

      // When triangle vertices are added to 'vertices', they'll be
      // transformed by this matrix. This is useful because
      // 'osg::TriangleFunctor' operates on the model coordinate
      // system, and we want do draw our normals in the world
      // coordinate system.
      osg::Matrix transformMatrix;

      // This will be called once for each triangle in the
      // geometry. As parameters, it takes the three triangle vertices
      // and a boolean parameter indicating if the vertices are coming
      // from a "real" vertex array or from a temporary vertex array
      // created from some other geometry representation.  The
      // implementation is quite simple: we just store the vertices
      // (transformed by 'transformMatrix') in a 'std::vector'.
      void operator() (const osg::Vec3& v1, const osg::Vec3& v2,
                       const osg::Vec3& v3, bool treatVertexDataAsTemporary)
      {
         vertices.push_back (v1 * transformMatrix);
         vertices.push_back (v2 * transformMatrix);
         vertices.push_back (v3 * transformMatrix);
      }

      // This is where we create the lines representing the triangle
      // normals. See example '03_GeometryFromCode' for a simpler example that
      // explains how 'osg::Geometry' works.
      osg::ref_ptr<osg::Geometry> getLinesRepresentingNormals()
      {
         osg::ref_ptr<osg::Geometry> geometry (new osg::Geometry());
         osg::ref_ptr<osg::Vec3Array> normalVertices (new osg::Vec3Array());

         for (unsigned i = 0; i < vertices.size(); i += 3)
         {
            const osg::Vec3 v1 = vertices[i+1] - vertices[i];
            const osg::Vec3 v2 = vertices[i+2] - vertices[i];

            osg::Vec3 normal = v1 ^ v2;
            normal.normalize();

            const osg::Vec3 center =
               (vertices[i] + vertices[i+1] + vertices[i+2]) / 3.0;

            const osg::Vec3 tip = center + normal * v1.length();

            normalVertices->push_back (center);
            normalVertices->push_back (tip);
         }

         geometry->setVertexArray (normalVertices.get());
         geometry->addPrimitiveSet(
            new osg::DrawArrays (osg::PrimitiveSet::LINES, 0,
                                 normalVertices->size()));

         return geometry;
      }
};

// The class 'NormalLinesCreationFunctor' defined above would be
// almost all we need if we were interested in a single
// 'osg::Drawable' only. But no! We want whole scene, possibly
// composed of many 'osg::Geode's with many 'osg::Drawable's attached
// to each one. So, we create a 'NodeVisitor' that will traverse the
// scene graph using an 'osg::TriangleVisitor' and our just defined
// 'NormalLinesCreationFunctor' to get all scene triangles and create
// lines representing their normals.  Check example '11_NodeVisitor'
// for an introduction on 'osg::NodeVisitor'.

class SceneTriangleNormalsCreationVisitor: public osg::NodeVisitor
{
   private:

      // We'll be creating the lines representing the triangle normals
      // for (possibly) several 'Drawable's as we traverse the scene
      // graph. In order to do this, we'll use the
      // 'osg::TriangleFunctor' and our
      // 'NormalLinesCreationFunctor'. 'osg::TriangleFunctor' will do
      // its best to transform the geometry in triangles, and, for
      // each triangle found, it will call
      // 'NormalLinesCreationFunctor::operator()'.
      osg::TriangleFunctor<NormalLinesCreationFunctor> triangleFunctor_;

   public:
      // This geode will store the geometry representing the triangle normals.
      osg::ref_ptr<osg::Geode> normalsGeode;


      SceneTriangleNormalsCreationVisitor()
         : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), normalsGeode (
            new osg::Geode())
      { }


      // When visiting an 'osg::Geode', we use our 'triangleFunctor_'
      // defined above to do the job we want.  So, this is how we
      // actually we use a 'TriangleVisitor': we call the 'Geode's
      // accept method, passing our 'TriangleVisitor' as parameter. In
      // some sense, the 'Geode' will pretend it is drawing itself,
      // and will call our 'NormalLinesCreationFunctor::operator()'
      // for each triangle.
      void SceneTriangleNormalsCreationVisitor::apply (osg::Geode& geode)
      {
         for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
         {
            geode.getDrawable(i)->accept (triangleFunctor_);
            normalsGeode->addDrawable(
               triangleFunctor_.getLinesRepresentingNormals().get());
         }

         //osg::NodeVisitor::apply (geode);
      }

      // Accumulate transforms as we traverse the scene graph, and set
      // the 'transformMatrix' member of our 'triangleFunctor_'
      // accordingly. This way, the 'TriangleFunctor' will generate
      // triangles in the world coordinate system, not on the model
      // coordinate system.
      void SceneTriangleNormalsCreationVisitor::apply (
         osg::Transform& transform)
      {
         const osg::MatrixTransform* matrixTransform =
            transform.asMatrixTransform();

         if (matrixTransform != 0)
         {
            osg::Matrix matOldMatrix = triangleFunctor_.transformMatrix;
            triangleFunctor_.transformMatrix.preMult(
               matrixTransform->getMatrix());

            traverse (transform);
            triangleFunctor_.transformMatrix = matOldMatrix;
         }
      }
};



int main (int argc, char* argv[])
{
   // Check command-line parameters
   if (argc != 2)
   {
      std::cerr << "Usage: " << argv[0] << " <model file>\n";
      exit (1);
   }

   // Create a Producer-based viewer
   osgProducer::Viewer viewer;
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Load the model
   osg::ref_ptr<osg::Group> sgRoot (new osg::Group());
   osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);

   if (!loadedModel)
   {
      std::cerr << "Problem opening '" << argv[1] << "'\n";
      exit (1);
   }
   sgRoot->addChild (loadedModel.get());

   // Traverse the loaded model, creating the lines that represent the
   // triangle normals and add these lines to the scene graph.
   SceneTriangleNormalsCreationVisitor stncv;
   loadedModel->accept (stncv);
   sgRoot->addChild (stncv.normalsGeode.get());

   // Set the scene data
   viewer.setSceneData (sgRoot.get());

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
